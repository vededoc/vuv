#include <gtest/gtest.h>

#include "UvBufStore.h"

using namespace vuv;

TEST(UvBufStoreTest, AllocAndRecycle) {
    UvBufStore store;
    char *buf = store.alloc(128);
    ASSERT_NE(buf, nullptr);
    store.recycle(buf);
    char *buf2 = store.alloc(64);
    ASSERT_NE(buf2, nullptr);
    // reused buffer possible
    store.recycle(buf2);
}

TEST(UvBufStoreTest, AllocLarge) {
    UvBufStore store;
    char *buf = store.alloc(1024 * 1024);
    ASSERT_NE(buf, nullptr);
    store.recycle(buf);
}

TEST(UvBufStoreTest, SetMaxBufSize) {
    UvBufStore store;
    store.setMaxBufSize(256);
    char *buf = store.alloc(512);
    ASSERT_NE(buf, nullptr);
    store.recycle(buf); // capacity > max, should free internally
}

TEST(UvBufStoreTest, ReuseCapacityGrowth) {
    UvBufStore store;
    char *buf = store.alloc(100);
    store.recycle(buf);
    char *buf2 = store.alloc(200); // needs growth
    ASSERT_NE(buf2, nullptr);
    store.recycle(buf2);
}
