// PageSmartUpdate.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "aPM12Tool.h"
#include "PageSmartUpdate.h"
#include "afxdialogex.h"
#include "FileIO.h"

extern CSerialProtocol *g_pSerialProtocol;
static const char AIO_DSP_APP_BIN_NAME[] = "aPM12_AIO_DSPAPP*.ldr";
static const char AIO_STM_APP_BIN_NAME[] = "aPM12_AIO_STMAPP*.bin";
// CPageSmartUpdate �Ի���

IMPLEMENT_DYNAMIC(CPageSmartUpdate, CPropertyPage)

CPageSmartUpdate::CPageSmartUpdate()
	: CPropertyPage(CPageSmartUpdate::IDD)
{
    m_AioBinFileCount   = 0;
    m_bUpdateThreadRun  = false;
    m_UpdateThread      = NULL;
}

CPageSmartUpdate::~CPageSmartUpdate()
{
}

void CPageSmartUpdate::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_PROGRESS_AIO, m_ProgressAIO);
    DDX_Control(pDX, IDC_EDIT_UPDATE_DISPLAY, m_EditUpdateDisplay);
}


BEGIN_MESSAGE_MAP(CPageSmartUpdate, CPropertyPage)
    ON_BN_CLICKED(IDC_BTN_AIO_UPDATE, &CPageSmartUpdate::OnBnClickedBtnAioUpdate)
    ON_BN_CLICKED(IDC_BTN_AM335X_UPDATE, &CPageSmartUpdate::OnBnClickedBtnAm335xUpdate)
END_MESSAGE_MAP()


// CPageSmartUpdate ��Ϣ�������

void CPageSmartUpdate::initApplication(void)
{
    CUpdate::initApplication();
}

/* 
 * �������ܣ� ���BIN�ļ��Ƿ����
 * ���������
 * ��������� 
 * �� �� ֵ�� BIN�ļ��ĸ���
 */  
BYTE CPageSmartUpdate::detectBinFile(void)
{
    CStringArray strArrayFileNames;
    LPCTSTR file;
    BYTE ID;
    CString name;
    BYTE count = 0;

    //========= AIO-DSP DETECT ==============
    file = AIO_DSP_APP_BIN_NAME;
    CFileIO::GetFileNamesInDir(strArrayFileNames, file);
    if (strArrayFileNames.GetSize() > 0)
    {
        ID = SF_AIO_DSP_UPDATE;
        name = strArrayFileNames.GetAt(0); // ֻ��ȡ��һ��
        strArrayFileNames.RemoveAll();
        m_Target.insert(std::map<BYTE, CString>::value_type(ID, name));
        count++;
    }

    //========= AIO-STM DETECT ==============
    file = AIO_STM_APP_BIN_NAME;
    CFileIO::GetFileNamesInDir(strArrayFileNames, file);
    if (strArrayFileNames.GetSize() > 0)
    {
        ID = SF_AIO_STM_UPDATE;
        name = strArrayFileNames.GetAt(0); // ֻ��ȡ��һ��
        strArrayFileNames.RemoveAll();
        m_Target.insert(std::map<BYTE, CString>::value_type(ID, name));
        count++;
    }

    return count;
}

int CPageSmartUpdate::checkTarget(BYTE &CID, CString &path)
{
    std::map<BYTE, CString>::iterator it;
    while((it = this->m_Target.find(SF_AIO_DSP_UPDATE)) != this->m_Target.end())
    {
        CID = it->first;
        path = it->second;
        this->m_Target.erase(it);
        return 0;
    }
    while((it = this->m_Target.find(SF_AIO_STM_UPDATE)) != this->m_Target.end())
    {
        CID = it->first;
        path = it->second;
        this->m_Target.erase(it);
        return 0;
    }
    return -1;
}
void CPageSmartUpdate::displayProgressAIO(BYTE index, int value)
{
    CString str;
    int process = 0;
    if (m_AioBinFileCount)
    {
        process = (index * 100 + value)/ m_AioBinFileCount;
    }
    m_ProgressAIO.SetPos(process);
    str.Format("%d",process);
    str += _T("%");
    GetDlgItem(IDC_STATIC_AIO_PROCESS)->SetWindowTextA(str);
}

UINT CPageSmartUpdate::UpdateThread(LPVOID pParam)
{
    int i;
    unsigned int len;   //bin file lenght
    int result;
    CPageSmartUpdate *pSmartUpdate = (CPageSmartUpdate *)pParam;
    CString name, str;
    BYTE CID;
    BYTE index = 0;

    pSmartUpdate->m_bUpdateThreadRun = true;

    INFO("��׼���������...\r\n");
    //>>>>>>>>S1:COM handle
    //�жϴ����Ƿ��
    if(false == g_pSerialProtocol->isSerialOpen())//�����Ѿ��򿪣�����йرղ���
    {
        INFO("����:���ȴ򿪴���\r\n");
        pSmartUpdate->addInfo2Display(_T("����:���ȴ򿪴���\r\n"));
        pSmartUpdate->m_bUpdateThreadRun = false;
        pSmartUpdate->GetDlgItem(IDC_BTN_AIO_UPDATE)->SetWindowTextA("AIO������");
        return 0;
    }
    
    str.Format("��ʾ:��⵽%d�������ļ�,���ڽ�����...\r\n", pSmartUpdate->m_AioBinFileCount);
    pSmartUpdate->addInfo2Display(str);

    while (!pSmartUpdate->checkTarget(CID,name))
    {
        if (CID != pSmartUpdate->getPacketCID())
        {
            pSmartUpdate->setPacketCID(CID);
        }
        if (pSmartUpdate->SaveFiletoRAM(&len, name) < 0)
        {
            INFO("����:�ļ��������ڴ�ʧ��\r\n");
            pSmartUpdate->addInfo2Display(_T("����:�ļ��������ڴ�ʧ��\r\n"));
            pSmartUpdate->m_bUpdateThreadRun = false;
            pSmartUpdate->GetDlgItem(IDC_BTN_AIO_UPDATE)->SetWindowTextA("AIO������");
            return 0;
        }
        else
        {
            INFO("��ʾ:�ļ����� = %d  �ֽ�\r\n", len);
            pSmartUpdate->displayProgressAIO(index, 1);
        }

        //>>>>>>>>S3:Send Update Tag, Target ready to download.
        i = 5;
        INFO("(1/5)�ȴ�Ŀ���ȷ�ϰ�...");
        while(--i)
        {
            if (pSmartUpdate->SendResetAndUpdateTag()) break;
        }
        pSmartUpdate->DisplayOKorError(i);
        if (i == 0)
        {
            pSmartUpdate->addInfo2Display(_T("����:�ȴ�Ŀ���ȷ�ϰ�ʧ��\r\n"));
            pSmartUpdate->m_bUpdateThreadRun = false;
            pSmartUpdate->GetDlgItem(IDC_BTN_AIO_UPDATE)->SetWindowTextA("AIO������");
            return 0;
        }
        pSmartUpdate->displayProgressAIO(index, 3);

        //>>>>>>>>S4:Send UPDATE_SOL
        i = 5;
        INFO("(2/5)�ȴ��ļ�����ȷ�ϰ�...");
        while(--i)
        {
            result = pSmartUpdate->SendUpdateStartOfLenght(len);
            if (0 != result) break;
        }
        if (-1 == result)
        {
            INFO("Error(�ļ���С����Flash��Χ)\r\n");
            pSmartUpdate->addInfo2Display(_T("����:�ļ���С����Flash��Χ\r\n"));
            pSmartUpdate->m_bUpdateThreadRun = false;
            pSmartUpdate->GetDlgItem(IDC_BTN_AIO_UPDATE)->SetWindowTextA("AIO������");
            return 0;
        }
        pSmartUpdate->DisplayOKorError(i);
        if (i == 0)
        {
            pSmartUpdate->addInfo2Display(_T("����:�ȴ�Ŀ��峤��ȷ�ϰ�ʧ��\r\n"));
            pSmartUpdate->m_bUpdateThreadRun = false;
            pSmartUpdate->GetDlgItem(IDC_BTN_AIO_UPDATE)->SetWindowTextA("AIO������");
            return 0;
        }
        pSmartUpdate->displayProgressAIO(index, 5);

        INFO("(3/5)���ڴ�������:\r\n");
        do 
        {
            result = pSmartUpdate->SendUpdateStartOfData(len);
            if (result > 0)
            {
                i = 5 + 90 * (len - result)/len;
                pSmartUpdate->displayProgressAIO(index, i);
            }
        } while(result > 0);
        
        INFO("\r\n(3/5)���ڴ������ݽ��:");
        if (0 == result)
        {
            pSmartUpdate->DisplayOKorError(1);
        }
        else 
        {
            pSmartUpdate->DisplayOKorError(0);
            if (-1 == result)
            {
                ERROR_INFO("Ŀ����쳣��ֹ");
            }
            else if (-2 == result)
            {
                ERROR_INFO("�ȴ���Ӧ��ʱ!");
            }
            pSmartUpdate->addInfo2Display(_T("����:���ݴ���ʧ��\r\n"));
            pSmartUpdate->m_bUpdateThreadRun = false;
            pSmartUpdate->GetDlgItem(IDC_BTN_AIO_UPDATE)->SetWindowTextA("AIO������");
            return 0;
        }

        //>>>>>>>>S6:Send UPDATE_EOT
        i = 5;
        INFO("(4/5)�ȴ��ļ�����ȷ�ϰ�...");
        while(--i)
        {
            if (pSmartUpdate->SendUpdateEndOfTransmit()) break;
        }
        pSmartUpdate->DisplayOKorError(i);
        if (i == 0)
        {
            pSmartUpdate->addInfo2Display(_T("����:�ȴ��ļ�����ȷ�ϰ�ʧ��\r\n"));
            pSmartUpdate->m_bUpdateThreadRun = false;
            pSmartUpdate->GetDlgItem(IDC_BTN_AIO_UPDATE)->SetWindowTextA("AIO������");
            return 0;
        }
        pSmartUpdate->displayProgressAIO(index, 97);

        INFO("(5/5)�ȴ���д��Flash:");
        for (i = 0; i < 1000; i++)
        {  
            result = pSmartUpdate->WaitUpdateWrite2FlashDone();
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
        pSmartUpdate->displayProgressAIO(index, 100);
        index++;
        if (index != pSmartUpdate->m_AioBinFileCount)
        {
            MSG("׼����д��һ���ļ�!\r\n");
            Sleep(6000); //wait for reboot!
        }
    }
    
    INFO("\r\n===========================");
    INFO("\r\n===========================");
    INFO("\r\n===========================");
    INFO("\r\n>>>>>>>>��д�����<<<<<<<<<\r\n");
    pSmartUpdate->addInfo2Display(_T("\r\n>>>>��д�ɹ�<<<<\r\n"));
    pSmartUpdate->m_bUpdateThreadRun = false;
    pSmartUpdate->GetDlgItem(IDC_BTN_AIO_UPDATE)->SetWindowTextA("AIO������");
    return 0;
}


void CPageSmartUpdate::addInfo2Display(CString str)
{
	m_EditUpdateDisplay.SetSel(-1, -1);
	m_EditUpdateDisplay.ReplaceSel(str);
	//this->UpdateData(FALSE);//���±༭������
}

void CPageSmartUpdate::cleanInfo2Display(void)
{
	m_EditUpdateDisplay.SetWindowText("");
    m_EditUpdateDisplay.SetSel(-1);
	//this->UpdateData(FALSE);//���±༭������
}

void CPageSmartUpdate::OnBnClickedBtnAioUpdate()
{
    if (m_bUpdateThreadRun)
    {
        WARNING("����������\r\n");
    }
    cleanInfo2Display();
    m_AioBinFileCount = 0;
    m_AioBinFileCount = detectBinFile();
    if (m_AioBinFileCount > 0)
    {
        displayProgressAIO(0, 0);
        MSG("��⵽%d�������ļ�\r\n", m_AioBinFileCount);
        
        m_UpdateThread = AfxBeginThread(UpdateThread, this);
        if (NULL == m_UpdateThread)
        {
            ERROR_INFO("����ʧ��:�����߳�ERROR\r\n");
            addInfo2Display(_T("����:�����߳�ʧ��\r\n"));
        }
        else
        {
            GetDlgItem(IDC_BTN_AIO_UPDATE)->SetWindowTextA("AIO������...");
        }
    }
    else
    {
        WARNING("û�м�⵽���������ļ�:\r\n");
        INFO("%s\r\n",AIO_DSP_APP_BIN_NAME);
        INFO("%s\r\n",AIO_STM_APP_BIN_NAME);
        addInfo2Display(_T("����:û�м�⵽�����ļ�\r\n"));
    }
    return;
}


void CPageSmartUpdate::OnBnClickedBtnAm335xUpdate()
{
    // TODO: �ڴ���ӿؼ�֪ͨ����������
}


BOOL CPageSmartUpdate::OnInitDialog()
{
    __super::OnInitDialog();
    
    m_ProgressAIO.SetRange(0, 100);
    GetDlgItem(IDC_STATIC_AIO_PROCESS)->SetWindowTextA("0%");
    GetDlgItem(IDC_STATIC_AM335X_PROCESS)->SetWindowTextA("0%");
    GetDlgItem(IDC_BTN_AIO_UPDATE)->SetWindowTextA("AIO������");
    GetDlgItem(IDC_BTN_AM335X_UPDATE)->SetWindowTextA("AM335X������");
    
    GetDlgItem(IDC_BTN_AM335X_UPDATE)->ShowWindow(FALSE);
    GetDlgItem(IDC_PROGRESS_AM335X)->ShowWindow(FALSE);
    GetDlgItem(IDC_STATIC_AM335X_PROCESS)->ShowWindow(FALSE);

    return TRUE;  // return TRUE unless you set the focus to a control
}
