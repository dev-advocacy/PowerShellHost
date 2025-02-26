#include "pch.h"
#include "resource.h"
#include "ClientCertificate.h"
#include "CertificateDlg.h"
#include "osutility.h"

// https://certtestdemo.azurewebsites.net/

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

LRESULT CCertificateDlg::OnCtrlColor(UINT, WPARAM wParam, LPARAM, BOOL& handled)
{
	return (m_theme_color.SetWindowBackgroudColor(wParam));
}

LRESULT CCertificateDlg::OnSettingChange(UINT, WPARAM wParam, LPARAM lParam, BOOL&)
{
	if (wParam == 0 && lParam != 0)
	{
		const std::wstring param{ (wchar_t*)lParam };
		if (param == L"ImmersiveColorSet")
		{
			os::utility::SetWindowBackgroud(this->m_hWnd);
		}
	}
	return 0;
}
CCertificateDlg::CCertificateDlg(std::vector<ClientCertificate> client_certificates,std::wstring host_name, HWND hwnd_parent) : m_client_certificates(client_certificates), m_hwnd_parent(hwnd_parent), m_host_name(host_name)
{}
LRESULT CCertificateDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	m_OK.SubclassWindow(GetDlgItem(IDOK));
	m_OK.SetBkColor(BLUE_COLOR);
	m_OK.SetTextColor(WHITE_COLOR);
	
	m_Cancel.SubclassWindow(GetDlgItem(IDCANCEL));
	m_Cancel.SetBkColor(BLUE_COLOR);
	m_Cancel.SetTextColor(WHITE_COLOR);

	BOOL value = TRUE;

	os::utility::SetWindowBackgroud(this->m_hWnd);

	m_ImageList_certificate.Create(80, 64, TRUE | ILC_COLOR32, 1, 1);
	m_List_certificate.SubclassWindow(GetDlgItem(IDC_LIST_CERTIFICATE));
	m_List_certificate.SetItemHeight(0, 70);
	m_List_certificate.SetImageList(m_ImageList_certificate.m_hImageList, ILSIL_NORMAL);
	m_List_certificate.SetImageList(m_ImageList_certificate.m_hImageList, ILSIL_SELECTED);

	CSize size(16,16);
	m_ImageList_certificate.SetIconSize(size);

	auto hIconCert = LoadIcon(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_ICON_CERTIFICATE));	
	m_ImageList_certificate.AddIcon(hIconCert);

	auto hIconPin = LoadIcon(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_ICON_PIN));
	m_ImageList_certificate.AddIcon(hIconPin);

	for (auto client_certificate : m_client_certificates)
	{
		ILBITEM item = { 0 };
		item.mask = ILBIF_TEXT | ILBIF_IMAGE | ILBIF_SELIMAGE | ILBIF_STYLE | ILBIF_FORMAT;
		item.iItem = 0;
		item.iImage = 0;
		item.iSelImage = 0;
		item.iSelImage = 0;

		std::wstring wstr = client_certificate.DisplayName.get();
		wstr += L"\n";
		wstr += client_certificate.Issuer.get();
		wstr += L"\n";
		wstr += UnixEpochToDateTime(client_certificate.ValidTo);

		item.pszText = const_cast<LPTSTR>(wstr.c_str());

		item.style = ILBS_IMGLEFT | ILBS_SELROUND;
		m_List_certificate.InsertItem(&item);
	}	
	if (m_client_certificates.size() > 0)
		m_List_certificate.SelectString(0, m_client_certificates[0].DisplayName.get());

	SetWindowPos(this->m_hwnd_parent, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);	

	CenterWindow(GetParent());
	
	// load ddx values
	DoDataExchange(TRUE);

	std::wstring site = L"Site ";
	site += m_host_name;
	site += L" needs your credentials : ";

	m_site_information = site.c_str();
	DoDataExchange(FALSE);
	return TRUE;
}

std::wstring CCertificateDlg::UnixEpochToDateTime(double value) 
{
	WCHAR rawResult[32] = {};
	std::time_t rawTime = std::time_t(value);
	struct tm timeStruct = {};
	gmtime_s(&timeStruct, &rawTime);
	_wasctime_s(rawResult, 32, &timeStruct);
	std::wstring result(rawResult);
	return result;
}
bool CCertificateDlg::AddCertificate(const std::wstring& name, const std::wstring& issuer, const std::wstring& expirationDate)
{
	return true;
}
LRESULT CCertificateDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	set_selectedItem();
	EndDialog(wID);
	return wID;
}
void CCertificateDlg::set_selectedItem()
{
	m_selectedItem = m_List_certificate.GetCurSel();
}
int CCertificateDlg::get_selectedItem()
{
	return m_selectedItem;
}