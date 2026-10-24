#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <thread>
#include <sys/socket.h>
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

TEST(UvPollTest, ReadableTriggered) {
    Loop::Open(nullptr);
    int fds[2];
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);

    UvPoll poll;
    ASSERT_EQ(poll.init(fds[0]), 0);
    std::atomic<bool> done{false};
    poll.setOnEvent([&](int events) {
        if (events & UV_READABLE) done = true;
    });
    ASSERT_EQ(poll.start(UV_READABLE), 0);

    // make data available
    char byte = 'x';
    ASSERT_EQ(write(fds[1], &byte, 1), 1);

    runUntil(done);
    EXPECT_TRUE(done.load());

    poll.stop();
    poll.close();
    close(fds[0]);
    close(fds[1]);
    Loop::Close();
}
