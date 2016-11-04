#include "msghandler.h"

void  MsgHandler::setSocketWrapper(SocketWrapper * socketWrapper)
{
	m_pSocketWrapper = socketWrapper;
}

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

