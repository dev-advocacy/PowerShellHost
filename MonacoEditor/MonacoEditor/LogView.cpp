#include "pch.h"
#include "LogView.h"

CLogView::CLogView()
{
}

void CLogView::DeleteItem(DELETEITEMSTRUCT* lParam)
{
    COwnerDraw<CLogView>::DeleteItem(lParam);
}

void CLogView::SetFont(HFONT hFont)
{
    CListViewCtrl::SetFont(hFont);
    GetHeader().Invalidate();

    // Trigger WM_MEASUREPOS
    // See: http://www.codeproject.com/Articles/1401/Changing-Row-Height-in-an-owner-drawn-Control
    CRect rect;
    GetWindowRect(&rect);
    WINDOWPOS wp;
    wp.hwnd = *this;
    wp.cx = rect.Width();
    wp.cy = rect.Height();
    wp.flags = SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER;
    SendMessage(WM_WINDOWPOSCHANGED, 0, reinterpret_cast<LPARAM>(&wp));
}

void CLogView::Create(HWND hWndParent, ATL::_U_RECT rect)
{
    __super::Create(hWndParent, rect, CListViewCtrl::GetWndClassName(), WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL, WS_EX_CLIENTEDGE);
}

void CLogView::MeasureItem(MEASUREITEMSTRUCT* pMeasureItemStruct)
{
    OutputDebugStringA("CLogView::MeasureItem - debug");
    CClientDC dc(*this);

    //Win32::GdiObjectSelection font(dc, GetFont());
    //TEXTMETRIC metric;
    //dc.GetTextMetrics(&metric);
    //pMeasureItemStruct->itemHeight = metric.tmHeight;
}

/// <summary>
/// 
/// </summary>
/// <param name="pDrawItemStruct"></param>
void CLogView::DrawItem(DRAWITEMSTRUCT* pDrawItemStruct)
{
    OutputDebugStringA("CLogView::DrawItem - debug");
    DrawItem(pDrawItemStruct->hDC, pDrawItemStruct->itemID, pDrawItemStruct->itemState);
}


RECT CLogView::GetItemRect(int iItem, unsigned code) const
{
    return RECT();
}

/// <summary>
/// 
/// </summary>
/// <param name="dc"></param>
/// <param name="iItem"></param>
/// <param name=""></param>
void CLogView::DrawItem(CDCHandle dc, int iItem, unsigned /*iItemState*/)
{
    auto rect = GetItemRect(iItem, LVIR_BOUNDS);

    bool selected = GetItemState(iItem, LVIS_SELECTED) == LVIS_SELECTED;
    bool focused = GetItemState(iItem, LVIS_FOCUSED) == LVIS_FOCUSED;
    auto bkColor = selected ? RGB(255, 200, 200) : RGB(255, 255, 255);
    auto txColor = RGB(0, 0, 0);

    dc.FillSolidRect(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, bkColor);

    //Win32::ScopedBkColor bcol(dc, bkColor);
    //Win32::ScopedTextColor tcol(dc, txColor);

    //std::wstring text = wstringbuilder() << iItem + 1 << L" If the logview is rendered correctly then exactly 40 lines will fit";
    //dc.DrawText(text.c_str(), text.size(), &rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

    //if (focused)
    //    dc.DrawFocusRect(&rect);
}