#include "stdafx.h"
#include "aPM12Tool.h"
#include "SerialProtocol.h"


CSerialProtocol *g_pSerialProtocol = NULL;

CSerialProtocol::CSerialProtocol(void)
{
    int i;
    m_bSerialOpen = false;
    m_bIsNeedCheckIDValid = false;

    for (i = 0; i < PacketID_MAX_COUNT; i++)
    {
        m_pPktHandleFunc[i] = NULL;
        m_pPktHandleParam[i] = NULL;
    }

    m_RxPktSrcAddr = UART_AIO_ADDR;
    m_RxPktDesAddr = UART_MCU_ADDR;
}


CSerialProtocol::~CSerialProtocol(void)
{
    if (m_bSerialOpen)
    {
        closeDevice();
    }
}

void    CSerialProtocol::initApplication(void)
{

}

bool    CSerialProtocol::isSerialOpen(void)
{
    return m_bSerialOpen;
}

int CSerialProtocol::openDevice(CWnd* pPortOwner, UINT portNr, unsigned long nBaudRate)
{
    if (m_SerialDriver.OpenSerialPort(    pPortOwner,
                                    portNr,
                                    nBaudRate,
                                    BYTE_SIZE,
                                    PARITY_CHECK,
                                    STOP_BIT))   
    {   
        m_bSerialOpen = true;
        m_SerialDriver.StartMonitoring();
        startPktMonitoring();
        return 0;
    }
    return -1;
}

int CSerialProtocol::closeDevice(void)
{
    stopPktMonitoring();
    m_SerialDriver.StopMonitoring();
    m_SerialDriver.CloseSerialPort();//关闭串口
    m_bSerialOpen = false;
    return 0;
}

/* 函数名 : getSerialPortsReg
*  说  明 : 通过注册表枚举系统串口数量，并列出所有串口的系统名
*
*  参  数 : pListStr, 串口名称列表
*  返回值 : int, 系统串口个数
*/
int CSerialProtocol::getSerialPortsReg(char (*pListStr)[80])
{
    return m_SerialDriver.getSerialPortsReg(pListStr);
}

BOOL CSerialProtocol::startPktMonitoring()
{
    if (!(m_unPacketThread = AfxBeginThread(unPacketThread, this)))   
    {
        return FALSE;
    }
    return TRUE;  
}
BOOL CSerialProtocol::stopPktMonitoring()
{
    m_bPktThreadRun = false;
    return TRUE;
}

UINT CSerialProtocol::unPacketThread(LPVOID pParam)   
{
    CSerialProtocol *pSerialProtocol = (CSerialProtocol*)pParam;   
      
    pSerialProtocol->m_bPktThreadRun = true;
    int count = 0;
    int res;

    TRACE("unPacketThread started\n");  
    while(pSerialProtocol->m_bPktThreadRun)
    {
        res = pSerialProtocol->getOnePacket(&pSerialProtocol->m_rxPkt,UINT_PKT_RX_DELAY_MS);
        if (1 == res)
        {
            pSerialProtocol->unPacketHandle();
        }
        else if (res < 0)
        {
            ERROR_INFO("unPacketThread pkt-timeout[err_no=%d]\r\n",res);
        }
        if(count++ > 10)
        {
            count = 0;
            Sleep(1);
        }
    }
    TRACE("unPacketThread stoped\n");  
    // Kill this thread.  break is not needed, but makes me feel better.   
    AfxEndThread(100);
    return 0;
}   

void CSerialProtocol::setRxPacketAddr(BYTE src, BYTE des)
{
    this->m_RxPktSrcAddr = src;
    this->m_RxPktDesAddr = des;
}

int CSerialProtocol::bindPaktFuncByID(BYTE id, LPVOID pParam,  int (WINAPI *pFunc)(LPVOID pParam, UartProtocolPacket *pPacket))
{
    if ((id >= PacketID_MAX_COUNT) || (NULL != m_pPktHandleFunc[id]) || (NULL != m_pPktHandleParam[id]))
    {
        return -1;
    }

    m_pPktHandleFunc[id] = pFunc;
    m_pPktHandleParam[id] = pParam;
    return 0;
}

int CSerialProtocol::releasePaktFuncByID(BYTE id)
{
    if (id >= PacketID_MAX_COUNT)
    {
        return -1;
    }
    m_pPktHandleFunc[id] = NULL;
    m_pPktHandleParam[id] = NULL;
    return 0;
}


void CSerialProtocol::unPacketHandle(void)
{
    if (m_bIsNeedCheckIDValid)
    {
        Packet_NumValidCheck();
    }

    if (NULL != m_pPktHandleFunc[m_rxPkt.PacketID])
    {
        m_pPktHandleFunc[m_rxPkt.PacketID](m_pPktHandleParam[m_rxPkt.PacketID], &m_rxPkt);
    }
}


bool CSerialProtocol::isPacketValid(UartProtocolPacket *pPacket)
{
    BYTE crc = getCRC(pPacket);
    if (crc == pPacket->DataAndCRC[pPacket->Length])
    {
        return true;
    }
    return false;
}

int CSerialProtocol::getSerialRxBufLen(void)
{
    return m_SerialDriver.RxGetDataLen();
}

void CSerialProtocol::resetSerialRxBuf(void)
{
	m_SerialDriver.RxBuffeerReset();
}

/*****************************************************************************
 Prototype    : getSerialRxByte
 Description  : get one byte by uart
 Input        : u8 *pdata     
                int *pBufLen  
 Output       : None
 Return Value : 1 -- success 0 -- timeout
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2014/12/12
    Author       : qiuweibo
    Modification : Created function

*****************************************************************************/
int CSerialProtocol::getSerialRxByte(BYTE *pdata, int *pBufLen, int timeout_ms)
{
    DWORD timeout ,startTime, stopTime;

    startTime = GetTickCount();
    timeout = startTime+timeout_ms;
    while(1)
    {
        stopTime =  GetTickCount();
        if (m_SerialDriver.RxPopResult(pdata) > 0)
        {
            *pBufLen = *pBufLen -1; 
            if ((stopTime - startTime) > 20)
            {
                Log2File("ID=0x%02X Eat Time=%d ms\r\n",m_rxPkt.PacketID,(stopTime - startTime));
            }
            return 1;
        }

        if (stopTime >= timeout) // timeout
        {
            break;
        }
    }
    return 0;
}
//@return : 1 -- ok; 0 -- no packet ; <0  --  Error
int CSerialProtocol::getOnePacket(UartProtocolPacket *pPacket, int timeout_ms)
{
    BYTE crc = 0;
    int i;
    int len = getSerialRxBufLen();
    memset((char *)pPacket, 0, sizeof(UartProtocolPacket));

    while(len >= PACKET_FIXED_LENGHT)
    {
        if (!getSerialRxByte(&pPacket->DR_Addr, &len, timeout_ms)) return -1;
        if (m_RxPktDesAddr == pPacket->DR_Addr)
        {
            if (!getSerialRxByte(&pPacket->SR_Addr, &len, timeout_ms)) return -2;
            if (m_RxPktSrcAddr == pPacket->SR_Addr) //get pkt header
            {
                if ((UART_BACK_ADDR == m_RxPktSrcAddr) \
                    || (UART_RECORD_ADDR == m_RxPktSrcAddr))
                {
                    //先ID，再NUM
                    if (!getSerialRxByte(&pPacket->PacketID, &len, timeout_ms)) return -3;
                    if (!getSerialRxByte(&pPacket->PacketNum, &len, timeout_ms)) return -4;
                }
                else
                {
                    if (!getSerialRxByte(&pPacket->PacketNum, &len, timeout_ms)) return -3;
                    if (!getSerialRxByte(&pPacket->PacketID, &len, timeout_ms)) return -4;
                }
                if (!getSerialRxByte(&pPacket->Length, &len, timeout_ms)) return -5;

                for (i=0; i <= pPacket->Length; i++)  //get data and crc value
                {
                    if (!getSerialRxByte(&pPacket->DataAndCRC[i], &len, 10*timeout_ms)) return -6;
                }
            
                //check CRC vlaue
                if (isPacketValid(pPacket))
                {
                    return 1;
                }
                else
                {
                    return -7;
                }
            }
            else
            {
                continue;
            } //end of SR_ADDR
        }
        else
        {
            continue;
        } // end of DR_ADDR
    } // end of while
    return 0;
}


int CSerialProtocol::sendOnePacket(BYTE id, BYTE pktNum, BYTE *pData, int nDatafLen)
{
    UartProtocolPacket pkt;
    int sendLen;

    //地址与接收相反
    pkt.SR_Addr = m_RxPktDesAddr;
    pkt.DR_Addr = m_RxPktSrcAddr;

    pkt.PacketID = id;
    pkt.PacketNum = pktNum;
    pkt.Length = nDatafLen;

    if (nDatafLen > 0)
    {
        memcpy(pkt.DataAndCRC, pData, nDatafLen);
    }

    addCRC2Tail(&pkt);

    sendLen = PACKET_FIXED_LENGHT + nDatafLen;
    m_SerialDriver.WriteSerial((unsigned char *)&pkt, sendLen);
    return 0;
}

void CSerialProtocol::Packet_NumValidCheck(void)
{
    char pStrLog[200] = {0,};
    static unsigned long ecg_err_count = 0;
    static unsigned long aio_err_count = 0;
    switch(m_rxPkt.PacketID)
    {
    case AIO_TX_ECG_REALTIME_ID:
        
        if (m_rxPkt.PacketNum != m_pkt_num_array[AIO_TX_ECG_REALTIME_ID])
        {
            ecg_err_count++;
            sprintf(pStrLog, "AIO:ID=0X%02X NUM cur:%d cal:%d total:%d\r\n",
                m_rxPkt.PacketID, 
                m_rxPkt.PacketNum, 
                m_pkt_num_array[AIO_TX_ECG_REALTIME_ID],
                ecg_err_count);
            INFO("%s",pStrLog);
            Log2File("%s",pStrLog);
            m_pkt_num_array[AIO_TX_ECG_REALTIME_ID] = m_rxPkt.PacketNum + 1;
        }
        else
        {
            m_pkt_num_array[AIO_TX_ECG_REALTIME_ID]++;
            if (m_pkt_num_array[AIO_TX_ECG_REALTIME_ID]&0x01)TRACE("1");
            if (m_pkt_num_array[AIO_TX_ECG_REALTIME_ID] == 0)TRACE("\r\n");
        }
        break;
    case AIO_TX_ECG_POLAR_VOLTAGE_ID:
    case AIO_TX_ECG_HR_RR_ID:
    case AIO_TX_ARRHYTHMIA_RESULT_ID:
    case AIO_TX_ECG_PVCs_ID:
    case AIO_TX_ECG_ST_OFFSET_ID:
    case AIO_TX_ECG_Alarm_ID:
    case AIO_TX_ECG_LEAD_INFO_ID:
    case AIO_TX_ECG_OVERLOAD_ID:
    case AIO_TX_ECG_ANALYZE_CHANNEL_ID:
    case AIO_TX_ECG_ANALYZE_STATUS_ID:
    case AIO_TX_ECG_HEARTBEAT_INFO_ID:
    case AIO_ECG12_CHANNEL_ID:
    case AIO_ECG_ST_TEMPLATE_ID:
    case AIO_ECG_ANALYZE_STUDY_ID:
    case AIO_ECG_ST_SW_ID:
    case AIO_ECG_ARRHYTHMIA_SW_ID:
    case AIO_ECG_ST_MEASURE_ID:
    case AIO_ECG_NOTCH_SW_ID:
    case AIO_ECG_CAL_MODE_ID:
    case AIO_ECG_PROBE_MODE_ID:
    case AIO_ECG_PACE_SW_ID:
    case AIO_ECG_PACE_CHANNEL_ID:
    case AIO_ECG_PACE_OVERSHOOT_SW_ID:
    case AIO_RX_ECG_Debug_ID:
    case AIO_TX_RESP_REALTIME_ID:
    case AIO_TX_RESP_ASPHYXIA_ID:
    case AIO_TX_RESP_CVA_ID:
    case AIO_TX_RESP_OTHER_ALARM_ID:
    case AIO_RX_RESP_UPLOAD_TYPE_ID:
    case AIO_RX_RESP_THRESHOLD_ID:
    case AIO_RESP_CHANNEL_SEL_ID:
    case AIO_RESP_CARRIER_SW_ID:
    case AIO_RESP_ASPHYXIA_TIME_ID:
    case AIO_RX_RESP_Debug_ID:
    case AIO_TX_TEMP_REALTIME_ID:
    case AIO_TX_TEMP_ALARM_ID:
    case AIO_TX_TEMP_CAL_SW_ID:
    case AIO_RX_TEMP_Debug_ID:
    case AIO_NIBP_RESLULT_ID:
    case AIO_TX_NIBP_REALTIME_ID:
    case AIO_NIBP_VERIFY_STATE_ID:
    case AIO_TX_NIBP_ALARM_ID:
    case AIO_TX_NIBP_MMHG_ID:
    case AIO_TX_NIBP_COUNT_DOWN_S_ID:
    case AIO_NIBP_START_ID:
    case AIO_NIBP_STOP_ID:
    case AIO_NIBP_CYCLE_ID:
    case AIO_NIBP_VERIFY_ID:
    case AIO_NIBP_VERIFYING_ID:
    case AIO_NIBP_STM32_PRESS_ID:
    case AIO_NIBP_RESET_ID:
    case AIO_NIBP_PREPROCESS_PRESS_ID:
    case AIO_NIBP_STATIC_PRESS_ID:
    case AIO_NIBP_VENIPUNCTURE_ID:
    case AIO_NIBP_CONTINUED_ID:
    case AIO_NIBP_GAS_LEAK_ID:
    case AIO_RX_NIBP_Debug_ID:
    case COM_SOFTWARE_VERSION_ID:
    case COM_SELF_CHECK_ID:
    case COM_TX_PowerStatus_ID:
    case COM_TX_AbnormalReset_ID:
    case COM_PATIENT_TYPE_ID:
    case COM_PM_WORK_MODE_ID:
        
        if (m_rxPkt.PacketNum != m_pkt_num_array[m_rxPkt.PacketID])
        {
            aio_err_count++;
            sprintf(pStrLog, "AIO:ID=0X%02X NUM cur:%d cal:%d totoal:%d\r\n",
                m_rxPkt.PacketID, 
                m_rxPkt.PacketNum, 
                m_pkt_num_array[m_rxPkt.PacketID],
                aio_err_count);
            INFO("%s",pStrLog);
            Log2File("%s",pStrLog);
        }
        m_pkt_num_array[m_rxPkt.PacketID] = m_rxPkt.PacketNum + 1;
        
        break;

    case SpO2_MODEL_VERSION_ID:
    case SpO2_ALARM_INFO_ID:
    case SpO2_SELF_CHECK_ID:
    case SpO2_PATIENT_SPEED_ID:
    case SpO2_WORK_MODE_ID:
    case SpO2_MODEL_LOWPOWER_ID:
    case SpO2_POWER_DETECT_ID:
    case SpO2_SPO2_REALTIME_ID:
    case SPO2_NORMALIZED_ID:
    case SPO2_CALC_RESULT_ID:
    case SPO2_DEBUG_INTERFACE_ID:
//        INFO("\r\nSpO2:ID=0X%02X",m_rxPkt.PacketID);
        if (m_rxPkt.PacketNum != m_pkt_num_array[m_rxPkt.PacketID])
        {
            aio_err_count++;
            sprintf(pStrLog, "SpO2:ID=0X%02X NUM cur:%d cal:%d totoal:%d\r\n",
                m_rxPkt.PacketID, 
                m_rxPkt.PacketNum, 
                m_pkt_num_array[m_rxPkt.PacketID],
                aio_err_count);
            INFO("%s",pStrLog);
            Log2File("%s",pStrLog);
        }
        m_pkt_num_array[m_rxPkt.PacketID] = m_rxPkt.PacketNum + 1;
        break;
    case SYSTEM_ERR_ID:
        if ((BYTE)SYS_ERR_MCU_UART_TX_FULL == m_rxPkt.DataAndCRC[0])
        {
            int len = 0;
            len = (m_rxPkt.DataAndCRC[1] << 8) | m_rxPkt.DataAndCRC[2];
            sprintf(pStrLog,">>>>>>>>>>>>>>>>>>>>>>>>SYS_ERR:DMA_TX_FULL current Len=%d\r\n",len);
            ERROR_INFO("%s",pStrLog);
            Log2File("%s",pStrLog);
        }
        else if ((BYTE)SYS_ERR_UART_DMA_ERR == m_rxPkt.DataAndCRC[0])
        {
            sprintf(pStrLog,">>>>>>>>>>>>>>>>>>>>>>>>SYS_ERR:DMA_ERR\r\n");
            ERROR_INFO("%s",pStrLog);
            Log2File("%s",pStrLog);
        }
        else if ((BYTE)SYS_ERR_SPO2_UART_RX == m_rxPkt.DataAndCRC[0])
        {
            sprintf(pStrLog,">>>>>>>>>>>>>>>>>>>>>>>>SYS_ERR:SPO2_UART_RX\r\n");
            ERROR_INFO("%s",pStrLog);
            Log2File("%s",pStrLog);
        }
        else
        {
            sprintf(pStrLog,">>>>>>>>>>>>>>>>>>>>>>>>SYS_ERR:err_no=%d\r\n",m_rxPkt.DataAndCRC[0]);
            ERROR_INFO("%s",pStrLog);
            Log2File("%s",pStrLog);
        }
        break;
    case SF_SPO2_UPDATE:
    case SF_AIO_STM_UPDATE:
    case SF_AIO_DSP_UPDATE:
    case SF_BACK_UPDATE:
    case SF_RECORD_UPDATE:
    case SF_EXPAND_UPDATE:
        break;
    default:
        sprintf(pStrLog,"Unknowd:ID=0X%02X\r\n", m_rxPkt.PacketID);
        WARNING("%s",pStrLog);
        Log2File("%s",pStrLog);
        break;
    }
}

void    CSerialProtocol::setPacketNumValidCheck(bool newState)
{
    m_bIsNeedCheckIDValid = newState;
}

int CSerialProtocol::addCRC2Tail(UartProtocolPacket *pPacket)
{
    BYTE crc, tmp;
    crc = getCRC(pPacket);

    if ((UART_BACK_ADDR == m_RxPktSrcAddr) \
        || (UART_RECORD_ADDR == m_RxPktSrcAddr))
    {
        //调整顺序:先ID再NUM
        tmp = pPacket->PacketNum;
        pPacket->PacketNum = pPacket->PacketID;
        pPacket->PacketID = tmp;
    }
    pPacket->DataAndCRC[pPacket->Length] = crc;
    return 0;
}

BYTE CSerialProtocol::getCRC(UartProtocolPacket *pPacket)
{
    BYTE len = 3 + (pPacket->Length);//3(PacketNum,PacketID,Length)+Data length
    BYTE crc;

    if ((UART_BACK_ADDR == m_RxPktSrcAddr) \
        || (UART_RECORD_ADDR == m_RxPktSrcAddr))
    {
        //调整顺序:先ID再NUM
        UartProtocolPacket pkt = *pPacket;
        pkt.PacketNum = pPacket->PacketID;
        pkt.PacketID = pPacket->PacketNum;
        crc = CRC8(&pkt.PacketNum, len);
    }
    else
    {
        crc = CRC8(&pPacket->PacketNum, len);
    }
    return crc;
}

unsigned char CSerialProtocol::CRC8(unsigned char *ptr,unsigned char len)
{
	unsigned char crc;
	unsigned char i;
	crc = 0;
	while(len--)
	{
		crc ^= *ptr++;
		for(i = 0; i < 8; i++)
		{
			if(crc&0x01)
			{
				crc = (crc >> 1) ^ 0x8C;
			}	
			else
			{
				crc >>= 1;
			}
		}	
	}
	return crc;    
}