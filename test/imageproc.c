/* test/imageproc.c */
#include <unity.h>
#include <stdint.h> /* for int32_t */
#include <stdlib.h> /* for malloc */

#include "../include/imageproc.h"
#include "../include/imageio.h"

/* test prototypes */
void test_closestfrompal(void);
void test_pixelat(void);
void test_bayer_sqrmat(void);
void test_widen(void);
void test_narrow(void);

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_closestfrompal);
    RUN_TEST(test_pixelat);
    RUN_TEST(test_bayer_sqrmat);
    RUN_TEST(test_widen);
    RUN_TEST(test_narrow);
}

void setUp(void)
{
    // set stuff up here
}

void tearDown(void)
{
    // clean stuff up here
}

void test_closestfrompal(void)
{
    int32_t pal[] = {
        0x000000, 0x008000, 0x00FF00,
        0x0000FF, 0x0080FF, 0x00FFFF,
        0x800000, 0x808000, 0x80FF00,
        0x8000FF, 0x8080FF, 0x80FFFF,
        0xFF0000, 0xFF8000, 0xFFFF00,
        0xFF00FF, 0xFF80FF, 0xFFFFFF
    };
    int32_t closest = 0xFFFFFF;

    // all black returns all black
    closest = closestfrompal(0x000000, pal, sizeof(pal)/sizeof(pal[0]));
    TEST_ASSERT_EQUAL(0x000000, closest);
    
    // all white returns all white
    closest = closestfrompal(0xFFFFFF, pal, sizeof(pal)/sizeof(pal[0]));
    TEST_ASSERT_EQUAL(0xFFFFFF, closest);
    
    // black off by 1 returns black
    closest = closestfrompal(0x000001, pal, sizeof(pal)/sizeof(pal[0]));
    TEST_ASSERT_EQUAL(0x000000, closest);

    // inbetween two pallette colors returns the first one
    closest = closestfrompal(0x004000, pal, sizeof(pal)/sizeof(pal[0]));     
    TEST_ASSERT_EQUAL(0x000000, closest); 
    closest = closestfrompal(0x004F00, pal, sizeof(pal)/sizeof(pal[0]));     
    TEST_ASSERT_EQUAL(0x008000, closest); 
}

void test_pixelat(void)
{
    /**
     * generate a 4x4 byte image buffer (ie a 1x4 pixel buffer)
     */
    struct image32_t img = {0};
    img.w = img.h = 4; // width is guaranteed to be a multiple of 4
    img.buf = calloc(img.w * img.h, 1);
    
    /* normal iteration through every byte */
    TEST_ASSERT_EQUAL(pixel_at(&img, 0, 0), 0);
    TEST_ASSERT_EQUAL(pixel_at(&img, 1, 0), 0);
    TEST_ASSERT_EQUAL(pixel_at(&img, 2, 0), 0);
    TEST_ASSERT_EQUAL(pixel_at(&img, 3, 0), 0);

    TEST_ASSERT_EQUAL(pixel_at(&img, 0, 1), 1);
    TEST_ASSERT_EQUAL(pixel_at(&img, 1, 1), 1);
    TEST_ASSERT_EQUAL(pixel_at(&img, 2, 1), 1);
    TEST_ASSERT_EQUAL(pixel_at(&img, 3, 1), 1);
    
    TEST_ASSERT_EQUAL(pixel_at(&img, 0, 2), 2);
    TEST_ASSERT_EQUAL(pixel_at(&img, 1, 2), 2);
    TEST_ASSERT_EQUAL(pixel_at(&img, 2, 2), 2);
    TEST_ASSERT_EQUAL(pixel_at(&img, 3, 2), 2);
    
    TEST_ASSERT_EQUAL(pixel_at(&img, 0, 3), 3);
    TEST_ASSERT_EQUAL(pixel_at(&img, 1, 3), 3);
    TEST_ASSERT_EQUAL(pixel_at(&img, 2, 3), 3);
    TEST_ASSERT_EQUAL(pixel_at(&img, 3, 3), 3);

    /* error on bound overflow */
    TEST_ASSERT_EQUAL(pixel_at(&img, 4, 0), -1);
    TEST_ASSERT_EQUAL(pixel_at(&img, 0, 4), -1);
    TEST_ASSERT_EQUAL(pixel_at(&img, 4, 4), -1);
    
    /* foreach-type iteration through each pixel */
    int res[4] = {0};
    int count = 0;
    for (size_t y = 0; y < img.h; ++y)
        for (size_t x = 0; x < img.w; x += 4) {
            res[count++] = pixel_at(&img, x, y);
        }

    TEST_ASSERT_EQUAL(res[0], 0);
    TEST_ASSERT_EQUAL(res[1], 1);
    TEST_ASSERT_EQUAL(res[2], 2);
    TEST_ASSERT_EQUAL(res[3], 3);

    free(img.buf);
}

void test_bayer_sqrmat(void)
{
    /* generate an 8x8 Bayer matrix */
    size_t dim = 8;
    int32_t mat[dim * dim];
    bayer_sqrmat(mat, dim);

    int32_t ref[] = {
        0, 32, 8, 40, 2, 34, 10, 42,
        48, 16, 56, 24, 50, 18, 58, 26,
        12, 44, 4, 36, 14, 46, 6, 38, 
        60, 28, 52, 20, 62, 30, 54, 22, 
        3, 35, 11, 43, 1, 33, 9, 41, 
        51, 19, 59, 27, 49, 17, 57, 25, 
        15, 47, 7, 39, 13, 45, 5, 37, 
        63, 31, 55, 23, 61, 29, 53, 21
    };

    TEST_ASSERT_EQUAL_INT32_ARRAY(ref, mat, sizeof(mat) / sizeof(mat[0]));
}

void test_widen(void)
{
    /* start with an 12 byte array filled with:
     * [0xaa, 0xbb, 0xcc,
     *  0xaa, 0xbb, 0xcc,
     *  0xaa, 0xbb, 0xcc,
     *  0xaa, 0xbb, 0xcc]*/
    size_t size = 12;
    uint8_t src[size];
    
    for (size_t i = 0; i < size; ++i) {
        switch (i % 3) {
            case 0:
                src[i] = 0xaa;
                break;
            case 1:
                src[i] = 0xbb;
                break;
            case 2:
                src[i] = 0xcc;
                break;
        }
    }   
    TEST_ASSERT_EQUAL(0xaa, src[0]);
    TEST_ASSERT_EQUAL(0xaa, src[3]);
    TEST_ASSERT_EQUAL(0xaa, src[6]);
    TEST_ASSERT_EQUAL(0xbb, src[1]);
    TEST_ASSERT_EQUAL(0xbb, src[4]);
    TEST_ASSERT_EQUAL(0xbb, src[7]);
    TEST_ASSERT_EQUAL(0xcc, src[2]);
    TEST_ASSERT_EQUAL(0xcc, src[5]);

    /* fit into a 32bit array such that:
     * [0x00aabbcc, 0x00aabbcc]*/

    uint32_t dest[size];
    int count = 0;
    count = widen((int32_t *)&dest, (int8_t *)&src, size);
    
    TEST_ASSERT_EQUAL(3, count);
    TEST_ASSERT_EQUAL(0x00ccbbaa, dest[0]);
    TEST_ASSERT_EQUAL(0x00ccbbaa, dest[1]);
    TEST_ASSERT_EQUAL(0x00ccbbaa, dest[2]);
}

void test_narrow(void)
{
    /* narrow a 3 pixel 32bit array into a 9byte array */
    size_t size = 12;
    int32_t src[] = { 0x00ccbbaa, 0x00ccbbaa, 0x00ccbbaa};

    uint8_t dest[size]; 
    int count = 0;
    count = narrow((int8_t *)&dest, &src, size);

    TEST_ASSERT_EQUAL(count, 9);
    TEST_ASSERT_EQUAL(0xaa, dest[0]);
    TEST_ASSERT_EQUAL(0xbb, dest[1]);
    TEST_ASSERT_EQUAL(0xcc, dest[2]);
    TEST_ASSERT_EQUAL(0xaa, dest[3]);
    TEST_ASSERT_EQUAL(0xbb, dest[4]);
    TEST_ASSERT_EQUAL(0xcc, dest[5]);
}
