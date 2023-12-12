#include <stdint.h>

#define DOCCOUNT 24732
#define SHINGLECOUNT 11270
// #define DOCCOUNT 4
// #define SHINGLECOUNT 4

#define BANDCOUNT 50
// #define BANDCOUNT 2
#define LINEOFROWS 2
#define HASHCOUNT (BANDCOUNT * LINEOFROWS)

#define THRESHOLD 0.5

void read_shingle_matrix();

void compute_sig();

void compute_LSH();

void check_valid_pairs();

