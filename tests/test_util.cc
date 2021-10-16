#include <gtest/gtest.h>

extern "C"
{
#include "util.h"
}

TEST(test_doy_to_date_string, 1969)
{
    char s[11];
    int ret;

    ret = doy_to_date_string(1969, 1, s);
    ASSERT_STREQ("1969-01-01", s);

    ret = doy_to_date_string(1969, 31, s);
    ASSERT_STREQ("1969-01-31", s);

    ret = doy_to_date_string(1969, 32, s);
    ASSERT_STREQ("1969-02-01", s);

    ret = doy_to_date_string(1969, 59, s);
    ASSERT_STREQ("1969-02-28", s);

    ret = doy_to_date_string(1969, 60, s);
    ASSERT_STREQ("1969-03-01", s);

    ret = doy_to_date_string(1969, 365, s);
    ASSERT_EQ(TRUE, ret);
    ASSERT_STREQ("1969-12-31", s);

    ret = doy_to_date_string(1969, 366, s);
    ASSERT_EQ(FALSE, ret);
}

TEST(test_doy_to_date_string, 1972)
{
    char s[11];
    int ret;

    ret = doy_to_date_string(1972, 1, s);
    ASSERT_STREQ("1972-01-01", s);

    ret = doy_to_date_string(1972, 31, s);
    ASSERT_STREQ("1972-01-31", s);

    ret = doy_to_date_string(1972, 32, s);
    ASSERT_STREQ("1972-02-01", s);

    ret = doy_to_date_string(1972, 59, s);
    ASSERT_STREQ("1972-02-28", s);

    ret = doy_to_date_string(1972, 60, s);
    ASSERT_STREQ("1972-02-29", s);

    ret = doy_to_date_string(1972, 61, s);
    ASSERT_STREQ("1972-03-01", s);

    ret = doy_to_date_string(1972, 365, s);
    ASSERT_EQ(TRUE, ret);
    ASSERT_STREQ("1972-12-30", s);

    ret = doy_to_date_string(1972, 366, s);
    ASSERT_EQ(TRUE, ret);
    ASSERT_STREQ("1972-12-31", s);

    ret = doy_to_date_string(1972, 367, s);
    ASSERT_EQ(FALSE, ret);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
