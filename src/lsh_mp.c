#include "lsh.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <time.h>
#include <stdbool.h>

#include "mpi.h"

#define MIN(a, b) (a < b ? a : b)
#define MAX(a, b) (a > b ? a : b)

// const char *pathdata = "../data/doc_shingle_matrix.txt";
const char *pathdata = "../data/doc_shingle_real_data.txt";
// const char *pathdata = "../data/test_matrix.txt";
// const char *pathdata = "E:/553 data mining homework/HW3/data/doc_shingle_matrix.txt";

uint32_t sig_hash_a[HASHCOUNT]; // the slope a of the signature hash functions
uint32_t sig_hash_b[HASHCOUNT]; // the interception b of the signature hash functions

uint8_t shingle[DOCCOUNT][SHINGLECOUNT]; // read the doc-shingle 0-1 matrix to this matrix
uint16_t sig[DOCCOUNT][HASHCOUNT];       // the signature matrix

int lshValidPairs = 0;

// ====================Read the doc-shingle 0-1 matrix from the text file to variable matrix====================
void read_shingle_matrix()
{
    FILE *file = fopen(pathdata, "r");
    if (file == NULL)
    {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    int row = 0, col = 0; // index
    char ch;
    while ((ch = fgetc(file)) != EOF) // read character by character to avoid memory overflow
    {
        if (ch != '\n')
        {
            if (ch != ' ')
            {
                shingle[row][col] = (uint8_t)(ch - '0');
                col++;
            }
        }
        else
        {
            col = 0;
            row++;
        }

        // Check if the matrix is fully populated
        if (row >= DOCCOUNT)
        {
            break;
        }
    }

    fclose(file);
}

// ====================Generate Hash Function Coefficients for Minhash====================
void generate_hash_function()
{
    srand(time(NULL));
    for (int i = 0; i < HASHCOUNT; i++)
    {
        sig_hash_a[i] = (uint32_t)rand();
        sig_hash_b[i] = (uint32_t)rand();
    }
}

// ==================== Minhash Function to Generate Signature Matrix====================
void compute_sig()
{
    clock_t start_time = clock();
    printf("Compute Sig ...\n");

    memset(sig, 0xFFFF, sizeof(sig));
    // for (int i = 0; i < DOCCOUNT; i++)
    //     for (int j = 0; j < HASHCOUNT; j++)
    //         sig[i][j] = 0xFFFF;

    for (int i = 0; i < DOCCOUNT; i++) // for every document
    {
        for (int k = 0; k < HASHCOUNT; k++) // every hash function
        {
            for (int j = 0; j < SHINGLECOUNT; j++) // loop through all shingle indexes to find min index with value 1
            {
                if (shingle[i][j] == 1)
                {
                    unsigned int res = (((uint64_t)(sig_hash_a[k] * j + sig_hash_b[k])) % 233333333333ULL) % SHINGLECOUNT;
                    // printf("%d ", res);
                    sig[i][k] = MIN(sig[i][k], res);
                }
            }
            // printf("\n");
        }
    }
    clock_t end_time = clock();

    // Calculate the elapsed time in seconds
    double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("Time for computing signature matrix: %f seconds\n", elapsed_time);
}

void compute_sig_partial(int start_doc, int end_doc) {
    printf("Compute Sig Partial ..., writing to rows: [%d, %d)\n", start_doc, end_doc);

    for (int i = start_doc; i < end_doc; i++) {
        for (int k = 0; k < HASHCOUNT; k++) {
            for (int j = 0; j < SHINGLECOUNT; j++) {
                if (shingle[i][j] == 1) {
                    unsigned int res = ((((long long) sig_hash_a[k] * j + sig_hash_b[k])) % 233333333333ULL) % SHINGLECOUNT;
                    // printf("%d ", res);
                    // if (res == 0) {
                    //     printf("a:%d, b:%d, j:%d \n", sig_hash_a[k], sig_hash_b[k], j);
                    // }
                    sig[i][k] = MIN(sig[i][k], res);
                }
            }
            // break;
            // printf("\n");
        }
        // break;
    }
}

// ==================== Banded LSH Function to Generate Candidate Pairs====================
struct Tuple {
    uint16_t first;
    uint16_t second;
};

struct Set {
    struct Tuple *elements;
    size_t size;
    size_t capacity;
};

// struct Set candidatePairSet;
struct Set validPairSet;

// initialize a set
void initializeSet(struct Set *set, size_t initialCapacity) {
    set->elements = (struct Tuple *)malloc(initialCapacity * sizeof(struct Tuple));
    if (set->elements == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    set->size = 0;
    set->capacity = initialCapacity;
}

// Function to check if a tuple is in the set
bool isInSet(const struct Set *set, const struct Tuple *element) {
    for (size_t i = 0; i < set->size; i++) {
        if (set->elements[i].first == element->first && set->elements[i].second == element->second) {
            return true;  // Tuple found in the set
        }
    }
    return false;  // Tuple not found in the set
}

// Function to resize the set
void resizeSet(struct Set *set, size_t newCapacity) {
    set->elements = (struct Tuple *)realloc(set->elements, newCapacity * sizeof(struct Tuple));
    if (set->elements == NULL) {
        perror("Memory reallocation failed");
        exit(EXIT_FAILURE);
    }
    set->capacity = newCapacity;
}

// Function to add a tuple to the set
void addToSet(struct Set *set, const struct Tuple *element) {
    // Check if the tuple is already in the set
    // if (!isInSet(set, element)) {
        // Add the tuple to the set
    if (set->size == set->capacity) {
        // Double the capacity (you can choose a different resizing strategy)
        resizeSet(set, set->capacity * 2);
    }
    // Add the tuple to the set
    set->elements[set->size++] = *element;
    // }
}

// Function to print the elements of the set
void printSet(const struct Set *set) {
    printf("{ ");
    for (size_t i = 0; i < set->size; i++) {
        printf("(%d, %d) ", set->elements[i].first, set->elements[i].second);
    }
    printf("}\n");
}

// Function to free the memory allocated for the set
void freeSet(struct Set *set) {
    free(set->elements);
    set->elements = NULL;
    set->size = 0;
    set->capacity = 0;
}

void compute_LSH()
{
    clock_t start_time = clock();

    int flag=0;
    size_t initialCapacity = DOCCOUNT;
    // initializeSet(&candidatePairSet, initialCapacity);
    initializeSet(&validPairSet, initialCapacity);
    for (int i = 0; i < BANDCOUNT; i++) // iterate through every band
    {
        for (int j = 0; j < DOCCOUNT-1; j++) // hash every sig. piece in the band to the bucket
        {
            for (int k = j+1; k<DOCCOUNT; k++){
                flag = 0;
                for (int r = 0; r < LINEOFROWS; r++){
                    if (sig[j][i * LINEOFROWS + r] != sig[k][i * LINEOFROWS + r]){
                        flag=1;
                        break;
                    }
                }
                if(flag==0){
                    // printf("%d, %d\n", j, k);
                    // struct Tuple candidate = {(uint16_t)j, (uint16_t)k};
                    // addToSet(&candidatePairSet, &candidate);
                    check_valid_pairs(&validPairSet, j, k);
                }
            }
        }
    }
    // Record the end time
    clock_t end_time = clock();

    // Calculate the elapsed time in seconds
    double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("Time for generating valid pairs: %f seconds\n", elapsed_time);
}


//====================Check Candidate Pairs to Filter Out Valid Pairs====================

void check_valid_pairs(struct Set *set, int j, int k){
    double intersection_num=0.0, union_num=0.0;
    double similarity = 0.0;
    // for (size_t i = 0; i < set->size; i++) {
    // intersection_num=0;
    // union_num=0;
    // printf("(%d, %d) ", set->elements[i].first, set->elements[i].second);
    struct Tuple candidate = {(uint16_t)j, (uint16_t)k};
    if (!isInSet(set, &candidate)) {
        for (int s =0;s<SHINGLECOUNT; s++){
            if (shingle[j][s]==1 && shingle[k][s]==1){
                intersection_num++;
                union_num++;
            }
            else if((shingle[j][s]==1 && shingle[k][s]==0)||
            (shingle[j][s]==0 && shingle[k][s]==1)){
                union_num++;
            }
        }
        similarity = intersection_num/union_num;
        // printf("\n%f", similarity);
        if (similarity>=THRESHOLD){
            // struct Tuple candidate = {(uint16_t)j, (uint16_t)k};
            // if (!isInSet(&validPairSet, &candidate)) {
            lshValidPairs++;
            addToSet(set, &candidate);
                // printf("(%d, %d) ", set->elements[i].first, set->elements[i].second);
                            
            // }
        // printf("(%d, %d) ", set->elements[i].first, set->elements[i].second);
        }
    }
    // printf("\nValid Pairs In Total: %d", lshValidPairs);
}

void compute_LSH_MPI(struct Set *set, int start_band, int end_band, int rank) {
    int flag;
    // printf("ckpt6, %d \n", rank);
    for (int i = start_band; i < end_band; i++) {
        for (int j = 0; j < DOCCOUNT - 1; j++) {
            for (int k = j + 1; k < DOCCOUNT; k++) {
                flag = 0;
                for (int r = 0; r < LINEOFROWS; r++) {
                    if (sig[j][i * LINEOFROWS + r] != sig[k][i * LINEOFROWS + r]) {
                        flag = 1;
                        break;
                    }
                }
                if (flag == 0) {
                    // struct Tuple candidate = {(uint16_t)j, (uint16_t)k};
                    // addToSet(set, &candidate);
                    // printf("ckpt7, %d \n", rank);
                    check_valid_pairs(set, j, k);
                    // printf("ckpt8, %d \n", rank);
                }
            }
        }
    }
}

// int mergeSets(struct Set *destination, const struct Set *sources, int num_sets) {
//     int lshValidPairs = 0;
//     for (int i = 0; i < num_sets; i++) {
//         for (size_t j = 0; j < sources[i].size; j++) {
//             if (!isInSet(destination, &sources[i].elements[j])) {
//                 lshValidPairs++;
//                 addToSet(destination, &sources[i].elements[j]);
//             }
//         }
//     }
//     return lshValidPairs;
// }

int mergeSets(struct Set *destination, const struct Set *sources) {
    int lshValidPairs = 0;
    for (size_t j = 0; j < sources->size; j++) {
        if (!isInSet(destination, &sources->elements[j])) {
            lshValidPairs++;
            addToSet(destination, &sources->elements[j]);
        }
    }
    return lshValidPairs;
}

// char* serializeSet(const struct Set* set) {
//     // 计算序列化所需的总字节大小
//     size_t totalSize = sizeof(struct Tuple) * set->size;
//     char* buffer = malloc(totalSize);
//     if (buffer == NULL) {
//         return NULL;
//     }

//     // 将大小信息和元素复制到 buffer
//     memcpy(buffer, set->elements, sizeof(struct Tuple) * set->size);

//     return buffer;
// }

// void deserializeSet(struct Set* set, const char* buffer, size_t size) {
//     // 为元素分配空间并从 buffer 中复制
//     set->elements = malloc(size);
//     if (set->elements == NULL) {
//         set->size = 0;
//         set->capacity = 5;
//         return;
//     }
//     memcpy(set->elements, buffer, size);
// }

uint16_t* serializeSet(const struct Set* set) {
    uint16_t* buffer = malloc(set->size * 2 * sizeof(uint16_t));
    if (buffer == NULL) {
        return NULL;
    }

    for (size_t i = 0; i < set->size; i++) {
        buffer[2 * i] = set->elements[i].first;
        buffer[2 * i + 1] = set->elements[i].second;
    }

    return buffer;
}

void deserializeSet(struct Set* set, uint16_t* buffer, size_t size) {
    for (size_t i = 0; i < size; i++) {
        struct Tuple tuple = {buffer[2 * i], buffer[2 * i + 1]};
        addToSet(set, &tuple);
    }
}


size_t getSerializedSize(const struct Set* set) {
    // 计算序列化所需的总字节大小
    // 包括 size 和 capacity 字段以及 elements 数组中所有元素的总大小
    size_t totalSize = set->size * 2 * sizeof(uint16_t); // 对于 elements 数组
    return totalSize;
}


int main(int argc, char **argv)
{
    read_shingle_matrix();
    // for (int i = 0; i < 4; i++)
    // {
    //     for (int j = 0; j < 4; j++)
    //         printf("%u", shingle[i][j]);
    //     printf("\n");
    // }
    generate_hash_function();
    // for (int i = 0; i < HASHCOUNT; i++)
    // {
    //     printf("%d+%d\n", sig_hash_a[i], sig_hash_b[i]);
    // }

    // compute_sig();

    // printf("\nSig Matrix\n");
    // for (int i = 0; i < DOCCOUNT; i++)
    // {
    //     for (int j = 0; j < HASHCOUNT; j++)
    //     {
    //         printf("%d ", sig[i][j]);
    //     }
    //     printf("\n");
    // }

    // ==============mpi signature computing===============
    MPI_Init(&argc,&argv);

    int world_size, world_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    clock_t start_time;
    if (world_rank == 0) {
        start_time = clock();
    }
    // if (world_rank == 0) {
    //     for (int i = 0; i < 4; i++)
    //     {
    //         for (int j = 0; j < 4; j++)
    //             printf("%u", shingle[i][j]);
    //         printf("\n");
    //     }
    // }

    // initialize sig matrix
    memset(sig, 0xFFFF, sizeof(sig));
    MPI_Barrier(MPI_COMM_WORLD);

    // determine range according to pid
    int docs_per_proc = DOCCOUNT / world_size;
    int start_index = world_rank * docs_per_proc;
    int end_index = (world_rank + 1) * docs_per_proc;
    if (world_rank == world_size - 1) {
        end_index = DOCCOUNT;
    }

    // calculate part of the signature
    compute_sig_partial(start_index, end_index);

    // gather result to master process
    MPI_Gather(sig[start_index], docs_per_proc * HASHCOUNT, MPI_UINT16_T,
            sig, docs_per_proc * HASHCOUNT, MPI_UINT16_T,
            0, MPI_COMM_WORLD);




    if (world_rank == 0) {
        // 主进程发送 sig 数组到其他所有进程
        for (int i = 1; i < world_size; i++) {
            MPI_Send(sig, DOCCOUNT * HASHCOUNT, MPI_UINT16_T, i, 0, MPI_COMM_WORLD);
        }
    } else {
        // 其他进程接收 sig 数组
        MPI_Recv(sig, DOCCOUNT * HASHCOUNT, MPI_UINT16_T, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    MPI_Barrier(MPI_COMM_WORLD);


    // ====================== LSH MPI =========================


    // int nproc = atoi(argv[1]);
    size_t initialCapacity = DOCCOUNT;
    // struct Set procsSet[nproc];

    // for (int i = 0; i < nproc; i++) {
    //     initializeSet(&procsSet[i], initialCapacity);
    // }
    // initializeSet(&candidatePairSet, initialCapacity);
    initializeSet(&validPairSet, initialCapacity);

    int bands_per_proc = BANDCOUNT / world_size;
    int start_band = world_rank * bands_per_proc;
    int end_band = (world_rank == world_size - 1) ? BANDCOUNT : (world_rank + 1) * bands_per_proc;

    struct Set mySet;
    initializeSet(&mySet, DOCCOUNT);
    // printf("ckpt1\n");

    compute_LSH_MPI(&mySet, start_band, end_band, world_rank);
    // printf("ckpt2, %d \n", world_rank);

    MPI_Barrier(MPI_COMM_WORLD);

    if (world_rank == 0) {

        // 合并当前进程的结果
        mergeSets(&validPairSet, &mySet);

        // 接收其他进程的数据
        for (int i = 1; i < world_size; i++) {
            uint64_t buffer_size;
            // printf("ckpt3\n");
            MPI_Recv(&buffer_size, 1, MPI_UINT64_T, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            uint16_t* buffer = malloc(buffer_size);
            MPI_Recv(buffer, buffer_size, MPI_UINT16_T, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            printf("Recv from %d, length: %d \n", i, buffer_size / (2 * sizeof(uint16_t)));
            struct Set otherSet;
            initializeSet(&otherSet, buffer_size / (2 * sizeof(uint16_t)));
            deserializeSet(&otherSet, buffer, buffer_size);

            // printSet(&otherSet);
            // printf("\n\n\n\n\n\n\n\n");

            mergeSets(&validPairSet, &otherSet);

            freeSet(&otherSet);
            free(buffer);
        }

        printf("Final Set Size: %zu\n", validPairSet.size);
        printSet(&validPairSet);

        freeSet(&validPairSet);
    } else {
        // 序列化 mySet 并发送到主进程
        // printf("ckpt5, %d \n", world_rank);


        uint16_t* buffer = serializeSet(&mySet);
        uint64_t buffer_size = (uint64_t) getSerializedSize(&mySet); // 假设这个函数可以计算序列化数据的大小

        printf("send from %d, length: %d \n", world_rank, buffer_size / (2 * sizeof(uint16_t)));
        MPI_Send(&buffer_size, 1, MPI_UINT64_T, 0, 0, MPI_COMM_WORLD);
        MPI_Send(buffer, buffer_size, MPI_UINT16_T, 0, 0, MPI_COMM_WORLD);

        free(buffer);
    }










    MPI_Finalize();

    if (world_rank == 0) {
        // Record the end time
        clock_t end_time = clock();

        // Calculate the elapsed time in seconds
        double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
        printf("Time for whole process: %f seconds\n", elapsed_time);
    }

    // if (world_rank == 0) {
    //     printf("\nSig Matrix\n");
    //     for (int i = 0; i < DOCCOUNT; i++)
    //     {
    //         for (int j = 0; j < HASHCOUNT; j++)
    //         {
    //             printf("%d ", sig[i][j]);
    //         }
    //         printf("\n");
    //     }
    // }
    return 0;






    // compute_LSH();
    // // printSet(&candidatePairSet);
    // // printf("Valid Pair Results: \n");
    // printf("Valid Pairs In Total: %d\n", lshValidPairs);
    // printSet(&validPairSet);
    // // check_valid_pairs(&candidatePairSet);
    // // freeSet(&candidatePairSet);
    // freeSet(&validPairSet);
}