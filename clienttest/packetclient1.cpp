#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#define BUFFLEN 1024
#define SERVER_PORT 9995

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>
#include <malloc.h>
using namespace std;

struct PacketHead
{
	int packetID;
	int packetLen;
	
};


void *create(void *arg)
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
		cout << "errorno is :  " << errno << endl;
		return 0;
	}

	cout << "connect res: " << conRes << endl;

	char msg[] = "1234567890";
	
	PacketHead packetHead1;
	packetHead1.packetID = 1;
	packetHead1.packetLen = strlen(msg) + 1;
	
	char * sendData1 = (char *)malloc(sizeof(char ) * (packetHead1.packetLen + 8 ));

	
	memset(sendData1, 0, packetHead1.packetLen + 8 );
		
	memcpy(sendData1, &packetHead1, 8);
	memcpy(sendData1 + 8, msg, strlen(msg));


	int sendLen = send(s, sendData1, packetHead1.packetLen + 8 , 0);

	cout << "send datalen: " <<  sendLen << endl;
  	free(sendData1);
	
	char msg2[] = "abcdefghijklmnopqrstuvwxyz";
	
	PacketHead packetHead2;
	packetHead2.packetID = 2;
	packetHead2.packetLen = strlen(msg2) + 1;
	
	char * sendData2 = (char *)malloc(sizeof(char ) * (packetHead2.packetLen + 8 ));
	
	memset(sendData2, 0, packetHead2.packetLen + 8 );
	
	memcpy(sendData2, &packetHead2, 8);
	memcpy(sendData2 + 8, msg2, strlen(msg2));
	
	int sendLen2 = send(s, sendData2, packetHead2.packetLen + 8 , 0);

	cout << "send datalen: " <<  sendLen2 << endl;
	free(sendData2);
	
	return NULL;
}


	
int main(int argc, char * argv[])
{

	//Linux一个进程最多开辟1024线程
	for(int i = 0; i < 1000; i++)
	{
		pthread_t tid;
		int error = pthread_create(&tid, NULL, create, NULL);
		if(error!=0)
		{
			printf("pthread_create is created is not created ... ");
			return -1;
		}
		
		sleep(1);
		//pthread_join(tid, NULL);
	}
	
	
	
	
	//cout << "send data is : " << sendData2 << endl;
   
	
	getchar();
	
	

	return 0;	
}
