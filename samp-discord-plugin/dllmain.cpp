#include <process.h>
#include "client.h"
#include "discord.h"
#include "query.h"
#include "http.h"

static void process(void*)
{
	SAMP::ServerData data;
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);
	Discord::initialize();
	if (SAMP::readServerData(GetCommandLine(), data)) {
		std::string logo = "logo";
		{
			std::stringstream httpResponseStream;
			if (
				HTTP::WebRequest(
					[&httpResponseStream](auto data, auto len)
					{
						httpResponseStream.write(data, len);
						return true;
					}, "Mozilla/5.0", "raw.githubusercontent.com", INTERNET_DEFAULT_HTTPS_PORT)
					.get("Hual/samp-discord-plugin/custom-logos/custom-logos.txt")
			   ) {
				logo = data.logoFromStream(httpResponseStream, logo);
			}
		}

		auto start = std::time(0);
		if (data.connect == SAMP::SAMP_CONNECT_SERVER) {
			SAMP::Query query(data.address, std::stoi(data.port));
			while (true) {
				SAMP::Query::Information information;
				if (query.info(information)) {
					auto fullAddress = data.address + ':' + data.port;
					auto players = std::to_string(information.basic.players) + "/" + std::to_string(information.basic.maxPlayers) + " players online";
					auto info = "Playing " + information.gamemode + " as " + data.username + " in " + information.language;
					auto image = logo;
					if (image == "logo") {
						if (information.basic.password) {
							image = "lock";
						}
						else if (information.basic.players < 10) {
							image = "tumbleweed";
						}
					}
					Discord::update(start, fullAddress, information.hostname, image, info, players);
					Sleep(15000-QUERY_DEFAULT_TIMEOUT*2);
				}
			}
		}
		else if (data.connect == SAMP::SAMP_CONNECT_DEBUG) {
			while (true) {
				Discord::update(start, "localhost", "Debug server", "tumbleweed", "Playing debug mode in English", "Most likely 1 player online as it's debug mode");
				Sleep(15000);
			}
		}
	}
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID reserved)
{
    switch (reason)
    {
		case DLL_PROCESS_ATTACH: {
			DisableThreadLibraryCalls(module);
			_beginthread(&process, 0, nullptr);
			break;
		}
		case DLL_PROCESS_DETACH: {
			WSACleanup();
			break;
		}
    }
    return TRUE;
}
