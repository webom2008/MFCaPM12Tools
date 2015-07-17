#pragma once

#include <map>
#include "Update.h"
#include "afxcmn.h"
#include "afxwin.h"

// CPageSmartUpdate 对话框

class CPageSmartUpdate : public CPropertyPage, public CUpdate
{
	DECLARE_DYNAMIC(CPageSmartUpdate)

public:
	CPageSmartUpdate();
	virtual ~CPageSmartUpdate();

// 对话框数据
	enum { IDD = IDD_DLG_SMART_UPDATE };
    
public:
    void        initApplication(void);
    int         checkTarget(BYTE &CID, CString &path);
protected:
    BYTE        detectBinFile(void);
    void        displayProgressAIO(BYTE index, int value);
    void        addInfo2Display(CString str);
    void        cleanInfo2Display(void);
private:
    std::map<BYTE, CString> m_Target;
    BYTE        m_AioBinFileCount;
    // Thread
    bool                m_bUpdateThreadRun;
    static UINT	        UpdateThread(LPVOID pParam);
	CWinThread*	        m_UpdateThread;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedBtnAioUpdate();
    afx_msg void OnBnClickedBtnAm335xUpdate();
    CProgressCtrl m_ProgressAIO;
    virtual BOOL OnInitDialog();
    CEdit m_EditUpdateDisplay;
};
