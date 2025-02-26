#pragma once

class CURLCombo : public CWindowImpl<CURLCombo, CComboBoxEx>, public CMessageFilter
{
private:
	HWND parentFrame_;
public:

	CURLCombo();

	BEGIN_MSG_MAP(CURLCombo)
		MESSAGE_HANDLER(WM_COMMAND, OnCommand)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	BOOL SubclassWindow(HWND hWnd);
	LRESULT OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	void SetFrame(HWND hwnd);
	BOOL PreTranslateMessage(MSG* pMsg);
};
