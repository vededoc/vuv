//
// Created by khkim on 2017-01-22.
//

#include "HandleSender.h"
#include "uvcpplog.h"

namespace vuv {
	int HandleSender::connect(const char *name) {
		_pipe.init(1);
		_status = 0;
        _pipe.setOnCnnLis([this](int status) {
            if(!status) {
                uli("handler sender connected");
                _status = status;
            }
        });
		_pipe.connect(name);
		return 0;
	}

	int HandleSender::sendHandle(UvStream *strm) {
		return _pipe.write2(strm);
	}

	void HandleSender::close() {
		_pipe.close();
	}

	int HandleSender::connect(const std::string &name) {
		return connect(name.c_str());
	}
}