#include "server.h"

#define Log(msg) this->InfoLog(msg)
#define SuccessLog Log("Success!")
#define Error(msg,fn) this->ErrorLog(msg,fn)

namespace Network
{
	Server::Server()
	{
		Log("Initializing...");
		this->addresHints.ai_family = AF_INET;
		this->addresHints.ai_flags = AI_PASSIVE;
		this->addresHints.ai_socktype = SOCK_STREAM;
		this->addresHints.ai_protocol = IPPROTO_TCP;

		//Initialize Winsock 
		if (WSAStartup(MAKEWORD(2, 2), &this->wsadata))
			Error("WSAStartup failed", "Server::Start()");
		else if (getaddrinfo(NULL, DEFAULT_ANSI_PORT, &this->addresHints, &this->addressResult))
			Error("GetAddressInfo failed", "Server::Start()");
		SuccessLog;
	}

	void Server::Start()
	{
		Log("Starting server...");
		this->Bind();
		this->Listen();
		for (;;)
		{
			if (this->ReceiveData())
				break;
			this->ProcessData();
			if(this->SendDataTo())
				break;
		}
		this->Close();
	}

	void Server::Bind()
	{
		Log("Creating server socket...");
		this->serverSocket = socket(
			this->addressResult->ai_family,
			this->addressResult->ai_socktype,
			this->addressResult->ai_protocol
		);
		if (this->serverSocket == INVALID_SOCKET)
		{
			freeaddrinfo(this->addressResult);
			Error("Socket creation error", "Server::Bind");
			return;
		}
		SuccessLog;
		Log("Binding socket...");
		if (bind(this->serverSocket, this->addressResult->ai_addr, this->addressResult->ai_addrlen))
		{
			freeaddrinfo(this->addressResult);
			closesocket(this->serverSocket);
			Error("Binding process failed", "Server::Bind()");
			return;
		}
		freeaddrinfo(this->addressResult);
		SuccessLog;
	}
	
	void Server::Listen()
	{
		Log("Start listening...");
		if (listen(this->serverSocket, SOMAXCONN) == SOCKET_ERROR)
		{
			closesocket(this->serverSocket);
			Error("Listening process failed", "Server::Listen()");
			return;
		}

		this->clientSocket = accept(this->serverSocket, NULL, NULL);
		if (this->clientSocket == INVALID_SOCKET) {
			closesocket(this->serverSocket);
			Error("Accept failed", "Server::Listen()");
			return;
		}
		else
		{
			Log("Socket connected");
		}
		closesocket(this->serverSocket);
	}

	int Server::ReceiveData()
	{
		this->receiveLen = recv(this->clientSocket, this->buffer, TCP_DATA_PACK, 0);
		if (this->receiveLen == SOCKET_ERROR)
		{
			Error("Error when receiving data", "Server::ReceiveData");
			return 1;
		}
		return 0;
	}

	void Server::ProcessData()
	{
		string receivedstr = "";
		for (size_t i = 0; i < this->receiveLen; i++)
		{
			receivedstr += this->buffer[i];
			this->buffer[i] += 1;
		}
		for (size_t i = this->receiveLen; i < TCP_DATA_PACK; i++)
			this->buffer[i] = '\0';
		Log("RECEIVED DATA FROM CLIENT: " + receivedstr);
	}

	int Server::SendDataTo()
	{
		if (send(this->clientSocket, this->buffer, TCP_DATA_PACK, 0) == SOCKET_ERROR)
		{
			Error("Sending data failed","Server::SendDataTo()");
			return 1;
		}
		return 0;
	}

	void Server::Close() 
	{
		if (shutdown(this->clientSocket, SD_SEND) == SOCKET_ERROR) 
		{
			closesocket(this->clientSocket);
			Error("Shutdown failed", "Server::Close()");
		}
		else
		{
			closesocket(this->clientSocket);
			WSACleanup();
		}
	}
}