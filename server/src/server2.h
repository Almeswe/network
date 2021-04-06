#pragma once
#include "conslog.h"

#include <vector>
#include <thread>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

#define Ok   0
#define Fail 1
#define TcpPackSize 512

#define Empty	      Console::Informator::Empty();
#define Log(msg,inf)  Console::Informator::Log(msg,inf)
#define Infolog(msg)  Console::Informator::InfoLog(msg)
#define Errorlog(msg) Console::Informator::ErrorLog(msg)

#define Panic(msg) this->ReportError(msg,1)
#define Error(msg) this->ReportError(msg,0)
#define Success    Infolog("Success.")

#define SendFromServerTo(client, msg)	 this->SendDataTo(client, msg, "SERVER")
#define SendFromServerToAll(except, msg) this->SendDataTo(this->clients, except, msg, "SERVER")
#define SendFromClientToAll(except, msg, from) this->SendDataTo(this->clients, except, msg, from)

#define DisconnectAllClients for(ConnectedClient* client : this->clients) this->DisconnectClient(client)

namespace Network
{
	class ConnectedClient
	{
		public:
			int	bufSize;
			int	addrSize;

			char host[NI_MAXHOST];
			char service[NI_MAXSERV];
			char buffer[TcpPackSize];

			SOCKET		socket;
			sockaddr_in addr;
	};

	class Server2
	{
		private:
			int	port;
			int	serverAddrSize;

			WSADATA		wsa;
			SOCKET		serverSocket;
			sockaddr_in serverAddr;

			std::string					  ip;
			std::vector<std::thread>	  activeThreads;
			std::vector<ConnectedClient*> clients;

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

			int AcceptClient(SOCKET serverSocket, ConnectedClient* client);

		private:
			int Init();
			void StartListening();
			void StartListening2();

			ConnectedClient* GetClientWithSocket(SOCKET socket);
			int DisconnectClient(ConnectedClient* client);

		private:
			int ReceiveData(ConnectedClient* client);
			int ProcessData(ConnectedClient* client);
			void ExchangeDataWith(ConnectedClient* client);
			int SendDataTo(ConnectedClient* client, std::string buffer, std::string from);
			int SendDataTo(ConnectedClient* client, const char* buffer, const char* from);
			int SendDataTo(std::vector<ConnectedClient*> clients, ConnectedClient* exceptTo, const char* buffer, const char* from);
			int SendDataTo(std::vector<ConnectedClient*> clients, ConnectedClient* exceptTo, std::string buffer, std::string from);

		private:
			void ReportError(std::string msg, int type);
	};
}