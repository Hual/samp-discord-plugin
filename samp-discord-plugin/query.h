#pragma once

#include <string>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

#define QUERY_DEFAULT_TIMEOUT 1000

namespace SAMP
{
	class Query
	{
	public:
#pragma pack(push, 1)
		struct BasicInformation {
			UINT8 password;
			UINT16 players;
			UINT16 maxPlayers;
		};
#pragma pack(pop)

		struct Information {
			BasicInformation basic;
			std::string hostname;
			std::string gamemode;
			std::string language;
		};

		bool info(Information& information);

		Query(std::string ip, const unsigned short port, long timeout = QUERY_DEFAULT_TIMEOUT);
		~Query();

	private:
		enum {
			QUERY_BASIC_PACKET_LENGTH = 11,
			QUERY_INCOMING_BUFFER_SIZE = 512
		};

		int sock;
		struct sockaddr_in server;

		int send(const char opcode, char out[QUERY_BASIC_PACKET_LENGTH]);
		bool recvInfo(Information& info, char in[QUERY_BASIC_PACKET_LENGTH]);
		bool assemble(const char opcode, char out[QUERY_BASIC_PACKET_LENGTH]);
	};
}
