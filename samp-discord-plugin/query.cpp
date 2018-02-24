#include "query.h"

namespace SAMP
{
	typedef int socklen_t;

	bool Query::info(Information& information)
	{
		char packet[QUERY_BASIC_PACKET_LENGTH];
		if (send('i', packet) < 0) return false;
		return recvInfo(information, packet);
	}

	int Query::send(const char opcode, char out[QUERY_BASIC_PACKET_LENGTH])
	{
		if (sock < 1 || server.sin_port < 1) return -1;
		if (!assemble(opcode, out)) {
			return -1;
		}

		return sendto(sock, out, QUERY_BASIC_PACKET_LENGTH, 0, reinterpret_cast<sockaddr*>(&server), sizeof(sockaddr_in));
	}

	bool Query::recvInfo(Information& info, char in[QUERY_BASIC_PACKET_LENGTH])
	{
		if (sock < 1) return false;

		char cbuffer[QUERY_INCOMING_BUFFER_SIZE] = { 0 };

		sockaddr_in from;
		socklen_t fromlen = sizeof(from);
		int recvbytes = 0;

		recvbytes = recvfrom(sock, cbuffer, QUERY_INCOMING_BUFFER_SIZE, 0, reinterpret_cast<sockaddr*>(&from), &fromlen);
		if (recvbytes > QUERY_BASIC_PACKET_LENGTH) {
			for (size_t i = 0; i < QUERY_BASIC_PACKET_LENGTH; ++i) {
				if (cbuffer[i] != in[i]) {
					return false;
				}
			}

			char* current = cbuffer + QUERY_BASIC_PACKET_LENGTH;
			char* end = cbuffer + recvbytes;

			if (current + sizeof(BasicInformation) > end) {
				return false;
			}

			info.basic = *reinterpret_cast<BasicInformation*>(current);
			current += sizeof(BasicInformation);

			if (current + sizeof(UINT32) > end) {
				return false;
			}

			UINT32 hostnameLen = *reinterpret_cast<UINT32*>(current);
			current += sizeof(UINT32);

			if (current + hostnameLen > end) {
				return false;
			}

			info.hostname = std::string(current, hostnameLen);
			current += hostnameLen;

			if (current + sizeof(UINT32) > end) {
				return false;
			}

			UINT32 gamemodeLen = *reinterpret_cast<UINT32*>(current);
			current += sizeof(UINT32);

			if (current + gamemodeLen > end) {
				return false;
			}

			info.gamemode = std::string(current, gamemodeLen);
			current += gamemodeLen;

			if (current + sizeof(UINT32) > end) {
				return false;
			}

			UINT32 languageLen = *reinterpret_cast<UINT32*>(current);
			current += sizeof(UINT32);

			if (current + languageLen > end) {
				return false;
			}

			info.language = std::string(current, languageLen);
			current += languageLen;

			return true;
		}

		return false;
	}

	bool Query::assemble(const char opcode, char out[QUERY_BASIC_PACKET_LENGTH])
	{
		out[0] = 'S';
		out[1] = 'A';
		out[2] = 'M';
		out[3] = 'P';

		out[4] = server.sin_addr.s_net;
		out[5] = server.sin_addr.s_host;
		out[6] = server.sin_addr.s_lh;
		out[7] = server.sin_addr.s_impno;

		out[8] = LOBYTE(server.sin_port);
		out[9] = HIBYTE(server.sin_port);

		out[10] = opcode;

		return true;
	}

	Query::Query(std::string ip, const unsigned short port, long timeout) : server{ 0 }
	{
		sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (sock < 1) {
			sock = 0;
			return;
		}

		struct timeval timeoutVal = { 0 };
		timeoutVal.tv_usec = timeout;

		if (
			setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeoutVal), sizeof(timeoutVal)) ||
			setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&timeoutVal), sizeof(timeoutVal))
			) {
			sock = 0;
			return;
		}

		server.sin_family = AF_INET;
		server.sin_port = htons(port);
		server.sin_addr.s_addr = inet_addr(ip.c_str());
	}

	Query::~Query()
	{
		if (sock) {
			closesocket(sock);
		}
	}
}
