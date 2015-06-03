#pragma once
#include < functional>
#include "SerialDrive.h"

#define	BAUD_RATE		CBR_256000
#define	BYTE_SIZE		8
#define	PARITY_CHECK	NOPARITY
#define	STOP_BIT		ONESTOPBIT

const UINT UINT_SERIAL_TX_BUFFER        = 1024;
const UINT UINT_SERIAL_RX_BUFFER_LEN    = 10240;
const UINT UINT_PKT_RX_DELAY_MS         = 20;

typedef enum 
{
    /*----------- SpO2 Start ------------*/
    SpO2_MODEL_VERSION_ID	    = 0x10,
    SpO2_ALARM_INFO_ID		    = 0x11,
    SpO2_SELF_CHECK_ID		    = 0x12,
    SpO2_PATIENT_SPEED_ID       = 0x13,
    SpO2_WORK_MODE_ID           = 0x14,
    SpO2_MODEL_LOWPOWER_ID      = 0x16,//only for debug
    SpO2_POWER_DETECT_ID  	    = 0x17,
    SpO2_SPO2_REALTIME_ID  	    = 0x19,//only for debug
    SPO2_NORMALIZED_ID          = 0x1A,
    SPO2_CALC_RESULT_ID 	 	= 0x1B,
    SPO2_DEBUG_INTERFACE_ID 	= 0x1F,
    /*----------- SpO2 Stop ------------*/

    /*----------- AIO Start ------------*/
    AIO_TX_ECG_REALTIME_ID       = 0xA0,
    AIO_TX_ECG_POLAR_VOLTAGE_ID  = 0xA1,//polarization voltage
    AIO_TX_ECG_HR_RR_ID          = 0xA2,//心率HR和呼吸率RR
    AIO_TX_ARRHYTHMIA_RESULT_ID  = 0xA3,//心律失常分析结果
    AIO_TX_ECG_PVCs_ID           = 0xA4,//室早个数
    AIO_TX_ECG_ST_OFFSET_ID      = 0xA5,//ST分析结果
    AIO_TX_ECG_Alarm_ID          = 0xA6,
    AIO_TX_ECG_LEAD_INFO_ID      = 0xA7,
    AIO_TX_ECG_OVERLOAD_ID       = 0xA8,
    AIO_TX_ECG_ANALYZE_CHANNEL_ID = 0xA9,
    AIO_TX_ECG_ANALYZE_STATUS_ID = 0xAB,
    AIO_TX_ECG_HEARTBEAT_INFO_ID = 0xAC,
    
    AIO_ECG12_CHANNEL_ID	     = 0x20,
    AIO_ECG_ST_TEMPLATE_ID	     = 0x21,//ST模板数据
    AIO_ECG_ALGO_PRINTF_ID	     = 0x22,
    AIO_ECG_ANALYZE_STUDY_ID	 = 0x23,
    AIO_ECG_ST_SW_ID	         = 0x25,
    AIO_ECG_ARRHYTHMIA_SW_ID	 = 0x26,
    AIO_ECG_ST_MEASURE_ID	     = 0x27,
    AIO_ECG_NOTCH_SW_ID	         = 0x28,
    AIO_ECG_CAL_MODE_ID          = 0x29,
    AIO_ECG_PROBE_MODE_ID	     = 0x2A,
    AIO_ECG_PACE_SW_ID	         = 0x2B,
    AIO_ECG_PACE_CHANNEL_ID	     = 0x2C,
    AIO_ECG_PACE_OVERSHOOT_SW_ID = 0x2D,
    AIO_RX_ECG_Debug_ID          = 0x2F,//Add for Debug ECG Module
        
    AIO_TX_RESP_REALTIME_ID      = 0xB0,
    AIO_TX_RESP_ASPHYXIA_ID      = 0xB1,
    AIO_TX_RESP_CVA_ID           = 0xB2,
    AIO_TX_RESP_OTHER_ALARM_ID   = 0xB3,

    AIO_RX_RESP_UPLOAD_TYPE_ID   = 0x30,//调试功能
    AIO_RX_RESP_THRESHOLD_ID     = 0x31,
    AIO_RESP_CHANNEL_SEL_ID      = 0x32,
    AIO_RESP_CARRIER_SW_ID       = 0x33,
    AIO_RESP_ASPHYXIA_TIME_ID    = 0x34,
    AIO_RX_RESP_Debug_ID         = 0x3E,//Add for Debug RESP Module
        
    AIO_TX_TEMP_REALTIME_ID      = 0xB8,
    AIO_TX_TEMP_ALARM_ID         = 0xB9,
    AIO_TX_TEMP_CAL_SW_ID        = 0xBA,
    AIO_RX_TEMP_Debug_ID         = 0x3F,//Add for Debug TEMP Module
        
    AIO_NIBP_RESLULT_ID          = 0xC0,
    AIO_TX_NIBP_REALTIME_ID      = 0xC1,
    AIO_NIBP_VERIFY_STATE_ID     = 0xC2,
    AIO_TX_NIBP_ALARM_ID         = 0xC4,
    AIO_TX_NIBP_MMHG_ID          = 0xC5,
    AIO_TX_NIBP_COUNT_DOWN_S_ID  = 0xC7,

    AIO_NIBP_START_ID            = 0x40,
    AIO_NIBP_STOP_ID             = 0x41,
    AIO_NIBP_CYCLE_ID            = 0x42,
    AIO_NIBP_VERIFY_ID           = 0x43,
    AIO_NIBP_VERIFYING_ID        = 0x44,
    AIO_NIBP_STM32_PRESS_ID      = 0x45,
    AIO_NIBP_RESET_ID            = 0x46,
    AIO_NIBP_PREPROCESS_PRESS_ID = 0x47,
    AIO_NIBP_STATIC_PRESS_ID     = 0x48,
    AIO_NIBP_VENIPUNCTURE_ID     = 0x49,
    AIO_NIBP_CONTINUED_ID        = 0x4C,
    AIO_NIBP_GAS_LEAK_ID         = 0x4D,
    AIO_RX_NIBP_Debug_ID         = 0x4F,//Add for Debug NIBP Module
    
    AIO_STM_DEBUG_ID             = 0x50,
    AIO_EEPROM_DEBUG_ID          = 0x51,

    /*----------- common Start ------------*/
    COM_SOFTWARE_VERSION_ID      = 0xE0,
    COM_SELF_CHECK_ID            = 0xE1,
    COM_TX_PowerStatus_ID        = 0xE2,
    COM_TX_AbnormalReset_ID      = 0xE3,
    COM_PATIENT_TYPE_ID          = 0xE4,
    COM_PM_WORK_MODE_ID          = 0xE5,
    /*----------- common Stop ------------*/
    
    /*----------- AIO Stop ------------*/

    SF_SPO2_UPDATE               = 0xD1,
    SF_AIO_STM_UPDATE            = 0xD2,
    SF_AIO_DSP_UPDATE            = 0xD3,
    SF_BACK_UPDATE               = 0xD4,
    SF_RECORD_UPDATE             = 0xD5,
    SF_EXPAND_UPDATE             = 0xD6,

    /*----------- Error Info Start ------------*/
    ERR_LICENSE_FAILED           = 0xF0,//U-Boot used
    SYSTEM_ERR_ID                = 0xF1,
    /*----------- Error Info Stop ------------*/

    PacketID_MAX_COUNT
} PacketID_Typedef;

typedef enum
{
    SYS_ERR_MCU_UART_TX_FULL      = 0x00,
    SYS_ERR_UART_DMA_ERR,
    SYS_ERR_SPO2_UART_RX,
    
} SysErrSubID;


const BYTE UART_AIO_ADDR                = 0x55;
const BYTE UART_MCU_ADDR                = 0xAA;
const BYTE UART_SpO2_ADDR               = 0xCC;
const BYTE UART_BACK_ADDR               = 0xEE;
const BYTE UART_RECORD_ADDR             = 0xBB;
const BYTE UART_EXPAND_ADDR             = 0xDD;

//串口协议包格式定义
typedef struct 
{ 
    BYTE DR_Addr;             //目的地址 
    BYTE SR_Addr;             //源地址 
    BYTE PacketNum;           //包编号 
    BYTE PacketID;            //包标识
    BYTE Length;              //数据长度 
    BYTE DataAndCRC[251];     //数据内容和CRC校验值
} UartProtocolPacket;

const BYTE PACKET_FIXED_LENGHT          = 6;   //Packet中固有元素的总长度

typedef int (WINAPI *PktHandleFunc)(LPVOID , UartProtocolPacket *);

class CSerialProtocol
{
public:
    CSerialProtocol(void);
    ~CSerialProtocol(void);
    
    void    initApplication(void);
    int     openDevice(CWnd* pPortOwner, UINT portNr, unsigned long nBaudRate);
    int     closeDevice(void);
    int     getSerialPortsReg(char (*pListStr)[80]);
    bool    isSerialOpen(void);
    
    void    setRxPacketAddr(BYTE src, BYTE des);

    //发送 packet
    int     sendOnePacket(BYTE id, BYTE pktNum, BYTE *pData, int nDatafLen);
    void    resetSerialRxBuf(void);
	
    //Packet ID对应处理函数
    int     bindPaktFuncByID(BYTE id, LPVOID pParam, int (WINAPI *pFunc)(LPVOID pParam, UartProtocolPacket *));
    int     releasePaktFuncByID(BYTE id);
    void    Packet_NumValidCheck(void);

    void    setPacketNumValidCheck(bool newState);
protected:
    
	BOOL    startPktMonitoring(); 
	BOOL	stopPktMonitoring();
    static UINT	unPacketThread(LPVOID pParam);

    //CRC
    int     addCRC2Tail(UartProtocolPacket *pPacket);
    BYTE    getCRC(UartProtocolPacket *pPacket);
    unsigned char CRC8(unsigned char *ptr,unsigned char len);
	
    //Packet 解析
    bool    isPacketValid(UartProtocolPacket *pPacket);
    int     getSerialRxBufLen(void);
    int     getSerialRxByte(BYTE *pdata, int *pBufLen, int timeout_ms);
    void    unPacketHandle(void);
    int     getOnePacket(UartProtocolPacket *pPacket, int timeout_ms);

private:
    CSerialDrive        m_SerialDriver;
    bool                m_bSerialOpen;
    
	CWinThread*	        m_unPacketThread;
    bool                m_bPktThreadRun;
    bool                m_bIsNeedCheckIDValid;

    BYTE                m_RxPktSrcAddr;
    BYTE                m_RxPktDesAddr;
    PktHandleFunc       m_pPktHandleFunc[PacketID_MAX_COUNT];
    LPVOID              m_pPktHandleParam[PacketID_MAX_COUNT];
    UartProtocolPacket  m_rxPkt;
    BYTE                m_pkt_num_array[PacketID_MAX_COUNT];
};
