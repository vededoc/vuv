#pragma once

#include <cstdint>
#include <functional>
#include <uv.h>
#include <string>
#include "etc.h"

namespace vuv {

    class UvUdp {

    public:
        typedef std::function<void(const char* buf, size_t len, const struct sockaddr *, unsigned)> RecvLis;
        typedef std::function<void(int)> SendLis;
        UvUdp();
        virtual ~UvUdp();
        int init();
        int initEx(unsigned int flags);
        void close();

        int bind(const struct sockaddr *addr, unsigned int flags);
        int bind(const char *ipaddr, uint16_t port, unsigned int flags);
        int send(const uint8_t* buf, size_t len, const sockaddr* addr);
        int send(const std::string& msg, const sockaddr* addr);

        int recvStart();
        int recvStop();
        void setOnRecvLis(RecvLis lis);
        void setOnSendLis(SendLis lis);
        int getSockName(struct sockaddr* name, int* namelen);
        int setMemberShip(const char* multicast_addr, const char* interface_addr, uv_membership membership);
        int setMulticastLoop(int on);
        int setMulticastTtl(int ttl);
        int setMulticastInterface(const char* interface_addr);
        int setBroadcast(int on);
        int setTtl(int ttl);
        uv_udp_t *getRawHandle();
    private:
        struct UdpHandle;
        UdpHandle* _uvh;
        RecvLis _recvLis;
        SendLis _sendLis;
        uv_loop_t *_loop;

        void procSendCallback(int status);
    };
    typedef std::unique_ptr<UvUdp> upUvUdp;
    typedef std::shared_ptr<UvUdp> spUvUdp;
    typedef std::weak_ptr<UvUdp> wpUvUdp;
}
