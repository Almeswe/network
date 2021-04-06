#include "server2.h"
#include "server.h"
using namespace Network;

int main()
{
	(new Server2(27015,"127.0.0.1"))->Run();
}