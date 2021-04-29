/* sad.h */
#ifndef SAD_H
#define SAD_H

#include <limits.h>

typedef struct {
    void **arr;
    unsigned int width;
    unsigned int height;
} Buffer2D;

int c_sad(unsigned char *, int , int ,
         unsigned char *, int , int );
int do_sad_calculation(unsigned char *, int , int , int,
                        unsigned char *, int , int );
int min(int *arr, int len);
int min_vla(int height, int width, int arr[][width]);
int sum(int height, int width, int arr[][width]);
void print_arr(int *arr, int len);
void print_arr_vla(int height, int width, int arr[][width]);

#endif
