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

TEST(UvCheckTest, CallbackInvoked) {
    Loop::Open(nullptr);
    UvCheck check;
    UvTimer timer;
    ASSERT_EQ(check.init(), 0);
    ASSERT_EQ(timer.init(), 0);
    timer.setOn([] {});
    ASSERT_EQ(timer.start(1, 1), 0);

    std::atomic<int> count{0};
    std::atomic<bool> done{false};
    check.setOn([&] {
        if (++count >= 3) {
            check.stop();
            timer.stop();
            done = true;
        }
    });
    ASSERT_EQ(check.start(), 0);
    runUntil(done);
    EXPECT_GE(count.load(), 3);
    check.close();
    timer.close();
    Loop::Close();
}

TEST(UvPrepareTest, CallbackInvoked) {
    Loop::Open(nullptr);
    UvPrepare prep;
    UvTimer timer;
    ASSERT_EQ(prep.init(), 0);
    ASSERT_EQ(timer.init(), 0);
    timer.setOn([] {});
    ASSERT_EQ(timer.start(1, 1), 0);

    std::atomic<int> count{0};
    std::atomic<bool> done{false};
    ASSERT_EQ(prep.start([&] {
        if (++count >= 3) {
            prep.stop();
            timer.stop();
            done = true;
        }
    }), 0);
    runUntil(done);
    EXPECT_GE(count.load(), 3);
    prep.close();
    timer.close();
    Loop::Close();
}
