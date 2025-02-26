#include "pch.h"
#include "CViewEdit.h"

LRESULT CViewEdit::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CPaintDC dc(m_hWnd);
	return 0;
}
LRESULT CViewEdit::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	auto m_hWndClient = m_editlog.Create(m_hWnd, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | ES_AUTOVSCROLL
		| WS_HSCROLL | ES_MULTILINE | WS_VSCROLL);
	return 0;
}

void CViewEdit::InitializeFont(void)
{
	if (m_font == nullptr)
	{
		std::wstring fontName = L"Cascadia Code";
		m_font = std::make_unique<WTL::CFont>();

		if (!FontExists(fontName))
		{
			fontName = L"Consolas";
		}

		CLogFont logFont;
		CClientDC dc(this->m_hWnd);
		logFont.SetHeight(-10, dc);
		::lstrcpy(logFont.lfFaceName, fontName.data());

		this->m_font->Attach(logFont.CreateFontIndirect());
		this->m_editlog.SetFont(*m_font);
	}
}

LRESULT CViewEdit::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if (m_editlog != nullptr)
	{
		CRect rcClient;
		GetClientRect(&rcClient);
		if (rcClient.Width() <= 0 || rcClient.Height() <= 0)
		{
			return 0;
		}
		else
		{
			m_editlog.SetWindowPos(nullptr, &rcClient, SWP_NOZORDER | SWP_NOACTIVATE);
			InitializeFont();
		}
	}
	return 0;
}

bool CViewEdit::FontExists(std::wstring_view fontName)
{
	LOGFONT logFont = { 0 };
	wcscpy_s(logFont.lfFaceName, fontName.data());

	HDC hdc = GetDC();
	bool fontFound = false;

	EnumFontFamiliesEx(hdc, &logFont, [](const LOGFONT* lpelfe, const TEXTMETRIC* lpntme, DWORD FontType, LPARAM lParam) -> int
		{
			*(bool*)lParam = true;
			return 0;
		}, (LPARAM)&fontFound, 0);

	ReleaseDC(hdc);
	return fontFound;
}