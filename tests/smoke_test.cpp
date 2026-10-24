#include <gtest/gtest.h>

#include "vuv.h"

using namespace vuv;

TEST(VuvSmoke, LoopOpenClose) {
    Loop::Open(nullptr);
    ASSERT_NE(Loop::Get(), nullptr);
    ASSERT_NE(Loop::GetUvLoop(), nullptr);
    Loop::Close();
    EXPECT_EQ(Loop::Get(), nullptr);
}

TEST(VuvSmoke, LoopRunNoPending) {
    Loop::Open(nullptr);
    int rc = Loop::Run(UV_RUN_NOWAIT);
    EXPECT_GE(rc, 0);
    Loop::Close();
}
