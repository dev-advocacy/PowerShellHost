// View.h : interface of the CView class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "WebView2Impl2.h"


class CWebView2 : public CWindowImpl<CWebView2>, public WebView2::CWebView2Impl2<CWebView2>
{
public:
	DECLARE_WND_CLASS(NULL)

	CWebView2(std::wstring browerdirectory, std::wstring userdatedirectory, std::wstring url);
	~CWebView2();

	void CreationCompleted();
	void NavigationCompleted(std::wstring url);
	void AuthenticationCompleted();

	BOOL PreTranslateMessage(MSG* pMsg);

	BEGIN_MSG_MAP(CWebView2)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		CHAIN_MSG_MAP(CWebView2Impl2<CWebView2>)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);



};
