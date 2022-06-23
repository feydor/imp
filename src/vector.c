#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "vector.h"

// on success returns 0
int UCharVec_init(UCharVec *vec) {
    vec->arr = malloc(sizeof(uchar) * INITIAL_CAPACITY);
    if (!vec->arr) return -1;
    vec->cap = INITIAL_CAPACITY;
    vec->size = 0;
    return 0;
}

// crashes if realloc fails
void UCharVec_push(UCharVec *vec, uchar data) {
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
    return vec->size;
}

// crashes when index is out of bounds
uchar UCharVec_get(UCharVec *vec, int i) {
    if (i > vec->size) {
        fprintf(stderr, "UCharVec_get: index out of bounds %d\n", i);
        exit(EXIT_FAILURE);
    }

    return vec->arr[i];
}

void UCharVec_free(UCharVec *vec) {
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

    for (int i = 0; i < UCharVec_size(&vec); ++i) {
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