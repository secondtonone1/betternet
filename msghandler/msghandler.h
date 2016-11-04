#ifndef __MSG_HANDLER_H__
#define __MSG_HANDLER_H__

#include "../netmodel/socketwrapper.h"

struct HeadPacket
{
	int packetID;
	int packetLen;
};

class MsgNode
{
public:
	MsgNode():m_pMsg(NULL), m_nPacketID(0),m_nPacketLen(0),
		m_nMsgOff(0){}
	
	void clear()
	{
		if(m_pMsg)
		{
			free(m_pMsg);
			m_pMsg = NULL;
		}

		m_nPacketID = 0;
		m_nPacketLen = 0;
		m_nMsgOff = 0;
	}
	char * m_pMsg;
	int m_nPacketID;
	int m_nPacketLen;
	int m_nMsgOff;
  
};

class MsgHandler
{
public :
	MsgHandler():m_pSocketWrapper(NULL),m_bIsPending(false){}
	void  setSocketWrapper(SocketWrapper * socketWrapper);
	~MsgHandler(){
		m_pSocketWrapper = NULL;
		m_msgNode.clear();
		m_bIsPending = false;
	}
	bool unserilizeMsg();
private:
	SocketWrapper * m_pSocketWrapper;
	MsgNode m_msgNode;
	bool m_bIsPending;
	
};

#endif