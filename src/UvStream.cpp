#include "Loop.h"
#include "UvStream.h"

#include <cassert>

#include "uvcpplog.h"
#include "etc.h"

#define RAWH() ((uv_stream_t*)getRawHandle())
#define GETOBJH(H) ((UvStream*)(((HandleHolder*)H->data))->uvh)

namespace vuv {

    struct UvStream::write_req_t {
        uv_write_t req;
        uv_buf_t buf;
    };

    void UvStream::StreamHandle::closeUv() {
        uv_close((uv_handle_t *) &rawh, +[](uv_handle_t *handle) {
            uld("on closed");
            delete (StreamHandle *) handle->data;
        });
    }

    char *UvStream::StreamHandle::allocWriteBuf(size_t len) {
        return wbufs.alloc(len);
    }

    UvStream::write_req_t *UvStream::StreamHandle::allocWriteReq() {
        return (write_req_t *) writeHandleBufs.alloc(sizeof(write_req_t));
    }

    void UvStream::StreamHandle::recycleWriteReq(write_req_t *req) {
        wbufs.recycle((char *) req->buf.base);
        writeHandleBufs.recycle((char *) req);
    }

    void UvStream::StreamHandle::procCnnCb(int status) {
        if (owner && owner->_cnnCb) {
            owner->_cnnCb(status);
        }
    }

	UvStream::UvStream(): _loop{}, _uvh{} {
        _writeCb = [](){};
	}
	UvStream::~UvStream() {

	}

	int UvStream::readStart() {
        if(!_uvh) {
            return -1;
        }

        static auto alloccb = +[](uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
            ulv("on alloc buf");
            buf->base = ((StreamHandle *) handle->data)->rbufs.alloc(suggested_size);
            buf->len = suggested_size;
        };

        static auto readcb = +[](uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
            auto uvh = ((StreamHandle *) stream->data);
            ulv("on read cb, nread=%ld", nread);
            if (uvh->owner) {
                if (nread > 0) {
                    if (uvh->owner->_readCb) {
                        uvh->owner->_readCb(buf->base, nread);
                    }
                } else if (nread < 0) {
                    auto owner = uvh->owner;
                    uvh->closeUv();
                    owner->_uvh = nullptr;
                    if (owner->_cnnCb) {
                        owner->_cnnCb(-1);
                    }
                }
            }
            uvh->rbufs.recycle(buf->base);
        };
        return uv_read_start((uv_stream_t *) &_uvh->rawh, alloccb, readcb);
	}

	int UvStream::readStop() {
        if (!_uvh) {
            return -1;
        }
		return uv_read_stop((uv_stream_t *)getRawHandle());
	}

	int UvStream::listen(int backlog) {
        if (!_uvh) {
            return -1;
        }
        static auto lcb = +[](uv_stream_t *server, int status) {
            (void) status;
            auto uvh = (StreamHandle *) server->data;
            if (uvh->owner && uvh->owner->_lisCb) {
                uvh->owner->_lisCb(*uvh->owner);
            }
        };

        return uv_listen(&_uvh->rawh.stream, backlog, lcb);
	}

	int UvStream::accept(UvStream* newstrm) {
        if (!_uvh || !newstrm || !newstrm->getRawHandle()) {
            return -1;
        }
		auto newrawh = (uv_stream_t *)newstrm->getRawHandle();
		auto ret = uv_accept(RAWH(), newrawh);
		if (!ret) {
			return ret;
		} else {
			ulw("### accept error, err=%s", UvStrErr(ret));
			newstrm->close();
			return -1;
		}
	}

	int UvStream::write(const char *buf, size_t len) {
        static auto wcb = +[](uv_write_t *wh, int status) {
            assert(wh->nbufs == 1);
            if (status) {
                ule("### write status error, status=%s", UvStrErr(status));
            }
            auto uvh = (StreamHandle *) wh->data;
            uvh->recycleWriteReq((write_req_t *) wh);
            if(uvh->owner && uvh->owner->_writeCb) {
                uvh->owner->_writeCb();
            }
        };
        if (!_uvh || (!buf && len > 0)) {
            ulw("### write fail: tcp idle status");
            return -1;
        }

        auto req = _uvh->allocWriteReq();
        req->buf.base = _uvh->allocWriteBuf(len);
        req->buf.len = len;
        memcpy(req->buf.base, buf, len);
        req->req.data = _uvh;
        auto rc = uv_write((uv_write_t *) req, (uv_stream_t *) &_uvh->rawh, (uv_buf_t *) &req->buf, 1, wcb);
        if (rc) {
            wcb((uv_write_t *) req, rc);
        }
        return rc;
	}

	int UvStream::write(const std::string &msg) {
		return write((const char*)msg.data(), msg.size());
	}

//	int UvStream::tryWrite(const uint8_t *buf, size_t len) {
//		if(_handleHolder->writeReqQue.size() == 0 ) {
//			uv_buf_t uvbuf;
//			uvbuf.base = (char*)buf;
//			uvbuf.len = len;
//			return uv_try_write(RAWH(), &uvbuf, 1);
//		}
//		return UV_EAGAIN ;
//	}


	int UvStream::write2(UvStream* send_handle) {
        static auto wcb = +[](uv_write_t *wh, int status) {
            if (status) {
                ule("### write status error, status=%s", UvStrErr(status));
            }
            auto uvh = (StreamHandle *) wh->data;
            uvh->recycleWriteReq((write_req_t *) wh);
            if(uvh->owner && uvh->owner->_writeCb) {
                uvh->owner->_writeCb();
            }
        };

        if (!_uvh || !send_handle || !send_handle->getRawHandle()) {
            return -1;
        }
        int rc;
        auto req = _uvh->allocWriteReq();
        req->buf.base = _uvh->allocWriteBuf(4);
        req->buf.len = 4;
        memcpy(req->buf.base, "PING", 4);
        req->req.data = _uvh;


		rc = uv_write2((uv_write_t*)req, getRawHandle(), &req->buf, 1, (uv_stream_t*)send_handle->getRawHandle(), wcb);
        if (rc) {
            ule("### uv_write2 error, rc=%s", UvStrErr(rc));
            wcb((uv_write_t *) req, rc);
        }
        return rc;
	}

	void UvStream::setOnCnnLis(CnnCb lis) {
		_cnnCb = lis;
	}


	void UvStream::setOnReadLis(UvStream::ReadCb cb) {
		_readCb = cb;
	}

	void UvStream::setOnWriteLis(std::function<void()> cb) {
		_writeCb = cb;
	}

    void UvStream::setOnListen(UvStream::ListenCb cb) {
        _lisCb = cb;
    }

	int UvStream::isReadable() {
		return uv_is_readable(RAWH());
	}

	int UvStream::isWritable() {
		return uv_is_writable(RAWH());
	}

    void UvStream::close() {
        if (_uvh) {
            Loop::CheckLoop(_loop);//CHECK_UV_LOOP();
            _uvh->owner = nullptr;
            _uvh->closeUv();
            _uvh = nullptr;
        }
    }

    uv_stream_t *UvStream::getRawHandle() {
        if (_uvh) {
            return &_uvh->rawh.stream;
        } else {
            return nullptr;
        }
    }

    int UvStream::initInternal() {
        if (_uvh) {
            assert(0 && "### handle not empty");
            return -1;
        }
        _loop = Loop::GetUvLoop();
        _uvh = new StreamHandle;
        _uvh->owner = this;
        _uvh->rawh.stream.data = _uvh;
        return 0;
    }
}