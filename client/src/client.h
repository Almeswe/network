#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>

#include <iomanip>
#include <ctime>
#include <chrono>

#pragma warning(disable : 4996)

#define TCP_DATA_PACK 512
#define DEFAULT_ANSI_PORT port.c_str() 

using namespace std;

namespace Network
{
	class Client
	{
	private:
		string port;
		string address;

		WSADATA wsadata;

		SOCKET connectSocket;

		addrinfo addresHints;
		addrinfo* addressResult = NULL;

		int receivedLen;
		char buffer[TCP_DATA_PACK];

	public:
		Client(string port,string procname);
		~Client();

	public: 
		void Connect();
		void Disconnect();

	private:
		int ReceiveData();
		int SendDataTo();
		void ProcessData();

	private:
		inline void ErrorLog(string msg, string fn)
		{
			this->InfoLog("ERROR in func " + fn + " MSG: " + msg);
			WSACleanup();
		}
		inline void InfoLog(string msg)
		{
			time_t const now = time(0);
			tm* gmtm = gmtime(&now);
			printf("[%d.%d %d:%d:%d] %s\n", gmtm->tm_mday, gmtm->tm_mon, gmtm->tm_hour, gmtm->tm_min, gmtm->tm_sec, msg.c_str());
		}
	};
}
