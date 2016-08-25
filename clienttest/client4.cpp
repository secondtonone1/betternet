#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#define BUFFLEN 1024
#define SERVER_PORT 9995

#include <iostream>
using namespace std;

int main(int argc, char * argv[])
{

	int s;

	char buff[BUFFLEN];

	int n = 0;

	//建立套接字
	s = socket(AF_INET, SOCK_STREAM, 0);


	//初始化服务器地址
	struct sockaddr_in server;

	memset(&server, 0, sizeof(server));

	server.sin_family = AF_INET;
	server.sin_port = htons(SERVER_PORT);
	server.sin_addr.s_addr = inet_addr("192.168.1.40");
	//server.sin_addr.s_addr = inet_addr("192.168.1.99");


	cout << "begin connect" <<endl;

	//连接服务器

	int conRes = connect(s,(struct sockaddr *)&server, sizeof(server));

	if(conRes < 0)
	{
		cout << "connect error !!" << endl;
		return 0;
	}

	cout << "connect res: " << conRes << endl;

	memset(buff, 0, BUFFLEN);

	strcpy(buff, "TIME");

	char sendData[19] = {0};
	int times = 0;	
	for(int i = 0 ; i < 100000; i++)
	{
			
		char msg[] = "1234567890";
	
      
		cout << "msg len is : " << strlen(msg) << endl;
		//发送数据

		int sendLen = send(s, msg, strlen(msg), 0);

		cout << "send datalen: " <<  sendLen << endl;
		cout << "send data is : " << msg << endl;	
		times++;
		cout << "send times is: "<<times<<endl;

	}
	

	getchar();	
	close(s);

	return 0;	
}
