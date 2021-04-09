#include "client.h"
using namespace Network;

int main()
{
	Client* c = new Client();
	c->Connect(27015, "127.0.0.1");
}