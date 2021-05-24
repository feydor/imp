/* test/imageproc.c */
#include <unity.h>

#include "../include/imageproc.h"

/* test prototypes */
void test_closestcolor(void);

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_closestcolor);
}

void setUp(void)
{
    // set stuff up here
}

void tearDown(void)
{
    // clean stuff up here
}

void test_closestcolor(void)
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
    closest = closestcolor(0x000000, pal, sizeof(pal));
    TEST_ASSERT_EQUAL(closest, 0x000000);
}
