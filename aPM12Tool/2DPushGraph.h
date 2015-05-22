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
 
    //�÷������ڴ����ؼ�ʵ����ͨ���ڶԻ����ʼ�������е��ã�
    //nStaticIDΪ�ؼ�ID��pPatentΪ�������Ǹ������е�ID��
    bool CreateFromStatic( UINT nStaticID, CWnd* pParent );
 
    /* Functions that retrieve attributes */
    LPCTSTR   GetLabelForMax() const;           //��ȡ���ֵ���ı�ǩ
    LPCTSTR   GetLabelForMin() const;           //��ȡ��Сֵ���ı�ǩ
    COLORREF  GetBGColor()   const;             //��ñ���ɫ 
    COLORREF  GetGridColor() const;             //���դ����ɫ  

    //�÷������ڻ��ID��ΪuiLineID�Ĳ��ε�������ɫ��
    //��ΪҪ�ڲ��οؼ��л��߱����ȵ���bool AddLine( UINT uiLineID, COLORREF crColor );
    //����һ�����Σ�ID��ΪuiLineID��
    COLORREF  GetLineColor( UINT uiLineID );

    COLORREF  GetTextColor() const;             //��ñ�ǩ�ı�����ɫ
    int       GetGridSize()  const;             //���դ����
    int       GetMaxPeek()   const;             //������ֵ������
    int       GetMinPeek()   const;             //�����Сֵ������  
    // int    m_nPeekOffset;
    // int    m_nMoveOffset;
    unsigned short GetInterval() const;         //��ú�������
 
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
    bool    AddLine( UINT uiLineID, COLORREF crColor );     //�÷��������ڸÿؼ��ϴ���һ�����Ρ�
    void    RemoveLine( UINT uiLineID );                    //���ID��ΪuiLineID�Ĳ���
    bool    Push( double nMagnitude, UINT uiLineID );       //��ID��ΪuiLineID�ŵĲ��������һ�����ݵ㣬nMagnitudeΪ����
    void    ShowAsBar( UINT uiLineID, bool bAsBar );        //��״ͼ��ʾ
    void    Update();                                       //����
 
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
    COLORREF m_crTextColor;     //��ǩ������ɫ
    COLORREF m_crBGColor;       //������ɫ
    COLORREF m_crGridColor;     //դ����ɫ
    CString  m_strMaxLabel;     //���ֵ֮���ı�ǩ
    CString  m_strMinLabel;     //��Сֵ֮���ı�ǩ
    bool   m_bShowMinMax;       //��С���ֵ��ʾ
    bool   m_bShowGrid;         //�Ƿ���ʾդ��
    bool   m_bStylesModified;   //�Ƿ��޸�
    int    m_nMoveOffset;       //ƫ��
    int    m_nMaxCoords;        //��󻺳�
    int    m_nMaxPeek;          //��ʾ���ݵ����ֵ
    int    m_nMinPeek;          //��ʾ���ݵ���Сֵ
    int    m_nGridSize;         //դ����
    int    m_nPeekOffset;       //��ֵƫ��
 
    CBitmap  *m_pOldBitmap; // Original bitmap
    CBitmap  m_bmBack;      // Bitmap for backbuffering
    CDC      m_dcBack;
 
    unsigned short m_usLineInterval;
    CArray<PushGraphLine*, PushGraphLine*> m_aLines;
};
 
#endif // !defined(AFX_2DPUSHGRAPH_H_INCLUDED)