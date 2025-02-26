// MainFrm.cpp : implmentation of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "CViewHTMLEditor.h"
#include "resource.h"
#include "aboutdlg.h"
#include "MainFrm.h"
#include "WebView2Impl2.h"
#include "CertificateDlg.h"
#include "Utility.h"
#include "LLMDialog.h"
#include <codecvt>
#include <string>
#include "../../PowerShellHost.module/PowerShellHostmodule.h"


std::string wstring_to_utf8(const std::wstring& str) {
	std::string ret;
	int len = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), (int)str.length(), nullptr, 0, nullptr, nullptr);
	if (len > 0) {
		ret.resize(len);
		WideCharToMultiByte(CP_UTF8, 0, str.c_str(), (int)str.length(), &ret[0], len, nullptr, nullptr);
	}
	return ret;
}


std::vector<std::wstring> split(const std::wstring& str, char delim) {
	std::vector<std::wstring> strings;
	size_t start;
	size_t end = 0;
	while ((start = str.find_first_not_of(delim, end)) != std::wstring::npos) {
		end = str.find(delim, start);
		strings.push_back(str.substr(start, end - start));
	}
	return strings;
}
BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	if (CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg))
		return TRUE;
	if (m_wndCombo.PreTranslateMessage(pMsg))
		return TRUE;
	if (m_editView.PreTranslateMessage(pMsg))
		return TRUE;
	return m_viewHtmlEditor.PreTranslateMessage(pMsg);
}
BOOL CMainFrame::OnIdle()
{
	UIUpdateToolBar();
	return FALSE;
}
HWND CMainFrame::CreateAddressBarCtrl(HWND hWndParent)
{
	RECT rc = { 50, 0, 300, 100 };
	THROW_LAST_ERROR_IF_NULL(m_wndCombo.Create(hWndParent, rc, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBS_DROPDOWN | CBS_AUTOHSCROLL | CBS_SORT));
	m_wndCombo.SetFrame(this->m_hWnd);
	return m_wndCombo;
}

// Update the layout of child windows
void CMainFrame::UpdateLayoutWindow()
{

}


LRESULT CMainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// create command bar window
	HWND hWndCmdBar = m_CmdBar.Create(m_hWnd, rcDefault, nullptr, ATL_SIMPLE_CMDBAR_PANE_STYLE);
	HWND hWndAddressBar = CreateAddressBarCtrl(m_hWnd);
	// attach menu
	m_CmdBar.AttachMenu(GetMenu());
	// load command bar images
	//m_CmdBar.LoadImages(IDR_MAINFRAME);
	// remove old menu
	SetMenu(nullptr);


	// Load large toolbar bitmap
	HBITMAP hbm = (HBITMAP)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAMEL), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADMAP3DCOLORS);
	// Create image list for large icons
	CImageList imgList;
	imgList.Create(32, 32, ILC_COLOR32 | ILC_MASK, 4, 4);
	imgList.Add(hbm, RGB(255, 0, 255));


	HWND hWndToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_MAINFRAME, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);
	CToolBarCtrl wndToolBar = hWndToolBar;
	wndToolBar.SetImageList(imgList);

	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	AddSimpleReBarBand(hWndCmdBar);
	AddSimpleReBarBand(hWndToolBar, nullptr, TRUE);
	AddSimpleReBarBand(hWndAddressBar, _T("Address"), TRUE);

	CreateSimpleStatusBar();
	m_hWndClient = m_splitter.Create(m_hWnd, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

	m_paneeditlog.Create(m_splitter, _T("Logs"), WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

	m_editView.Create(m_paneeditlog, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_NOHIDESEL | WS_VSCROLL | WS_HSCROLL, WS_EX_CLIENTEDGE);

	m_viewHtmlEditor.Create(m_splitter, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_CLIENTEDGE);

	m_paneeditlog.SetClient(m_editView);

	m_splitter.SetSplitterPanes(m_viewHtmlEditor, m_paneeditlog);
	UpdateLayout();

	// set the splitter position to 80% of the client area
	m_splitter.SetSplitterPosPct(80);

	m_viewHtmlEditor.SetMainFrame(this);

	UIAddToolBar(hWndToolBar);
	UISetCheck(ID_VIEW_TOOLBAR, 1);
	UISetCheck(ID_VIEW_STATUS_BAR, 1);
	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != nullptr);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	return 0;
}
LRESULT CMainFrame::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	// unregister message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != nullptr);
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);

	bHandled = FALSE;
	return 1;
}
LRESULT CMainFrame::OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	PostMessage(WM_CLOSE);
	return 0;
}
LRESULT CMainFrame::OnFileNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: add code to initialize document

	return 0;
}
LRESULT CMainFrame::OnFileNewWindow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	::PostThreadMessage(_Module.m_dwMainThreadID, WM_USER, 0, 0L);
	return 0;
}
LRESULT CMainFrame::OnFileOpen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ATL::CComPtr<IFileOpenDialog> 	FileDlg;

	HRESULT hr;
	CComPtr<IFileOpenDialog> pDlg;
	COMDLG_FILTERSPEC aFileTypes[] = { { L"PowerShell file", L"*.ps1" },{ L"All files", L"*.*" } };

	// Create the file-open dialog COM object.
	hr = pDlg.CoCreateInstance(__uuidof(FileOpenDialog));
	if (FAILED(hr))
		return 0;

	pDlg->SetFileTypes(_countof(aFileTypes), aFileTypes);
	pDlg->SetTitle(L"Open PowerShell File");
	hr = pDlg->Show(m_hWnd);
	if (SUCCEEDED(hr))
	{
		CComPtr<IShellItem> pItem;

		hr = pDlg->GetResult(&pItem);

		if (SUCCEEDED(hr))
		{
			LPOLESTR pwsz = nullptr;

			hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pwsz);

			if (SUCCEEDED(hr))
			{
				//Read the file
				m_powershellfile = pwsz;

				std::ifstream file(pwsz);
				if (file.is_open())
				{
					std::stringstream buffer;
					buffer << file.rdbuf();
					m_sourceCode = std::make_unique<std::string>(buffer.str());

					std::wstring wsourceCode(m_sourceCode->begin(), m_sourceCode->end());
					std::wstring script = L"editor.setValue(`";
					script += wsourceCode;
					script += L"`);";
					m_viewHtmlEditor.m_webview2->execute_script(script);
				}
				else
				{
					this->MessageBox(L"Unable to open the file", L"Error", MB_OK | MB_ICONERROR);
				}
				CoTaskMemFree(pwsz);
			}
		}
	}
	return 0;
}

LRESULT CMainFrame::OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	static BOOL bVisible = TRUE;	// initially visible
	bVisible = !bVisible;
	CReBarCtrl rebar = m_hWndToolBar;
	int nBandIndex = rebar.IdToIndex(ATL_IDW_BAND_FIRST + 1);	// toolbar is 2nd added band
	rebar.ShowBand(nBandIndex, bVisible);
	UISetCheck(ID_VIEW_TOOLBAR, bVisible);
	UpdateLayout();
	return 0;
}
LRESULT CMainFrame::OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	BOOL bVisible = !::IsWindowVisible(m_hWndStatusBar);
	::ShowWindow(m_hWndStatusBar, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
	UISetCheck(ID_VIEW_STATUS_BAR, bVisible);
	UpdateLayout();
	return 0;
}
LRESULT CMainFrame::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CAboutDlg dlg;
	dlg.DoModal();
	return 0;
}

//setup.exe(under Webview installation directory) with following targets.
//--force - uninstall
//--uninstall
//--msedgewebview
//--system - level
//setup.exe --uninstall --msedgewebview --system-level --verbose-logging --force-uninstall

LRESULT CMainFrame::OnNavigate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	std::wstring text_url = reinterpret_cast<wchar_t*>(lParam);

	if (!text_url.empty())
	{
		m_viewHtmlEditor.m_webview2->navigate(text_url);
	}
	return 0;
}
LRESULT CMainFrame::OnEditCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	HWND hwnd = GetFocus();
	CEdit edit = m_wndCombo.GetEditCtrl();
	auto webview_hwnd = m_viewHtmlEditor.m_webview2->get_hwnd();
	if (hwnd == edit.m_hWnd)
	{
		m_wndCombo.Copy();
	}
	else if (webview_hwnd == hwnd)
	{
		m_viewHtmlEditor.m_webview2->copy();
	}
	return 0L;
}
LRESULT CMainFrame::OnEditPaste(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	HWND hwnd = GetFocus();
	CEdit edit = m_wndCombo.GetEditCtrl();

	auto webview_hwnd = m_viewHtmlEditor.m_webview2->get_hwnd();
	if (hwnd == edit.m_hWnd)
	{
		m_wndCombo.Paste();
	}
	else
	{
		// not currently supported

	}
	return 0L;
}
LRESULT CMainFrame::OnEditCut(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	HWND hwnd = GetFocus();
	CEdit edit = m_wndCombo.GetEditCtrl();

	auto webview_hwnd = m_viewHtmlEditor.m_webview2->get_hwnd();
	if (hwnd == edit.m_hWnd)
	{
		m_wndCombo.Cut();
	}
	else if (webview_hwnd == hwnd)
	{
		m_viewHtmlEditor.m_webview2->cut();
	}
	return 0L;
}
/// <summary>
/// Handle the Run Functor message
/// </summary>
/// <param name=""></param>
/// <param name="wParam"></param>
/// <param name="lParam"></param>
/// <param name=""></param>
/// <returns></returns>
LRESULT CMainFrame::OnRunFunctor(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	auto* pFunctor = reinterpret_cast<WebView2::UIFunctorBase*>(wParam);
	auto* pID = reinterpret_cast<unsigned int*>(lParam);
	if (pFunctor)
	{
		pFunctor->Invoke();         // Execute the functor
		pFunctor->SignalComplete(); // Signal completion
	}
	if (pID != nullptr && *pID == RUN_SCRIPT)
	{
		ExecuteScriptContent();
	}
	if (pID != nullptr)
		delete pID;

	return 0;
}
/// <summary>
/// 
/// </summary>
/// <returns></returns>
HWND CMainFrame::get_hwnd()
{
	return m_hWnd;
}
/// <summary>
/// 
/// </summary>
/// <param name="script"></param>
void CMainFrame::set_script(std::wstring script)
{
	m_powershellscript = script;
}

LRESULT CMainFrame::ExecuteScriptContent()
{
	USES_CONVERSION;
	LOG_DEBUG << __FUNCTION__;
	PowerShellHostmodule console;

	if (!m_powershellscript.empty())
	{
		auto output = console.run_pwsh_lib(m_powershellscript, ExecutionPolicy::Unrestricted, PowerShellLibType::Script);
		if (!output.empty())
		{
			::SendMessageW(m_editView.m_editlog.m_hWnd, WM_SETTEXT, 0, (LPARAM)output.c_str());
		}
	}
	return 0;
}

/// <summary>
/// Execute the powershell script using the file path or the content of the editor, use the OnRunFunctor for async execute_script_with_result
/// </summary>
/// <param name=""></param>
/// <param name=""></param>
/// <param name=""></param>
/// <param name=""></param>
/// <returns></returns>
LRESULT CMainFrame::OnScenarioMonacoRun(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	USES_CONVERSION;

	LOG_DEBUG << __FUNCTION__;

	PowerShellHostmodule HostPowerShell;

	if (fs::exists(m_powershellfile))
	{
		auto output = HostPowerShell.run_pwsh_lib(m_powershellfile, ExecutionPolicy::Unrestricted, PowerShellLibType::File);
		if (!output.empty())
		{
			::SendMessageW(m_editView.m_editlog.m_hWnd, WM_SETTEXT, 0, (LPARAM)output.c_str());
		}
	}
	else
	{
		std::wstring ret;
		std::wstring script = L"editor.getValue();";
		auto var = m_viewHtmlEditor.m_webview2->execute_script_with_result(script, this);
	}
	return 0;
}

LRESULT CMainFrame::OnScenarioMonacoOpenVSCode(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	fs::path vscodepath;
	auto error = WebView2::Utility::DetectVisualStudioCode(vscodepath);
	if (error)
	{
		this->MessageBox(L"Visual Studio Code not found", L"Error", MB_OK | MB_ICONERROR);
	}
	else
	{
		if (!m_powershellfile.empty())
		{
			error = WebView2::Utility::LaunchVisualStudioCode(vscodepath, m_powershellfile);
		}
	}
	return 0;
}
LRESULT CMainFrame::OnScenarioMonacoCoPilot(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	PowerShellHostmodule HostPowerShell;

	//set mouse cursor to wait

	if (!m_powershellfile.empty())
	{
		SetCursor(::LoadCursor(nullptr, IDC_WAIT));
		auto output = HostPowerShell.llmconvert(m_powershellfile, m_connectionstring);
		SetCursor(::LoadCursor(nullptr, IDC_ARROW));
		if (!output.empty())
		{
			std::wstring wsourceCode = output;
			// add ps1 extension to the file name
			m_powershellfile += L".ps1";
			std::ofstream fileout(m_powershellfile);
			fileout << wstring_to_utf8(wsourceCode);
			fileout.close();

			std::ifstream file(m_powershellfile);
			if (file.is_open())
			{
				std::stringstream buffer;
				buffer << file.rdbuf();
				m_sourceCode = std::make_unique<std::string>(buffer.str());

				std::wstring wsourceCode(m_sourceCode->begin(), m_sourceCode->end());
				std::wstring script = L"editor.setValue(`";
				script += wsourceCode;
				script += L"`);";
				m_viewHtmlEditor.m_webview2->execute_script(script);
			}
		}
	}
	return 0;
}

LRESULT CMainFrame::OnScenarioLLMSettings(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	auto dlg = std::make_unique<LLMDialog>();
	if (dlg->DoModal() == IDOK)
	{
		// store endpoint secrety and deployment name in member variables
		auto endpoint = dlg->GetEndpoint();
		auto secret = dlg->GetSecret();
		auto deploymentname = dlg->GetDeploymentName();

		//if endpoint and secret are not empty, then set the current LLM to Azure
		// use this syntax uri=;key=;modele=;
		if (!endpoint.empty() && !secret.empty() && !deploymentname.empty())
		{
			m_connectionstring = L"uri=" + endpoint + L";key=" + secret + L";modele=" + deploymentname;
			m_currentLLM = CurentLLM::Azure;
		}
		else
		{
			m_currentLLM = CurentLLM::None;
		}
	}
	return 0;
}
