#pragma once

#include "PageSysCfg.h"
#include "PageUpdate.h"
#include "PageDebug.h"
#include "PageNIBP.h"
#include "PageWave.h"
#include "PageFactory.h"
// CToolSheet

class CToolSheet : public CPropertySheet
{
	DECLARE_DYNAMIC(CToolSheet)

public:
	CToolSheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CToolSheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	virtual ~CToolSheet();
private:
    CPageSysCfg     m_PageSysCfg;
    CPageUpdate     m_PageUpdate;
    CPageDebug      m_PageDebug;
    CPageNIBP       m_PageNIBP;
    CPageWave       m_PageWave;
    CPageFactory    m_PageFactory;

protected:
	DECLARE_MESSAGE_MAP()
public:
    virtual BOOL OnInitDialog();
};


