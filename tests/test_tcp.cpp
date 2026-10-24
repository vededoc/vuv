#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <thread>
#include <memory>
#include <netinet/in.h>

#include "vuv.h"
#include "UvTcp.h"

using namespace vuv;

static void runUntil(std::atomic<bool> &done, int maxIters = 100000) {
    for (int i = 0; i < maxIters && !done.load(); ++i) {
        int rc = Loop::Run(UV_RUN_ONCE);
        if (rc == 0 && !done.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}

TEST(UvTcpTest, EchoRoundTrip) {
    Loop::Open(nullptr);
    UvTcp server, client;
    ASSERT_EQ(server.init(), 0);
    ASSERT_EQ(client.init(), 0);
    ASSERT_EQ(server.bindAndListen(0, "127.0.0.1"), 0);

    sockaddr_storage saddr{};
    int slen = sizeof(saddr);
    ASSERT_EQ(server.getSockName((sockaddr *)&saddr, &slen), 0);
    uint16_t port = ntohs(((sockaddr_in *)&saddr)->sin_port);

    std::atomic<bool> done{false};
    std::string received;
    std::unique_ptr<UvTcp> peer;

    server.setOnListen([&](UvStream &srv) {
        peer = std::make_unique<UvTcp>();
        peer->init();
        if (server.accept(peer.get()) == 0) {
            peer->setOnReadLis([&](const char *buf, size_t len) {
                received.assign(buf, len);
                done = true;
            });
            peer->readStart();
        }
    });

    client.setOnCnnLis([&](int status) {
        ASSERT_EQ(status, 0);
        client.write("ping");
    });

    sockaddr_in dest{};
    ASSERT_EQ(uv_ip4_addr("127.0.0.1", port, &dest), 0);
    ASSERT_EQ(client.connect((const sockaddr *)&dest), 0);

    runUntil(done);
    EXPECT_TRUE(done.load());
    EXPECT_EQ(received, "ping");

    if (peer) peer->close();
    client.close();
    server.close();
    Loop::Close();
}
