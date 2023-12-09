#include "lsh.h"
// #include "hashfactory.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <time.h>

#include "mpi.h"

#define MIN(a, b) (a < b ? a : b)
#define MAX(a, b) (a > b ? a : b)

// const char *pathdata = "../data/doc_shingle_matrix.txt";
const char *pathdata = "../data/test_matrix.txt";

uint32_t sig_hash_a[HASHCOUNT]; // the slope a of the signature hash functions
uint32_t sig_hash_b[HASHCOUNT]; // the interception b of the signature hash functions

uint8_t shingle[DOCCOUNT][SHINGLECOUNT]; // read the doc-shingle 0-1 matrix to this matrix
uint16_t sig[DOCCOUNT][HASHCOUNT];       // the signature matrix

// struct HashFamily hashfamily;

// struct HashTable checkedpairs;

// int lshValidPairs = 0;

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

// ====================Generate Hash Function Coefficients====================
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
    printf("Compute Sig ...\n");

    memset(sig, 0xFFFF, sizeof(sig));
    for (int i = 0; i < DOCCOUNT; i++)
        for (int j = 0; j < HASHCOUNT; j++)
            sig[i][j] = 0xFFFF;

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
}

void compute_sig_partial(int start_doc, int end_doc) {
    printf("Compute Sig Partial ..., writing to rows: [%d, %d)\n", start_doc, end_doc);

    for (int i = start_doc; i < end_doc; i++) {
        for (int k = 0; k < HASHCOUNT; k++) {
            for (int j = 0; j < SHINGLECOUNT; j++) {
                if (shingle[i][j] == 1) {
                    unsigned int res = (((uint64_t)(sig_hash_a[k] * j + sig_hash_b[k])) % 233333333333ULL) % SHINGLECOUNT;
                    sig[i][k] = MIN(sig[i][k], res);
                }
            }
        }
    }
}



// ==================== Banded LSH Function to Generate Candidate Pairs====================
void compute_LSH()
{
    for (int i = 0; i < BANDCOUNT; i++)
    {
        for (int j = 0; j < DOCCOUNT; j++)
        {

            char str[LINEOFROWS * 4 + 1];
            str[LINEOFROWS * 4] = 'k';
            for (int k = 0; k < LINEOFROWS; k++)
                memcpy(str + k * 4, &sig[j][i * LINEOFROWS + k], 4);
            assert(str[LINEOFROWS * 4] == 'k');

            // uint32_t hashval = HashFamily_hash(&hashfamily, i, str, LINEOFROWS * 4);
            // Bucket_insert(hashval, j);
        }

        // Bucket_check(fout);
        // Bucket_clear();
    }
}


// void computeLSH() {
// 	FILE * fout = fopen("./data/lsh.res", "w");
// 	HashFamily_init(&hashfamily, BANDCOUNT);
// 	HashTable_init(&checkedpairs, 500*USERCOUNT, 8);

// 	for(int i=0 ; i<BANDCOUNT ; i++) {
// 		for(int j=0 ; j<USERCOUNT ; j++) {

// 			char str[LINEOFROWS*4 + 1];
// 			str[LINEOFROWS*4] = 'k';
// 			for(int k=0 ; k<LINEOFROWS ; k++)
// 				memcpy(str+k*4, &sig[j][i*LINEOFROWS+k], 4);
// 			assert(str[LINEOFROWS*4] == 'k');

// 			uint32_t hashval = HashFamily_hash(&hashfamily, i, str, LINEOFROWS*4);
// 			Bucket_insert(hashval, j);
// 		}

// 		Bucket_check(fout);
// 		Bucket_clear();
// 	}
// 	HashFamily_destroy(&hashfamily);
// 	HashTable_destroy(&checkedpairs);
// 	fclose(fout);

// 	printf("LSH validPairs:\t%d\n", lshValidPairs);
// 	printf("%d %d\n", ccnt, cccnt);
// }


// ==================== Output Function ====================

// void genCandbySig() {
// 	printf("Generating Candidate by Sig ...\n");
// 	int validpair = 0 ;

// 	FILE * fout = fopen("./data/sig.res", "w");
// 	for(int a=0 ; a<DOCCOUNT ; a++)
// 		for(int b=a+1 ; b<DOCCOUNT ; b++) {
// 			if(isValidPairs(a, b)) {
// 				fprintf(fout, "%d\t%d\n", a, b);
// 				validpair++;
// 			}
// 		}
// 	fclose(fout);
// 	printf("Complete. Valid pair by Sig:\t%d\n", validpair);
// }

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


    // ==============mpi signature computing===============
    MPI_Init(&argc,&argv);

    int world_size, world_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    if (world_rank == 0) {
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
                printf("%u", shingle[i][j]);
            printf("\n");
        }
    }

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

    MPI_Finalize();

    if (world_rank == 0) {
        printf("\nSig Matrix\n");
        for (int i = 0; i < DOCCOUNT; i++)
        {
            for (int j = 0; j < HASHCOUNT; j++)
            {
                printf("%d ", sig[i][j]);
            }
            printf("\n");
        }
    }
    return 0;
}