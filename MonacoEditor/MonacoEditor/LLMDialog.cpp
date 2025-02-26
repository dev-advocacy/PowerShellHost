#include "pch.h"

#include "resource.h"
#include "LLMDialog.h"

LRESULT LLMDialog::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CenterWindow(GetParent());
	DoDataExchange(FALSE);
	return TRUE;
}

LRESULT LLMDialog::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	DoDataExchange(TRUE);
	EndDialog(wID);
	return 0;
}
// Get all 	CString m_strEndpoint;m_strSecret;m_strDeploymentName;, return string_t
string_t LLMDialog::GetEndpoint()
{
	return string_t(m_strEndpoint);
}
string_t LLMDialog::GetSecret()
{
	return string_t(m_strSecret);
}
string_t LLMDialog::GetDeploymentName()
{
	return string_t(m_strDeploymentName);
}

