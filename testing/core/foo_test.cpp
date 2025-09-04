#include <gtest/gtest.h>
#include "foo.hpp"

TEST(FooTest, Baz) {
    EXPECT_EQ(foo::bar::baz(), -1);
}