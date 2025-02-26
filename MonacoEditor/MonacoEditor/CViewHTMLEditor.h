#pragma once
#include "WebView2.h"
#include "WebViewProfile.h"
#include "WebViewDlg.h"
#include "WebView2Impl2.h"

class CMainFrame;

class CViewHTMLEditor : public CWindowImpl<CViewHTMLEditor>
{

public:
	std::unique_ptr <CWebView2>			m_webview2 = nullptr;
	ProfileInformation_t				m_webviewprofile;
	HWND								m_hWndClient = nullptr;
	CMainFrame*							m_pMainFrame = nullptr;
	std::unique_ptr<CDlgWebView2>		m_dlgWebWiew2Modeless = nullptr;

public:
	DECLARE_WND_CLASS(NULL)

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		pMsg;

		return m_webview2->PreTranslateMessage(pMsg);

		//return FALSE;
	}

	void	SetMainFrame(CMainFrame *pMainFrame);
	LRESULT OnScenarioWebView2Modeless(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnScenarioWebView2Modal(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnScenarioInstallation(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	BEGIN_MSG_MAP(CViewHTMLEditor)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
	END_MSG_MAP()

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
};


