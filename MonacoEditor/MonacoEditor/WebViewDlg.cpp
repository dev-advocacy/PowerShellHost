#include "pch.h"
#include "resource.h"
#include "WebViewDlg.h"

CDlgWebView2::CDlgWebView2(std::wstring browerdirectory, std::wstring userdatedirectory, std::wstring url)
{
	m_user_data_directory = userdatedirectory;
	m_url = url;
	m_browser_directory = browerdirectory;
}


LRESULT CDlgWebView2::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CenterWindow(GetParent());
	return (CWebView2Impl2<CDlgWebView2>::OnInitDialog(uMsg,wParam,lParam,bHandled));
}
LRESULT CDlgWebView2::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_modeless == true ? DestroyWindow() : EndDialog(wID);
	return 0;
}