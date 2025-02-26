#include "pch.h"
#include "MainFrm.h"
#include "CViewHTMLEditor.h"

LRESULT CViewHTMLEditor::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{

	std::wstring version = WebView2::Utility::GetWebView2Version();
	if (version.empty())
		this->MessageBoxW(L"Please install the WebView2 Runtime: menu Scenario/Installation",
			L"Warning", MB_OK | MB_ICONWARNING);


	HRESULT hr = CWebViewProfile::Profile(m_webviewprofile);
	if (FAILED(hr))
	{
		return 0;
	}
	m_webview2 = std::make_unique<CWebView2>(m_webviewprofile.browserDirectory, m_webviewprofile.userDataDirectory, L"file://monaco/index.html");	
	auto m_hWndClient = m_webview2->Create(m_hWnd, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_CLIENTEDGE);
	m_webview2->set_parent(this->m_hWnd);

	return 0;
}
void CViewHTMLEditor::SetMainFrame(CMainFrame* pMainFrame)
{
	if (pMainFrame != nullptr)
	{
		m_pMainFrame = pMainFrame;
	}	
}
LRESULT CViewHTMLEditor::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CPaintDC dc(m_hWnd);
	return 0;
}
LRESULT CViewHTMLEditor::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if (m_webview2 != nullptr)
	{
		CRect rcClient;
		GetClientRect(&rcClient);
		if (rcClient.Width() <= 0 || rcClient.Height() <= 0)
		{
			return 0;
		}	
		else
		{
			m_webview2->SetWindowPos(nullptr, &rcClient, SWP_NOZORDER | SWP_NOACTIVATE);						
		}
	}
	return 0;
}

LRESULT CViewHTMLEditor::OnScenarioWebView2Modal(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	CDlgWebView2 dlg(m_webviewprofile.browserDirectory, m_webviewprofile.userDataDirectory, L"https://www.google.fr");
	dlg.DoModal();
	return 0;
}

LRESULT CViewHTMLEditor::OnScenarioWebView2Modeless(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (m_dlgWebWiew2Modeless == nullptr)
		m_dlgWebWiew2Modeless = std::make_unique<CDlgWebView2>(m_webviewprofile.browserDirectory, m_webviewprofile.userDataDirectory, L"https://msdn.microsoft.com");

	if ((m_dlgWebWiew2Modeless != nullptr) && !::IsWindow(m_dlgWebWiew2Modeless->m_hWnd))
	{
		m_dlgWebWiew2Modeless->put_modeless(true);
		m_dlgWebWiew2Modeless->Create(this->m_hWnd);
		m_dlgWebWiew2Modeless->ShowWindow(SW_SHOW);
		m_dlgWebWiew2Modeless->navigate(L"https://msdn.microsoft.com");
	}
	return 0;
}
LRESULT CViewHTMLEditor::OnScenarioInstallation(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	constexpr size_t MaxMessageLength = 1024;
	std::wstring message(MaxMessageLength, L'\0');
	std::wstring version = WebView2::Utility::GetWebView2Version();

	if (!version.empty())
	{	//TODO: Replace with std::format when C++20 is enabled.
		_snwprintf_s(message.data(), message.size(), _TRUNCATE,
			L"WebView2 version %s is already installed.", version.c_str());
		this->MessageBoxW(message.c_str(), L"Information", MB_OK | MB_ICONINFORMATION);
		return 0;
	}

	// Download WebView2 boostrapper from the web.
	std::wstring path;
	HRESULT hr = WebView2::Utility::DownloadWebView2Bootstrapper(path);

	if FAILED(hr)
	{   //TODO: Replace with std::format when C++20 is enabled.
		_snwprintf_s(message.data(), message.size(), _TRUNCATE,
			L"Failed to download the latest WebView2 version. Error code: 0x%08X", hr);
		this->MessageBoxW(message.c_str(), L"Error", MB_OK | MB_ICONERROR);
		return 0;
	}

	// Install WebView2 in per-user mode.
	hr = WebView2::Utility::InstallWebView2(path, /*elevated*/ false);

	if FAILED(hr)
	{	//TODO: Replace with std::format when C++20 is enabled.
		_snwprintf_s(message.data(), message.size(), _TRUNCATE,
			L"Failed to install the latest WebView2 version. Error code: 0x%08X", hr);
		this->MessageBoxW(message.c_str(), L"Error", MB_OK | MB_ICONERROR);
		return 0;
	}

	this->MessageBoxW(L"Successfully installed the latest WebView2 version. "
		L"Restart the application to refresh.",
		L"Success", MB_OK | MB_ICONINFORMATION);
	return 0;
}