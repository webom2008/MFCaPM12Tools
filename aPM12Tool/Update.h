#pragma once

#include "SerialProtocol.h"

class CUpdate
{
public:
    CUpdate(void);
    ~CUpdate(void);

    static int WINAPI    PktHandleUpdateSoftware(LPVOID pParam, UartProtocolPacket *pPacket);
    void initApplication(void);
    void setPacketCID(BYTE id);
    BYTE getPacketCID(void);

protected:
    int SaveFiletoRAM(unsigned int *pFileLen, CString &filePath);
    void DisplayOKorError(int state);
	int SendResetAndUpdateTag(void);
	int SendUpdateStartOfLenght(const unsigned int file_len);
	int SendUpdateStartOfData(const unsigned int file_len);
	int SendUpdateEndOfTransmit(void);
    int WaitUpdateWrite2FlashDone(void);

private:
    BYTE    *pFileRamAddr;
    unsigned int m_packetCount;
    unsigned int m_Mem_addr_offset;

    BYTE		        m_BChildID;
	UartProtocolPacket  m_PktForUpdate;
	// handles 
	HANDLE	            m_hGetUpdatePacketEvent;
	CRITICAL_SECTION	m_csCommunicationSync; 
};

