#include "msghandler.h"

void  MsgHandler::setSocketWrapper(SocketWrapper * socketWrapper)
{
	m_pSocketWrapper = socketWrapper;
}

//这个消息只能处理一个包，容易造成应用层丢包，下面接口会进行改进
bool MsgHandler::unserilizeMsg()
{
	//判断是否处理过头部消息
	if(!m_bIsPending)
	{
		int totallen = m_pSocketWrapper->getTotalRead();
		if(totallen < 8)
		{
			return true; 
		}
		HeadPacket packetHead;
		//处理头部消息

		m_pSocketWrapper->readFromBuffer((char *)&packetHead, 8);

		if(packetHead.packetID > 1024)
		{
			return false;
		}

		if(packetHead.packetLen > 2048)
		{
			return false;
		}

		m_msgNode.clear();
		m_msgNode.m_nPacketID = packetHead.packetID;
		m_msgNode.m_nPacketLen = packetHead.packetLen;

		if(totallen -8 >= packetHead.packetLen)
		{

			m_msgNode.m_pMsg = (char *)malloc(sizeof(char) * (packetHead.packetLen+1));
			memset(m_msgNode.m_pMsg, 0, packetHead.packetLen+1);
			m_pSocketWrapper->readFromBuffer(m_msgNode.m_pMsg, packetHead.packetLen);
			m_msgNode.m_nMsgOff = packetHead.packetLen;
			//可以添加一些逻辑处理逻辑
			//.....例如输出收到的字符串数据
			cout << m_msgNode.m_pMsg <<endl;
			//将得到的消息发送给客户端，也可以不发送，这里做测试
			m_pSocketWrapper->writeToBuffer(m_msgNode.m_pMsg, m_msgNode.m_nMsgOff);
		}
		else
		{
			//设置pending标记位，下次不需要处理头部消息
			m_bIsPending = true;

			m_msgNode.m_pMsg = (char *)malloc(sizeof(char) * (packetHead.packetLen+1));
			memset(m_msgNode.m_pMsg, 0, packetHead.packetLen+1);
			m_pSocketWrapper->readFromBuffer(m_msgNode.m_pMsg, totallen -8);
			m_msgNode.m_nMsgOff = totallen -8;
		}//if(totallen -8 >= packetHead.packetLen)

	}
	else
	{
		int totallen = m_pSocketWrapper->getTotalRead();
		//剩余要读取的数量
		int remain = m_msgNode.m_nPacketLen- m_msgNode.m_nMsgOff;
		if(totallen >= remain )
		{
			m_pSocketWrapper->readFromBuffer(m_msgNode.m_pMsg+ m_msgNode.m_nMsgOff, remain);
			m_msgNode.m_nMsgOff += remain;
			m_bIsPending = false;

			//可以添加一些逻辑处理逻辑
			//.....
			//.....例如输出收到的字符串数据
			cout << m_msgNode.m_pMsg <<endl;
			//将得到的消息发送给客户端，也可以不发送，这里做测试
			m_pSocketWrapper->writeToBuffer(m_msgNode.m_pMsg, m_msgNode.m_nMsgOff);
		}
		else
		{
			m_pSocketWrapper->readFromBuffer(m_msgNode.m_pMsg+ m_msgNode.m_nMsgOff, totallen);
			m_msgNode.m_nMsgOff += totallen;
		}
	}

	return true;
}

//这个消息只能处理一个包，容易造成应用层丢包，下面接口会进行改进
bool MsgHandler::unserilizeMsgNew()
{
	int totallen = m_pSocketWrapper->getTotalRead();
	//SocketWrapper剩余的可读字节数
	int remain = totallen;
	if(m_bIsPending)
	{
		//剩余要读取的数量
		int remainToRead = m_msgNode.m_nPacketLen- m_msgNode.m_nMsgOff;

		//如果剩余未读取的数据大于当前SocketWrapper存储的总共数据大小
		if(remainToRead > totallen)
		{
			m_pSocketWrapper->readFromBuffer(m_msgNode.m_pMsg+ m_msgNode.m_nMsgOff, totallen);
			m_msgNode.m_nMsgOff += totallen;

			remain -= totallen;
			return true;
		}

		//如果当前SocketWrapper底层接受的数据大于msgNode剩余要读取的数据

		m_pSocketWrapper->readFromBuffer(m_msgNode.m_pMsg+ m_msgNode.m_nMsgOff, remainToRead);
		m_msgNode.m_nMsgOff += remainToRead;
		m_bIsPending = false;
		//更新处理后SocketWrapper剩余的可读字节数
		remain -= remainToRead;

		//可以添加一些逻辑处理逻辑
		//.....
		//.....例如输出收到的字符串数据
		cout << m_msgNode.m_pMsg <<endl;
		//将得到的消息发送给客户端，也可以不发送，这里做测试
		m_pSocketWrapper->writeToBuffer(m_msgNode.m_pMsg, m_msgNode.m_nMsgOff);

	}//	if(m_bIsPending)

	//处理完上一个节点未读完数据后继续处理新的节点

	return readToMax(remain);



	return true;
}


//新增函数，尽可能多的读取数据进入MsgNode
bool MsgHandler::readToMax(int remain)
{
	while(1)
	{
		if(remain < sizeof(HeadPacket))
		{
			return true; 
		}

		HeadPacket packetHead;
		bool res = dealPacketHead(&packetHead);
		if(!res)
		{
			return false;
		}

		remain -= sizeof(HeadPacket);

		if(remain >= packetHead.packetLen )
		{
			m_msgNode.m_pMsg = (char *)malloc(sizeof(char) * (packetHead.packetLen+1));
			memset(m_msgNode.m_pMsg, 0, packetHead.packetLen+1);
			m_pSocketWrapper->readFromBuffer(m_msgNode.m_pMsg, packetHead.packetLen);
			m_msgNode.m_nMsgOff = packetHead.packetLen;
			remain -= packetHead.packetLen;
			//可以添加一些逻辑处理逻辑
			//.....例如输出收到的字符串数据
			cout << m_msgNode.m_pMsg <<endl;
			//将得到的消息发送给客户端，也可以不发送，这里做测试
			m_pSocketWrapper->writeToBuffer(m_msgNode.m_pMsg, m_msgNode.m_nMsgOff);
		}
		else
		{
			//设置pending标记位，下次不需要处理头部消息
			m_bIsPending = true;

			m_msgNode.m_pMsg = (char *)malloc(sizeof(char) * (packetHead.packetLen+1));
			memset(m_msgNode.m_pMsg, 0, packetHead.packetLen+1);
			m_pSocketWrapper->readFromBuffer(m_msgNode.m_pMsg, remain);
			m_msgNode.m_nMsgOff = remain;
			remain-= remain;

			return true;
		}//if(remain >= packetHead.packetLen )
	}//while(1)

	return true;

}


bool MsgHandler::dealPacketHead(HeadPacket * pHeadPacket)
{

	//处理头部消息
	memset(pHeadPacket, 0, sizeof(HeadPacket));
	m_pSocketWrapper->readFromBuffer((char *)&pHeadPacket, sizeof(HeadPacket));

	if(pHeadPacket->packetID > 1024)
	{
		return false;
	}

	if(pHeadPacket->packetLen > 2048)
	{
		return false;
	}

	m_msgNode.clear();
	m_msgNode.m_nPacketID = pHeadPacket->packetID;
	m_msgNode.m_nPacketLen = pHeadPacket->packetLen;

	return true;

}