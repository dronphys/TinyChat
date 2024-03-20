#include "net.h"
#include "client.h"


int main()
{
	CustomClient c;
	c.Connect("185.237.206.136", 60000);
	//c.Connect("127.0.0.1", 60000);
	c.Run();

	return 0;

}