#pragma once

#include "configs.h"
#include "PageSysCfg.h"
#include "PageUpdate.h"
#include "PageDebug.h"
#include "PageNIBP.h"
#include "PageWave.h"
#include "PageFactory.h"
#include "PageFileMaker.h"
#include "PageSmartUpdate.h"

// CToolSheet

class CToolSheet : public CPropertySheet
{
	DECLARE_DYNAMIC(CToolSheet)

public:
	CToolSheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CToolSheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	virtual ~CToolSheet();

    void initApplication(void);

private:
#if defined(CONFIG_SYSTEM_CFG_USED)
    CPageSysCfg         m_PageSysCfg;
#endif
#if defined(CONFIG_SMART_UPDATE_USED)
    CPageSmartUpdate    m_PageSmartUpdate;
#endif
#if defined(CONFIG_NORMAL_UPDATE_USED)
    CPageUpdate         m_PageUpdate;
#endif
#if defined(CONFIG_SYSTEM_DEBUG_USED)
    CPageDebug          m_PageDebug;
#endif
#if defined(CONFIG_NIBP_USED)
    CPageNIBP           m_PageNIBP;
#endif
#if defined(CONFIG_WAVE_USED)
    CPageWave           m_PageWave;
#endif
#if defined(CONFIG_FACTORY_USED)
    CPageFactory        m_PageFactory;
#endif
#if defined(CONFIG_FILE_MAKER_USED)
    CPageFileMaker      m_PageFileMaker;
#endif

protected:
	DECLARE_MESSAGE_MAP()
public:
    virtual BOOL OnInitDialog();
};


