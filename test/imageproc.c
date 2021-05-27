/* test/imageproc.c */
#include <unity.h>
#include <stdlib.h> /* for malloc */

#include "../include/imageproc.h"

/* test prototypes */
void test_closestfrompal(void);
void test_pixelat(void);

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_closestfrompal);
    RUN_TEST(test_pixelat);
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
