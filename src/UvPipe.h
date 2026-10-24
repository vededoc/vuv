#pragma once

#include "UvStream.h"

namespace vuv {
	class UvPipe: public UvStream {
	public:

		virtual ~UvPipe();

		UvPipe();

		int init(int ipc);

		int open(uv_file file);

		int bind(const char* name);

		void connect(const char* name);

		int getSockName(char* buffer, size_t* size);

		int getPeerName(char* buffer, size_t* size);

		void pendingInstances(int count);

		int pendingCount();

		uv_handle_type pendingType();
	};
}
