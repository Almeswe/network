#include "client.h"
using namespace Network;

int main(char** argv, int argc)
{
	//"C\\Users\\Almes\\source\\repos\\network\\network\\server\\Debug\\server.exe"
	//"C:\Users\Almes\source\repos\network\network\server\Debug\server.exe"
	Client* c = new Client("27015","localhost");
	c->Connect();
}