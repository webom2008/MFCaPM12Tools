// PageFileMaker.cpp : 实现文件
//

#include "stdafx.h"
#include "aPM12Tool.h"
#include "PageFileMaker.h"
#include "afxdialogex.h"

static const unsigned int FILE_OFFSET_DEF[] = 
{
    0x00000000,
    0x00004000,
    0x00020000
};

// CPageFileMaker 对话框

IMPLEMENT_DYNAMIC(CPageFileMaker, CPropertyPage)


CPageFileMaker::CPageFileMaker()
	: CPropertyPage(CPageFileMaker::IDD)
{

}

CPageFileMaker::~CPageFileMaker()
{
}

void CPageFileMaker::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_FILE1_PATH, m_EditFilePath1);
    DDX_Control(pDX, IDC_EDIT_FILE2_PATH, m_EditFilePath2);
    DDX_Control(pDX, IDC_CBO_FILE2_OFFSET, m_CBoxFile2Offset);
    DDX_Control(pDX, IDC_CBO_FILE1_OFFSET, m_CBoxFile1Offset);
    DDX_Control(pDX, IDC_EDIT_FILE_NAME, m_EditMergeName);
}


BEGIN_MESSAGE_MAP(CPageFileMaker, CPropertyPage)
    ON_BN_CLICKED(IDC_BTN_FILE1, &CPageFileMaker::OnBnClickedBtnFile1)
    ON_BN_CLICKED(IDC_BTN_FILE2, &CPageFileMaker::OnBnClickedBtnFile2)
    ON_BN_CLICKED(IDC_BTN_FILES_MAKE, &CPageFileMaker::OnBnClickedBtnFilesMake)
END_MESSAGE_MAP()


BOOL CPageFileMaker::OnInitDialog()
{
    CPropertyPage::OnInitDialog();
    CString str;
    int i = 0;

    str.Format("0x%08x",FILE_OFFSET_DEF[0]);
    m_CBoxFile1Offset.InsertString(0,str);
    m_CBoxFile1Offset.SetCurSel(0);
        
    for (i = 0; i < sizeof(FILE_OFFSET_DEF)/sizeof(FILE_OFFSET_DEF[0]); i++)
    {
        str.Format("0x%08x",FILE_OFFSET_DEF[i]);
        m_CBoxFile2Offset.InsertString(i,str);
    }
    m_CBoxFile2Offset.SetCurSel(2);

    m_EditMergeName.SetWindowTextA("out.bin");
    return TRUE;  // return TRUE unless you set the focus to a control
}

// CPageFileMaker 消息处理程序

void CPageFileMaker::initApplication(void)
{
}

void CPageFileMaker::OnBnClickedBtnFile1()
{
    CFileDialog  FDlg(  TRUE ,
                        NULL,
                        NULL ,
                        OFN_HIDEREADONLY ,
                        _T("所有文件(*.*) |*.*||"),
                        this);
    if(FDlg.DoModal() == IDOK)
    {
        m_strFilePath1 = FDlg.GetPathName();
        UpdateData(false);
        m_strFilePath1.Replace(_T("//"),_T("////"));
        m_EditFilePath1.SetWindowText(m_strFilePath1);
        UpdateData(FALSE);//更新编辑框内容
    }
}


void CPageFileMaker::OnBnClickedBtnFile2()
{
    CFileDialog  FDlg(  TRUE ,
                        NULL,
                        NULL ,
                        OFN_HIDEREADONLY ,
                        _T("所有文件(*.*) |*.*||"),
                        this);
    if(FDlg.DoModal() == IDOK)
    {
        m_strFilePath2 = FDlg.GetPathName();
        UpdateData(false);
        m_strFilePath2.Replace(_T("//"),_T("////"));
        m_EditFilePath2.SetWindowText(m_strFilePath2);
        UpdateData(FALSE);//更新编辑框内容
    }
}

void CPageFileMaker::OnBnClickedBtnFilesMake()
{
    BYTE *pFile1InRAM = NULL;
    BYTE *pFile2InRAM = NULL;
    BYTE *pMergeFileInRAM = NULL;
    CString strName;

    if ((FALSE == PathFileExists(m_strFilePath1)) \
        || (FALSE == PathFileExists(m_strFilePath2)))
    {
        ERROR_INFO("请选择正确的文件路径\r\n");
		return;
    }

    CFile file1(m_strFilePath1, CFile::modeRead);
    CFile file2(m_strFilePath2, CFile::modeRead);
    if ((NULL == file1) || (NULL == file2))
    {
        ERROR_INFO("请确保文件存在\r\n");
		return;
    }

    unsigned int file1_len = (unsigned int)file1.GetLength();
    unsigned int file2_len = (unsigned int)file2.GetLength();
    unsigned int mergeFileLen = file2_len + FILE_OFFSET_DEF[m_CBoxFile2Offset.GetCurSel()];
    
    INFO("file1_len = %d \r\nfile2_len = %d \r\n",file1_len ,file2_len);

    pFile1InRAM = (BYTE *)malloc(file1_len);
    pFile2InRAM = (BYTE *)malloc(file2_len);
    pMergeFileInRAM = (BYTE *)malloc(mergeFileLen);

    if ((NULL == pFile1InRAM) || (NULL == pFile2InRAM) || (NULL == pMergeFileInRAM))
    {
        ERROR_INFO("malloc出错\r\n");
        file1.Close();
        file2.Close();
		return;
    }

    file1.SeekToBegin();//移到文件头
    file1.Read(pFile1InRAM, file1_len);
    file1.Close();
    
    file2.SeekToBegin();//移到文件头
    file2.Read(pFile2InRAM, file2_len);
    file2.Close();

    memset(pMergeFileInRAM, 0xFF, mergeFileLen);
    memcpy(pMergeFileInRAM, pFile1InRAM, file1_len);
    memcpy(pMergeFileInRAM+FILE_OFFSET_DEF[m_CBoxFile2Offset.GetCurSel()], pFile2InRAM, file2_len);
    INFO("MergeFile_len = %d \r\n",mergeFileLen);

    m_EditMergeName.GetWindowTextA(strName);
    if (strName.IsEmpty())
    {
        ERROR_INFO("请输入目标文件的名称\r\n");
        free(pFile1InRAM);
        free(pFile2InRAM);
        free(pMergeFileInRAM);
        return;
    }

    CFile targetFile;
    CFileException e;
    if(!targetFile.Open( strName, CFile::modeCreate | CFile::modeWrite, &e ))
    {
        ERROR_INFO("文件%s打开或创建失败 %d\r\n",strName ,e.m_cause);
        free(pFile1InRAM);
        free(pFile2InRAM);
        free(pMergeFileInRAM);
        return;
    }

    targetFile.Write(pMergeFileInRAM,mergeFileLen);
    targetFile.Close();

    free(pFile1InRAM);
    free(pFile2InRAM);
    free(pMergeFileInRAM);
    INFO("合并文件成功！\r\n",strName ,e.m_cause);
}
