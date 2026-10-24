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

TEST(UvAsyncTest, CallbackInvoked) {
    Loop::Open(nullptr);
    UvAsync async;
    ASSERT_EQ(async.init(), 0);
    std::atomic<bool> done{false};
    int count = 0;
    async.setOn([&] {
        ++count;
        done = true;
    });
    ASSERT_EQ(async.send(), 0);
    runUntil(done);
    EXPECT_GE(count, 1);
    async.close();
    Loop::Close();
}

TEST(UvAsyncTest, MultipleSends) {
    Loop::Open(nullptr);
    UvAsync async;
    ASSERT_EQ(async.init(), 0);
    std::atomic<int> count{0};
    std::atomic<bool> done{false};
    async.setOn([&] {
        ++count;
        done = true;
    });
    for (int i = 0; i < 3; ++i) async.send();
    runUntil(done);
    EXPECT_GE(count.load(), 1);
    async.close();
    Loop::Close();
}

TEST(UvAsyncTest, SendWithoutInitFails) {
    Loop::Open(nullptr);
    UvAsync async;
    EXPECT_EQ(async.send(), -1);
    Loop::Close();
}
