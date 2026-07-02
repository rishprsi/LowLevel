#include "ctest.h"
#include "layout.h"

#include <stddef.h> /* offsetof */

int main(void) {
    SECTION("struct padding (char,int)");
    CHECK_INT_EQ(PRED_SIZEOF_CHAR_INT, (int)sizeof(LCharInt));
    CHECK_INT_EQ(PRED_OFFSETOF_CHAR_INT_I, (int)offsetof(LCharInt, i));

    SECTION("field ordering & tail padding");
    CHECK_INT_EQ(PRED_SIZEOF_CHAR_INT_CHAR, (int)sizeof(LCharIntChar));
    CHECK_INT_EQ(PRED_SIZEOF_INT_CHAR_CHAR, (int)sizeof(LIntCharChar));

    SECTION("double alignment padding");
    CHECK_INT_EQ(PRED_SIZEOF_CHAR_DOUBLE, (int)sizeof(LCharDouble));
    CHECK_INT_EQ(PRED_OFFSETOF_CHAR_DOUBLE_D, (int)offsetof(LCharDouble, d));

    SECTION("nested struct alignment");
    CHECK_INT_EQ(PRED_SIZEOF_NESTED, (int)sizeof(LNested));
    CHECK_INT_EQ(PRED_OFFSETOF_NESTED_INNER, (int)offsetof(LNested, inner));

    SECTION("single-char struct size");
    CHECK_INT_EQ(PRED_SIZEOF_JUST_CHAR, (int)sizeof(LJustChar));

    SECTION("long long alignment");
    CHECK_INT_EQ(PRED_SIZEOF_LONGLONG_CHAR, (int)sizeof(LLongLongChar));

    SECTION("short alignment");
    CHECK_INT_EQ(PRED_SIZEOF_CHARS_SHORT, (int)sizeof(LCharsShort));
    CHECK_INT_EQ(PRED_OFFSETOF_CHARS_SHORT_S, (int)offsetof(LCharsShort, s));

    /* array stride: measured with real pointer arithmetic */
    SECTION("array stride & tail padding");
    LCharInt arr[2];
    CHECK_INT_EQ(PRED_ARRAY2_STRIDE_BYTES,
                 (int)((char *)&arr[1] - (char *)&arr[0]));
    /* ...and it is by definition the element size, tail padding included */
    CHECK_INT_EQ(PRED_ARRAY2_STRIDE_BYTES, (int)sizeof(LCharInt));
    CHECK_INT_EQ(PRED_ARRAY2_STRIDE_BYTES * 2, (int)sizeof arr);

    CTEST_END();
}
