#include "client.h"

#define Log(msg) this->InfoLog(msg)
#define SuccessLog Log("Success!")
#define Error(msg,fn) this->ErrorLog(msg,fn)

namespace Network
{
	Client::Client(string port,string hostname)
	{
		Log("Initializing...");
		this->port = port;
		this->addresHints.ai_family = AF_INET;
		this->addresHints.ai_flags = AI_PASSIVE;
		this->addresHints.ai_socktype = SOCK_STREAM;
		this->addresHints.ai_protocol = IPPROTO_TCP;

		//Initialize Winsock 
		if (WSAStartup(MAKEWORD(2, 2), &this->wsadata))
			Error("WSAStartup failed", "Client::Client()");
		else if (getaddrinfo(hostname.c_str(), DEFAULT_ANSI_PORT, &this->addresHints, &this->addressResult))
			Error("GetAddressInfo failed", "Client::Client()");
		else if ((this->connectSocket = socket(this->addressResult->ai_family, this->addressResult->ai_socktype, this->addressResult->ai_protocol)) == SOCKET_ERROR)
			Error("Connect socket creation failed", "Client::Client()");
		SuccessLog;
	}

	void Client::Connect()
	{
		Log("Connecting to server...");
		if(connect(this->connectSocket, this->addressResult->ai_addr, this->addressResult->ai_addrlen) == SOCKET_ERROR)
		{
			closesocket(this->connectSocket);
			Error("Connecting failed","Client::Connect()");
		}
		else
		{
			SuccessLog;
			freeaddrinfo(this->addressResult);
			for (;;)
			{
				if (this->SendDataTo())
					break;
				if (this->ReceiveData())
					break;
				this->ProcessData();
			}
			this->Disconnect();
		}
	}

	int Client::SendDataTo()
	{
		string msg;
		printf("Enter message: ");
		getline(cin, msg);
		if (send(this->connectSocket, msg.c_str(), msg.size(), 0) == SOCKET_ERROR)
		{
			Error("Sending data failed", "Client::SendDataTo()");
			return 1;
		}
		return 0;
	}

	int Client::ReceiveData()
	{
		if ((this->receivedLen = recv(this->connectSocket, this->buffer, TCP_DATA_PACK, 0)) == SOCKET_ERROR)
		{
			Error("Error when receiving data", "Client::ReceiveData");
			return 1;
		}
		return 0;
	}
	
	void Client::ProcessData()
	{
		string receivedstr = "";
		for (size_t i = 0; i < this->receivedLen; i++)
		{
			receivedstr += this->buffer[i];
			this->buffer[i] += 1;
		}
		Log("RECEIVED DATA FROM SERVER: " + receivedstr);
	}

	void Client::Disconnect()
	{
		closesocket(this->connectSocket);
		WSACleanup();
	}
}