#pragma once
#pragma warning(disable : 4996)
#include "server.h"

#define EmptyIP   ""
#define EmptyPort -1

namespace Network
{
	class Client
	{
		private:
			int bufSize;
			int port = EmptyPort;
			int clientAddrSize;

			WSADATA wsa;

			SOCKET clientSocket;
			sockaddr_in clientAddr;

			char buffer[TcpPackSize];
			std::string ip = EmptyIP;

		public:
			Client();
			~Client();

		private:
			int Init();

		public:
			void Connect(int port, std::string ip);
			void Disconnect();

		private:
			int ReceiveData();
			int SendDataTo();
			int ProcessData();
			void ExchangeData();
	};
}