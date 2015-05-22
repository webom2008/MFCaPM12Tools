#include "stdafx.h"
#include "aPM12Tool.h"
#include "SerialDrive.h"

const unsigned int IN_QUEUE_SIZE = 10240;
const unsigned int OUT_QUEUE_SIZE = 10240;

CSerialDrive::CSerialDrive(void)
{
    m_hSerial = NULL;
	m_bSerialOpen = false;
}

CSerialDrive::~CSerialDrive(void)
{

}

bool CSerialDrive::OpenSerialPort(CWnd* pOwner,
                    UINT portNr,
                    unsigned long nBaudRate,
                    unsigned char nByteSize,
                    unsigned char nParity,
                    unsigned char nStopBits)
{
    char *szPort = new char[50];
    sprintf(szPort, "COM%d", portNr);

    m_hSerial = CreateFile(szPort,					    //串口名称或设备路径
		GENERIC_READ | GENERIC_WRITE,					//读写方式
		0,												//共享方式：独占
		NULL,											//默认的安全描述符
		OPEN_EXISTING,									//创建方式
		FILE_ATTRIBUTE_NORMAL,							//FILE_FLAG_OVERLAPPED异步方式，FILE_ATTRIBUTE_NORMAL同步方式
		NULL);											//不需参照模板文件

	if(INVALID_HANDLE_VALUE == m_hSerial)
	{
		ProcessErrorMessage("CreateFile()");
		return false;									//打开串口失败
	}

	DCB dcb;											//串口控制块
	if(!GetCommState(m_hSerial, &dcb))
	{
		ProcessErrorMessage("GetCommState()");
		CloseSerialPort();
		return false;									//获取DCB失败
	}
	dcb.BaudRate = nBaudRate;
	dcb.ByteSize = nByteSize;
	dcb.Parity = nParity;
	dcb.StopBits = nStopBits;
	SetCommState(m_hSerial, &dcb);						//设置DCB

	COMMTIMEOUTS timeouts;								//串口超时控制参数
	timeouts.ReadIntervalTimeout = 1;					//读字符间隔超时时间:1ms
	timeouts.ReadTotalTimeoutMultiplier = 0;			//读操作时每字符的时间:1ms (n个字符总共为nms)
	timeouts.ReadTotalTimeoutConstant = 2;				//基本的(额外的)读超时时间:5ms
	timeouts.WriteTotalTimeoutMultiplier = 1;			//写操作时每字符的时间:1ms (n个字符总共为nms)
	timeouts.WriteTotalTimeoutConstant = 100;			//基本的(额外的)写超时时间:100ms
	SetCommTimeouts(m_hSerial, &timeouts);				//设置超时

	SetupComm(m_hSerial, IN_QUEUE_SIZE, OUT_QUEUE_SIZE);	//设置输入输出缓冲区大小
	
														//PURGE_TXABORT	中断所有写操作并立即返回，即使写操作还没有完成。
														//PURGE_RXABORT	中断所有读操作并立即返回，即使读操作还没有完成。
														//PURGE_TXCLEAR	清除输出缓冲区
														//PURGE_RXCLEAR	清除输入缓冲区
	PurgeComm(m_hSerial,PURGE_TXCLEAR|PURGE_RXCLEAR);		//清空缓冲区

    m_pDriveOwner = pOwner;
    
    // initialize critical section   
    InitializeCriticalSection(&m_csDriverSync);   

	m_bSerialOpen = true;

    RxBuffeerReset();

	return true;
}

bool CSerialDrive::CloseSerialPort()
{
	if(NULL != m_hSerial)
	{
		if(!CloseHandle(m_hSerial))
		{
            ProcessErrorMessage("CloseHandle()");
			return false;
		}
		m_hSerial = NULL;
		m_bSerialOpen = false;
	}
	return true;
}

/* 函数名 : getSerialPortsReg
*  说  明 : 通过注册表枚举系统串口数量，并列出所有串口的系统名
*
*  参  数 : pListStr, 串口名称列表
*  返回值 : int, 系统串口个数
*/
int     CSerialDrive::getSerialPortsReg(char (*pListStr)[80])
{
    HKEY hKey;
    LPCTSTR data_Set=_T("HARDWARE\\DEVICEMAP\\SERIALCOMM\\");
    long ret0 = (::RegOpenKeyEx(HKEY_LOCAL_MACHINE, data_Set, 0, KEY_READ, &hKey));

    if(ret0 != ERROR_SUCCESS)
    {
        return -1;
    }

    int i = 0;
	TCHAR Name[25]={0,};
	UCHAR szPortName[80]={0,};
    long Status;

    DWORD dwIndex = 0;
    DWORD dwName;
    DWORD dwSizeofPortName;
    DWORD Type;

    dwName = sizeof(Name);
    dwSizeofPortName = sizeof(szPortName);

    do
    {
        Status = RegEnumValue(hKey, dwIndex++, Name, &dwName, NULL, &Type,
            szPortName, &dwSizeofPortName);

        if((Status == ERROR_SUCCESS)||(Status == ERROR_MORE_DATA))
        {
            if (pListStr != NULL)
            {
                memcpy(*(pListStr + i), (LPCSTR)szPortName, 80);
            }
            i++;
        }
    } while((Status == ERROR_SUCCESS)||(Status == ERROR_MORE_DATA));

    RegCloseKey(hKey);

    return i;
}

  
//   
// If there is a error, give the right message   
//   
void CSerialDrive::ProcessErrorMessage(char* ErrorText)   
{   
    char *Temp = new char[200];   
       
    LPVOID lpMsgBuf;   
   
    FormatMessage(    
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,   
        NULL,   
        GetLastError(),   
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language   
        (LPTSTR) &lpMsgBuf,   
        0,   
        NULL    
    );   
   
    sprintf(Temp, "WARNING:  %s Failed with the following error: \n%s\n", (char*)ErrorText, lpMsgBuf);    
    MessageBox(NULL, Temp, "Application Error", MB_ICONSTOP);   
   
    LocalFree(lpMsgBuf);   
    delete [] Temp;   
} 

int CSerialDrive::ReadSerial(unsigned char* pData, unsigned long nLength)
{
	int readSize = 0;

    if(NULL == m_hSerial) return -1;

	if(!ReadFile(m_hSerial, pData, nLength, (DWORD *)(&readSize), NULL))
	{
		ERROR_INFO("\r\n>>ReadFile Error!");
		CloseSerialPort();
		return -1;
	}
	return readSize;
}

int CSerialDrive::WriteSerial(const unsigned char* pData, unsigned long nLength)
{
	int writeSize = 0;

    if(NULL == m_hSerial) return -1;

	if(!WriteFile(m_hSerial, pData, nLength, (DWORD *)(&writeSize), NULL))
	{
		ERROR_INFO("\r\n>>WriteFile Error!");
		CloseSerialPort();
		return -1;
	}
	return writeSize;
}

int CSerialDrive::StartMonitoring(void)
{
	if (!(m_Thread = AfxBeginThread(SerialReceiveThread, this)))
    {
		return 0;
    }
	return 1;	
}

int CSerialDrive::StopMonitoring(void)
{
    m_bThreadRuning = false;
	return 1;	
}


#define COMMTHREAD_BUF_LEN      4096


#define BUF_OK 0
#define BUF_FULL 1
#define BUF_EMPTY 2
static BYTE ReceiveBuffer[IN_QUEUE_SIZE];;

typedef struct _CIRCULARBUFFER
{
    BYTE *rx_buf;
    int rx_len_max;
    int rx_head;
    int rx_tail;

}CIRCULARBUFFER;

static CIRCULARBUFFER gCirCularBuffer;
static CIRCULARBUFFER *gpCirCularBuffer = &gCirCularBuffer;

static int BufferInit(CIRCULARBUFFER *pDev)
{
    pDev->rx_buf = ReceiveBuffer;
    pDev->rx_head = 0;
    pDev->rx_tail = 0;
    pDev->rx_len_max = IN_QUEUE_SIZE;
    
    return 0;
}

static int isBufferFull(const CIRCULARBUFFER *pDev)
{
    if((pDev->rx_head+1)%(pDev->rx_len_max) == pDev->rx_tail)
    {
        return BUF_FULL;
    }
    
    return BUF_OK;
}

static int isBufferEmpty(const CIRCULARBUFFER *pDev)
{
    if(pDev->rx_head != pDev->rx_tail)
    {
        return BUF_OK;
    }
    return BUF_EMPTY;
}

static int BufferEnqueue(CIRCULARBUFFER *pDev,const BYTE data)
{
    if(BUF_FULL == isBufferFull(pDev))
    {
        return BUF_FULL;
    }
    
    pDev->rx_buf[pDev->rx_head] = data;
    pDev->rx_head = (pDev->rx_head+1)%(pDev->rx_len_max);

    return BUF_OK;
}

static int BufferDequeue(CIRCULARBUFFER *pDev,BYTE *pBuf)
{
    if(BUF_EMPTY == isBufferEmpty(pDev))
    {
        return BUF_EMPTY;
    }

    *pBuf = pDev->rx_buf[pDev->rx_tail];
    pDev->rx_tail = (pDev->rx_tail+1) % (pDev->rx_len_max);
        
    return BUF_OK;
}



//
//  The SerialReceiveThread Function.
//
UINT CSerialDrive::SerialReceiveThread(LPVOID pParam)
{
    CSerialDrive *pCOM = (CSerialDrive *)pParam;
    pCOM->m_bThreadRuning = true;
    unsigned char pBuf[COMMTHREAD_BUF_LEN];
    int rLen;
    
	TRACE("SerialReceiveThread started\n");
    BufferInit(gpCirCularBuffer);
    while(pCOM->m_bThreadRuning)
    {
        memset(pBuf, 0, sizeof(pBuf));
        rLen = pCOM->ReadSerial(pBuf,COMMTHREAD_BUF_LEN);

        EnterCriticalSection(&pCOM->m_csDriverSync); // now it critical!  
        for (int i=0; i < rLen; i++)//加入队列
        {
            BufferEnqueue(gpCirCularBuffer,pBuf[i]); 
        }
        LeaveCriticalSection(&pCOM->m_csDriverSync); // release critical section  

        Sleep(1);
    }
	TRACE("SerialReceiveThread Stopted\n");
    DWORD dwExitCode;
    GetExitCodeThread( pCOM->m_Thread->m_hThread, &dwExitCode );
    AfxEndThread(dwExitCode, TRUE);
    return 0;
}


int CSerialDrive::RxGetDataLen(void)
{
    int len = gpCirCularBuffer->rx_head - gpCirCularBuffer->rx_tail;
    if (len < 0) len += gpCirCularBuffer->rx_len_max;
    return len;   
}

/*
 * Breif    :pop out data from buffer
 * Return   :>0 -- pop success; =0 -- not data;
 */
 int CSerialDrive::RxPopResult(unsigned char *pChar)
{
    int res;
//    EnterCriticalSection(&m_csDriverSync); // now it critical!  
    res = BufferDequeue(gpCirCularBuffer,pChar);
//    LeaveCriticalSection(&m_csDriverSync); // release critical section 

    if (BUF_OK == res)
    {
        return 1;
    }
    return 0;
}

void CSerialDrive::RxBuffeerReset(void)
{
    gpCirCularBuffer->rx_tail = gpCirCularBuffer->rx_head;
}
