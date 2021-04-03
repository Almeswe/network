#include "server.h"

using namespace Network;

int main()
{
	Server* s = new Server();
	s->Start();
}