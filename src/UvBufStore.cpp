#define UVCLOG_LEVEL_INFO

#include <cassert>
#include "UvBufStore.h"

#include "uvcpplog.h"

namespace vuv {
    struct UvBufStore::Buf {
        struct Buf *next{};
        size_t capacity{};
    };

    UvBufStore::UvBufStore() : _maxCapacity(10 * 1024 * 1024), _head{}, _listCnt{} {
      static_assert(sizeof(Buf) == 16);
    }

    UvBufStore::~UvBufStore() {
      assert(_allocCnt == 0);
      freeBufs();

    }

    char *UvBufStore::alloc(size_t dsize) {
      uld("alloc buf=%ld", dsize);
#ifndef NDEBUG
      _allocCnt++;
#endif
      if (_head) {
        auto pBuf = _head;
        _head = _head->next;
        if (pBuf->capacity < dsize) {
          free(pBuf);
          auto alloc_size = dsize * 2;
          if (alloc_size > _maxCapacity) {
            alloc_size = dsize;
          }
          uld("alloc cap size=%ld, dsize=%ld", alloc_size, dsize);
          pBuf = (Buf *) malloc(sizeof(Buf) + alloc_size);
          pBuf->capacity = alloc_size;
        }
        _listCnt--;
        return (char *) (pBuf + 1);
      } else {
        uld("alloc new buf, size=%ld", dsize);
        auto pBuf = (Buf *) malloc(sizeof(Buf) + dsize);
        pBuf->capacity = dsize;
        return (char *) (pBuf + 1);
      }

    }

    void UvBufStore::recycle(char *buf) {
#ifndef NDEBUG
      _allocCnt--;
      uld("recycle buf, allocCnt=%d, bufCnt=%d", _allocCnt, _listCnt);
#endif
      auto pBuf = ((Buf *) buf) - 1;
      if (pBuf->capacity > _maxCapacity) {
        ulw("*** give up buf for too large");
        free(pBuf);
        return;
      }
      if (_head) {
        pBuf->next = _head;
        _head = pBuf;
      } else {
        _head = ((Buf *) buf) - 1;
        _head->next = nullptr;
      }
      _listCnt++;
    }

    void UvBufStore::freeBufs() {
      uld("free bufs, listCnt=%d", _listCnt);
      for (; _head;) {
        auto pBuf = _head;
        _head = _head->next;
        free(pBuf);
      }
      _listCnt = 0;
    }

    void UvBufStore::setMaxBufSize(size_t cap) {
      _maxCapacity = cap;
    }
}