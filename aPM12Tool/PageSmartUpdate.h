#pragma once


// CPageSmartUpdate �Ի���

class CPageSmartUpdate : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageSmartUpdate)

public:
	CPageSmartUpdate();
	virtual ~CPageSmartUpdate();

// �Ի�������
	enum { IDD = IDD_DLG_SMART_UPDATE };
    
public:
    void        initApplication(void);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
};
