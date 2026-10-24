#include <vuv/vuv.h>

#include <atomic>
#include <cstdio>
#include <memory>
#include <string>

#include <netinet/in.h>

using namespace vuv;

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

    // Periodic timer: prints a tick every 200 ms. On the 5th tick it stops
    // itself and disconnects the client, which lets the loop exit naturally.
    UvTimer timer;
    timer.init();
    std::atomic<int> ticks{0};
    timer.setOn([&] {
        ++ticks;
        std::printf("timer tick %d\n", ticks.load());
        if (ticks.load() >= 5) {
            timer.stop();
            timer.close();
            client.close();
        }
    });
    timer.start(200, 200);

    server.setOnListen([&](UvStream &srv) {
        peer = std::make_unique<UvTcp>();
        peer->init();
        if (server.accept(peer.get()) == 0) {
            peer->setOnReadLis([&](const char *buf, size_t len) {
                std::string received(buf, len);
                std::printf("server received: %s\n", received.c_str());
                peer->write("pong");
            });
            peer->setOnCnnLis([&](int status) {
                if (status < 0) {
                    std::printf("peer disconnected\n");
                    server.close();
                    if (peer) {
                        peer->close();
                    }
                }
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
    });

    sockaddr_in dest{};
    uv_ip4_addr("127.0.0.1", port, &dest);
    client.connect((const sockaddr *)&dest);

    // Block until no active handles remain; the loop exits naturally.
    Loop::Run();

    if (peer) {
        peer.reset();
    }
    Loop::Close();
    return 0;
}
