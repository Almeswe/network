#include "server.h"

#define Log(msg) this->InfoLog(msg)
#define SuccLog Log("Success!")
#define Error(msg,fn) this->ErrorLog(msg,fn)

int main()
{
	Network::Server* s = new Network::Server();
}

namespace Network
{
	Server::Server()
	{
		Log("Starting server...");
		this->addresHints.ai_family = AF_INET;
		this->addresHints.ai_flags = AI_PASSIVE;
		this->addresHints.ai_socktype = SOCK_STREAM;
		this->addresHints.ai_protocol = IPPROTO_TCP;

		if (getaddrinfo(NULL, DEFAULT_ANSI_PORT, &this->addresHints, &this->addressResult))
		{
			Error("GetAddressInfo failed", "Server::Start()");
			WSACleanup();
		}
		//Initialize Winsock 
		else if (WSAStartup(MAKEWORD(2, 2), &this->wsadata))
			Error("WSAStartup failed", "Server::Start()");
		else
		{
			SuccLog;
			this->Start();
		}
	}
	void Server::Start()
	{
		//Creating socket for data exchanging
		this->Bind();
		for (;;)
		{
			//this->ReceiveData();
			//this->ProcessData();
			//this->SendDataTo();
		}
	}

	void Server::Bind()
	{
		Log("Creating server socket...");
		this->serverSocket = socket(
			AF_INET,
			SOCK_STREAM,
			IPPROTO_TCP
		);
		if (this->serverSocket == INVALID_SOCKET)
			Error("Socket creation error","Server::Bind");
		SuccLog;
		Log("Binding socket...");
		//if (bind(this->serverSocket,,))

		SuccLog;
	}
}