#include <cassert>
#include "DNSResolver.h"
#include "Loop.h"
#include "uvcpplog.h"

using namespace std;

namespace vuv {

    struct DNSResolver::ReqStub {
        DNSResolver *owner;
        uint32_t rid;
        uv_getaddrinfo_t uvreq;
        std::function<void(int, struct addrinfo *)> callback;
        QueryNetCb queryNetCb;
        QueryCb callback2;
    };

    DNSResolver::DNSResolver() {
        reqIdSeed_ = 1;
    }

    DNSResolver::~DNSResolver() {
        close();
    }



    uint32_t DNSResolver::query(const string &node, QueryCb cb) {
        auto rid = reqIdSeed_;
        reqIdSeed_ += 2;

        auto reqstub = new ReqStub;
        reqstub->owner = this;
        reqstub->callback2 = cb;
        reqstub->rid = rid;
        reqstub->uvreq.data = reqstub;

        static auto info_cb = +[](uv_getaddrinfo_t *req,
                                  int status,
                                  struct addrinfo *res) {
            auto stub = (ReqStub *) req->data;
            if (stub->owner) {
                stub->owner->reqMap_.erase(stub->rid);
                uli("on res, remain=%ld", stub->owner->reqMap_.size());
                std::unordered_set<string> v4ips;
                std::unordered_set<string> v6ips;
                auto addrs = res;
                for (;addrs;) {
                    if (addrs->ai_addr && addrs->ai_family == AF_INET) {
                        sockaddr_in *inaddr = (sockaddr_in *) addrs->ai_addr;
                        char dst[128];
                        uv_ip4_name(inaddr, dst, 128);
                        v4ips.emplace(dst);
                    } else if (addrs->ai_addr && addrs->ai_family == AF_INET6) {
                        auto inaddr = (sockaddr_in6 *) addrs->ai_addr;
                        char dst[128];
                        uv_ip6_name(inaddr, dst, 128);
                        v6ips.emplace(dst);
                    }
                    addrs = addrs->ai_next;
                }
                stub->callback2(status, std::move(v4ips), std::move(v6ips));
            }

            uv_freeaddrinfo(res);
            delete stub;
        };

        auto rc = uv_getaddrinfo(Loop::GetUvLoop(), &(reqstub->uvreq), info_cb, node.data(), nullptr, nullptr);
        if (!rc) {
            reqMap_[rid] = reqstub;
        } else {
            delete reqstub;
        }
        assert(!rc);
        return rid;
    }

    uint32_t DNSResolver::query(const string &node, DNSResolver::QueryNetCb cb) {
        auto rid = reqIdSeed_;
        reqIdSeed_ += 2;

        auto reqstub = new ReqStub;
        reqstub->owner = this;
        reqstub->queryNetCb = cb;
        reqstub->rid = rid;
        reqstub->uvreq.data = reqstub;

        static auto info_cb = +[](uv_getaddrinfo_t *req,
                                  int status,
                                  struct addrinfo *res) {
            auto stub = (ReqStub *) req->data;
            if (stub->owner) {
                stub->owner->reqMap_.erase(stub->rid);
                uli("on res, remain=%ld", stub->owner->reqMap_.size());
                std::vector<sockaddr_storage> v4ips;
                std::vector<sockaddr_storage> v6ips;
                auto addrs = res;
                for (; addrs;) {
                    if (addrs->ai_addr && addrs->ai_family == AF_INET) {
                        v4ips.emplace_back();
                        auto &ni = v4ips.back();
                        memcpy(&ni, addrs->ai_addr, addrs->ai_addrlen);

                    } else if (addrs->ai_addr && addrs->ai_family == AF_INET6) {
                        v6ips.emplace_back();
                        auto &ni = v6ips.back();
                        memcpy(&ni, addrs->ai_addr, addrs->ai_addrlen);
                    }
                    addrs = addrs->ai_next;
                }
                stub->queryNetCb(status, std::move(v4ips), std::move(v6ips));
            }

            uv_freeaddrinfo(res);
            delete stub;
        };

        auto rc = uv_getaddrinfo(Loop::GetUvLoop(), &(reqstub->uvreq), info_cb, node.data(), nullptr, nullptr);
        if (!rc) {
            reqMap_[rid] = reqstub;
        } else {
            delete reqstub;
        }
        assert(!rc);
        return rid;
    }

    void DNSResolver::close() {
        for (auto &kv: reqMap_) {
            kv.second->owner = nullptr;
        }
        reqMap_.clear();
    }

    void DNSResolver::abortReq(uint32_t req_id) {
        auto itr = reqMap_.find(req_id);
        if (itr != reqMap_.end()) {
            itr->second->owner = nullptr;
            reqMap_.erase(itr);
        }
    }
}