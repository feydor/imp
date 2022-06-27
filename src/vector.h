#ifndef VECTOR_H
#define VECTOR_H
#include <stdint.h>

typedef unsigned char uchar;
typedef unsigned int uint;

#define INITIAL_CAPACITY 10
#define SCALING_FACTOR 2

typedef struct {
    uchar *arr;
    uint size;
    uint cap;
} UCharVec;

int UCharVec_init(UCharVec *vec);
uint UCharVec_size(UCharVec *vec);
void UCharVec_push(UCharVec *vec, uchar data);
uchar UCharVec_get(UCharVec *vec, uint i);
void UCharVec_copyto(UCharVec *vec, uchar* dest, uint dest_size);
void UCharVec_free(UCharVec *vec);

typedef struct {
    uint32_t *arr;
    uint size;
    uint cap;
} U32Vec;

int U32Vec_init(U32Vec *vec);
uint U32Vec_size(U32Vec *vec);
void U32Vec_push(U32Vec *vec, uint32_t data);
uint32_t U32Vec_get(U32Vec *vec, uint i);

/** replaces contents of vec with src */
void U32Vec_from(U32Vec *vec, uint32_t *src, uint nmemb);
void U32Vec_free(U32Vec *vec);

#endif