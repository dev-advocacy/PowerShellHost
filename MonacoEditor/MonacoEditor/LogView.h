#pragma once
class CLogView : public CWindowImpl<CLogView, CListViewCtrl,
    CWinTraitsOR<
    LVS_OWNERDRAWFIXED | LVS_REPORT | LVS_OWNERDATA | LVS_NOSORTHEADER | LVS_SHOWSELALWAYS,
    LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_HEADERDRAGDROP>>,
    public WTL::COwnerDraw<CLogView>
{
public:
    CLogView();

    DECLARE_WND_SUPERCLASS(nullptr, CListViewCtrl::GetWndClassName())

    BEGIN_MSG_MAP(CLogView)
        CHAIN_MSG_MAP_ALT(COwnerDraw<CLogView>, 1)
        DEFAULT_REFLECTION_HANDLER()
    END_MSG_MAP()

    void Create(HWND hWndParent, ATL::_U_RECT rect = nullptr);
    void DeleteItem(DELETEITEMSTRUCT* lParam);
    void MeasureItem(MEASUREITEMSTRUCT* pMeasureItemStruct);
    void DrawItem(DRAWITEMSTRUCT* pDrawItemStruct);
    void SetFont(HFONT hFont);

private:
    RECT GetItemRect(int iItem, unsigned code) const;
    void DrawItem(CDCHandle dc, int iItem, unsigned iItemState);
};