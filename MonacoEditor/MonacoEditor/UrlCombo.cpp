#include "pch.h"
#include "resource.h"
#include "UrlCombo.h"

CURLCombo::CURLCombo() : parentFrame_(nullptr)
{
	
}

BOOL CURLCombo::SubclassWindow(HWND hWnd)
{
	return(CWindowImpl< CURLCombo, CComboBoxEx >::SubclassWindow(hWnd));
}

LRESULT CURLCombo::OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	int wNotifyCode = HIWORD(wParam);

	if (WM_COMMAND == uMsg && wNotifyCode == CBN_SELCHANGE)
	{
		if (parentFrame_ != nullptr)
		{
			std::wstring text_url;
			text_url.resize(1024);
			if (GetItemText(this->GetCurSel(), &text_url[0], 1024) == TRUE)
			{
				int index = this->FindStringExact(0, text_url.c_str());
				if (index != CB_ERR)
				{
					this->SetCurSel(index);
					::SendMessageW(parentFrame_, MSG_NAVIGATE_CALLBACK, 0, LPARAM(text_url.c_str()));
				}
			}
		}
	}
	return 0;
}

void CURLCombo::SetFrame(HWND hwnd)
{
	parentFrame_ = hwnd;
}

BOOL CURLCombo::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		int nVirtKey = (int)pMsg->wParam;

		if (nVirtKey == VK_RETURN)
		{
			if (parentFrame_ != nullptr)
			{
				auto len = GetWindowTextLength() + 1;
				std::wstring text_url;
				text_url.resize(len);
				if (GetWindowText(&text_url[0], len) > 0)
				{
					if (PathIsURL(text_url.c_str()) == TRUE)
					{
						if (this->FindStringExact(0, const_cast<LPWSTR>(text_url.c_str())) == CB_ERR)
						{
							COMBOBOXEXITEM item = { 0 };
							item.mask = CBEIF_TEXT;
							item.iItem = 0;
							item.pszText = (LPWSTR)text_url.c_str();
							this->InsertItem(&item);
						}
						::SendMessageW(parentFrame_, MSG_NAVIGATE_CALLBACK, 0, LPARAM(text_url.c_str()));
					}
				}
			}
		}
	}
	return FALSE;
}