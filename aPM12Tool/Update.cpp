#include "stdafx.h"
#include "aPM12Tool.h"
#include "Update.h"


extern CSerialProtocol *g_pSerialProtocol;


const BYTE SOFTWARE_UPDATE_ASK[] = \
{0xAA,0x99,0x88,0x77,0x66,0x55,0x44,0x33,0x22,0x11,0x00};
const BYTE SOFTWARE_UPDATE_ANSWER[] = \
{0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xAA};

typedef enum
{
    UPDATE_ACK       = 0x01,
    UPDATE_NAK,
    UPDATE_EOT,
    UPDATE_SOL, //start of *.bin lenght
    UPDATE_SOD, //start of data:length = packet data lenght - 2(CID + Number)
    UPDATE_CA,  //one of these in succession aborts transfer
    UPDATE_RP,  //resend the packet
    UPDATE_W_F_DONE,  //Write data into flash done
    UPDATE_W_F_ERR,  //Write data into flash error
} UPDATE_COMMUNICATE_CID;


#define UPDATE_PACKET_DATA_LEN  (128)


CUpdate::CUpdate(void)
{
	m_hGetUpdatePacketEvent = NULL;

    // initialize critical section   
    InitializeCriticalSection(&m_csCommunicationSync); 
	
    if (NULL != m_hGetUpdatePacketEvent)
	{
        ResetEvent(m_hGetUpdatePacketEvent);
	}   
    else   
	{
        m_hGetUpdatePacketEvent = CreateEvent(NULL, TRUE, FALSE, NULL); 
	} 

    m_BChildID = (BYTE)SF_AIO_DSP_UPDATE;
}


CUpdate::~CUpdate(void)
{
    if(NULL != m_hGetUpdatePacketEvent)
	{
        CloseHandle( m_hGetUpdatePacketEvent ); 
	}  
}

void CUpdate::initApplication(void)
{
    g_pSerialProtocol->bindPaktFuncByID(SF_SPO2_UPDATE ,this, CUpdate::PktHandleUpdateSoftware);
    g_pSerialProtocol->bindPaktFuncByID(SF_AIO_STM_UPDATE ,this, CUpdate::PktHandleUpdateSoftware);
    g_pSerialProtocol->bindPaktFuncByID(SF_AIO_DSP_UPDATE ,this, CUpdate::PktHandleUpdateSoftware);
    g_pSerialProtocol->bindPaktFuncByID(SF_BACK_UPDATE ,this, CUpdate::PktHandleUpdateSoftware);
    g_pSerialProtocol->bindPaktFuncByID(SF_RECORD_UPDATE ,this, CUpdate::PktHandleUpdateSoftware);
    g_pSerialProtocol->bindPaktFuncByID(SF_EXPAND_UPDATE ,this, CUpdate::PktHandleUpdateSoftware);
}

void CUpdate::setPacketCID(BYTE id)
{
    m_BChildID  = id;
}

BYTE CUpdate::getPacketCID(void)
{
    return m_BChildID;
}

int WINAPI CUpdate::PktHandleUpdateSoftware(LPVOID pParam, UartProtocolPacket *pPacket)
{
    CUpdate *pUpdate = (CUpdate *)pParam;
	if (pUpdate->m_BChildID == pPacket->PacketID)
	{
        if ((BYTE)UPDATE_NAK == pPacket->DataAndCRC[0])
        {
            INFO(".");
        }
        EnterCriticalSection(&pUpdate->m_csCommunicationSync); 
		pUpdate->m_PktForUpdate = *pPacket;
		SetEvent(pUpdate->m_hGetUpdatePacketEvent);
        LeaveCriticalSection(&pUpdate->m_csCommunicationSync); 
		return 1;
	}
	else
	{
        EnterCriticalSection(&pUpdate->m_csCommunicationSync);
		ResetEvent(pUpdate->m_hGetUpdatePacketEvent);
        LeaveCriticalSection(&pUpdate->m_csCommunicationSync); 
		return 0;
	}
}

/*
 * brief    :   Get File and Save to RAM
 * param    :   
 *              unsigned int *pFileLen: the lenght of the file.
 * return   :   0 -- OK
 *              -1 -- ERROR
 */
int CUpdate::SaveFiletoRAM(unsigned int *pFileLen, CString &filePath)
{
    if (FALSE == PathFileExists(filePath))
    {
        MSG("请选择正确的文件路径\r\n");
		return -1;
    }

    CFile bin_file( filePath,
                    CFile::modeRead);
    if (NULL == bin_file)
    {
        MSG("请确保文件存在\r\n");
		return -1;
    }
    unsigned int file_len = (unsigned int)bin_file.GetLength();

    pFileRamAddr = (BYTE *)malloc(file_len);
    if (NULL == pFileRamAddr)
    {
        MSG("malloc出错\r\n");
        bin_file.Close();
		return -1;
    }
    bin_file.SeekToBegin();//移到文件头
    bin_file.Read(pFileRamAddr, file_len);
    bin_file.Close();
    *pFileLen = file_len;
    return 0;
}

//0 -- display error 1 -- display Ok
void CUpdate::DisplayOKorError(int state)
{
	if (!state)
    {
        INFO("Error\r\n");
    }
    else
    {
        INFO("OK\r\n");
    }
}


/*
 * brief    :   Send updata Tag and judge receive Tag
 * return   :   1 -- get a update Tag packet
 *              0 -- get a not update Tag packet
 */
int CUpdate::SendResetAndUpdateTag(void)
{
    DWORD timeout, Event;

	//S1:发送Packet包
    EnterCriticalSection(&m_csCommunicationSync);  
	ResetEvent(m_hGetUpdatePacketEvent); 
    LeaveCriticalSection(&m_csCommunicationSync);
	//g_pSerialProtocol->resetSerialRxBuf();
    memset(&m_PktForUpdate, 0, sizeof(UartProtocolPacket));
	g_pSerialProtocol->sendOnePacket(m_BChildID, 0,  (BYTE *)&SOFTWARE_UPDATE_ASK[0], sizeof(SOFTWARE_UPDATE_ASK));
	
	//S2:等待DSP的回应
    timeout = GetTickCount()+1000; //1s
    while(1)
    {
		Event = WaitForSingleObject(m_hGetUpdatePacketEvent, 100);

        if (WAIT_OBJECT_0 ==  Event)
        {
            if ((sizeof(SOFTWARE_UPDATE_ANSWER) == m_PktForUpdate.Length) \
                && (0 == memcmp(&m_PktForUpdate.DataAndCRC[0],&SOFTWARE_UPDATE_ANSWER[0],m_PktForUpdate.Length)))
            {
                return 1;
            }
			else
			{
                EnterCriticalSection(&m_csCommunicationSync);  
	            ResetEvent(m_hGetUpdatePacketEvent); 
                LeaveCriticalSection(&m_csCommunicationSync);
			}
        }

        if (GetTickCount() >= timeout) // timeout
        {
            break;
        }
    }
    return 0;
}

/*
 * brief    :   
 * return   :   1   -- success
 *              0   -- fail
 *              -1  -- lenght out of flash
 */
int CUpdate::SendUpdateStartOfLenght(const unsigned int file_len)
{
	BYTE	pBuffer[6];
    DWORD	timeout, Event;

	//S1:发送Packet包
    m_packetCount = 0;
	pBuffer[0] = (BYTE)UPDATE_SOL;
    pBuffer[1] = (BYTE)(m_packetCount & 0xFF);
    pBuffer[2] = (BYTE)(file_len >> 24) & (0xFF); //MSB
    pBuffer[3] = (BYTE)(file_len >> 16) & (0xFF);
    pBuffer[4] = (BYTE)(file_len >> 8) & (0xFF);
    pBuffer[5] = (BYTE)(file_len >> 0) & (0xFF);  //LSB
    
    EnterCriticalSection(&m_csCommunicationSync);  
	ResetEvent(m_hGetUpdatePacketEvent); 
    LeaveCriticalSection(&m_csCommunicationSync);

	//g_pSerialProtocol->resetSerialRxBuf();
    memset(&m_PktForUpdate, 0, sizeof(UartProtocolPacket));
	g_pSerialProtocol->sendOnePacket(m_BChildID, 0,  pBuffer, sizeof(pBuffer));
	
	//S2:等待DSP的回应
    timeout = GetTickCount()+1000;
    while(1)
    {
		Event = WaitForSingleObject(m_hGetUpdatePacketEvent, 100);

        if (WAIT_OBJECT_0 ==  Event)
        {
			if ((BYTE)UPDATE_ACK == m_PktForUpdate.DataAndCRC[0]) //success
            {
                m_packetCount++;
                m_Mem_addr_offset = 0;
                return 1;
            }
            else if ((BYTE)UPDATE_SOL == m_PktForUpdate.DataAndCRC[0]) //resend again.
            {
                return 0;
            }
            else if ((BYTE)UPDATE_CA == m_PktForUpdate.DataAndCRC[0]) //lenght out of flash
            {
                return -1;
            }
			else
			{
                EnterCriticalSection(&m_csCommunicationSync);  
	            ResetEvent(m_hGetUpdatePacketEvent); 
                LeaveCriticalSection(&m_csCommunicationSync);
			}
        }

        if (GetTickCount() >= timeout) // timeout
        {
            break;
        }
    }
    return 0;
}

/*
 * brief    :   
 * return   :    >0 -- need send data len
 *               0 -- success
 *              -1 -- fail for lenght out of flash
 *              -2 -- fail for timeout
 */
int CUpdate::SendUpdateStartOfData(const unsigned int file_len)
{
	BYTE	pBuffer[130];
    DWORD	Event;
    unsigned int i, tryTime = 10;
	int		nLen = 0;
    unsigned int not_w_len;

    if (m_Mem_addr_offset >= file_len) //传输完成
    {
        return 0;
    }

    for (i = 0; i < tryTime;)
    {
	    //S1:封装Packet包
        not_w_len = file_len - m_Mem_addr_offset;
        if (not_w_len >= UPDATE_PACKET_DATA_LEN)
        {
	        nLen = UPDATE_PACKET_DATA_LEN + 2;
            memcpy(&pBuffer[2], (BYTE *)(pFileRamAddr + m_Mem_addr_offset), UPDATE_PACKET_DATA_LEN);
        }
        else
        {
	        nLen = not_w_len+2;
            memcpy(&pBuffer[2], (BYTE *)(pFileRamAddr + m_Mem_addr_offset), not_w_len);
        }
        pBuffer[0] = (BYTE)UPDATE_SOD;
        pBuffer[1] = (BYTE)(m_packetCount & 0xFF);
        
	    //S2:发送Packet包
        EnterCriticalSection(&m_csCommunicationSync);  
	    ResetEvent(m_hGetUpdatePacketEvent); 
        LeaveCriticalSection(&m_csCommunicationSync); 
		//g_pSerialProtocol->resetSerialRxBuf();
        memset(&m_PktForUpdate, 0, sizeof(UartProtocolPacket));
		g_pSerialProtocol->sendOnePacket(m_BChildID, 0,  pBuffer, nLen);
		
		//S3:等待DSP的回应
		Event = WaitForSingleObject(m_hGetUpdatePacketEvent, 1000);
		if (WAIT_OBJECT_0 ==  Event)
		{
			if ((BYTE)UPDATE_ACK == m_PktForUpdate.DataAndCRC[0]) //success
			{
				i = 0;
				m_Mem_addr_offset = m_packetCount * UPDATE_PACKET_DATA_LEN;
				m_packetCount++;
				if (m_packetCount % 32 == 0)
				{
					INFO("\r\n");
				}
				INFO("#");
		        if ((BYTE)SF_AIO_STM_UPDATE == m_BChildID)
		        {
			        Sleep(50);
		        }
				return not_w_len; 
			}
			else if ((BYTE)UPDATE_SOD == m_PktForUpdate.DataAndCRC[0]) //resend again.
			{
				m_packetCount = ((m_packetCount>>8)<<8)+m_PktForUpdate.DataAndCRC[1];
				m_Mem_addr_offset = (m_packetCount-1) * UPDATE_PACKET_DATA_LEN;
				INFO("R");
				if ((BYTE)SF_AIO_STM_UPDATE == m_BChildID)
				{
					Sleep(50);
				}
				return not_w_len; 
			}
			else if ((BYTE)UPDATE_CA == m_PktForUpdate.DataAndCRC[0])
			{
				return -1;
			}
			else
			{
				INFO("Unknow Packet(CID=0X%02X) for SendUpdateStartOfData\r\n",m_PktForUpdate.DataAndCRC[0]);
                EnterCriticalSection(&m_csCommunicationSync);  
	            ResetEvent(m_hGetUpdatePacketEvent); 
                LeaveCriticalSection(&m_csCommunicationSync); 
                Sleep(10);
			}
        }
        else if (WAIT_TIMEOUT ==  Event)
		{
            i++;
            INFO("T");
		}
    } //end of for()
    return -2; 
}

/*
 * brief    :   
 * return   :   1 -- success
 *              0 -- fail
 */
int CUpdate::SendUpdateEndOfTransmit(void)
{
	BYTE	pBuffer[1];
    DWORD	timeout, Event;

	//S1:发送Packet包
	pBuffer[0] = (BYTE)UPDATE_EOT;
    
    EnterCriticalSection(&m_csCommunicationSync);  
	ResetEvent(m_hGetUpdatePacketEvent); 
    LeaveCriticalSection(&m_csCommunicationSync); 
	//g_pSerialProtocol->resetSerialRxBuf();
    memset(&m_PktForUpdate, 0, sizeof(UartProtocolPacket));
	g_pSerialProtocol->sendOnePacket(m_BChildID, 0,  pBuffer, sizeof(pBuffer));
	
	//S2:等待DSP的回应
    timeout = GetTickCount()+1000;
    while(1)
    {
		Event = WaitForSingleObject(m_hGetUpdatePacketEvent, 100);

        if (WAIT_OBJECT_0 ==  Event)
        {
            if ((BYTE)UPDATE_NAK == m_PktForUpdate.DataAndCRC[0]) //success
            {
                return 1;
            }
			else
			{
                EnterCriticalSection(&m_csCommunicationSync);  
	            ResetEvent(m_hGetUpdatePacketEvent); 
                LeaveCriticalSection(&m_csCommunicationSync);
			}
        }
        else
        {
            TRACE("\r\nID success Timeout!");
        }

        if (GetTickCount() >= timeout) // timeout
        {
            break;
        }
    }
    return 0;
}


/*
 * brief    :   
 * return   :
 *              0 -- write flash done
 *              -1 -- write flash error
 *              -2 -- time out
 */
int CUpdate::WaitUpdateWrite2FlashDone(void)
{
    DWORD	timeout, Event;

    timeout = GetTickCount()+100;
    while(1)
    {
		Event = WaitForSingleObject(m_hGetUpdatePacketEvent, 10);

        if (WAIT_OBJECT_0 ==  Event)
        {
            TRACE("\r\nID success!"); 
			
            if ((BYTE)UPDATE_W_F_DONE == m_PktForUpdate.DataAndCRC[0]) //done
            {
                return 0;
            }
            else if ((BYTE)UPDATE_W_F_ERR == m_PktForUpdate.DataAndCRC[0]) //error
            {
                return -1;
            }
			else
			{
                EnterCriticalSection(&m_csCommunicationSync);  
	            ResetEvent(m_hGetUpdatePacketEvent); 
                LeaveCriticalSection(&m_csCommunicationSync);
			}
        }

        if (GetTickCount() >= timeout) // timeout
        {
            break;
        }
    }
    return -2; 
}
