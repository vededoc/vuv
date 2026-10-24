#define UVCLOG_LEVEL_INFO

#include <utility>

#include "UvSpawn.h"

#include <cassert>

#include "uvcpplog.h"
#include "UvBufStore.h"
#include "Loop.h"

namespace vuv {

    namespace {
        struct PipeCtx {
            int type;
            UvBufStore readBufStore;
            UvSpawn *owner;
        };
    }

    UvSpawn::UvSpawn(): _epipe{}, _spipe{}, _loop{}, _proc{} {
        _stdoutCb = [](const char*, size_t){};
        _stderrCb = [](const char *, size_t){};
        _exitCb = [](int){};
    }


    int UvSpawn::processKill(int signum) {
        if(_proc) {
            return uv_process_kill(_proc, signum);
        } else {
            return -1;
        }
    }


    int UvSpawn::run(const std::vector<std::string> &args) {
        int rc;
        _loop = Loop::GetUvLoop();
        _proc = (uv_process_t*)malloc(sizeof(uv_process_t));
        _proc->data = this;

        _spipe = (uv_pipe_t*)malloc(sizeof(uv_pipe_t)) ;
        _epipe = (uv_pipe_t*)malloc(sizeof(uv_pipe_t)) ;
        uv_pipe_init(_loop, _spipe, 0);
        uv_pipe_init(_loop, _epipe, 0);

        auto sctx = new PipeCtx;
        sctx->type = 1;
        sctx->owner = this;
        _spipe->data = sctx;

        auto ectx = new PipeCtx;
        ectx->type = 2;
        ectx->owner = this;
        _epipe->data = ectx;


        auto alloc_cb = +[](uv_handle_t* handle, size_t suggested_size,uv_buf_t* buf) {
            auto ctx = (PipeCtx*)handle->data;
            uld("on alloc cb, size=%ld, type=%d", suggested_size, ctx->type);
//			*buf = uv_buf_init((char*) malloc(suggested_size), suggested_size);
            buf->base = ctx->readBufStore.alloc(suggested_size);
            assert(buf->base);
            buf->len = suggested_size;
        };

        auto s_read_cb = +[](uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
            uld("on read");
            auto ctx = (PipeCtx*)stream->data;
            assert(ctx);
            if (nread < 0) {
                //fprintf(stderr, "Read error!\n");
            } else if (nread > 0) {
//				fwrite(buf->base, 1, nread, stdout);

                if(ctx->owner) {
                    if (ctx->owner->_stdoutCb) {
                        ctx->owner->_stdoutCb(buf->base, nread);
                    }
                }
            }

            if (buf->base) {
                ctx->readBufStore.recycle(buf->base);
            }
        };

        auto e_read_cb = +[](uv_stream_t* stream,
                             ssize_t nread,
                             const uv_buf_t* buf) {
            auto ctx = (PipeCtx*)stream->data;
            uld("on error read, nread=%ld, type=%d", nread, ctx->type);
            assert(ctx);
            if (nread < 0) {
                //fprintf(stderr, "Read error!\n");
            } else if (nread > 0) {
//				fwrite(buf->base, 1, nread, stdout);
                if(ctx->owner) {
                    if (ctx->owner->_stderrCb) {
                        ctx->owner->_stderrCb(buf->base, nread);
                    }
                }
            }

            if(buf->base) {
                ctx->readBufStore.recycle(buf->base);
            }
        };

        uv_process_options_t options={0};
//		uv_process_options_t options;
        std::vector<const char*> cmd_opts;
        for(int i=0;i<args.size();i++) {
            cmd_opts.emplace_back(args[i].data());
        }

        options.stdio_count = 3;
        uv_stdio_container_t child_stdio[3];
        child_stdio[0].flags = UV_IGNORE;

        child_stdio[1].flags = (uv_stdio_flags )(UV_CREATE_PIPE | UV_WRITABLE_PIPE);
        child_stdio[1].data.stream = (uv_stream_t*)_spipe;
//		child_stdio[1].data.fd = 1;

        child_stdio[2].flags = (uv_stdio_flags )(UV_CREATE_PIPE | UV_WRITABLE_PIPE);;
        child_stdio[2].data.stream = (uv_stream_t*)_epipe;
//		child_stdio[2].flags = UV_INHERIT_FD;
//		child_stdio[2].data.fd = 2;
        options.stdio = child_stdio;

        if(!cmd_opts.empty()) {
            cmd_opts.emplace_back(nullptr);
            options.args = (char **) (cmd_opts.data());
        }

        options.exit_cb = +[](uv_process_t *rawh, int64_t exit_status, int term_signal) {
//			uli("on exit");
            uld("on proc exit cb");
            if(rawh->data) {
                auto sp = (UvSpawn*)rawh->data;
//				sp->_proc = nullptr;
//				sp->close();
                if (sp->_exitCb) {
                    sp->_exitCb(exit_status);
                }
            } else {

            }
//			uv_close((uv_handle_t *)rawh, +[](uv_handle_t* chandle){
//				free(chandle);
//			});
        };

        options.file = (char*)args[0].data();
        rc = uv_spawn(_loop, _proc, &options );
        if(!rc) {
            uv_read_start((uv_stream_t *) _spipe, alloc_cb, s_read_cb);
            uv_read_start((uv_stream_t *) _epipe, alloc_cb, e_read_cb);
        } else {
            ulw("### fail spawn, rc=%d", rc);
        }
        return rc;
    }

    void UvSpawn::close() {
        if(_spipe) {
            ((PipeCtx*)_epipe->data)->owner = nullptr;
            uv_close((uv_handle_t*)_spipe, +[](uv_handle_t* handle){
                uld("on s pipe close cb");
                delete (PipeCtx*)handle->data;
                free(handle);
            });
            _spipe = nullptr;
        }
        if(_epipe) {
            ((PipeCtx*)_epipe->data)->owner = nullptr;
            uv_close((uv_handle_t*)_epipe, +[](uv_handle_t* handle){
                uld("on e pipe close cb");
                delete (PipeCtx*)handle->data;
                free(handle);
            });
            _epipe = nullptr;
        }
        if(_proc) {
            _proc->data = nullptr;
            uv_unref((uv_handle_t *)_proc);
            uv_close((uv_handle_t *)_proc, +[](uv_handle_t* chandle){
                free(chandle);
            });
            _proc = nullptr;
        }
    }

    void UvSpawn::setOnConsoleOut(UvSpawn::ConsoleOutCb sout, UvSpawn::ConsoleOutCb eout) {
        _stdoutCb = std::move(sout);
        _stderrCb = std::move(eout);
    }

    void UvSpawn::setOnExit(std::function<void(int)> cb) {
        _exitCb = std::move(cb);
    }
}