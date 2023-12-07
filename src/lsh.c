#include "lsh.h"
// #include "hashfactory.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#define MIN(a, b) (a < b ? a : b)
#define MAX(a, b) (a > b ? a : b)

const char *pathdata = "../data/doc_shingle_matrix.txt";
// const char *pathdata = "../data/test_matrix.txt";

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

void genCandbySig() {
	printf("Generating Candidate by Sig ...\n");
	int validpair = 0 ;

	FILE * fout = fopen("./data/sig.res", "w");
	for(int a=0 ; a<DOCCOUNT ; a++)
		for(int b=a+1 ; b<DOCCOUNT ; b++) {
			if(isValidPairs(a, b)) {
				fprintf(fout, "%d\t%d\n", a, b);
				validpair++;
			}
		}
	fclose(fout);
	printf("Complete. Valid pair by Sig:\t%d\n", validpair);
}

int main()
{
    read_shingle_matrix();
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
            printf("%u", shingle[i][j]);
        printf("\n");
    }
}