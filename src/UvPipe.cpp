#include "UvPipe.h"
#include "uvcpplog.h"

#define RAWH() ((uv_pipe_t*)getRawHandle())


namespace vuv {

	UvPipe::UvPipe() {

	}

	UvPipe::~UvPipe() {

	}

	int UvPipe::init(int ipc) {
        int rc;
        rc = initInternal();
        if (rc) {
            return rc;
        }
        return uv_pipe_init(_loop, &_uvh->rawh.pipe, ipc);
	}

	int UvPipe::bind(const char *name) {
		return uv_pipe_bind(RAWH(), name);
	}

	void UvPipe::connect(const char *name) {
        _uvh->cnn.data = _uvh;
        static auto connect_cb = +[](uv_connect_t* req, int status) {
            UvPipe *pp = static_cast<UvPipe *>(((StreamHandle *) req->data)->owner );
            if (!status) {
                uld("connected");
            } else {
                uld("### disconnected, err_n=%d, err=%s, ename=%s", status, uv_strerror(status), uv_err_name(status));
            }

            if (pp) {
                if (status) {
                    pp->_uvh->closeUv();
                    pp->_uvh = nullptr;
                } else {
                    pp->readStart();
                }
                pp->_cnnCb(status);
            }
        };
        uv_pipe_connect(&_uvh->cnn, &_uvh->rawh.pipe, name, connect_cb);
	}

	int UvPipe::getSockName(char *buffer, size_t *size) {
		return uv_pipe_getsockname(RAWH(), buffer, size);
	}

	int UvPipe::getPeerName(char *buffer, size_t *size) {
		return uv_pipe_getpeername(RAWH(), buffer, size);
	}

	void UvPipe::pendingInstances(int count) {
		uv_pipe_pending_instances(RAWH(), count);
	}

	int UvPipe::pendingCount() {
		return uv_pipe_pending_count(RAWH());
	}

	uv_handle_type UvPipe::pendingType() {
		return uv_pipe_pending_type(RAWH());
	}

	int UvPipe::open(uv_file file) {
		return uv_pipe_open(RAWH(), file);
	}
}