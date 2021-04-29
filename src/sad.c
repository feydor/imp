/* sad.c */
#include "../include/common.h"
#include "../include/sad.h"

// begins on upper right corner of frame
int c_sad(unsigned char *template, int template_w, int template_h,
        unsigned char *frame, int frame_w, int frame_h) {
    
    int results[frame_h * frame_w]; // must be less than this
    memset(results, 0, frame_h * frame_w * sizeof(int));
    
    int itr_count = 0;
    
    for (int row = 0; row < frame_h; row++) {
        for (int col = 0; col < frame_w; col++) {
            if (col + template_w <= frame_w && row + template_h <= frame_h) {
                itr_count++;
                results[itr_count - 1] = do_sad_calculation(frame, col, row,
                                                       frame_w, template, template_w, template_h);
            }
        }
    }
    // print_arr(results, itr_count);

    return min(results, itr_count);
}

int do_sad_calculation(unsigned char *frame, int curr_frame_col, int curr_frame_row,
                       int frame_w, unsigned char *template, int template_w, int template_h) {
    
    // iterate template && frame using the current frame row and col as the limit
    // for frow and fcol
    int temp[template_h][template_h];
    for (int row = 0, frow = curr_frame_row; row < template_h; row++, frow++) {
        for (int col = 0, fcol = curr_frame_col; col < template_w; col++, fcol++) {
            temp[row][col] = abs(frame[frow * frame_w + fcol] - template[row * template_w + col]);
        }
    }
    return sum(template_h, template_w, temp);
}

int sum(int height, int width, int arr[][width]) {
    int sum = 0;
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            sum += arr[row][col];
        }
    }
    return sum;
}

int min(int *arr, int len) {
    int min = INT_MAX;
    for (int i = 0; i < len; i++) {
        if (min > arr[i])
            min = arr[i];
    }
    return min;
}

int min_vla(int height, int width, int arr[][width]) {
    int min = INT_MAX;
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            if (min > arr[row][col])
                min = arr[row][col];
        }
    }
    return min;
}

void print_arr(int *arr, int len) {
    printf("Printing array...\n");
    for (int i = 0; i < len; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

void print_arr_vla(int height, int width, int arr[][width]) {
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            printf("%d ", arr[row][col]);
            if (col == width - 1) printf("\n");
        }
    }
}

/*
int sad_itr(unsigned int fx, unsigned int fy, int fw, int fh) {
    return fx > fh ?
}
*/
