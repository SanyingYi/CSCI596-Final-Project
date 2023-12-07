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
uint8_t sig[DOCCOUNT][HASHCOUNT];        // the signature matrix

struct HashFamily hashfamily;

struct HashTable checkedpairs;

// int lshValidPairs = 0;

// ==================== Core Minhash Function ====================
void load_computeSig()
{
    printf("Load and Compute Sig ...\n");

    memset(sig, 0xff, sizeof(sig));
    for (int i = 0; i < DOCCOUNT; i++)
        for (int j = HASHCOUNT >> 1; j < HASHCOUNT; j++)
            sig[i][j] = 0;

    HashFamily_init(&hashfamily, HASHCOUNT >> 1);

    FILE *fin = fopen(pathdata, "r");
    for (int i = 0; i < DOCCOUNT; i++)
    {
        int num;
        char feature[16];
        if (fscanf(fin, "%d%d", &i, &num) != 2)
        {
            fprintf(stderr, "read format error!\n");
            exit(1);
        }

        for (int j = 0; j < num; j++)
        {
            if (fscanf(fin, "%s", feature) != 1 || strlen(feature) != 11)
            {
                fprintf(stderr, "read format error!\n");
                exit(1);
            }

            int half = HASHCOUNT >> 1;
            for (int k = 0; k < half; k++)
            {
                unsigned int res = HashFamily_hash(&hashfamily, k, feature, 11);
                sig[i][k] = MIN(sig[i][k], res);
                //				if(sig[i][k] > res) {
                //					sig[i][k+half] = sig[i][k];
                //					sig[i][k] = res;
                //				}
                //				else if(sig[i][k+half] > res)
                //					sig[i][k+half] = res;
                sig[i][k + half] = MAX(sig[i][k + half], res);
            }
        }

        if (i && i % (DOCCOUNT / 10) == 0)
            printf("Sig Complete:\t%.2lf%%\n", ((double)i) / DOCCOUNT);
    }

    fclose(fin);
    HashFamily_destroy(&hashfamily);

    //	genCandbySig();
}
