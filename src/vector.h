#ifndef VECTOR_H
#define VECTOR_H

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
uchar UCharVec_get(UCharVec *vec, int i);
void UCharVec_free(UCharVec *vec);

#endif