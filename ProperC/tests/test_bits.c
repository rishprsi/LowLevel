#include "ctest.h"
#include "bits.h"

int main(void) {
    /* count_set_bits */
    CHECK_INT_EQ(count_set_bits(0), 0);
    CHECK_INT_EQ(count_set_bits(0xFFFFFFFFu), 32);
    CHECK_INT_EQ(count_set_bits(0xBu), 3);   /* 1011 */
    CHECK_INT_EQ(count_set_bits(0x80000000u), 1);

    /* is_power_of_two */
    CHECK_FALSE(is_power_of_two(0));
    CHECK_TRUE(is_power_of_two(1));
    CHECK_TRUE(is_power_of_two(2));
    CHECK_FALSE(is_power_of_two(3));
    CHECK_TRUE(is_power_of_two(0x80000000u));
    CHECK_FALSE(is_power_of_two(6));

    /* reverse_bits */
    CHECK_UINT_EQ(reverse_bits(0u), 0u);
    CHECK_UINT_EQ(reverse_bits(0xFFFFFFFFu), 0xFFFFFFFFu);
    CHECK_UINT_EQ(reverse_bits(1u), 0x80000000u);
    CHECK_UINT_EQ(reverse_bits(0x80000000u), 1u);
    CHECK_UINT_EQ(reverse_bits(2u), 0x40000000u);

    /* set / clear / toggle */
    CHECK_UINT_EQ(set_bit(0u, 0), 1u);
    CHECK_UINT_EQ(set_bit(0u, 31), 0x80000000u);
    CHECK_UINT_EQ(set_bit(0xFu, 1), 0xFu);     /* already set */
    CHECK_UINT_EQ(clear_bit(0xFu, 0), 0xEu);
    CHECK_UINT_EQ(clear_bit(0xFu, 4), 0xFu);   /* already clear */
    CHECK_UINT_EQ(toggle_bit(0u, 5), 0x20u);
    CHECK_UINT_EQ(toggle_bit(0x20u, 5), 0u);

    /* count_trailing_zeros */
    CHECK_INT_EQ(count_trailing_zeros(1u), 0);
    CHECK_INT_EQ(count_trailing_zeros(8u), 3);
    CHECK_INT_EQ(count_trailing_zeros(0x80000000u), 31);
    CHECK_INT_EQ(count_trailing_zeros(0u), 32);
    CHECK_INT_EQ(count_trailing_zeros(0xFFFFFFF0u), 4);

    CTEST_END();
}
