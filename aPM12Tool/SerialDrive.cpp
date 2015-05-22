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

    m_hSerial = CreateFile(szPort,					    //�������ƻ��豸·��
		GENERIC_READ | GENERIC_WRITE,					//��д��ʽ
		0,												//����ʽ����ռ
		NULL,											//Ĭ�ϵİ�ȫ������
		OPEN_EXISTING,									//������ʽ
		FILE_ATTRIBUTE_NORMAL,							//FILE_FLAG_OVERLAPPED�첽��ʽ��FILE_ATTRIBUTE_NORMALͬ����ʽ
		NULL);											//�������ģ���ļ�

	if(INVALID_HANDLE_VALUE == m_hSerial)
	{
		ProcessErrorMessage("CreateFile()");
		return false;									//�򿪴���ʧ��
	}

	DCB dcb;											//���ڿ��ƿ�
	if(!GetCommState(m_hSerial, &dcb))
	{
		ProcessErrorMessage("GetCommState()");
		CloseSerialPort();
		return false;									//��ȡDCBʧ��
	}
	dcb.BaudRate = nBaudRate;
	dcb.ByteSize = nByteSize;
	dcb.Parity = nParity;
	dcb.StopBits = nStopBits;
	SetCommState(m_hSerial, &dcb);						//����DCB

	COMMTIMEOUTS timeouts;								//���ڳ�ʱ���Ʋ���
	timeouts.ReadIntervalTimeout = 1;					//���ַ������ʱʱ��:1ms
	timeouts.ReadTotalTimeoutMultiplier = 0;			//������ʱÿ�ַ���ʱ��:1ms (n���ַ��ܹ�Ϊnms)
	timeouts.ReadTotalTimeoutConstant = 2;				//������(�����)����ʱʱ��:5ms
	timeouts.WriteTotalTimeoutMultiplier = 1;			//д����ʱÿ�ַ���ʱ��:1ms (n���ַ��ܹ�Ϊnms)
	timeouts.WriteTotalTimeoutConstant = 100;			//������(�����)д��ʱʱ��:100ms
	SetCommTimeouts(m_hSerial, &timeouts);				//���ó�ʱ

	SetupComm(m_hSerial, IN_QUEUE_SIZE, OUT_QUEUE_SIZE);	//�������������������С
	
														//PURGE_TXABORT	�ж�����д�������������أ���ʹд������û����ɡ�
														//PURGE_RXABORT	�ж����ж��������������أ���ʹ��������û����ɡ�
														//PURGE_TXCLEAR	������������
														//PURGE_RXCLEAR	������뻺����
	PurgeComm(m_hSerial,PURGE_TXCLEAR|PURGE_RXCLEAR);		//��ջ�����

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

/* ������ : getSerialPortsReg
*  ˵  �� : ͨ��ע���ö��ϵͳ�������������г����д��ڵ�ϵͳ��
*
*  ��  �� : pListStr, ���������б�
*  ����ֵ : int, ϵͳ���ڸ���
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
        for (int i=0; i < rLen; i++)//�������
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
