#pragma once
#include "WebView2Impl2.h"

class CDlgWebView2 : public CDialogImpl<CDlgWebView2>, public WebView2::CWebView2Impl2<CDlgWebView2> 
{
public:
	enum { IDD = IDD_DIALOG_WEB_VIEW };
	CDlgWebView2(std::wstring browerdirectory, std::wstring userdatedirectory, std::wstring url);
	CDlgWebView2() {}
	void put_modeless(bool modeless) { m_modeless = modeless; }

	BEGIN_MSG_MAP(CDlgWebView2)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		CHAIN_MSG_MAP(WebView2::CWebView2Impl2<CDlgWebView2>)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:
	bool m_modeless = false;
};
