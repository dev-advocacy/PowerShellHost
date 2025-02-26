#pragma once
#include "pch.h"
#include "resource.h"
#include "CheckFailure.h"
#include "logger.h"
#include "CompositionHost.h"
#include "WebViewEvents.h"
#include "WebViewAuthentication.h"

class ContextDataCallBack
{
public:
	virtual HWND get_hwnd() = 0;
	virtual void set_script(std::wstring script) = 0;
};


namespace WebView2
{
	class ContextData
	{
	public:
		std::wstring m_classname = L"Chrome_WidgetWin_0";
		HWND					m_hwnd = nullptr;
	};

	template <class T>
	class CWebView2Impl2 : public CCompositionHost<T>, public IWebWiew2ImplEventCallback
	{
	public:
		std::wstring										m_url;
		std::wstring										m_browser_directory;
		std::wstring										m_user_data_directory;
		bool												m_is_modal = false;
	private:
		HWND												m_hwnd = nullptr;
		wil::com_ptr<ICoreWebView2Environment>				m_webViewEnvironment = nullptr;
		wil::com_ptr<ICoreWebView2CompositionController>	m_compositionController = nullptr;
		wil::com_ptr<ICoreWebView2Controller>				m_controller = nullptr;
		wil::com_ptr<ICoreWebView2>							m_webView = nullptr;
		std::unique_ptr<webview2_events>					m_webview2_events = nullptr;
		std::unique_ptr<webview2_authentication_events>		m_webview2_authentication_events = nullptr;
		std::list<std::future<void>>						m_asyncResults;
		std::mutex											m_asyncResultsMutex;
		HWND												m_hwnd_parent = nullptr;
		std::wstring										m_scriptResult;
	public:
		// Message map and handlers
		BEGIN_MSG_MAP(CWebView2Impl2)
			CHAIN_MSG_MAP(CCompositionHost<T>)
			MESSAGE_HANDLER(WM_CREATE, OnCreate)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			MESSAGE_HANDLER(WM_RUN_FUNCTOR, OnRunFunctor)
		END_MSG_MAP()

		CWebView2Impl2()
		{
			m_webview2_events = std::make_unique<webview2_events>();
			m_webview2_authentication_events = std::make_unique<webview2_authentication_events>();
		};
		void set_parent(HWND hwnd)
		{
			m_hwnd_parent = hwnd;
		}
		virtual ~CWebView2Impl2()
		{
			LOG_TRACE << __FUNCTION__;
		}
		#pragma region windows_event
		LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
		{
			LOG_TRACE << __FUNCTION__;

			T* pT = static_cast<T*>(this);
			if (::IsWindow(pT->m_hWnd))
			{
				m_hwnd = pT->m_hWnd;				
				auto hr = InitializeWebView(true);
				if (FAILED(hr))
				{
					LOG_TRACE << __FUNCTION__ << hr;
					RETURN_IF_FAILED(hr);
				}
				m_is_modal = true;
			}
			return 0L;
		}
		LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
		{
			LOG_TRACE << __FUNCTION__;
			T* pT = static_cast<T*>(this);
			if (::IsWindow(pT->m_hWnd))
			{
				m_hwnd = pT->m_hWnd;
				auto hr = InitializeWebView(true);
				if (FAILED(hr))
				{
					LOG_TRACE << __FUNCTION__ << hr;
					RETURN_IF_FAILED(hr);
				}

			}
			return 0L;
		}
		LRESULT	OnRunFunctor(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
		{
			auto* functor = reinterpret_cast<UIFunctorBase*>(wParam);

			if (functor == nullptr)
				return -1; // Error while posting the message. 

			functor->Invoke();
			functor->SignalComplete();

			// Clean up future<void> instances for fire-and-forget operations that have completed.
			std::lock_guard guard(m_asyncResultsMutex);
			m_asyncResults.remove_if([](std::future<void>& value)
				{
					return value.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready;
				});

			return 0;
		}
		#pragma endregion windows_event

		#pragma region WebView2_event
		HRESULT navigate(std::wstring url)
		{
			if (m_webView)
			{
				m_url = url;
				auto hr = m_webView->Navigate(m_url.c_str());
				if (FAILED(hr))
				{
					LOG_TRACE << __FUNCTION__ << hr;
					RETURN_IF_FAILED(hr);
				}
			}
			else
			{
				LOG_TRACE << __FUNCTION__ << ERROR_INVALID_ADDRESS;
				return ERROR_INVALID_ADDRESS;
			}

			return S_OK;
		}
		HRESULT SetPerm()
		{
			HRESULT hr = S_OK;
			if (m_webView)
			{
				m_webView->add_PermissionRequested(
					Microsoft::WRL::Callback<ICoreWebView2PermissionRequestedEventHandler>(
						[this](							
							ICoreWebView2* sender,
							ICoreWebView2PermissionRequestedEventArgs* args) -> HRESULT
						{
							COREWEBVIEW2_PERMISSION_KIND kind;
							HRESULT hr = args->get_PermissionKind(&kind);
							if (kind == COREWEBVIEW2_PERMISSION_KIND_CLIPBOARD_READ)
							{
								hr = args->put_State(COREWEBVIEW2_PERMISSION_STATE_ALLOW);
							}
							if (FAILED(hr))
							{
								LOG_TRACE << __FUNCTION__ << hr;
								RETURN_IF_FAILED(hr);
							}
							return hr;
						}).Get(), nullptr);
			}
			return hr;
		}

		void open_devtools_window()
		{
			if (m_webView)
			{
				m_webView->OpenDevToolsWindow();
			}
		}

		std::wstring get_Url()
		{
			return m_url;
		}

		void execute_script_reload(std::wstring script)
		{
			HRESULT hr;
			if (m_webView)
			{
				hr = m_webView->AddScriptToExecuteOnDocumentCreated(script.c_str(), nullptr);
				if (FAILED(hr))
				{
					LOG_TRACE << __FUNCTION__ << hr;
					RETURN_IF_FAILED(hr);
				}
				hr = m_webView->Reload();
				if (FAILED(hr))
				{
					LOG_TRACE << __FUNCTION__ << hr;
					RETURN_IF_FAILED(hr);
				}
				hr = m_webView->ExecuteScript(script.c_str(), nullptr);
				if (FAILED(hr))
				{
					LOG_TRACE << __FUNCTION__ << hr;
					RETURN_IF_FAILED(hr);
				}
			}
		}

		HRESULT execute_script(std::wstring script)
		{
			HRESULT hr;
			if (m_webView)
			{
				hr = m_webView->ExecuteScript(script.c_str(), nullptr);
				if (FAILED(hr))
				{
					LOG_TRACE << __FUNCTION__ << hr;
					RETURN_IF_FAILED(hr);
				}
			}
			return hr;
		}
		std::wstring execute_script_with_result(std::wstring script, ContextDataCallBack *contextcallback)
		{
			HRESULT hr;
			std::wstring ret;	
			if (m_webView)
			{
				hr = m_webView->ExecuteScript(script.c_str(), Microsoft::WRL::Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
					[this, &ret, contextcallback](HRESULT error, PCWSTR result) -> HRESULT
					{
						if (error == S_OK)
						{
							m_scriptResult = result;

							auto asyncResult = std::async(std::launch::async, [this, contextcallback]()
								{
									UIFunctor functor([this, contextcallback]()
										{
											LOG_TRACE << "UIFunctor functor([&ret, result, contextcallback]() mutable";
											contextcallback->set_script(m_scriptResult);
											m_scriptResult.clear();
										});

									functor.PostToQueue(contextcallback->get_hwnd(), RUN_SCRIPT);
								});
							KeepAliveAsyncResult(std::move(asyncResult));
						}
						return error;
					}).Get() );
				if (FAILED(hr))
				{
					LOG_TRACE << __FUNCTION__ << hr;
				}
			}
			return(ret);
		}
		HWND get_hwnd()
		{
			HWND parent = nullptr;
			if (SUCCEEDED(m_controller->get_ParentWindow(&parent)))
			{
				ContextData context;

				EnumChildWindows(parent, EnumChildProc, (LPARAM)&context);

				return context.m_hwnd;
			}
			return nullptr;
		}
		HRESULT copy()
		{
			HRESULT hr = S_OK;
			if (m_webView)
			{

				hr = m_webView->ExecuteScript(L"document.execCommand(\"copy\")", nullptr);
				if (FAILED(hr))
				{
					LOG_TRACE << __FUNCTION__ << hr;
					RETURN_IF_FAILED(hr);
				}
			}
			return hr;
		}
		// need to reimplement 
		//m_webView->ExecuteScript(L"document.execCommand(\"paste\")", nullptr);
		HRESULT paste(HWND hwnd)
		{
			HRESULT hr = S_OK;

			if (m_webView)
			{
				hr = m_webView->ExecuteScript(L"navigator.clipboard.writeText(\"blabla\")", nullptr);
				if (FAILED(hr))
				{
					LOG_TRACE << __FUNCTION__ << hr;
					RETURN_IF_FAILED(hr);
				}
			}
			return hr;
		}
		HRESULT cut()
		{
			HRESULT hr = S_OK;
			if (m_webView)
			{
				hr = m_webView->ExecuteScript(L"document.execCommand(\"cut\")", nullptr);
				if (FAILED(hr))
				{
					LOG_TRACE << __FUNCTION__ << hr;
					RETURN_IF_FAILED(hr);
				}
			}
			return hr;
		}
	
		
		#pragma endregion WebView2_event

		// Implement IWebWiew2ImplEventCallback
		virtual HWND GetHWnd() override
		{
			return m_hwnd;
		}


		virtual void KeepAliveAsyncResult(std::future<void>&& result) override
		{
			// Need to keep alive future<void> instance until fire-and-forget async operation completes.
			// See Herb Sutter's paper: https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2012/n3451.pdf
			std::lock_guard guard(m_asyncResultsMutex);
			m_asyncResults.push_back(std::move(result));
		}
		virtual void NavigationStartingEvent(std::wstring_view uri, unsigned long long navigationId, 
			                                 bool isRedirected, bool isUserInitiated) override
		{
			#ifdef DEBUG_WEB_EVENTS
				LOG_TRACE << __FUNCTION__;
				LOG_TRACE << L"  uri=" << uri << L", ID=" << navigationId 
						  << L", redirected=" << isRedirected 
						  << L", user initiated=" << isUserInitiated;
			#endif // DEBUG_WEB_EVENTS
		}
		virtual void NavigationCompleteEvent(bool isSuccess, unsigned long long navigationId,
			                                 COREWEBVIEW2_WEB_ERROR_STATUS errorStatus) override
		{
			#ifdef DEBUG_WEB_EVENTS
				LOG_TRACE << __FUNCTION__;
				LOG_TRACE << L"  success=" << isSuccess << L", ID=" << navigationId
						  << L", error status=" << errorStatus;
			#endif // DEBUG_WEB_EVENTS
		}
		virtual void ResponseReceivedEvent(std::wstring_view method, std::wstring_view uri) override
		{
			#ifdef DEBUG_WEB_EVENTS
				LOG_TRACE << __FUNCTION__;
				LOG_TRACE << L"  method=" << method << L", uri=" << uri;
			#endif // DEBUG_WEB_EVENTS
		}
		virtual void RequestEvent(std::wstring_view method, std::wstring_view uri,
			                      COREWEBVIEW2_WEB_RESOURCE_CONTEXT resourceContext) override
		{
			#ifdef DEBUG_WEB_EVENTS
				LOG_TRACE << __FUNCTION__;
				LOG_TRACE << L"  method=" << method << L", uri=" << uri
						  << L", resource context=" << resourceContext;
			#endif // DEBUG_WEB_EVENTS	
		}
		virtual void ClientCertificateRequestedEvent(std::vector<ClientCertificate> client_certificates, wil::com_ptr<ICoreWebView2Deferral> deferral) override
		{
			#ifdef DEBUG_WEB_EVENTS
				LOG_TRACE << __FUNCTION__;
			#endif // DEBUG_WEB_EVENTS
		}
	private:

		static BOOL CALLBACK EnumChildProc(HWND hwnd, LPARAM lParam)
		{
			ContextData* pThis = (ContextData*)lParam;

			std::array<wchar_t, 1024> dataclassName;

			::GetClassName(hwnd, dataclassName.data(), (int)dataclassName.size());
			std::wstring classname(dataclassName.data());
			if (classname == pThis->m_classname)
			{
				pThis->m_hwnd = hwnd;
			}

			return TRUE;
		}

		HRESULT OnCreateCoreWebView2ControllerCompleted(HRESULT result, ICoreWebView2CompositionController* compositionController)
		{
			LOG_TRACE << __FUNCTION__;
			if (result != S_OK)
				return result;
			m_compositionController = compositionController;
			RETURN_IF_FAILED(m_compositionController->QueryInterface(IID_PPV_ARGS(&m_controller)));
			RETURN_IF_FAILED(m_controller->get_CoreWebView2(&m_webView));


			RETURN_IF_FAILED(m_webview2_events->initialize(this, m_webView, m_controller, m_hwnd_parent, m_webViewEnvironment));
			RETURN_IF_FAILED(m_webview2_authentication_events->initialize(m_hwnd, m_webView, m_controller));
			RETURN_IF_FAILED((static_cast<T*>(this))->initialize(m_hwnd, m_controller, m_compositionController));
			CRect bounds;
			GetClientRect(m_hwnd , &bounds);

			RETURN_IF_FAILED(m_controller->put_IsVisible(true));		
			RETURN_IF_FAILED(m_webView->Navigate(m_url.c_str()));
			(static_cast<T*>(this))->put_bounds(bounds);
			RETURN_IF_FAILED(SetPerm());
			return S_OK;
		}
		HRESULT InitializeWebView(bool log=false)
		{
			LOG_TRACE << __FUNCTION__ << " Using browser directory:" << m_browser_directory.data();
			auto options = Microsoft::WRL::Make<CoreWebView2EnvironmentOptions>();
			HRESULT hr = options->put_ExclusiveUserDataFolderAccess(FALSE);
			if (FAILED(hr))
			{
				LOG_TRACE << __FUNCTION__ << hr;
				RETURN_IF_FAILED(hr);
			}

			if (log == true)
			{
				fs::path log_file;
				if (!Utility::GetProcessName(log_file))
				{
					log_file.append(".log");
					LOG_DEBUG << "Create log file for log-net-log filename: " << log_file;
					auto log = L"--log-net-log=" + log_file.native();
					hr = options->put_AdditionalBrowserArguments(log.c_str()); // Network logs include the network requests, responses, and details on any errors when loading files.
					if (FAILED(hr))
					{
						LOG_TRACE << __FUNCTION__ << hr;
						RETURN_IF_FAILED(hr);
					}
				}
				else
				{
					LOG_ERROR << "Failed to create unique log file name for log-net-log";
				}
			}
			LOG_DEBUG << "Browser cache directory:" << m_user_data_directory.data();
			hr = CreateCoreWebView2EnvironmentWithOptions(
				 m_browser_directory.empty() ? nullptr : m_browser_directory.data(),
				 m_user_data_directory.data(),
				 options.Get(),
				 Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>([this](
				 HRESULT result, ICoreWebView2Environment* environment) -> HRESULT
				 {
					HRESULT hr = S_OK;
					m_webViewEnvironment = environment;
					wil::com_ptr<ICoreWebView2Environment3> webViewEnvironment3 = m_webViewEnvironment.try_query<ICoreWebView2Environment3>();
					if (webViewEnvironment3)
					{
						auto hr = webViewEnvironment3->CreateCoreWebView2CompositionController(m_hwnd,
												Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2CompositionControllerCompletedHandler>(
												[this](HRESULT hr, ICoreWebView2CompositionController* compositionController) -> HRESULT
												{
													if (SUCCEEDED(hr)) {

														hr = OnCreateCoreWebView2ControllerCompleted(hr, compositionController);
														if (FAILED(hr))
														{
															LOG_TRACE << __FUNCTION__ << hr;
															RETURN_IF_FAILED(hr);
														}
													}
													else
													{
														LOG_TRACE << __FUNCTION__ << hr;
														RETURN_IF_FAILED(hr);
													}
													return hr;
										})
									.Get());
							}
							return hr;
						}).Get());

			if (FAILED(hr))
			{
				LOG_TRACE << __FUNCTION__ << hr;
				RETURN_IF_FAILED(hr);
			}
			return hr;
		}
	};
}