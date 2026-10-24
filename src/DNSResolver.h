#pragma once

#include <uv.h>
#include <functional>
#include <unordered_set>
#include <string>

namespace vuv {

    class DNSResolver {
    public:
        typedef std::function<void(int, std::unordered_set<std::string> v4ips, std::unordered_set<std::string> v6ips)> QueryCb;
        typedef std::function<void(int, std::vector<struct sockaddr_storage>, std::vector<struct sockaddr_storage> )> QueryNetCb;
        DNSResolver();

        virtual ~DNSResolver();

        uint32_t query(const std::string &node, QueryCb cb);

        uint32_t query(const std::string &node, QueryNetCb cb);

        void abortReq(uint32_t req_id);

        void close();

    private:
        struct ReqStub;
        uint32_t reqIdSeed_;
        std::unordered_map<uint32_t, ReqStub *> reqMap_;
    };
}
