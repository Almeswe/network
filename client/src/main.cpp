#include "client.h"
using namespace Network;

int main(char** argv, int argc)
{
	Client* c = new Client("27015","localhost");
	c->Connect();
}