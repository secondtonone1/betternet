#ifndef  _WRITE_BUFFER_H__
#define _WRITE_BUFFER_H__

#include "socketbuffer.h"
#include "netmodeldef.h"
//写缓存
class SocketWrapper;
class WriteBuffer: public SocketBuffer
{
	friend class SocketWrapper;
 private:  
	 WriteBuffer(sockfd fd);
	~WriteBuffer();
	WriteBuffer(const WriteBuffer & writebuffer);
	WriteBuffer &  operator = (const WriteBuffer & socketBuffer);
	Node * m_pCurSend;
	//每次发送节点中数据，从moff开始，长度为mlen
	//在发送功能的node里，moff表示自从上次发送完之后开始发送的位置
	int sendto( Node * node);

	//获取发送节点
	Node * getSendNode();
	//清除发送节点内存
	void clearSendNode();
	//内部调用，想buffer中添加数据
	int pushData(char * buf, int len);
};

#endif
