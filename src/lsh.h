#ifndef LSH_H
#define LSH_H

#include <stdint.h>

#define DOCCOUNT 24000
// #define DOCCOUNT 4
#define SHINGLECOUNT 10232

#define BANDCOUNT 50
// #define BANDCOUNT 2
#define LINEOFROWS 2
#define HASHCOUNT (BANDCOUNT * LINEOFROWS)

// #define THRESHOLD 0.83

void load_computeSig();

void genCandbySig();

void computeLSH();

#endif
