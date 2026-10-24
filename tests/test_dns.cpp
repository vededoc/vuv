#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <thread>
#include <string>

#include "vuv.h"
#include "DNSResolver.h"
#include "UvGetAddrInfo.h"

using namespace vuv;

static void runUntil(std::atomic<bool> &done, int maxIters = 100000) {
    for (int i = 0; i < maxIters && !done.load(); ++i) {
        int rc = Loop::Run(UV_RUN_ONCE);
        if (rc == 0 && !done.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}

TEST(DNSResolverTest, QueryLocalhostV4) {
    Loop::Open(nullptr);
    DNSResolver dns;
    std::atomic<bool> done{false};
    int status = -999;
    dns.query("localhost", [&](int s, std::unordered_set<std::string> v4, std::unordered_set<std::string> v6) {
        status = s;
        done = true;
    });
    runUntil(done);
    EXPECT_TRUE(done.load());
    EXPECT_EQ(status, 0);
    dns.close();
    Loop::Close();
}

TEST(DNSResolverTest, QueryNetLocalhost) {
    Loop::Open(nullptr);
    DNSResolver dns;
    std::atomic<bool> done{false};
    int status = -999;
    dns.query("localhost", [&](int s, std::vector<struct sockaddr_storage> v4, std::vector<struct sockaddr_storage> v6) {
        status = s;
        done = true;
    });
    runUntil(done);
    EXPECT_TRUE(done.load());
    EXPECT_EQ(status, 0);
    dns.close();
    Loop::Close();
}

TEST(UvGetAddrInfoTest, ResolveLocalhost) {
    Loop::Open(nullptr);
    UvGetAddrInfo gai;
    ASSERT_EQ(gai.init(), 0);
    std::atomic<bool> done{false};
    int status = -999;
    gai.getAddrInfo("localhost", nullptr, nullptr, [&](int s, struct addrinfo *res) {
        status = s;
        done = true;
    });
    runUntil(done);
    EXPECT_TRUE(done.load());
    EXPECT_EQ(status, 0);
    gai.close();
    Loop::Close();
}
