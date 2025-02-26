#pragma once

#include "LogView.h"
#include "ImageListBox.h"
#include "osutility.h"
#include "resource.h"
#include "ColoredControls.h"

class CCertificateDlg : public CDialogImpl<CCertificateDlg>,
						public CWinDataExchange<CCertificateDlg>
{
public:
	enum { IDD = IDD_DIALOG_CERTIFICATE };

	CColoredButtonCtrl m_OK;
	CColoredButtonCtrl m_Cancel;

	BEGIN_DDX_MAP(CCertificateDlg)
		DDX_TEXT(IDC_STATIC_SITE_INFORMATION, m_site_information)
	END_DDX_MAP()

	BEGIN_MSG_MAP(CCertificateDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_SETTINGCHANGE, OnSettingChange)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtrlColor)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtrlColor)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		DEFAULT_REFLECTION_HANDLER()
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCtrlColor(UINT, WPARAM wParam, LPARAM, BOOL&);
	LRESULT OnSettingChange(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);

public:
	CCertificateDlg() = delete;
	CCertificateDlg(std::vector<ClientCertificate> client_certificates,std::wstring host_name, HWND hwnd_parent);
	bool AddCertificate(const std::wstring& name, const std::wstring& issuer, const std::wstring& expirationDate);
	int  get_selectedItem();

private:
	std::wstring	UnixEpochToDateTime(double value);
	void			set_selectedItem();

private:
	CImageListBoxCtrl				m_List_certificate;
	CImageList						m_ImageList_certificate;
	std::vector<ClientCertificate> 	m_client_certificates;
	int								m_selectedItem = -1;								
	HWND							m_hwnd_parent = nullptr;
	std::wstring					m_host_name;	
	os::colortheme					m_theme_color;
	ATL::CString					m_site_information;	
};