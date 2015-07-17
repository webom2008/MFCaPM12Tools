#pragma once
#include "afxwin.h"
#include "Update.h"
// CPageUpdate 对话框

class CPageUpdate : public CPropertyPage, public CUpdate
{
	DECLARE_DYNAMIC(CPageUpdate)

public:
	CPageUpdate();
	virtual ~CPageUpdate();
	
	void initApplication(void);
    static int WINAPI    PktHandleGetVersion(LPVOID pParam, UartProtocolPacket *pPacket);
    
    void    setSoftwareVersion(char *pVersion);

// 对话框数据
	enum { IDD = IDD_DLG_UPDATE };

private:
    // Thread
    bool                m_bUpdateThreadRun;
    static UINT	        UpdateThread(LPVOID pParam);
	CWinThread*	        m_UpdateThread;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
    
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedBtnPathSel();
    afx_msg void OnBnClickedBtnUpdate();
    CComboBox m_TargtSelect;
    virtual BOOL OnInitDialog();
    afx_msg void OnCbnSelchangeCboUpdateTargetSel();
    CEdit   m_filePath;
    afx_msg void OnBnClickedBtnAioVersion();
    CEdit    m_EditVersion;
};

