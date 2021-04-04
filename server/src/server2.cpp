#include "server2.h"

#define Ok   0
#define Fail 1

#define CLIENT_DISCONNECTED (this->receivedBytes <= 0)

namespace Network
{
	Server2::Server2(int port,std::string ip) : port(port),ip(ip),threadCount(0),receivedBytes(0)
	{
		Infolog("Initializing a server...");
		if (this->Init())
			Panic("Initializing is failed.");
		Infolog("Initialization finished successfully.");
	}

	int Server2::Init()
	{
		if (WSAStartup(MAKEWORD(2, 2), &this->wsa))
			return Fail;

		this->CreateAddressHints(&this->serverAddr);
		this->serverAddrSize = sizeof(this->serverAddr);
		this->serverSocket = this->CreateServerSocket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		return this->BindServerSocket(this->serverSocket,(sockaddr*)&this->serverAddr,this->serverAddrSize);
	}

	void Server2::CreateAddressHints(sockaddr_in* addr)
	{
		addr->sin_family = AF_INET;
		addr->sin_port = htons(this->port);
		inet_pton(AF_INET, this->ip.c_str(), &this->serverAddr.sin_addr);
	}

	SOCKET Server2::CreateServerSocket(int af,int type,int protocol)
	{
		SOCKET sckt = socket(af, type, protocol);
		if (sckt == INVALID_SOCKET)
			Panic("Can't create a server socket...");
		Infolog("Server socket created successfully.");
		return sckt;
	}

	int Server2::BindServerSocket(SOCKET socket, sockaddr* addr, int addrsize)
	{
		int res = bind(socket, addr, addrsize);
		if (res)
			Panic("Can't bind a server socket.");
		Infolog("Binding a server socket finished successfully.");
		return Ok;
	}

	int Server2::ListenServerSocket(SOCKET socket,int backlog)
	{
		int res = listen(socket, backlog);
		if (res == SOCKET_ERROR)
			Panic("Can't set a server socket as listening socket...");
		Infolog("Server socket setted as listening socket successfully.");
		return Ok;
	}

	SOCKET Server2::AcceptToServerSocket(SOCKET serversocket, sockaddr* clientaddr, int* clientaddrsize)
	{
		SOCKET sckt = accept(serversocket, clientaddr, clientaddrsize);
		if (sckt != INVALID_SOCKET)
		{
			ZeroMemory(this->host, NI_MAXHOST);
			ZeroMemory(this->service, NI_MAXSERV);
			if (!getnameinfo((sockaddr*)&this->clientAddr, this->clientAddrSize, this->host, NI_MAXHOST, this->service, NI_MAXSERV, 0))
				Log("Client " + std::string(this->host)+ " as Thread " + std::to_string(this->threadCount+1) + " connected.","CONNECTION");
		}
		return sckt;
	}

	void Server2::StartListening()
	{
		this->ListenServerSocket(this->serverSocket, SOMAXCONN);
		Empty;
		Infolog("Waiting for connections...");
		Empty;
		while(true)
		{
			this->clientAddrSize = sizeof(this->clientAddr);
			this->clientSocket = AcceptToServerSocket(this->serverSocket, (sockaddr*)&this->clientAddr, &this->clientAddrSize);
			this->activeThreads.push_back(std::thread(&Server2::ExchangeData, this, this->clientSocket));
			this->activeThreads[this->activeThreads.size() - 1].detach();
		}
	}

	void Server2::ExchangeData(SOCKET clientsocket)
	{
		int received = 0;
		
		this->threadCount++;
		int currthread = this->threadCount;

		char buffer[TcpPackSize];
		do
		{
			ZeroMemory(buffer, TcpPackSize);
			received = recv(clientsocket, buffer, TcpPackSize, 0);
			if (received > 0)
			{
				Log(std::string(buffer), "Thread " + std::to_string(currthread)); // processing 
				send(clientsocket, buffer, TcpPackSize, 0);
			}
		} 
		while (received > 0);
		Log("Thread "+ std::to_string(currthread) +" is closed. Client disconnected.","DISCONNECTION");
		this->threadCount--;
	}

	void Server2::Run()
	{
		this->StartListening();
		this->Stop();
	}

	void Server2::Stop()
	{
		this->~Server2();
	}

	void Server2::ReportError(std::string msg, int type)
	{
		Errorlog(msg);
		if (type == 1)
		{
			Errorlog("Error above is unexpected. Press any key to quit...");
			std::cin.get();
			this->~Server2();
			exit(0);
		}
	}

	Server2::~Server2()
	{
		this->activeThreads.clear();
		closesocket(this->serverSocket);
		WSACleanup();
	}
}