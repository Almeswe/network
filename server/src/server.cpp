#include "server.h"

namespace Network
{
	int ListenSocket(SOCKET socket, int backlog)
	{
		int res = listen(socket, backlog);
		if (res == SOCKET_ERROR)
			Panic("Can't set a socket as listening socket...");
		InfoLogc("Socket setted as listening socket successfully.");
		return Ok;
	}
	int BindSocket(SOCKET socket, sockaddr* addr, int addrSize)
	{
		int res = bind(socket, addr, addrSize);
		if (res)
			Panic("Can't bind a socket.");
		InfoLogc("Binding a socket finished successfully.");
		return Ok;
	}
	SOCKET CreateSocket(int af, int type, int protocol)
	{
		SOCKET sckt = socket(af, type, protocol);
		if (sckt == INVALID_SOCKET)
			Panic("Can't create a socket...");
		InfoLogc("Socket created successfully.");
		return sckt;
	}

	bool IsEmptyMessage(const char* buffer)
	{
		for (size_t i = 0; i < std::strlen(buffer); i++)
			if (!std::iscntrl(buffer[i]) && buffer[i] != ' ')
				return false;
		return true;
	}

	void ReportError(std::string msg, bool panic)
	{
		ErrorLogc(msg);
		if (panic)
		{
			ErrorLogc("Error above is unexpected. Press any key to quit...");
			std::cin.get();
			exit(0);
		}
	}
	void CreateAddressHints(int af, int port, std::string ip, sockaddr_in* addr)
	{
		addr->sin_family = af;
		addr->sin_port = htons(port);
		inet_pton(AF_INET, ip.c_str(), &addr->sin_addr);
	}

	Server::Server(int port, std::string ip) : port(port),ip(ip)
	{
		InfoLogc("Initializing a server...");
		if (this->Init())
			Panic("Initializing is failed.");
		InfoLogc("Initialization finished successfully.");
		Seprtr;
	}
	Server::~Server()
	{
		//this->activeThreads.clear();
		DisconnectAllClients;
		this->clients.clear();
		closesocket(this->serverSocket);
		WSACleanup();
	}

	void Server::Run()
	{
		this->StartListening();
		this->Stop();
	}
	void Server::Stop()
	{
		this->~Server();
	}

	int Server::DisconnectClient(ConnectedClient* client)
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
	ConnectedClient* Server::GetClientWithSocket(SOCKET socket)
	{
		for (ConnectedClient* client : this->clients)
			if (client->socket == socket)
				return client;
		return nullptr;
	}
	int Server::AcceptClient(SOCKET serverSocket, ConnectedClient* client)
	{
		client->addrSize = sizeof(client->addr);
		client->socket = accept(serverSocket, (sockaddr*)&client->addr, &client->addrSize);
		if (client->socket != INVALID_SOCKET)
		{
			ZeroMemory(client->host, NI_MAXHOST);
			ZeroMemory(client->service, NI_MAXSERV);
			if (!getnameinfo((sockaddr*)&client->addr, client->addrSize, client->host, NI_MAXHOST, client->service, NI_MAXSERV, 0))
				Logc("Client " + std::string(client->host) + " as " + std::to_string(client->socket) + " connected.", "CONNECTION");
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
			this->clients.push_back(client);
			return Ok;
		}
		return Fail;
	}

	int Server::Init()
	{
		if (WSAStartup(MAKEWORD(2, 2), &this->wsa))
			return Fail;

		this->serverAddrSize = sizeof(this->serverAddr);
		CreateAddressHints(AF_INET, this->port, this->ip, &this->serverAddr);
		this->serverSocket = CreateSocket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		return BindSocket(this->serverSocket, (sockaddr*)&this->serverAddr, this->serverAddrSize);
	}
	void Server::StartListening()
	{
		ListenSocket(this->serverSocket, SOMAXCONN);
		Emptyc;
		InfoLogc("Waiting for connections...");
		Emptyc;
		while(true)
		{
			ConnectedClient* client = new ConnectedClient();
			if (this->AcceptClient(this->serverSocket, client))
				continue;
			std::thread(&Server::ExchangeDataWith, this, client).detach();
		}
	}

	int Server::ReceiveData(ConnectedClient* client)
	{
		ZeroMemory(client->buffer, TcpPackSize);
		client->bufSize = recv(client->socket, client->buffer, TcpPackSize, 0);
		return client->bufSize;
	}
	int Server::ProcessData(ConnectedClient* client)
	{
		if (client->bufSize > 0 && !IsEmptyMessage(client->buffer))
			Logc(std::string(client->buffer), std::to_string(client->socket));
		return Ok;
	}
	void Server::ExchangeDataWith(ConnectedClient* client)
	{
		do
		{
			if (this->ReceiveData(client) < 0)
				break;
			this->ProcessData(client);
			SendFromClientToAll(client, client->buffer, std::to_string(client->socket).c_str());
		} while (client->bufSize > 0);
		if (this->DisconnectClient(client))
			ErrorLogc("Client " + std::to_string(client->socket) + " already disconnected.");
		else
		{
			Logc("Client " + std::to_string(client->socket) + " disconnected.", "DISCONNECTION");
			SendFromServerToAll(client, "Client " + std::to_string(client->socket) + " disconnected.");
		}
	}
	int Server::SendDataTo(ConnectedClient* client, std::string buffer, std::string from)
	{
		return this->SendDataTo(client, buffer.c_str(), std::string(from).c_str()) ? Fail : Ok;
	}
	int Server::SendDataTo(ConnectedClient* client, const char* buffer, const char* from)
	{
		if (IsEmptyMessage(buffer))
			return Ok;
		std::string message = std::string(from) + ": " + std::string(buffer) + "\r\n";
		if (send(client->socket, message.c_str(), message.size(), 0) == SOCKET_ERROR)
		{
			ErrorLogc("Can't send message to " + std::to_string(client->socket));
			return Fail;
		}
		else
			return Ok;
	}
	int Server::SendDataTo(std::vector<ConnectedClient*> clients, ConnectedClient* exceptTo, const char* buffer, const char* from)
	{
		for (ConnectedClient* client : clients)
			if (client != exceptTo)
				this->SendDataTo(client, buffer, from);
		return Ok;
	}
	int Server::SendDataTo(std::vector<ConnectedClient*> clients, ConnectedClient* exceptTo, std::string buffer, std::string from)
	{
		return this->SendDataTo(clients, exceptTo, buffer.c_str(), from.c_str()) ? Fail : Ok;
	}
}