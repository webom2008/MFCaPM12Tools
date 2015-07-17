// PageSmartUpdate.cpp : 实现文件
//

#include "stdafx.h"
#include "aPM12Tool.h"
#include "PageSmartUpdate.h"
#include "afxdialogex.h"
#include "FileIO.h"

extern CSerialProtocol *g_pSerialProtocol;
static const char AIO_DSP_APP_BIN_NAME[] = "aPM12_AIO_DSPAPP*.ldr";
static const char AIO_STM_APP_BIN_NAME[] = "aPM12_AIO_STMAPP*.bin";
// CPageSmartUpdate 对话框

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


// CPageSmartUpdate 消息处理程序

void CPageSmartUpdate::initApplication(void)
{
    CUpdate::initApplication();
}

/* 
 * 函数介绍： 检测BIN文件是否存在
 * 输入参数：
 * 输出参数： 
 * 返 回 值： BIN文件的个数
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
        name = strArrayFileNames.GetAt(0); // 只获取第一个
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
        name = strArrayFileNames.GetAt(0); // 只获取第一个
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

    INFO("正准备软件升级...\r\n");
    //>>>>>>>>S1:COM handle
    //判断串口是否打开
    if(false == g_pSerialProtocol->isSerialOpen())//串口已经打开，则进行关闭操作
    {
        INFO("错误:请先打开串口\r\n");
        pSmartUpdate->addInfo2Display(_T("错误:请先打开串口\r\n"));
        pSmartUpdate->m_bUpdateThreadRun = false;
        pSmartUpdate->GetDlgItem(IDC_BTN_AIO_UPDATE)->SetWindowTextA("AIO板升级");
        return 0;
    }
    
    str.Format("提示:检测到%d待升级文件,正在进行中...\r\n", pSmartUpdate->m_AioBinFileCount);
    pSmartUpdate->addInfo2Display(str);

    while (!pSmartUpdate->checkTarget(CID,name))
    {
        if (CID != pSmartUpdate->getPacketCID())
        {
            pSmartUpdate->setPacketCID(CID);
        }
        if (pSmartUpdate->SaveFiletoRAM(&len, name) < 0)
        {
            INFO("错误:文件拷贝至内存失败\r\n");
            pSmartUpdate->addInfo2Display(_T("错误:文件拷贝至内存失败\r\n"));
            pSmartUpdate->m_bUpdateThreadRun = false;
            pSmartUpdate->GetDlgItem(IDC_BTN_AIO_UPDATE)->SetWindowTextA("AIO板升级");
            return 0;
        }
        else
        {
            INFO("提示:文件长度 = %d  字节\r\n", len);
            pSmartUpdate->displayProgressAIO(index, 1);
        }

        //>>>>>>>>S3:Send Update Tag, Target ready to download.
        i = 5;
        INFO("(1/5)等待目标板确认包...");
        while(--i)
        {
            if (pSmartUpdate->SendResetAndUpdateTag()) break;
        }
        pSmartUpdate->DisplayOKorError(i);
        if (i == 0)
        {
            pSmartUpdate->addInfo2Display(_T("错误:等待目标板确认包失败\r\n"));
            pSmartUpdate->m_bUpdateThreadRun = false;
            pSmartUpdate->GetDlgItem(IDC_BTN_AIO_UPDATE)->SetWindowTextA("AIO板升级");
            return 0;
        }
        pSmartUpdate->displayProgressAIO(index, 3);

        //>>>>>>>>S4:Send UPDATE_SOL
        i = 5;
        INFO("(2/5)等待文件长度确认包...");
        while(--i)
        {
            result = pSmartUpdate->SendUpdateStartOfLenght(len);
            if (0 != result) break;
        }
        if (-1 == result)
        {
            INFO("Error(文件大小超出Flash范围)\r\n");
            pSmartUpdate->addInfo2Display(_T("错误:文件大小超出Flash范围\r\n"));
            pSmartUpdate->m_bUpdateThreadRun = false;
            pSmartUpdate->GetDlgItem(IDC_BTN_AIO_UPDATE)->SetWindowTextA("AIO板升级");
            return 0;
        }
        pSmartUpdate->DisplayOKorError(i);
        if (i == 0)
        {
            pSmartUpdate->addInfo2Display(_T("错误:等待目标板长度确认包失败\r\n"));
            pSmartUpdate->m_bUpdateThreadRun = false;
            pSmartUpdate->GetDlgItem(IDC_BTN_AIO_UPDATE)->SetWindowTextA("AIO板升级");
            return 0;
        }
        pSmartUpdate->displayProgressAIO(index, 5);

        INFO("(3/5)正在传送数据:\r\n");
        do 
        {
            result = pSmartUpdate->SendUpdateStartOfData(len);
            if (result > 0)
            {
                i = 5 + 90 * (len - result)/len;
                pSmartUpdate->displayProgressAIO(index, i);
            }
        } while(result > 0);
        
        INFO("\r\n(3/5)正在传送数据结果:");
        if (0 == result)
        {
            pSmartUpdate->DisplayOKorError(1);
        }
        else 
        {
            pSmartUpdate->DisplayOKorError(0);
            if (-1 == result)
            {
                ERROR_INFO("目标板异常终止");
            }
            else if (-2 == result)
            {
                ERROR_INFO("等待响应超时!");
            }
            pSmartUpdate->addInfo2Display(_T("错误:数据传输失败\r\n"));
            pSmartUpdate->m_bUpdateThreadRun = false;
            pSmartUpdate->GetDlgItem(IDC_BTN_AIO_UPDATE)->SetWindowTextA("AIO板升级");
            return 0;
        }

        //>>>>>>>>S6:Send UPDATE_EOT
        i = 5;
        INFO("(4/5)等待文件结束确认包...");
        while(--i)
        {
            if (pSmartUpdate->SendUpdateEndOfTransmit()) break;
        }
        pSmartUpdate->DisplayOKorError(i);
        if (i == 0)
        {
            pSmartUpdate->addInfo2Display(_T("错误:等待文件结束确认包失败\r\n"));
            pSmartUpdate->m_bUpdateThreadRun = false;
            pSmartUpdate->GetDlgItem(IDC_BTN_AIO_UPDATE)->SetWindowTextA("AIO板升级");
            return 0;
        }
        pSmartUpdate->displayProgressAIO(index, 97);

        INFO("(5/5)等待烧写进Flash:");
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
            ERROR_INFO("超时\r\n");
        }
        pSmartUpdate->displayProgressAIO(index, 100);
        index++;
        if (index != pSmartUpdate->m_AioBinFileCount)
        {
            MSG("准备烧写下一个文件!\r\n");
            Sleep(6000); //wait for reboot!
        }
    }
    
    INFO("\r\n===========================");
    INFO("\r\n===========================");
    INFO("\r\n===========================");
    INFO("\r\n>>>>>>>>烧写已完成<<<<<<<<<\r\n");
    pSmartUpdate->addInfo2Display(_T("\r\n>>>>烧写成功<<<<\r\n"));
    pSmartUpdate->m_bUpdateThreadRun = false;
    pSmartUpdate->GetDlgItem(IDC_BTN_AIO_UPDATE)->SetWindowTextA("AIO板升级");
    return 0;
}


void CPageSmartUpdate::addInfo2Display(CString str)
{
	m_EditUpdateDisplay.SetSel(-1, -1);
	m_EditUpdateDisplay.ReplaceSel(str);
	//this->UpdateData(FALSE);//更新编辑框内容
}

void CPageSmartUpdate::cleanInfo2Display(void)
{
	m_EditUpdateDisplay.SetWindowText("");
    m_EditUpdateDisplay.SetSel(-1);
	//this->UpdateData(FALSE);//更新编辑框内容
}

void CPageSmartUpdate::OnBnClickedBtnAioUpdate()
{
    if (m_bUpdateThreadRun)
    {
        WARNING("升级进行中\r\n");
    }
    cleanInfo2Display();
    m_AioBinFileCount = 0;
    m_AioBinFileCount = detectBinFile();
    if (m_AioBinFileCount > 0)
    {
        displayProgressAIO(0, 0);
        MSG("检测到%d待升级文件\r\n", m_AioBinFileCount);
        
        m_UpdateThread = AfxBeginThread(UpdateThread, this);
        if (NULL == m_UpdateThread)
        {
            ERROR_INFO("升级失败:创建线程ERROR\r\n");
            addInfo2Display(_T("错误:创建线程失败\r\n"));
        }
        else
        {
            GetDlgItem(IDC_BTN_AIO_UPDATE)->SetWindowTextA("AIO板升级...");
        }
    }
    else
    {
        WARNING("没有检测到如下类型文件:\r\n");
        INFO("%s\r\n",AIO_DSP_APP_BIN_NAME);
        INFO("%s\r\n",AIO_STM_APP_BIN_NAME);
        addInfo2Display(_T("错误:没有检测到升级文件\r\n"));
    }
    return;
}


void CPageSmartUpdate::OnBnClickedBtnAm335xUpdate()
{
    // TODO: 在此添加控件通知处理程序代码
}


BOOL CPageSmartUpdate::OnInitDialog()
{
    __super::OnInitDialog();
    
    m_ProgressAIO.SetRange(0, 100);
    GetDlgItem(IDC_STATIC_AIO_PROCESS)->SetWindowTextA("0%");
    GetDlgItem(IDC_STATIC_AM335X_PROCESS)->SetWindowTextA("0%");
    GetDlgItem(IDC_BTN_AIO_UPDATE)->SetWindowTextA("AIO板升级");
    GetDlgItem(IDC_BTN_AM335X_UPDATE)->SetWindowTextA("AM335X板升级");
    
    GetDlgItem(IDC_BTN_AM335X_UPDATE)->ShowWindow(FALSE);
    GetDlgItem(IDC_PROGRESS_AM335X)->ShowWindow(FALSE);
    GetDlgItem(IDC_STATIC_AM335X_PROCESS)->ShowWindow(FALSE);

    return TRUE;  // return TRUE unless you set the focus to a control
}
