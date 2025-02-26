#pragma once
// Returns Program Files directory x86.

namespace os
{
	class utility
	{
		public:
			enum imagesize 
			{
				size24,
				size32
			};
		public:
			utility();
			static void SetWindowBackgroud(HWND hwnd);
			static LRESULT SetWindowBackgroudColor(WPARAM wParam);
			static void GetImages(imagesize isize);
			static std::wstring GetimageFolder(imagesize isize);
			static std::wstring GetSvgFolder();
			static std::wstring BoolToString(BOOL value);
			static std::wstring EncodeQuote(std::wstring raw);
			static std::wstring SecondsToString(UINT32 time);
			static std::wstring CookieToString(ICoreWebView2Cookie* cookie);		

		private:
			static std::wstring GetLangStringFromLangId(DWORD dwLangID_i, bool returnShortCode);
	
	};

	class theme
	{
	public:
		theme(theme const&) = delete;
		theme& operator=(theme const&) = delete;
		~theme() {}

		ColorMode get_color_mode()
		{
			ATL::CRegKey hkcu_themes_regkey_;

			if (ERROR_SUCCESS == hkcu_themes_regkey_.Open(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", KEY_READ))
			{
				DWORD apps_use_light_theme = 1;
				hkcu_themes_regkey_.QueryDWORDValue(L"AppsUseLightTheme", apps_use_light_theme);
				if (apps_use_light_theme == 0)
					return ColorMode::Dark;
				else if (apps_use_light_theme == 1)
					return ColorMode::Light;
				else
					return ColorMode::none;
			}
			else
				return ColorMode::none;
		}

		static theme* get_instance()
		{
			static theme instance{};
			return &instance;
		}

	private:
		explicit theme() : m_value{ 0 } {}
		std::mutex m_mutex;
		int m_value;
	};

	class colortheme
	{
	public:
		colortheme();
		LRESULT SetWindowBackgroudColor(WPARAM wParam);
		

	private:
		CBrush	m_brush_dark;
		CBrush	m_brush_light;
	};

	class svg
	{
	public:
		void SaveImage(std::wstring str_sourcefile, std::wstring str_destfile, D2D1_SIZE_F imagesize, FLOAT factor);
		HRESULT Init2D2();
		HRESULT GetSvgDocumentSize(std::wstring str_sourcefile, D2D1_SIZE_F& imagesize);

	private:
		HRESULT DrawImage(wil::com_ptr<ID2D1RenderTarget> pRT, std::wstring str_sourcefile, D2D1_SIZE_F imagesize, FLOAT factor);
		void ExportImage(std::wstring str_destfile, D2D1_SIZE_F imagesize, FLOAT factor);
	private:
		
		wil::com_ptr<ID2D1Factory>			m_pD2DFactory = nullptr;
		wil::com_ptr<IWICBitmap>			m_pWICBitmap = nullptr;
		wil::com_ptr<IWICImagingFactory>	m_pWICFactory = nullptr;
	};
};
