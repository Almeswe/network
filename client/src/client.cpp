#include "client.h"
#include "server.cpp"

namespace Network
{
	Client::Client()
	{
		InfoLogc("Initializing a client...");
		if (this->Init())
			Panic("Initializing is failed.");
		InfoLogc("Initialization finished successfully.");
		Seprtr;
	}
	Client::~Client()
	{
		this->Disconnect();
	}
	
	int Client::Init()
	{
		if (WSAStartup(MAKEWORD(2, 2), &this->wsa))
			return Fail;
		return Ok;
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
	void Client::Disconnect()
	{
		closesocket(this->clientSocket);
		WSACleanup();
		Panic("You've been disconnected from server...");
	}

	int Client::ReceiveData()
	{
		ZeroMemory(this->buffer, TcpPackSize);
		this->bufSize = recv(this->clientSocket, this->buffer, TcpPackSize, 0);
		return this->bufSize;
	}
	int Client::SendDataTo()
	{
		std::string msg;
		while (true)
		{
			std::getline(std::cin, msg);
			if (send(this->clientSocket, msg.c_str(), msg.size(), 0) == SOCKET_ERROR)
				Panic("Sending data failed.");
		}
		return Ok;
	}
	int Client::ProcessData()
	{
		if (this->bufSize > 0 && !IsEmptyMessage(this->buffer))
		{
			std::string received = " ";
			for (size_t i = 0; i < this->bufSize; i++)
				received += this->buffer[i];
			Printc(received);
		}
		return Ok;
	}
	void Client::ExchangeData()
	{
		std::thread(&Client::SendDataTo, this).detach();
		do
		{
			if (this->ReceiveData() < 0)
				break;
			this->ProcessData();
		} 
		while (this->bufSize > 0);
		this->Disconnect();
	}
}