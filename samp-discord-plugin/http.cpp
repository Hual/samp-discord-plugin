#include "http.h"
#include <memory>

namespace HTTP
{
	WebRequest::WebRequest(WebCallback cb, const char* agent, const char* domain, const INTERNET_PORT port, DWORD timeout) : callback(cb)
	{
		internet = InternetOpen(agent, INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);
		InternetSetOption(internet, INTERNET_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(timeout));
		InternetSetOption(internet, INTERNET_OPTION_SEND_TIMEOUT, &timeout, sizeof(timeout));
		InternetSetOption(internet, INTERNET_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof(timeout));
		session = InternetConnect(internet, domain, port, nullptr, nullptr, INTERNET_SERVICE_HTTP, 0, 0);
	}

	WebRequest::~WebRequest()
	{
		InternetCloseHandle(session);
		InternetCloseHandle(internet);
	}

	bool WebRequest::readRequest(HINTERNET request, const size_t bufferSize)
	{
		bool result = true;
		bool irfResult;
		auto buffer = std::make_unique<char[]>(bufferSize + 1);
		DWORD bytesRead = 0;

		while ((irfResult = InternetReadFile(request, buffer.get(), bufferSize, &bytesRead)) && bytesRead) {
			buffer[bytesRead] = 0;
			if (!(result = callback(buffer.get(), bytesRead))) {
				break;
			}
		}

		return result & irfResult;
	}

	HINTERNET WebRequest::sendRequest(const char* location, const char* method, const void* data, const size_t len)
	{
		HINTERNET request = HttpOpenRequest(session, method, location, nullptr, nullptr, nullptr, INTERNET_FLAG_NO_COOKIES | INTERNET_FLAG_NO_UI | INTERNET_FLAG_RELOAD | INTERNET_FLAG_SECURE, 0);

		if (!request) {
			return nullptr;
		}

		if (HttpSendRequest(request, "Content-Type: application/x-www-form-urlencoded", 47, const_cast<void*>(data), len)) {
			return request;
		}

		InternetCloseHandle(request);
		return nullptr;
	}

	bool WebRequest::post(const char* location, const void* data, const size_t len, const size_t bufferSize)
	{
		HINTERNET request = sendRequest(location, "POST", data, len);
		if (!request) {
			return false;
		}

		bool result = readRequest(request, bufferSize);
		InternetCloseHandle(request);
		return result;
	}

	bool WebRequest::get(const char* location, const size_t bufferSize) {
		HINTERNET request = sendRequest(location, "GET");
		if (!request) {
			return false;
		}

		bool result = readRequest(request, bufferSize);
		InternetCloseHandle(request);
		return result;
	}
}
