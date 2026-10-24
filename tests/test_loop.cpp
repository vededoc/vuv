#include <gtest/gtest.h>

#include "vuv.h"

using namespace vuv;

TEST(LoopTest, OpenClose) {
    Loop::Open(nullptr);
    ASSERT_NE(Loop::Get(), nullptr);
    ASSERT_NE(Loop::GetUvLoop(), nullptr);
    EXPECT_FALSE(Loop::IsRunning());
    Loop::Close();
    EXPECT_EQ(Loop::Get(), nullptr);
    EXPECT_EQ(Loop::GetUvLoop(), nullptr);
}

TEST(LoopTest, RunNoPending) {
    Loop::Open(nullptr);
    int rc = Loop::Run(UV_RUN_NOWAIT);
    EXPECT_GE(rc, 0);
    Loop::Close();
}

TEST(LoopTest, TickNonNegative) {
    Loop::Open(nullptr);
    EXPECT_GE(Loop::Get()->tick(), 0);
    Loop::Close();
}

TEST(LoopTest, ExternalLoopNotOwned) {
    uv_loop_t loop;
    ASSERT_EQ(uv_loop_init(&loop), 0);
    Loop::Open(&loop);
    EXPECT_EQ(Loop::GetUvLoop(), &loop);
    Loop::Close();
    EXPECT_EQ(Loop::Get(), nullptr);
    // external loop must be closed by caller
    EXPECT_EQ(uv_loop_close(&loop), 0);
}

TEST(LoopTest, EventHandlesInitiallyEmpty) {
    Loop::Open(nullptr);
    auto handles = Loop::Get()->getEventHandles();
    EXPECT_TRUE(handles.empty());
    Loop::Close();
}
