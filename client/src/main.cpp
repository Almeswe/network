#include "client.h"

int main()
{
	(new Network::Client())->Connect(27015, "127.0.0.1");
}