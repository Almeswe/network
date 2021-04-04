#pragma once
#include "conslog.h"

#include <vector>
#include <thread>
#include <winsock2.h>
#include <ws2tcpip.h>

#define TcpPackSize 512

#define Empty	      Console::Informator::Empty();
#define Log(msg,inf)  Console::Informator::Log(msg,inf)
#define Infolog(msg)  Console::Informator::InfoLog(msg)
#define Errorlog(msg) Console::Informator::ErrorLog(msg)

#define Panic(msg) this->ReportError(msg,1)
#define Error(msg) this->ReportError(msg,0)
#define Success    Infolog("Success.")

namespace Network
{

	class Server2
	{
		private:
			int	port;
			int	threadCount;
			int	clientCount;
			int	receivedBytes;
			int	serverAddrSize;
			int	clientAddrSize;

			char host[NI_MAXHOST];
			char service[NI_MAXSERV];
			char buffer[TcpPackSize];

			SOCKET		serverSocket;
			SOCKET		clientSocket;

			WSADATA		wsa;
			sockaddr_in serverAddr;
			sockaddr_in clientAddr;

			std::string ip;
			std::vector<std::thread> activeThreads;
			std::vector<ConnectedClient> clients;

		public:
			Server2(int port, std::string ip);
		private:
			~Server2();

		public:
			void Run();
			void Stop();

		private:
			void CreateAddressHints(sockaddr_in* addr);

			int ListenServerSocket(SOCKET socket, int backlog);
			int BindServerSocket(SOCKET socket, sockaddr* addr, int addrsize);

			SOCKET CreateServerSocket(int af, int type, int protocol);
			SOCKET AcceptToServerSocket(SOCKET serversocket, sockaddr* clientaddr, int* clientaddrsize);

		private:
			int Init();
			void StartListening();

		private:
			void ExchangeData(SOCKET clientsocket);

			int SendData(SOCKET clientsocket, char* buffer);
			int ProcessData(char* buffer, int bufsize);
			int ReceiveData(SOCKET clientsocket, char* buffer, int bufsize);

		private:
			void ReportError(std::string msg, int type);
	};
}