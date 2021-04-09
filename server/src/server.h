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

#define Panic(msg) Network::ReportError(msg,true)
#define Error(msg) Network::ReportError(msg,false)

#define SendFromServerTo(client, msg)		   this->SendDataTo(client, msg, "SERVER")
#define SendFromServerToAll(except, msg)	   this->SendDataTo(this->clients, except, msg, "SERVER")
#define SendFromClientToAll(except, msg, from) this->SendDataTo(this->clients, except, msg, from)

#define DisconnectAllClients for(ConnectedClient* client : this->clients) this->DisconnectClient(client)

namespace Network
{
	int ListenSocket(SOCKET socket, int backlog);
	int BindSocket(SOCKET socket, sockaddr* addr, int addrSize);
	SOCKET CreateSocket(int af, int type, int protocol);

	bool IsEmptyMessage(const char* buffer);

	void ReportError(std::string msg, bool panic);
	void CreateAddressHints(int af, int port, std::string ip, sockaddr_in* addr);

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
	class Server
	{
		private:
			int	port;
			int	serverAddrSize;

			WSADATA		wsa;
			SOCKET		serverSocket;
			sockaddr_in serverAddr;

			std::string					  ip;
			std::vector<ConnectedClient*> clients;

		public:
			Server(int port, std::string ip);
		private:
			~Server();

		public:
			void Run();
			void Stop();

		private:
			int DisconnectClient(ConnectedClient* client);
			ConnectedClient* GetClientWithSocket(SOCKET socket);
			int AcceptClient(SOCKET serverSocket, ConnectedClient* client);

		private:
			int Init();
			void StartListening();
			void ExchangeDataWith(ConnectedClient* client);

			int ReceiveData(ConnectedClient* client);
			int ProcessData(ConnectedClient* client);
			int SendDataTo(ConnectedClient* client, std::string buffer, std::string from);
			int SendDataTo(ConnectedClient* client, const char* buffer, const char* from);
			int SendDataTo(std::vector<ConnectedClient*> clients, ConnectedClient* exceptTo, const char* buffer, const char* from);
			int SendDataTo(std::vector<ConnectedClient*> clients, ConnectedClient* exceptTo, std::string buffer, std::string from);
	};
}