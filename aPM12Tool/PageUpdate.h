#pragma once
#include "afxwin.h"
// CPageUpdate 对话框

class CPageUpdate : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageUpdate)

public:
	CPageUpdate();
	virtual ~CPageUpdate();
	
	void initApplication(void);
    static int WINAPI    PktHandleUpdateSoftware(LPVOID pParam, UartProtocolPacket *pPacket);
    static int WINAPI    PktHandleGetVersion(LPVOID pParam, UartProtocolPacket *pPacket);
    
    void    setSoftwareVersion(char *pVersion);

// 对话框数据
	enum { IDD = IDD_DLG_UPDATE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

    int SaveFiletoRAM(unsigned int *pFileLen);
    void DisplayOKorError(int state);
	int SendResetAndUpdateTag(void);
	int SendUpdateStartOfLenght(const unsigned int file_len);
	int SendUpdateStartOfData(const unsigned int file_len);
	int SendUpdateEndOfTransmit(void);
    int WaitUpdateWrite2FlashDone(void);

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedBtnPathSel();
    afx_msg void OnBnClickedBtnUpdate();
    CComboBox m_TargtSelect;
    virtual BOOL OnInitDialog();
    afx_msg void OnCbnSelchangeCboUpdateTargetSel();
    CEdit m_filePath;

private:
    BYTE    *pFileRamAddr;
    unsigned int m_packetCount;
    unsigned int m_Mem_addr_offset;

    BYTE		        m_BChildID;
	UartProtocolPacket  m_PktForUpdate;
	// handles 
	HANDLE	            m_hGetUpdatePacketEvent;
	CRITICAL_SECTION	m_csCommunicationSync; 
    
    // Thread
    bool                m_bUpdateThreadRun;
    static UINT	        UpdateThread(LPVOID pParam);
	CWinThread*	        m_UpdateThread;
public:
    afx_msg void OnBnClickedBtnAioVersion();
    CEdit               m_EditVersion;
};

