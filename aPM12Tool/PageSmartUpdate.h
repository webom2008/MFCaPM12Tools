#pragma once


// CPageSmartUpdate 对话框

class CPageSmartUpdate : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageSmartUpdate)

public:
	CPageSmartUpdate();
	virtual ~CPageSmartUpdate();

// 对话框数据
	enum { IDD = IDD_DLG_SMART_UPDATE };
    
public:
    void        initApplication(void);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
};
