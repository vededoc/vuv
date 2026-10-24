#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <thread>

#include "vuv.h"

using namespace vuv;

static void runUntil(std::atomic<bool> &done, int maxIters = 100000) {
    for (int i = 0; i < maxIters && !done.load(); ++i) {
        int rc = Loop::Run(UV_RUN_ONCE);
        if (rc == 0 && !done.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}

TEST(UvTimerTest, OneShotFires) {
    Loop::Open(nullptr);
    UvTimer timer;
    ASSERT_EQ(timer.init(), 0);
    std::atomic<bool> done{false};
    int count = 0;
    timer.setOn([&] { ++count; done = true; });
    ASSERT_EQ(timer.start(10, 0), 0);
    runUntil(done);
    EXPECT_GE(count, 1);
    timer.close();
    Loop::Close();
}

TEST(UvTimerTest, RepeatFiresMultiple) {
    Loop::Open(nullptr);
    UvTimer timer;
    ASSERT_EQ(timer.init(), 0);
    std::atomic<int> count{0};
    std::atomic<bool> done{false};
    timer.setOn([&] {
        if (++count >= 3) {
            timer.stop();
            done = true;
        }
    });
    ASSERT_EQ(timer.start(10, 10), 0);
    runUntil(done);
    EXPECT_GE(count.load(), 3);
    timer.close();
    Loop::Close();
}

TEST(UvTimerTest, StartWithoutCallbackFails) {
    Loop::Open(nullptr);
    UvTimer timer;
    ASSERT_EQ(timer.init(), 0);
    EXPECT_EQ(timer.start(10, 0), -1);
    timer.close();
    Loop::Close();
}

TEST(UvTimerTest, SetRepeatAndGetRepeat) {
    Loop::Open(nullptr);
    UvTimer timer;
    ASSERT_EQ(timer.init(), 0);
    timer.setRepeat(50);
    EXPECT_EQ(timer.getRepeat(), 50u);
    timer.setRepeat(0);
    EXPECT_EQ(timer.getRepeat(), 0u);
    timer.close();
    Loop::Close();
}
