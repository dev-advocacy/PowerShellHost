#pragma once
class LLMDialog : public CDialogImpl<LLMDialog>, public CWinDataExchange<LLMDialog>
{
public:
	enum { IDD = IDD_DIALOG_LLM};

	BEGIN_MSG_MAP(LLMDialog)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	BEGIN_DDX_MAP(LLMDialog)
		DDX_TEXT(IDC_EDIT_ENDPOINT, m_strEndpoint)
		DDX_TEXT(IDC_EDIT_SECRET, m_strSecret)
		DDX_TEXT(IDC_EDIT_DEPLOYMENT_NAME, m_strDeploymentName)
	END_DDX_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

public:
	string_t GetEndpoint();
	string_t GetSecret();
	string_t GetDeploymentName();

protected:
	CString m_strEndpoint;
	CString m_strSecret;
	CString m_strDeploymentName;
};