#include <omp.h>
#include "lsh.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <time.h>
#include <stdbool.h>
#include <limits.h>

#define SIG_NTMS 72
#define SIG_NTRD HASHCOUNT

#define MIN(a, b) (a < b ? a : b)
#define MAX(a, b) (a > b ? a : b)

// const char *pathdata = "../data/doc_shingle_matrix.txt";
const char *pathdata = "../data/doc_shingle_real_data.txt";
// const char *pathdata = "../data/test_matrix.txt";
// const char *pathdata = "../data/doc_shingle_matrix_2400_10232.txt";

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
        sig_hash_a[i] = rand();
        sig_hash_b[i] = rand();
    }
}

// ==================== Minhash Function to Generate Signature Matrix====================
void compute_sig()
{
    // clock_t start_time = clock();
    double start_time = omp_get_wtime();
    printf("Compute Sig ...\n");

    memset(sig, 0xFFFF, sizeof(sig));

    // for (int i = 0; i < DOCCOUNT; i++)
    //     for (int j = 0; j < HASHCOUNT; j++)
    //         sig[i][j] = INT_MAX;
#pragma omp target teams map(DOCCOUNT, HASHCOUNT, SHINGLECOUNT, shingle, sig_hash_a, sig_hash_b, sig) num_teams(SIG_NTMS)
    {

        // for (int i = 0; i < DOCCOUNT; i++) // for every document
        // {
#pragma omp distribute
        for (int t = 0; t < SIG_NTMS; t++)
        {
            int ibgn = DOCCOUNT / SIG_NTMS * t;
            int iend = DOCCOUNT / SIG_NTMS * (t + 1);
            if (t == SIG_NTMS - 1)
                iend = DOCCOUNT;
            for (int i = ibgn; i < iend; i++)
            {
#pragma omp parallel for num_threads(SIG_NTRD)
                for (int k = 0; k < HASHCOUNT; k++) // every hash function
                {
                    for (int j = 0; j < SHINGLECOUNT; j++) // loop through all shingle indexes to find min index with value 1
                    {
                        if (shingle[i][j] == 1)
                        {
                            unsigned int res = (((long long)sig_hash_a[k] * j + sig_hash_b[k]) % 233333333333) % SHINGLECOUNT;
                            // printf("%d ", res);
                            sig[i][k] = MIN(sig[i][k], res);
                        }
                    }
                    // printf("\n");
                }
            }
        }
    }

    // clock_t end_time = clock();
    double end_time = omp_get_wtime();

    // Calculate the elapsed time in seconds
    // double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("Time for computing signature matrix: %f seconds\n", end_time - start_time);
}

// ==================== Banded LSH Function to Generate Candidate Pairs====================
struct Tuple
{
    uint16_t first;
    uint16_t second;
};

struct Set
{
    struct Tuple *elements;
    size_t size;
    size_t capacity;
};

// struct Set candidatePairSet;
struct Set validPairSet;

// initialize a set
void initializeSet(struct Set *set, size_t initialCapacity)
{
    set->elements = (struct Tuple *)malloc(initialCapacity * sizeof(struct Tuple));
    if (set->elements == NULL)
    {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    set->size = 0;
    set->capacity = initialCapacity;
}

// Function to check if a tuple is in the set
bool isInSet(const struct Set *set, const struct Tuple *element)
{
    for (size_t i = 0; i < set->size; i++)
    {
        if (set->elements[i].first == element->first && set->elements[i].second == element->second)
        {
            return true; // Tuple found in the set
        }
    }
    return false; // Tuple not found in the set
}

// Function to resize the set
void resizeSet(struct Set *set, size_t newCapacity)
{
    set->elements = (struct Tuple *)realloc(set->elements, newCapacity * sizeof(struct Tuple));
    if (set->elements == NULL)
    {
        perror("Memory reallocation failed");
        exit(EXIT_FAILURE);
    }
    set->capacity = newCapacity;
}

// Function to add a tuple to the set
void addToSet(struct Set *set, const struct Tuple *element)
{
    // Check if the tuple is already in the set
    // if (!isInSet(set, element)) {
    // Add the tuple to the set
    if (set->size == set->capacity)
    {
        // Double the capacity (you can choose a different resizing strategy)
        resizeSet(set, set->capacity * 2);
    }
    // Add the tuple to the set
    set->elements[set->size++] = *element;
    // }
}

// Function to print the elements of the set
void printSet(const struct Set *set)
{
    printf("{ ");
    for (size_t i = 0; i < set->size; i++)
    {
        printf("(%d, %d) ", set->elements[i].first, set->elements[i].second);
    }
    printf("}\n");
}

// Function to free the memory allocated for the set
void freeSet(struct Set *set)
{
    free(set->elements);
    set->elements = NULL;
    set->size = 0;
    set->capacity = 0;
}

void compute_LSH()
{
    clock_t start_time = clock();

    int flag = 0;
    size_t initialCapacity = DOCCOUNT;
    // initializeSet(&candidatePairSet, initialCapacity);
    initializeSet(&validPairSet, initialCapacity);
    for (int i = 0; i < BANDCOUNT; i++) // iterate through every band  for (int i = 0; i < BANDCOUNT; i++)
    {
        for (int j = 0; j < DOCCOUNT - 1; j++) // hash every sig. piece in the band to the bucket
        {
            for (int k = j + 1; k < DOCCOUNT; k++)
            {
                flag = 0;
                for (int r = 0; r < LINEOFROWS; r++)
                {
                    if (sig[j][i * LINEOFROWS + r] != sig[k][i * LINEOFROWS + r])
                    {
                        flag = 1;
                        break;
                    }
                }
                if (flag == 0)
                {
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

void check_valid_pairs(const struct Set *set, int j, int k)
{
    double intersection_num = 0.0, union_num = 0.0;
    double similarity = 0.0;
    // for (size_t i = 0; i < set->size; i++) {
    // intersection_num=0;
    // union_num=0;
    // printf("(%d, %d) ", set->elements[i].first, set->elements[i].second);
    struct Tuple candidate = {(uint16_t)j, (uint16_t)k};
    if (!isInSet(&validPairSet, &candidate))
    {
        for (int s = 0; s < SHINGLECOUNT; s++)
        {
            if (shingle[j][s] == 1 && shingle[k][s] == 1)
            {
                intersection_num++;
                union_num++;
            }
            else if ((shingle[j][s] == 1 && shingle[k][s] == 0) ||
                     (shingle[j][s] == 0 && shingle[k][s] == 1))
            {
                union_num++;
            }
        }
        similarity = intersection_num / union_num;
        // printf("\n%f", similarity);
        if (similarity >= THRESHOLD)
        {
            // struct Tuple candidate = {(uint16_t)j, (uint16_t)k};
            // if (!isInSet(&validPairSet, &candidate)) {
            lshValidPairs++;
            addToSet(&validPairSet, &candidate);
            // printf("(%d, %d) ", set->elements[i].first, set->elements[i].second);

            // }
            // printf("(%d, %d) ", set->elements[i].first, set->elements[i].second);
        }
    }
    // printf("\nValid Pairs In Total: %d", lshValidPairs);
}

int main()
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
    compute_sig();

    // printf("\nSig Matrix\n");
    // for (int i = 0; i < DOCCOUNT; i++)
    // {
    //     for (int j = 0; j < HASHCOUNT; j++)
    //     {
    //         printf("%d ", sig[i][j]);
    //     }
    //     printf("\n");
    // }

    // compute_LSH();
    // // printSet(&candidatePairSet);
    // // printf("Valid Pair Results: \n");
    // printf("Valid Pairs In Total: %d\n", lshValidPairs);
    // printSet(&validPairSet);
    // // check_valid_pairs(&candidatePairSet);
    // // freeSet(&candidatePairSet);
    // freeSet(&validPairSet);
    return 0;
}