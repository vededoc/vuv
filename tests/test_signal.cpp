#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <thread>
#include <csignal>
#include <unistd.h>

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

TEST(UvSignalTest, SignalDelivered) {
    Loop::Open(nullptr);
    UvSignal sig;
    ASSERT_EQ(sig.init(), 0);
    std::atomic<bool> done{false};
    sig.setOn([&](int s) {
        EXPECT_EQ(s, SIGUSR1);
        done = true;
    });
    ASSERT_EQ(sig.start(SIGUSR1), 0);
    raise(SIGUSR1);
    runUntil(done);
    EXPECT_TRUE(done.load());
    sig.stop();
    sig.close();
    Loop::Close();
}
