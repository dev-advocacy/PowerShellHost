#pragma once
/////////////////////////////////////////////////////////////////////////////
// CColoredButtonCtrl - The Button control with custom colours

//template< class T, class TBase = CButton, class TWinTraits = CControlWinTraits >
//class ATL_NO_VTABLE CColoredButtonImpl :
//    public CWindowImpl< T, TBase, TWinTraits >

class CColoredButtonCtrl : public CWindowImpl<CColoredButtonCtrl, CButton>,
    public COwnerDraw<CColoredButtonCtrl>
{
public:
    DECLARE_WND_SUPERCLASS(NULL, CButton::GetWndClassName())

        // Color codes for Button control
    COLORREF      m_clrText;
    COLORREF      m_clrBackUp;
    COLORREF      m_clrBackDown;
    COLORREF      m_clrBackDisabled;
    // Image support
    CImageList    m_ImageList;
    UINT          m_nImage;
    UINT          m_nSelImage;

    // Operations

    BOOL SubclassWindow(HWND hWnd)
    {
        ATLASSERT(m_hWnd == NULL);
        ATLASSERT(::IsWindow(hWnd));
#ifdef _DEBUG
        // Check class
        TCHAR szBuffer[20];
        if (::GetClassName(hWnd, szBuffer, (sizeof(szBuffer) / sizeof(TCHAR)) - 1)) 
        {
            ATLASSERT(::lstrcmpi(szBuffer, CButton::GetWndClassName()) == 0);
        }
#endif
        BOOL bRet = CWindowImpl< CColoredButtonCtrl, CButton >::SubclassWindow(hWnd);
        if (bRet) _Init();
        return bRet;
    }

    void SetTextColor(COLORREF clrText)
    {
        ATLASSERT(::IsWindow(m_hWnd));
        if (clrText == CLR_INVALID) clrText = ::GetSysColor(COLOR_BTNTEXT);
        m_clrText = clrText;
        // Repaint
        Invalidate();
    }
    void SetBkColor(COLORREF clrUp, COLORREF clrDown = -1)
    {
        ATLASSERT(::IsWindow(m_hWnd));
        if (clrUp == CLR_INVALID) clrUp = ::GetSysColor(COLOR_BTNFACE);
        if (clrDown == CLR_INVALID) clrDown = clrUp;
        m_clrBackUp = clrUp;
        m_clrBackDown = clrDown;
        // Repaint
        Invalidate();
    }
    void SetDisabledColor(COLORREF clrDisabled)
    {
        ATLASSERT(::IsWindow(m_hWnd));
        if (clrDisabled == CLR_INVALID) clrDisabled = ::GetSysColor(COLOR_BTNFACE);
        m_clrBackDisabled = clrDisabled;
        // Repaint
        Invalidate();
    }
    void SetImageList(HIMAGELIST hImageList)
    {
        ATLASSERT(::IsWindow(m_hWnd));
        m_ImageList = hImageList;
    }
    void SetImage(UINT nImage)
    {
        ATLASSERT(::IsWindow(m_hWnd));
        ATLASSERT(!m_ImageList.IsNull());
        m_nImage = nImage;
    }
    void SetSelImage(UINT nImage)
    {
        ATLASSERT(::IsWindow(m_hWnd));
        ATLASSERT(!m_ImageList.IsNull());
        m_nSelImage = nImage;
    }

    // Implementation

    void _Init()
    {
        ATLASSERT(::IsWindow(m_hWnd));

        // We need this style to prevent Windows from painting the button
        ModifyStyle(0, BS_OWNERDRAW);

        COLORREF clrNone = CLR_INVALID;
        SetTextColor(clrNone);
        SetBkColor(clrNone, clrNone);
        SetDisabledColor(clrNone);
        m_nImage = m_nSelImage = (UINT)-1;
    }

    // Message map and handlers

    BEGIN_MSG_MAP(CColoredButtonImpl)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
        MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDblClick)
        MESSAGE_HANDLER(OCM_DRAWITEM, OnDrawItem)
    END_MSG_MAP()

    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
    {
        LRESULT lRes = DefWindowProc(uMsg, wParam, lParam);
        _Init();
        return lRes;
    }
    LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        return 1;   // no background needed
    }
    LRESULT OnLButtonDblClick(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
    {
        return DefWindowProc(WM_LBUTTONDOWN, wParam, lParam);
    }
    LRESULT OnDrawItem(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
    {
        LPDRAWITEMSTRUCT lpDIS = (LPDRAWITEMSTRUCT)lParam;
        ATLASSERT(lpDIS->CtlType == ODT_BUTTON);
        CDCHandle dc = lpDIS->hDC;
        RECT rc = lpDIS->rcItem;
        DWORD dwStyle = GetStyle();
        bool bSelected = (lpDIS->itemState & ODS_SELECTED) != 0;
        bool bDisabled = (lpDIS->itemState & (ODS_DISABLED | ODS_GRAYED)) != 0;

        COLORREF clrBack = bSelected ? m_clrBackDown : m_clrBackUp;
        if (bDisabled) clrBack = m_clrBackDisabled;
        dc.FillSolidRect(&rc, clrBack);

        // Draw edge
        if (dwStyle & BS_FLAT) {
            dc.DrawEdge(&rc, bSelected ? BDR_SUNKENOUTER : BDR_RAISEDINNER, BF_RECT);
        }
        else {
            dc.DrawEdge(&rc, bSelected ? EDGE_SUNKEN : EDGE_RAISED, BF_RECT);
        }
        ::InflateRect(&rc, -2 * ::GetSystemMetrics(SM_CXEDGE), -2 * ::GetSystemMetrics(SM_CYEDGE));

        // Draw focus rectangle
        if (lpDIS->itemState & ODS_FOCUS) dc.DrawFocusRect(&rc);
        ::InflateRect(&rc, -1, -1);

        // Offset when button is selected
        if (bSelected) ::OffsetRect(&rc, 1, 1);

        // Draw image
        UINT nImage = m_nImage;
        if (bSelected && m_nSelImage != -1) nImage = m_nSelImage;
        if (!m_ImageList.IsNull() && nImage != -1) {
            POINT pt = { rc.left, rc.top - 1 };
            SIZE sizeIcon;
            m_ImageList.GetIconSize(sizeIcon);
            if (bDisabled) {
                //m_ImageList.DrawEx(nImage, dc, pt.x, pt.y, sizeIcon.cx, sizeIcon.cy, CLR_NONE, ::GetSysColor(COLOR_GRAYTEXT), ILD_BLEND50 | ILD_TRANSPARENT);
                HICON hIcon = m_ImageList.GetIcon(nImage);
                dc.DrawState(pt, sizeIcon, hIcon, DST_ICON | DSS_DISABLED);
                ::DestroyIcon(hIcon);
            }
            else {
                m_ImageList.Draw(dc, nImage, pt, bDisabled ? ILD_BLEND25 : ILD_TRANSPARENT);
            }
            rc.left += sizeIcon.cx + 3;
        }

        // Draw text
        UINT nLen = GetWindowTextLength();
        LPTSTR pstr = (LPTSTR)_alloca((nLen + 1) * sizeof(TCHAR));
        GetWindowText(pstr, nLen + 1);
        UINT uFlags = 0;
        if (dwStyle & BS_LEFT) uFlags |= DT_LEFT;
        else if (dwStyle & BS_RIGHT) uFlags |= DT_RIGHT;
        else if (dwStyle & BS_CENTER) uFlags |= DT_CENTER;
        else uFlags |= DT_CENTER;
        if (dwStyle & BS_TOP) uFlags |= DT_TOP;
        else if (dwStyle & BS_BOTTOM) uFlags |= DT_BOTTOM;
        else if (dwStyle & BS_VCENTER) uFlags |= DT_VCENTER;
        else uFlags |= DT_VCENTER;
        if ((dwStyle & BS_MULTILINE) == 0) uFlags |= DT_SINGLELINE;
        dc.SetBkMode(TRANSPARENT);
        dc.SetTextColor(bDisabled ? ::GetSysColor(COLOR_GRAYTEXT) : m_clrText);
        dc.DrawText(pstr, nLen, &rc, uFlags);

        return TRUE;
    }
};
