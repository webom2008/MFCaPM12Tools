// PageUpdate.cpp : 实现文件
//

#include "stdafx.h"
#include "aPM12Tool.h"
#include "PageUpdate.h"
#include "afxdialogex.h"

#include <iostream>

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

// CPageUpdate 对话框

IMPLEMENT_DYNAMIC(CPageUpdate, CPropertyPage)

CPageUpdate::CPageUpdate()
	: CPropertyPage(CPageUpdate::IDD)
{
	m_hGetUpdatePacketEvent = NULL;
    m_bUpdateThreadRun = false;
    m_UpdateThread = NULL;

    // initialize critical section   
    InitializeCriticalSection(&m_csCommunicationSync); 
}

CPageUpdate::~CPageUpdate()
{
	
    if(NULL != m_hGetUpdatePacketEvent)
	{
        CloseHandle( m_hGetUpdatePacketEvent ); 
	}   
}

void CPageUpdate::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_CBO_UPDATE_TARGET_SEL, m_TargtSelect);
    DDX_Control(pDX, IDC_EDIT_PATH, m_filePath);
    DDX_Control(pDX, IDC_EDIT_AIO_VERSION, m_EditVersion);
}


BEGIN_MESSAGE_MAP(CPageUpdate, CPropertyPage)
    ON_BN_CLICKED(IDC_BTN_PATH_SEL, &CPageUpdate::OnBnClickedBtnPathSel)
    ON_BN_CLICKED(IDC_BTN_UPDATE, &CPageUpdate::OnBnClickedBtnUpdate)
    ON_CBN_SELCHANGE(IDC_CBO_UPDATE_TARGET_SEL, &CPageUpdate::OnCbnSelchangeCboUpdateTargetSel)
    ON_BN_CLICKED(IDC_BTN_AIO_VERSION, &CPageUpdate::OnBnClickedBtnAioVersion)
END_MESSAGE_MAP()


// CPageUpdate 消息处理程序

BOOL CPageUpdate::OnInitDialog()
{
    CPropertyPage::OnInitDialog();

    m_TargtSelect.InsertString(0,"AIO-DSP");
    m_TargtSelect.InsertString(1,"AIO-STM");
    m_TargtSelect.InsertString(2,"SPO2");
    m_TargtSelect.SetCurSel(0);

	
    if (NULL != m_hGetUpdatePacketEvent)
	{
        ResetEvent(m_hGetUpdatePacketEvent);
	}   
    else   
	{
        m_hGetUpdatePacketEvent = CreateEvent(NULL, TRUE, FALSE, NULL); 
	} 

    m_BChildID = UPDATE_CID_AIO_DSP;
    return TRUE;
}

void CPageUpdate::initApplication(void)
{
    g_pSerialProtocol->bindPaktFuncByID(SF_SPO2_UPDATE ,this, CPageUpdate::PktHandleUpdateSoftware);
    g_pSerialProtocol->bindPaktFuncByID(SF_AIO_STM_UPDATE ,this, CPageUpdate::PktHandleUpdateSoftware);
    g_pSerialProtocol->bindPaktFuncByID(SF_AIO_DSP_UPDATE ,this, CPageUpdate::PktHandleUpdateSoftware);
    g_pSerialProtocol->bindPaktFuncByID(COM_SOFTWARE_VERSION_ID ,this, CPageUpdate::PktHandleGetVersion);
}

int WINAPI CPageUpdate::PktHandleUpdateSoftware(LPVOID pParam, UartProtocolPacket *pPacket)
{
    CPageUpdate *pPageUpdate = (CPageUpdate *)pParam;
	if (pPageUpdate->m_BChildID == pPacket->PacketID)
	{
        if ((BYTE)UPDATE_NAK == pPacket->DataAndCRC[0])
        {
            INFO(".");
        }
        EnterCriticalSection(&pPageUpdate->m_csCommunicationSync); 
		pPageUpdate->m_PktForUpdate = *pPacket;
		SetEvent(pPageUpdate->m_hGetUpdatePacketEvent);
        LeaveCriticalSection(&pPageUpdate->m_csCommunicationSync); 
		return 1;
	}
	else
	{
        EnterCriticalSection(&pPageUpdate->m_csCommunicationSync);
		ResetEvent(pPageUpdate->m_hGetUpdatePacketEvent);
        LeaveCriticalSection(&pPageUpdate->m_csCommunicationSync); 
		return 0;
	}
}

int WINAPI CPageUpdate::PktHandleGetVersion(LPVOID pParam, UartProtocolPacket *pPacket)
{
    CPageUpdate *pPageUpdate = (CPageUpdate *)pParam;
    pPageUpdate->setSoftwareVersion((char *)&pPacket->DataAndCRC[0]);
    return 0;
}

void CPageUpdate::setSoftwareVersion(char *pVersion)
{
    m_EditVersion.SetWindowTextA(pVersion);
}

void CPageUpdate::OnBnClickedBtnPathSel()
{
    CFileDialog  FDlg(  TRUE ,
                        NULL,
                        NULL ,
                        OFN_HIDEREADONLY ,
                        _T("文件|*.bin;*.ldr|所有文件(*.*) |*.*||"),
                        this);
    if(FDlg.DoModal() == IDOK)
    {
        CString  filePath = FDlg.GetPathName();
        UpdateData(false);
        filePath.Replace(_T("//"),_T("////"));
        m_filePath.SetWindowText(filePath);
        UpdateData(FALSE);//更新编辑框内容
    }
}


/*
 * brief    :   Get File and Save to RAM
 * param    :   
 *              unsigned int *pFileLen: the lenght of the file.
 * return   :   0 -- OK
 *              -1 -- ERROR
 */
int CPageUpdate::SaveFiletoRAM(unsigned int *pFileLen)
{
    //文件的读取至内存
    CString  filePath;
    m_filePath.GetWindowText(filePath);
    
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
void CPageUpdate::DisplayOKorError(int state)
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
int CPageUpdate::SendResetAndUpdateTag(void)
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
int CPageUpdate::SendUpdateStartOfLenght(const unsigned int file_len)
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
 * return   :   1 -- success
 *              0 -- fail
 */
int CPageUpdate::SendUpdateStartOfData(const unsigned int file_len)
{
	BYTE	pBuffer[130];
    DWORD	Event;
    unsigned int i;
	int		nLen = 0;
    m_packetCount = 1;

    for (i = 0; (i < 10) && (m_Mem_addr_offset < file_len);)
    {
	    //S1:封装Packet包
        unsigned int not_w_len = file_len - m_Mem_addr_offset;
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
		        if (UPDATE_CID_AIO_STM == m_BChildID)
		        {
			        Sleep(50);
		        }
				continue; 
			}
			else if ((BYTE)UPDATE_SOD == m_PktForUpdate.DataAndCRC[0]) //resend again.
			{
				m_packetCount = ((m_packetCount>>8)<<8)+m_PktForUpdate.DataAndCRC[1];
				m_Mem_addr_offset = (m_packetCount-1) * UPDATE_PACKET_DATA_LEN;
				INFO("R");
				if (UPDATE_CID_AIO_STM == m_BChildID)
				{
					Sleep(50);
				}
				continue;
			}
			else if ((BYTE)UPDATE_CA == m_PktForUpdate.DataAndCRC[0]) //lenght out of flash
			{
				return 0;
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

    if (m_Mem_addr_offset >= file_len)
    {
        return 1;
    }
    //超时
	return 0;
}

/*
 * brief    :   
 * return   :   1 -- success
 *              0 -- fail
 */
int CPageUpdate::SendUpdateEndOfTransmit(void)
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
int CPageUpdate::WaitUpdateWrite2FlashDone(void)
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

UINT CPageUpdate::UpdateThread(LPVOID pParam)
{
    int i;
    unsigned int len;   //bin file lenght
    int result;
    CPageUpdate *pPageUpdate = (CPageUpdate *)pParam;

    pPageUpdate->m_bUpdateThreadRun = true;

    INFO("正准备软件升级...\r\n");

    //>>>>>>>>S1:COM handle
	//判断串口是否打开
    if(false == g_pSerialProtocol->isSerialOpen())//串口已经打开，则进行关闭操作
	{
        INFO("错误:请先打开串口\r\n");
        pPageUpdate->m_bUpdateThreadRun = false;
		return 0;
	}

    //>>>>>>>>S2:File handle
    if (pPageUpdate->SaveFiletoRAM(&len) < 0)
    {
        INFO("错误:文件拷贝至内存失败\r\n");
        pPageUpdate->m_bUpdateThreadRun = false;
		return 0;
    }
    else
    {
        INFO("提示:文件长度 = %d  字节\r\n", len);
    }

    //>>>>>>>>S3:Send Update Tag, Target ready to download.
    i = 5;
    INFO("(1/5)等待目标板确认包...");
    while(--i)
    {
        if (pPageUpdate->SendResetAndUpdateTag()) break;
    }
    pPageUpdate->DisplayOKorError(i);
    if (i == 0)
    {
        pPageUpdate->m_bUpdateThreadRun = false;
		return 0;
    }

    //>>>>>>>>S4:Send UPDATE_SOL
    i = 5;
    INFO("(2/5)等待文件长度确认包...");
    while(--i)
    {
        result = pPageUpdate->SendUpdateStartOfLenght(len);
        if (0 != result) break;
    }
    if (-1 == result)
    {
        INFO("Error(文件大小超出Flash范围)\r\n");
        pPageUpdate->m_bUpdateThreadRun = false;
		return 0;
    }
    pPageUpdate->DisplayOKorError(i);
    if (i == 0)
    {
        pPageUpdate->m_bUpdateThreadRun = false;
		return 0;
    }
    
    INFO("(3/5)正在传送数据:\r\n");
    i = pPageUpdate->SendUpdateStartOfData(len);
    INFO("\r\n(3/5)正在传送数据结果:");
    pPageUpdate->DisplayOKorError(i);
    if (i == 0)
    {
        pPageUpdate->m_bUpdateThreadRun = false;
		return 0;
    }

    //>>>>>>>>S6:Send UPDATE_EOT
    i = 5;
    INFO("(4/5)等待文件结束确认包...");
    while(--i)
    {
        if (pPageUpdate->SendUpdateEndOfTransmit()) break;
    }
    pPageUpdate->DisplayOKorError(i);
    if (i == 0)
    {
        pPageUpdate->m_bUpdateThreadRun = false;
		return 0;
    }
    
    INFO("(5/5)等待烧写进Flash:");
    for (i = 0; i < 1000; i++)
    {  
        result = pPageUpdate->WaitUpdateWrite2FlashDone();
        if (0 == result) 
        {
            INFO("OK.\r\n");
            break;
        }
        else if (-1 == result)
        {
            INFO("Error.\r\n");
            break;
        }
        INFO(".");
        if (i % 50 == 0)
        {
            INFO("\r\n");
        }
    }
    if (1000 == i)
    {
        ERROR_INFO("超时\r\n");
    }
    pPageUpdate->m_bUpdateThreadRun = false;
	return 0;
}











void CPageUpdate::OnBnClickedBtnUpdate()
{
    if (!m_bUpdateThreadRun)
    {
        m_UpdateThread = AfxBeginThread(UpdateThread, this);
        if (NULL == m_UpdateThread)
        {
            ERROR_INFO("升级失败:创建线程ERROR");
        }
    }
    else
    {
        WARNING("升级进行中");
    }
}

void CPageUpdate::OnCbnSelchangeCboUpdateTargetSel()
{
    int index = 0;
	CString str;
	index = m_TargtSelect.GetCurSel();
	m_TargtSelect.GetLBText( index, str);
    MSG("升级板卡选择:%s\r\n",str);
    switch(index)
    {
    case 0:
        m_BChildID = UPDATE_CID_AIO_DSP;
        break;
    case 1:
        m_BChildID = UPDATE_CID_AIO_STM;
        break;
    case 2:
        m_BChildID = UPDATE_CID_SPO2;
        break;
    default:
        break;
    }
}


void CPageUpdate::OnBnClickedBtnAioVersion()
{
    BYTE id;
    
    id = 0xE0;
    if (g_pSerialProtocol->isSerialOpen())
    {
        m_EditVersion.SetWindowTextA("");
        UpdateData(FALSE);//更新编辑框内容
        g_pSerialProtocol->sendOnePacket(id, 0, NULL, 0);
    }
    else
    {
        MSG("请确保正确配置串口\r\n");
    }
}
