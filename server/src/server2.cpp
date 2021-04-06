#include "server2.h"

namespace Network
{
	Server2::Server2(int port,std::string ip) : port(port),ip(ip)
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

	int Server2::AcceptClient(SOCKET serverSocket, ConnectedClient* client)
	{
		client->addrSize = sizeof(client->addr);
		client->socket = accept(serverSocket, (sockaddr*)&client->addr, &client->addrSize);
		if (client->socket != INVALID_SOCKET)
		{
			ZeroMemory(client->host, NI_MAXHOST);
			ZeroMemory(client->service, NI_MAXSERV);
			if (!getnameinfo((sockaddr*)&client->addr, client->addrSize, client->host, NI_MAXHOST, client->service, NI_MAXSERV, 0))
				Log("Client " + std::string(client->host) + " as " + std::to_string(client->socket) + " connected.", "CONNECTION");
			SendFromServerTo(client, "You've entered the chatroom as: " + std::to_string(client->socket) + "\r\n");
			if (this->clients.size() > 0)
			{
				std::string parts = "Clients in this chatroom: \r\n";
				for (ConnectedClient* connectedClient : this->clients)
					parts += "   |" + std::to_string(connectedClient->socket) + "\r\n";
				SendFromServerTo(client, parts);
			}
			else
				SendFromServerTo(client, "This chatroom is empty right now.");
			SendFromServerToAll(client, "Client " + std::string(client->host) + " as " + std::to_string(client->socket) + " connected.");
			return Ok;
		}
		return Fail;
	}

	void Server2::StartListening()
	{
		this->ListenServerSocket(this->serverSocket, SOMAXCONN);
		Empty;
		Infolog("Waiting for connections...");
		Empty;
		while(true)
		{
			ConnectedClient* client = new ConnectedClient();
			if (this->AcceptClient(this->serverSocket, client))
				continue;
			this->clients.push_back(client);
			this->activeThreads.push_back(std::thread(&Server2::ExchangeDataWith, this, client));
			this->activeThreads[this->activeThreads.size()-1].detach();
		}
	}

	void Server2::StartListening2()
	{
		this->ListenServerSocket(this->serverSocket, SOMAXCONN);
		Empty;
		Infolog("Waiting for connections...");
		Empty;

		fd_set master;
		FD_ZERO(&master);
		FD_SET(this->serverSocket,&master);

		while (true)
		{
			fd_set copy = master;
			int socketCount = select(0, &copy, nullptr, nullptr, nullptr);
			
			for (size_t i = 0; i < socketCount; i++)
			{
				if (this->serverSocket == copy.fd_array[i])
				{
					ConnectedClient* client = new ConnectedClient();
					if (this->AcceptClient(this->serverSocket, client))
						break;
					this->clients.push_back(client);
					FD_SET(client->socket, &master);
				}
				else
				{
					ConnectedClient* listeningClient = this->GetClientWithSocket(copy.fd_array[i]);
					if (listeningClient == nullptr || this->ReceiveData(listeningClient) < 0)
						break;
					this->ProcessData(listeningClient);
					for (SOCKET nonListeningSocket : master.fd_array)
					{
						ConnectedClient* nonListeningClient = this->GetClientWithSocket(nonListeningSocket);
						if (nonListeningClient != nullptr)
							if (nonListeningClient->socket != this->serverSocket && nonListeningClient->socket != copy.fd_array[i])
								this->SendDataTo(nonListeningClient, listeningClient->buffer, std::to_string(nonListeningClient->socket).c_str());
					}
				}
			}
		}
	}
	
	ConnectedClient* Server2::GetClientWithSocket(SOCKET socket)
	{
		for (ConnectedClient* client : this->clients)
			if (client->socket == socket)
				return client;
		return nullptr;
	}

	void Server2::ExchangeDataWith(ConnectedClient* client)
	{
		do
		{
			if (this->ReceiveData(client) < 0)
				break;
			this->ProcessData(client);
			SendFromClientToAll(client, client->buffer, std::to_string(client->socket).c_str());
		}
		while (client->bufSize > 0);
		if (this->DisconnectClient(client))
			Errorlog("Client " + std::to_string(client->socket) + " already disconnected.");
		else
		{
			Log("Client " + std::to_string(client->socket) + " disconnected.", "DISCONNECTION");
			SendFromServerToAll(client, "Client " + std::to_string(client->socket) + " disconnected.");
		}
	}

	int Server2::DisconnectClient(ConnectedClient* client)
	{
		for (size_t i = 0; i < this->clients.size(); i++)
		{
			if (client->socket == clients[i]->socket)
			{
				closesocket(client->socket);
				this->clients.erase(this->clients.begin() + i);
				return Ok;
			}
		}
		return Fail;
	}

	int Server2::ReceiveData(ConnectedClient* client)
	{
		ZeroMemory(client->buffer, TcpPackSize);
		client->bufSize = recv(client->socket, client->buffer, TcpPackSize, 0);
		return client->bufSize;
	}

	int Server2::ProcessData(ConnectedClient* client)
	{
		if (client->bufSize > 0)
			Log(std::string(client->buffer), std::to_string(client->socket));
		return Ok;
	}

	int Server2::SendDataTo(ConnectedClient* client, std::string buffer, std::string from)
	{
		return this->SendDataTo(client, buffer.c_str(), std::string(from).c_str()) ? Fail : Ok;
	}

	int Server2::SendDataTo(ConnectedClient* client, const char* buffer, const char* from)
	{
		if (this->IsEmptyMessage(buffer))
			return Ok;
		std::string message = std::string(from) + ": " + std::string(buffer) + "\r\n";
		if (send(client->socket, message.c_str(), message.size(), 0) == SOCKET_ERROR)
		{
			Errorlog("Can't send message to " + std::to_string(client->socket));
			return Fail;
		}
		else
			return Ok;
	}

	int Server2::SendDataTo(std::vector<ConnectedClient*> clients, ConnectedClient* exceptTo, const char* buffer, const char* from)
	{
		for (ConnectedClient* client : clients)
			if (client != exceptTo)
				this->SendDataTo(client, buffer, from);
		return Ok;
	}

	int Server2::SendDataTo(std::vector<ConnectedClient*> clients, ConnectedClient* exceptTo, std::string buffer, std::string from)
	{
		return this->SendDataTo(clients, exceptTo, buffer.c_str(), from.c_str()) ? Fail : Ok;
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

	bool IsEmptyMessage(const char* buffer)
	{
		for (size_t i = 0; i < std::strlen(buffer); i++)
			if (!std::iscntrl(buffer[i]) && buffer[i] != ' ')
				return false;
		return true;
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
		DisconnectAllClients;
		this->clients.clear();
		closesocket(this->serverSocket);
		WSACleanup();
	}
}