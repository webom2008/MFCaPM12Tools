/* ===================================================================
 
C2DPushGraph Control (2DPushGraph.h and 2DPushGraph.cpp)
 
Author:  Stuart Konen
Contact: skonen@gmail.com (Job information welcome)
 
Description: A push graph control similiar to the graph control located
in Microsoft's Task Manager.
 
====================================================================*/
#if !defined(AFX_2DPUSHGRAPH_H_INCLUDED)
#define AFX_2DPUSHGRAPH_H_INCLUDED
 
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
 
 
#include <windows.h>
#include "afxtempl.h"
 
/////////////////////////////////////////////////////////////////////////////
// C2DPushGraph window
 
 
// ===================================================================
struct GraphColor
{
 
    GraphColor()
    {
        bRed = bGreen = bBlue = 0;
    }
 
    operator COLORREF()
    {
        return RGB(bRed, bGreen, bBlue);
    }

    BYTE bRed, bGreen, bBlue;
 
};
 
// ===================================================================
struct PushGraphLine
{
    PushGraphLine(UINT uiLineID)
    {
        crLine     = RGB(0, 255, 0);
        uiID       = uiLineID;
        bShowAsBar = false;
    }
 
    COLORREF crLine;
    UINT     uiID;
    bool     bShowAsBar;
 
    CArray<int, double> aMagnitudes;
};
 
// ===================================================================
class C2DPushGraph : public CWnd
{
 
public:
    C2DPushGraph();
    virtual ~C2DPushGraph();
 
    //该方法用于创建控件实例，通常在对话框初始化函数中调用，
    //nStaticID为控件ID，pPatent为创建在那个窗口中的ID。
    bool CreateFromStatic( UINT nStaticID, CWnd* pParent );
 
    /* Functions that retrieve attributes */
    LPCTSTR   GetLabelForMax() const;           //获取最大值处的标签
    LPCTSTR   GetLabelForMin() const;           //获取最小值处的标签
    COLORREF  GetBGColor()   const;             //获得背景色 
    COLORREF  GetGridColor() const;             //获得栅格颜色  

    //该方法用于获得ID号为uiLineID的波形的线条颜色，
    //因为要在波形控件中画线必须先调用bool AddLine( UINT uiLineID, COLORREF crColor );
    //创建一个波形，ID号为uiLineID。
    COLORREF  GetLineColor( UINT uiLineID );

    COLORREF  GetTextColor() const;             //获得标签文本的颜色
    int       GetGridSize()  const;             //获得栅格间隔
    int       GetMaxPeek()   const;             //获得最大值纵坐标
    int       GetMinPeek()   const;             //获得最小值纵坐标  
    // int    m_nPeekOffset;
    // int    m_nMoveOffset;
    unsigned short GetInterval() const;         //获得横坐标间距
 
    /* Functions that set attributes */
    void SetBGColor(COLORREF crColor);
    void SetGridColor(COLORREF crColor);
    void SetTextColor(COLORREF crColor);
    void SetGridSize( unsigned short usWidthAndHeight );
    void SetMaxPeek(int nMax);
    void SetMinPeek(int nMin);
    void SetPeekRange(int nMin, int nMax);
    void SetLabelForMax( LPCTSTR lpszLabel );
    void SetLabelForMin( LPCTSTR lpszLabel );
    void SetInterval( unsigned short usInterval );
    bool SetLineColor( COLORREF crColor, UINT uiLineID );
 
    /* Line control functions */
    bool    AddLine( UINT uiLineID, COLORREF crColor );     //该方法用于在该控件上创建一个波形。
    void    RemoveLine( UINT uiLineID );                    //清除ID号为uiLineID的波形
    bool    Push( double nMagnitude, UINT uiLineID );       //在ID号为uiLineID号的波形上添加一个数据点，nMagnitude为幅度
    void    ShowAsBar( UINT uiLineID, bool bAsBar );        //柱状图显示
    void    Update();                                       //更新
 
    /* Visibility Functions */
    inline void ShowGrid( bool bShow = true)
    {
        m_bShowGrid = bShow;
    }
 
    inline void ShowLabels( bool bShow = true)
    {
        m_bShowMinMax = bShow;
    }
 
    CRect& getClientRect();
 
// Generated message map functions
protected:
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnPaint();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
    static BOOL m_bRegistered;
    static BOOL RegisterClass();
 
    DECLARE_MESSAGE_MAP()
 
private:
 
    virtual void   internal_DrawGrid( CDC &dc, CRect &rect );
    virtual void   internal_DrawLines( CDC &dc, CRect &rect );
    virtual void   internal_DrawBar( CDC &dc, CRect &rect,
                                     PushGraphLine& rLine );
    virtual void   internal_DrawMinMax( CDC &dc, CRect& rect);
 
    CDC&           internal_InitBackBuffer( CPaintDC &dc );
    void           internal_FreeBackBuffer( CPaintDC &dc );
    PushGraphLine* internal_LineFromID( UINT uiLineID );
 
    /* Internal data members */
    COLORREF m_crTextColor;     //标签文字颜色
    COLORREF m_crBGColor;       //背景颜色
    COLORREF m_crGridColor;     //栅格颜色
    CString  m_strMaxLabel;     //最大值之处的标签
    CString  m_strMinLabel;     //最小值之处的标签
    bool   m_bShowMinMax;       //最小最大值显示
    bool   m_bShowGrid;         //是否显示栅格
    bool   m_bStylesModified;   //是否被修改
    int    m_nMoveOffset;       //偏移
    int    m_nMaxCoords;        //最大缓冲
    int    m_nMaxPeek;          //显示数据的最大值
    int    m_nMinPeek;          //显示数据的最小值
    int    m_nGridSize;         //栅格间距
    int    m_nPeekOffset;       //峰值偏移
 
    CBitmap  *m_pOldBitmap; // Original bitmap
    CBitmap  m_bmBack;      // Bitmap for backbuffering
    CDC      m_dcBack;
 
    unsigned short m_usLineInterval;
    CArray<PushGraphLine*, PushGraphLine*> m_aLines;
};
 
#endif // !defined(AFX_2DPUSHGRAPH_H_INCLUDED)