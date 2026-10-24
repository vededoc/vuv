#pragma once

#include <uv.h>
#include "UvBufStore.h"
#include <functional>
#include <string>

namespace vuv {
	class UvStream {
		friend class Loop;
	public:
        typedef std::function<void(const char *, size_t)> ReadCb;
        typedef std::function<void(int)> CnnCb;
        typedef std::function<void(UvStream &lisSock)> ListenCb;

		UvStream();

		virtual ~UvStream();

		int readStart();

		int readStop();

		int listen(int backlogs=128);

		int accept(UvStream* newtcp);

		int write(const char *buf, size_t len);

		int write(const std::string &msg);

		int write2(UvStream* send_handle);

//		int tryWrite(const uint8_t *buf, size_t len);

//		int shutDown(ShutdownLis lis);

		int isReadable();

		int isWritable();

		void setOnCnnLis(CnnCb lis);

		void setOnReadLis(ReadCb lis);

		void setOnWriteLis(std::function<void()> cb);

        void setOnListen(ListenCb cb);

		uint32_t getWriteQueCnt();

		uint32_t getSendQueCnt();

        void close();

        uv_stream_t *getRawHandle();

	protected:
        struct write_req_t;
        struct StreamHandle {
            uv_any_handle rawh;
            uv_connect_t cnn;
            UvStream *owner{};

            UvBufStore rbufs;
            UvBufStore wbufs;
            UvBufStore writeHandleBufs;

            void closeUv();

            char *allocWriteBuf(size_t len);

            write_req_t *allocWriteReq();

            void recycleWriteReq(write_req_t *req);

            void procCnnCb(int status);
        };

        uv_loop_t *_loop;
        StreamHandle *_uvh;
        CnnCb _cnnCb;

        int initInternal();

    private:
        ReadCb _readCb;
        ListenCb _lisCb;
        std::function<void()> _writeCb;

	};
}