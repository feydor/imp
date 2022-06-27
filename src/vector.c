#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "vector.h"

// on success returns 0
int UCharVec_init(UCharVec *vec) {
    assert(vec);
    vec->arr = calloc(INITIAL_CAPACITY, sizeof(uchar));
    if (!vec->arr) return -1;
    vec->cap = INITIAL_CAPACITY;
    vec->size = 0;
    return 0;
}

// crashes if realloc fails
void UCharVec_push(UCharVec *vec, uchar data) {
    assert(vec);
    if (vec->size == vec->cap) {
        vec->arr = realloc(vec->arr, SCALING_FACTOR * vec->cap * sizeof(uchar));
        if (!vec->arr) {
            fprintf(stderr, "UCharVec_push: realloc failed\n");
            exit(EXIT_FAILURE);
        }

        vec->cap *= SCALING_FACTOR;
    }

    vec->arr[vec->size++] = data;
}

uint UCharVec_size(UCharVec *vec) {
    assert(vec);
    return vec->size;
}

// crashes when index is out of bounds
uchar UCharVec_get(UCharVec *vec, uint i) {
    assert(vec);
    if (i > vec->size) {
        fprintf(stderr, "UCharVec_get: index out of bounds, i=%d\n", i);
        exit(EXIT_FAILURE);
    }

    return vec->arr[i];
}

// copy all the elements in vec into the array at dest
void UCharVec_copyto(UCharVec *vec, uchar* dest, uint dest_size) {
    assert(vec && dest);

    if (dest_size != vec->size) {
        fprintf(stderr, "UCharVec_copyto: size of destination does NOT equal vec, dest_size=%d, vec_size=%d\n", dest_size, vec->size);
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < dest_size; ++i) {
        dest[i] = vec->arr[i];
    }
}

void UCharVec_free(UCharVec *vec) {
    assert(vec);
    free(vec->arr);
    vec->arr = NULL;
}


int U32Vec_init(U32Vec *vec) {
    assert(vec);
    vec->arr = calloc(INITIAL_CAPACITY, sizeof(uint32_t));
    if (!vec->arr) return -1;
    vec->cap = INITIAL_CAPACITY;
    vec->size = 0;
    return 0;
}

uint U32Vec_size(U32Vec *vec) {
    assert(vec);
    return vec->size;
}

void U32Vec_push(U32Vec *vec, uint32_t data) {
    assert(vec);
    if (vec->size == vec->cap) {
        vec->arr = realloc(vec->arr, SCALING_FACTOR * vec->cap * sizeof(uint32_t));
        if (!vec->arr) {
            fprintf(stderr, "U32Vec_push: realloc failed\n");
            exit(EXIT_FAILURE);
        }

        vec->cap *= SCALING_FACTOR;
    }

    vec->arr[vec->size++] = data;
    
}

uint32_t U32Vec_get(U32Vec *vec, uint i) {
    assert(vec);
    if (i > vec->size) {
        fprintf(stderr, "U32Vec_get: index out of bounds, i=%d\n", i);
        exit(EXIT_FAILURE);
    }

    return vec->arr[i];
}

void U32Vec_from(U32Vec *vec, uint32_t *src, uint nmemb) {
    assert(vec && src);
    if (vec->cap < nmemb) {
        vec->arr = realloc(vec->arr, nmemb * sizeof(uint32_t));
        if (!vec->arr) {
            fprintf(stderr, "U32Vec_from: reallov failed");
            exit(EXIT_FAILURE);
        }
        vec->cap = nmemb;
    }

    memcpy(vec->arr, src, nmemb * sizeof(uint32_t));
    vec->size = nmemb;
}

void U32Vec_free(U32Vec *vec) {
    assert(vec);
    free(vec->arr);
    vec->arr = NULL;
}


// example usage
int example(void) {
    UCharVec vec;

    if (UCharVec_init(&vec) != 0) {
        perror("failed at init");
        exit(EXIT_FAILURE);
    }

    UCharVec_push(&vec, 0xFF);
    UCharVec_push(&vec, 0x69);
    UCharVec_push(&vec, 0x00);
    UCharVec_push(&vec, 0x01);
    UCharVec_push(&vec, 0x05);

    for (uint i = 0; i < UCharVec_size(&vec); ++i) {
        uchar data = UCharVec_get(&vec, i);
        printf("%d: %X\n", i, data);
    }

    // stress test, add 10000 elements
    for (int i = 0; i < 10000; ++i)
        UCharVec_push(&vec, 0xFF);
    printf("%X\n", UCharVec_get(&vec, 10000));

    UCharVec_free(&vec);
    return 0;
}