#include "client.h"
#include "server.cpp"

namespace Network
{
	int Client::Init()
	{
		if (WSAStartup(MAKEWORD(2, 2), &this->wsa))
			return Fail;
		return Ok;
	}

	Client::Client()
	{
		InfoLogc("Initializing a client...");
		if (this->Init())
			Panic("Initializing is failed.");
		InfoLogc("Initialization finished successfully.");
		Seprtr;
	}

	void Client::Connect(int port, std::string ip)
	{
		if (EmptyIP != ip && EmptyPort != port)
			closesocket(this->clientSocket);
		this->ip = ip;
		this->port = port;
		this->clientAddrSize = sizeof(this->clientAddr);
		CreateAddressHints(AF_INET, this->port, this->ip, &this->clientAddr);
		this->clientSocket = CreateSocket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (this->clientSocket == INVALID_SOCKET)
			Panic("Client socket creation failed.");
		InfoLogc("Connecting to server (port:" + std::to_string(port) + ", ip:" + ip + ") ...");
		if (connect(this->clientSocket, (sockaddr*)&this->clientAddr, this->clientAddrSize))
			Panic("Connecting is failed.");
		InfoLogc("Connected successfully.");
		this->ExchangeData();
	}

	void Client::ExchangeData()
	{
		do
		{
			//this->SendDataTo();
			if (this->ReceiveData())
				break;
			this->ProcessData();
		}
		while (this->receivedLen > 0);
		this->Disconnect();
	}
	
	int Client::SendDataTo()
	{
		std::string msg;
		printf("Enter message: ");
		std::getline(std::cin, msg);
		if (send(this->clientSocket, msg.c_str(), msg.size(), 0) == SOCKET_ERROR)
		{
			Panic("Sending data failed.");
			return 1;
		}
		return 0;
	}
	
	int Client::ReceiveData()
	{
		if ((this->receivedLen = recv(this->clientSocket, this->buffer, TcpPackSize, 0)) == SOCKET_ERROR)
		{
			Panic("Error when receiving data.");
			return 1;
		}
		return 0;
	}
	
	int Client::ProcessData()
	{
		if (this->receivedLen > 0 && !IsEmptyMessage(this->buffer))
		{
			std::string received = " ";
			for (size_t i = 0; i < this->receivedLen; i++)
				received += this->buffer[i];
			Printc(received);
		}
		return Ok;
	}
	
	void Client::Disconnect()
	{
		closesocket(this->clientSocket);
		//??
		WSACleanup();
	}
}