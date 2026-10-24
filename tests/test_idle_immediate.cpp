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

TEST(IdleTest, CallbackInvoked) {
    Loop::Open(nullptr);
    Idle idle;
    std::atomic<bool> done{false};
    int count = 0;
    idle.init();
    idle.setOn([&] {
        ++count;
        idle.stop();
        done = true;
    });
    idle.start();
    runUntil(done);
    EXPECT_GE(count, 1);
    idle.deinit();
    Loop::Close();
}

TEST(IdleTest, StopPreventsCallback) {
    Loop::Open(nullptr);
    Idle idle;
    int count = 0;
    idle.init();
    idle.setOn([&] { ++count; });
    // never start
    // run a single iteration
    Loop::Run(UV_RUN_NOWAIT);
    EXPECT_EQ(count, 0);
    idle.deinit();
    Loop::Close();
}

TEST(ImmediateTest, CallbackRuns) {
    Loop::Open(nullptr);
    Immediate imm;
    std::atomic<bool> done{false};
    int count = 0;
    imm.setImmediate([&] {
        ++count;
        done = true;
    });
    runUntil(done);
    EXPECT_GE(count, 1);
    imm.stop();
    Loop::Close();
}

TEST(ImmediateTest, ClearImmediatePreventsCallback) {
    Loop::Open(nullptr);
    Immediate imm;
    std::atomic<bool> done{false};
    int count = 0;
    uint32_t h = imm.setImmediate([&] { ++count; done = true; });
    imm.clearImmediate(h);
    // run a few iterations
    for (int i = 0; i < 50 && !done.load(); ++i) {
        Loop::Run(UV_RUN_ONCE);
    }
    EXPECT_EQ(count, 0);
    imm.stop();
    Loop::Close();
}

TEST(ImmediateTest, MultipleImmediates) {
    Loop::Open(nullptr);
    Immediate imm;
    std::atomic<int> count{0};
    std::atomic<bool> done{false};
    imm.setImmediate([&] { count++; });
    imm.setImmediate([&] { count++; if (count >= 2) done = true; });
    runUntil(done);
    EXPECT_GE(count.load(), 2);
    imm.stop();
    Loop::Close();
}
