#include <vuv/vuv.h>

#include <atomic>
#include <chrono>
#include <cstdio>
#include <memory>
#include <string>
#include <thread>

#include <netinet/in.h>

using namespace vuv;

static void runUntil(std::atomic<bool> &done, int maxIters = 100000) {
    for (int i = 0; i < maxIters && !done.load(); ++i) {
        int rc = Loop::Run(UV_RUN_ONCE);
        if (rc == 0 && !done.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}

int main() {
    Loop::Open(nullptr);

    UvTcp server, client;
    server.init();
    client.init();
    server.bindAndListen(0, "127.0.0.1");

    sockaddr_storage saddr{};
    int slen = sizeof(saddr);
    server.getSockName((sockaddr *)&saddr, &slen);
    uint16_t port = ntohs(((sockaddr_in *)&saddr)->sin_port);

    std::unique_ptr<UvTcp> peer;
    std::string received;
    std::atomic<bool> done{false};
    std::atomic<bool> pongReceived{false};
    std::atomic<bool> timerDone{false};

    auto maybeDone = [&] {
        if (pongReceived.load() && timerDone.load()) {
            done = true;
        }
    };

    // Periodic timer: prints a tick every 200 ms, 5 times.
    UvTimer timer;
    timer.init();
    std::atomic<int> ticks{0};
    timer.setOn([&] {
        ++ticks;
        std::printf("timer tick %d\n", ticks.load());
        if (ticks.load() >= 5) {
            timer.stop();
            timerDone = true;
            maybeDone();
        }
    });
    timer.start(200, 200);

    server.setOnListen([&](UvStream &srv) {
        peer = std::make_unique<UvTcp>();
        peer->init();
        if (server.accept(peer.get()) == 0) {
            peer->setOnReadLis([&](const char *buf, size_t len) {
                received.assign(buf, len);
                std::printf("server received: %s\n", received.c_str());
                peer->write("pong");
            });
            peer->readStart();
        }
    });

    client.setOnCnnLis([&](int status) {
        if (status == 0) {
            std::printf("client connected, sending ping\n");
            client.readStart();
            client.write("ping");
        }
    });
    client.setOnReadLis([&](const char *buf, size_t len) {
        std::string msg(buf, len);
        std::printf("client received: %s\n", msg.c_str());
        pongReceived = true;
        maybeDone();
    });

    sockaddr_in dest{};
    uv_ip4_addr("127.0.0.1", port, &dest);
    client.connect((const sockaddr *)&dest);

    runUntil(done);

    if (peer) {
        peer->close();
    }
    client.close();
    server.close();
    timer.close();
    Loop::Close();
    return 0;
}
