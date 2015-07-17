// PageUpdate.cpp : ʵ���ļ�
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

// CPageUpdate �Ի���

IMPLEMENT_DYNAMIC(CPageUpdate, CPropertyPage)

    CPageUpdate::CPageUpdate()
    : CPropertyPage(CPageUpdate::IDD)
{
    m_bUpdateThreadRun = false;
    m_UpdateThread = NULL;

}

CPageUpdate::~CPageUpdate()
{ 
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


// CPageUpdate ��Ϣ�������

BOOL CPageUpdate::OnInitDialog()
{
    CPropertyPage::OnInitDialog();

    m_TargtSelect.InsertString(0,"AIO-DSP");
    m_TargtSelect.InsertString(1,"AIO-STM");
    m_TargtSelect.InsertString(2,"SPO2");
    m_TargtSelect.InsertString(3,"����");
    m_TargtSelect.InsertString(4,"��¼��");
    m_TargtSelect.InsertString(5,"��չģ��");
    m_TargtSelect.SetCurSel(0);

    return TRUE;
}

void CPageUpdate::initApplication(void)
{
    CUpdate::initApplication();

    g_pSerialProtocol->bindPaktFuncByID(COM_SOFTWARE_VERSION_ID ,this, CPageUpdate::PktHandleGetVersion);
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
        _T("�ļ�|*.bin;*.ldr|�����ļ�(*.*) |*.*||"),
        this);
    if(FDlg.DoModal() == IDOK)
    {
        CString  filePath = FDlg.GetPathName();
        UpdateData(false);
        filePath.Replace(_T("//"),_T("////"));
        m_filePath.SetWindowText(filePath);
        UpdateData(FALSE);//���±༭������
    }
}

UINT CPageUpdate::UpdateThread(LPVOID pParam)
{
    int i;
    unsigned int len;   //bin file lenght
    int result;
    CPageUpdate *pPageUpdate = (CPageUpdate *)pParam;
    CString  filePath;

    pPageUpdate->m_bUpdateThreadRun = true;

    INFO("��׼���������...\r\n");

    //>>>>>>>>S1:COM handle
    //�жϴ����Ƿ��
    if(false == g_pSerialProtocol->isSerialOpen())//�����Ѿ��򿪣�����йرղ���
    {
        INFO("����:���ȴ򿪴���\r\n");
        pPageUpdate->m_bUpdateThreadRun = false;
        return 0;
    }

    //>>>>>>>>S2:File handle
    pPageUpdate->m_filePath.GetWindowText(filePath);
    if (pPageUpdate->SaveFiletoRAM(&len, filePath) < 0)
    {
        INFO("����:�ļ��������ڴ�ʧ��\r\n");
        pPageUpdate->m_bUpdateThreadRun = false;
        return 0;
    }
    else
    {
        INFO("��ʾ:�ļ����� = %d  �ֽ�\r\n", len);
    }

    //>>>>>>>>S3:Send Update Tag, Target ready to download.
    i = 5;
    INFO("(1/5)�ȴ�Ŀ���ȷ�ϰ�...");
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
    INFO("(2/5)�ȴ��ļ�����ȷ�ϰ�...");
    while(--i)
    {
        result = pPageUpdate->SendUpdateStartOfLenght(len);
        if (0 != result) break;
    }
    if (-1 == result)
    {
        INFO("Error(�ļ���С����Flash��Χ)\r\n");
        pPageUpdate->m_bUpdateThreadRun = false;
        return 0;
    }
    pPageUpdate->DisplayOKorError(i);
    if (i == 0)
    {
        pPageUpdate->m_bUpdateThreadRun = false;
        return 0;
    }

    INFO("(3/5)���ڴ�������:\r\n");
    do 
    {
        result = pPageUpdate->SendUpdateStartOfData(len);
    } while(result > 0);

    INFO("\r\n(3/5)���ڴ������ݽ��:");
    if (0 == result)
    {
        pPageUpdate->DisplayOKorError(1);
    }
    else 
    {
        pPageUpdate->DisplayOKorError(0);
        if (-1 == result)
        {
            ERROR_INFO("Ŀ����쳣��ֹ");
        }
        else if (-2 == result)
        {
            ERROR_INFO("�ȴ���Ӧ��ʱ!");
        }
        pPageUpdate->m_bUpdateThreadRun = false;
        return 0;
    }

    //>>>>>>>>S6:Send UPDATE_EOT
    i = 5;
    INFO("(4/5)�ȴ��ļ�����ȷ�ϰ�...");
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

    INFO("(5/5)�ȴ���д��Flash:");
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
        ERROR_INFO("��ʱ\r\n");
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
            ERROR_INFO("����ʧ��:�����߳�ERROR");
        }
    }
    else
    {
        WARNING("����������");
    }
}

void CPageUpdate::OnCbnSelchangeCboUpdateTargetSel()
{
    int index = 0;
    CString str;
    index = m_TargtSelect.GetCurSel();
    m_TargtSelect.GetLBText( index, str);
    MSG("�����忨ѡ��:%s\r\n",str);
    switch(index)
    {
    case 0:
        setPacketCID((BYTE)SF_AIO_DSP_UPDATE);
        break;
    case 1:
        setPacketCID((BYTE)SF_AIO_STM_UPDATE);
        break;
    case 2:
        setPacketCID((BYTE)SF_SPO2_UPDATE);
        break;
    case 3:
        setPacketCID((BYTE)SF_BACK_UPDATE);
        break;
    case 4:
        setPacketCID((BYTE)SF_RECORD_UPDATE);
        break;
    case 5:
        setPacketCID((BYTE)SF_EXPAND_UPDATE);
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
        UpdateData(FALSE);//���±༭������
        g_pSerialProtocol->sendOnePacket(id, 0, NULL, 0);
    }
    else
    {
        MSG("��ȷ����ȷ���ô���\r\n");
    }
}
