#include "client.h"
using namespace Network;

int main()
{
	Client* c = new Client("27015","localhost");
	c->Connect();
}