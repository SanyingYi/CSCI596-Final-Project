#include "lsh.h"
// #include "hashfactory.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#define MIN(a, b) (a < b ? a : b)
#define MAX(a, b) (a > b ? a : b)

// const char *pathdata = "../data/doc_shingle_matrix.txt";
const char *pathdata = "../data/test_matrix.txt";

uint8_t shingle[DOCCOUNT][SHINGLECOUNT]; // read the doc-shingle 0-1 matrix to this matrix
uint16_t sig[DOCCOUNT][HASHCOUNT];       // the signature matrix

// struct HashFamily hashfamily;

// struct HashTable checkedpairs;

// int lshValidPairs = 0;

// read the doc-shingle 0-1 matrix from the text file to shingle matrix
void read_shingle_matrix()
{
    FILE *file = fopen(pathdata, "r");
    if (file == NULL)
    {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    char line[DOCCOUNT * SHINGLECOUNT]; // each line has SHINGLECOUNT * 2 characters because of spaces
    int row = 0, col = 0;               // index
    // Read file line by line
    while (fgets(line, sizeof(line), file) != NULL)
    {
        col = 0;
        char *token = strtok(line, " \n");
        while (token != NULL)
        {
            // printf("%d", atoi(token));
            shingle[row][col] = (uint8_t)atoi(token);
            token = strtok(NULL, " \n");
            col++;
        }
        row++;

        // Check if the matrix is fully populated
        if (row >= DOCCOUNT)
        {
            break;
        }
    }

    fclose(file);
}

// ==================== Core Minhash Function ====================
// void load_computeSig()
// {
//     printf("Load and Compute Sig ...\n");

//     memset(sig, 0xff, sizeof(sig));
//     for (int i = 0; i < DOCCOUNT; i++)
//         for (int j = HASHCOUNT >> 1; j < HASHCOUNT; j++)
//             sig[i][j] = 0;

//     HashFamily_init(&hashfamily, HASHCOUNT >> 1);

//     FILE *fin = fopen(pathdata, "r");
//     for (int i = 0; i < DOCCOUNT; i++)
//     {
//         int num;
//         char feature[16];
//         if (fscanf(fin, "%d%d", &i, &num) != 2)
//         {
//             fprintf(stderr, "read format error!\n");
//             exit(1);
//         }

//         for (int j = 0; j < num; j++)
//         {
//             if (fscanf(fin, "%s", feature) != 1 || strlen(feature) != 11)
//             {
//                 fprintf(stderr, "read format error!\n");
//                 exit(1);
//             }

//             int half = HASHCOUNT >> 1;
//             for (int k = 0; k < half; k++)
//             {
//                 unsigned int res = HashFamily_hash(&hashfamily, k, feature, 11);
//                 sig[i][k] = MIN(sig[i][k], res);
//                 //				if(sig[i][k] > res) {
//                 //					sig[i][k+half] = sig[i][k];
//                 //					sig[i][k] = res;
//                 //				}
//                 //				else if(sig[i][k+half] > res)
//                 //					sig[i][k+half] = res;
//                 sig[i][k + half] = MAX(sig[i][k + half], res);
//             }
//         }

//         if (i && i % (DOCCOUNT / 10) == 0)
//             printf("Sig Complete:\t%.2lf%%\n", ((double)i) / DOCCOUNT);
//     }

//     fclose(fin);
//     HashFamily_destroy(&hashfamily);

//     //	genCandbySig();
// }

int main()
{
    read_shingle_matrix();
    for (int i = 0; i < DOCCOUNT; i++)
    {
        for (int j = 0; j < SHINGLECOUNT; j++)
            printf("%u", shingle[i][j]);
        printf("\n");
    }
}