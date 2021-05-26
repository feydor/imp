/* test/imageproc.c */
#include <unity.h>

#include "../include/imageproc.h"

/* test prototypes */
void test_closestfrompal(void);

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_closestfrompal);
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
