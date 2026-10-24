#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <thread>
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "vuv.h"
#include "UvUdp.h"

using namespace vuv;

static void runUntil(std::atomic<bool> &done, int maxIters = 100000) {
    for (int i = 0; i < maxIters && !done.load(); ++i) {
        int rc = Loop::Run(UV_RUN_ONCE);
        if (rc == 0 && !done.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}

TEST(UvUdpTest, SendRecvLocalhost) {
    Loop::Open(nullptr);
    UvUdp server, client;
    ASSERT_EQ(server.init(), 0);
    ASSERT_EQ(client.init(), 0);
    ASSERT_EQ(server.bind("127.0.0.1", 0, 0), 0);

    sockaddr_storage saddr{};
    int slen = sizeof(saddr);
    ASSERT_EQ(server.getSockName((sockaddr *)&saddr, &slen), 0);
    uint16_t port = ntohs(((sockaddr_in *)&saddr)->sin_port);

    std::atomic<bool> done{false};
    std::string received;
    server.setOnRecvLis([&](const char *buf, size_t len, const struct sockaddr *, unsigned) {
        received.assign(buf, len);
        done = true;
    });
    ASSERT_EQ(server.recvStart(), 0);

    sockaddr_in dest{};
    ASSERT_EQ(uv_ip4_addr("127.0.0.1", port, &dest), 0);
    ASSERT_EQ(client.send("hello", (const sockaddr *)&dest), 0);

    runUntil(done);
    EXPECT_TRUE(done.load());
    EXPECT_EQ(received, "hello");

    server.recvStop();
    server.close();
    client.close();
    Loop::Close();
}
