#include "server.h"

int main()
{
	(new Network::Server(27015,"127.0.0.1"))->Run();
}