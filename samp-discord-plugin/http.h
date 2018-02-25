#pragma once

#include <Windows.h>
#include <Wininet.h>
#include <functional>
#pragma comment(lib, "Wininet.lib")

namespace HTTP
{
	typedef std::function<bool(const char*, size_t)> WebCallback;

	class WebRequest {
	public:
		WebRequest(WebCallback cb, const char* agent, const char* domain, const INTERNET_PORT port = INTERNET_DEFAULT_HTTP_PORT, DWORD timeout = 1000);
		~WebRequest();

		inline bool post(const char* location, const char* data, const size_t bufferSize = 1024)
		{
			return post(location, reinterpret_cast<const void*>(data), strlen(data), bufferSize);
		}
		bool post(const char* location, const void* data, const size_t len, const size_t bufferSize = 1024);
		bool get(const char* location, const size_t bufferSize = 1024);

	private:
		bool readRequest(HINTERNET request, const size_t bufferSize);
		HINTERNET sendRequest(const char* location, const char* method, const void* data = nullptr, const size_t len = 0);

		HINTERNET internet;
		HINTERNET session;

		WebCallback callback;
	};
}
