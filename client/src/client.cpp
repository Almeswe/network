#include "client.h"


namespace Network
{
	{
		Log("Initializing...");
		this->port = port;
		this->addresHints.ai_family = AF_INET;
		this->addresHints.ai_flags = AI_PASSIVE;
		this->addresHints.ai_socktype = SOCK_STREAM;
		this->addresHints.ai_protocol = IPPROTO_TCP;

	}

	{
		{
		}
	}
	
	int Client::SendDataTo()
	{
		printf("Enter message: ");
		{
			return 1;
		}
		return 0;
	}
	int Client::ReceiveData()
	{
		{
			return 1;
		}
		return 0;
	}
	
	{
		{
		}
	}
	void Client::Disconnect()
	{
		WSACleanup();
	}
}