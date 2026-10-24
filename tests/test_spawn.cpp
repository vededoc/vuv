#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <thread>
#include <string>

#include "vuv.h"
#include "UvSpawn.h"

using namespace vuv;

static void runUntil(std::atomic<bool> &done, int maxIters = 100000) {
    for (int i = 0; i < maxIters && !done.load(); ++i) {
        int rc = Loop::Run(UV_RUN_ONCE);
        if (rc == 0 && !done.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}

TEST(UvSpawnTest, EchoExitZero) {
    Loop::Open(nullptr);
    UvSpawn sp;
    std::atomic<bool> done{false};
    std::string out;
    int exitCode = -1;
    sp.setOnConsoleOut([&](const char *buf, size_t len) { out.append(buf, len); },
                       [&](const char *, size_t) {});
    sp.setOnExit([&](int code) {
        exitCode = code;
        done = true;
    });
    std::vector<std::string> args = {"/bin/echo", "hello"};
    ASSERT_EQ(sp.run(args), 0);
    runUntil(done);
    EXPECT_TRUE(done.load());
    EXPECT_EQ(exitCode, 0);
    EXPECT_NE(out.find("hello"), std::string::npos);
    sp.close();
    Loop::Close();
}
