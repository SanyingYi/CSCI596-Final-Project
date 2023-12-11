#ifndef LSH_H
#define LSH_H

#include <stdint.h>

// #define DOCCOUNT 24000
#define DOCCOUNT 2400
#define SHINGLECOUNT 10232
// #define DOCCOUNT 4
// #define SHINGLECOUNT 4

#define BANDCOUNT 50
// #define BANDCOUNT 2
#define LINEOFROWS 2
#define HASHCOUNT (BANDCOUNT * LINEOFROWS)

#define THRESHOLD 0.4

void read_shingle_matrix();

void compute_sig();

void compute_LSH();

void check_valid_pairs();

#endif
