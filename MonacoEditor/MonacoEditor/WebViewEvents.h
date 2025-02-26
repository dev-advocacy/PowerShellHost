#pragma once
#include <urlmon.h>
#include "Utility.h"
#include "ClientCertificate.h"
#include "CertificateDlg.h"
#include "logger.h"

/// <summary>
/// https://github.com/TylerLeonhardt/monaco-powershell/blob/master/build.ps1
/// </summary>

namespace WebView2
{
	// Base class of functors that must run on the UI thread.
	class UIFunctorBase
	{
	public:
		UIFunctorBase()
			: _stopped(false) 
		{}

		virtual ~UIFunctorBase() = default;

		// Called from a background worker thread. Blocks until the Windows message handler completes.
		void PostToQueue(HWND wnd, unsigned int param)
		{
			{	// Reset stop event.
				std::lock_guard guard(_mutex);
				_stopped = false;
			}

			// create a new pointer of unsigned int param
			// Post a custom message to the UI thread.
			unsigned int *pparam = new unsigned int();
			*pparam = param;


			::PostMessageW(wnd, WM_RUN_FUNCTOR, reinterpret_cast<WPARAM>(this), reinterpret_cast<LPARAM>(pparam));

			// Wait for the messaged to be processed on the UI thread.
			std::unique_lock lock(_mutex);
			_stopEvent.wait(lock, [this]() { return _stopped; });

		}
		// Called by the custom Windows message handler => Runs on the UI thread.
		// Override in derived classes.
		virtual void Invoke() = 0;

		// Called by the custom Windows message handler ==> Signals that Invoke() has completed.
		void SignalComplete()
		{
			{
				std::lock_guard guard(_mutex);
				_stopped = true;
			}
			_stopEvent.notify_all();
		}



	protected:
		std::mutex				_mutex;
		std::condition_variable _stopEvent;
		bool					_stopped;
	};


	// Concrete class template for functors that must run on the UI thread.
	// The constructor supports lambda with capture clauses.
	template <typename T>
	class UIFunctor : public UIFunctorBase
	{
	public:
		UIFunctor<T>(T&& lambda)
			: _lambda(lambda)
		{}

		virtual ~UIFunctor() = default;

		virtual void Invoke() override
		{
			_lambda();
		}

	protected:
		T _lambda;
	};


	// Interface that WebView2Impl must implement in order to receive event notifications.
	class IWebWiew2ImplEventCallback
	{
	public:
		virtual HWND GetHWnd() = 0;
		virtual void KeepAliveAsyncResult(std::future<void>&& result) = 0;
		virtual void NavigationStartingEvent(std::wstring_view uri, unsigned long long navigationId,
			                                 bool isRedirected, bool isUserInitiated) = 0;
		virtual void NavigationCompleteEvent(bool isSuccess, unsigned long long navigationId,
											 COREWEBVIEW2_WEB_ERROR_STATUS errorStatus) = 0;
		virtual void ResponseReceivedEvent(std::wstring_view method, std::wstring_view uri) = 0;
		virtual void RequestEvent(std::wstring_view method, std::wstring_view uri,
			                      COREWEBVIEW2_WEB_RESOURCE_CONTEXT resourceContext) = 0;

		virtual void ClientCertificateRequestedEvent(std::vector<ClientCertificate> clientCertificates, wil::com_ptr<ICoreWebView2Deferral> deferral) = 0;
	};
	
	
	class webview2_events
	{
	private:
		wil::com_ptr<ICoreWebView2>				m_webviewEventSource = nullptr;
		wil::com_ptr<ICoreWebView2_2>           m_webviewEventSource2 = nullptr;
		wil::com_ptr<ICoreWebView2_10>          m_webviewEventSource3 = nullptr;
		wil::com_ptr<ICoreWebView2_5>			m_webviewEventSource5 = nullptr;
		wil::com_ptr<ICoreWebView2Controller>	m_controllerEventSource;
		IWebWiew2ImplEventCallback*				m_callback = nullptr;
		EventRegistrationToken                  m_webResourceResponseReceivedToken = {};
		EventRegistrationToken                  m_clientCertificateRequestedToken = {};
		EventRegistrationToken                  m_webResourceRequestedToken = {};	
		EventRegistrationToken                  m_navigationStartingToken = {};
		EventRegistrationToken                  m_navigationCompletedToken = {};

		wil::com_ptr<ICoreWebView2Environment>  m_webViewEnvironment;

		
		std::vector<ClientCertificate>			m_clientCertificates;
		HWND									m_hwnd_parent = nullptr;	
	public:
		webview2_events()
		{
		}
		~webview2_events()
		{


			if (m_webResourceResponseReceivedToken.value != 0)
				m_webviewEventSource2->remove_WebResourceResponseReceived(m_webResourceResponseReceivedToken);		
			if (m_clientCertificateRequestedToken.value != 0)
				m_webviewEventSource3->remove_ClientCertificateRequested(m_clientCertificateRequestedToken);
			if (m_navigationCompletedToken.value != 0)
				m_webviewEventSource->remove_NavigationCompleted(m_navigationCompletedToken);
			if (m_navigationStartingToken.value != 0)
				m_webviewEventSource->remove_NavigationStarting(m_navigationStartingToken);
			if (m_webResourceRequestedToken.value != 0)
				m_webviewEventSource->remove_WebResourceRequested(m_webResourceRequestedToken);			
		}



		HRESULT initialize(IWebWiew2ImplEventCallback* callback, wil::com_ptr<ICoreWebView2> webviewEventSource, wil::com_ptr<ICoreWebView2Controller> controllerEventSource, HWND hwnd_parent, wil::com_ptr<ICoreWebView2Environment> webViewEnvironment)
		{
			LOG_TRACE << __FUNCTION__;
			if (::IsWindow(hwnd_parent))
			{
				m_hwnd_parent = hwnd_parent;
			}

			if ((callback == nullptr) || (webviewEventSource == nullptr) || (controllerEventSource == nullptr))
			{
				RETURN_IF_FAILED_MSG(ERROR_INVALID_PARAMETER, "function = %s, message = %s, hr = %d", WIDE_FUNCTION , Utility::to_wstring(std::system_category().message(ERROR_INVALID_PARAMETER)).c_str(), ERROR_INVALID_PARAMETER);
			}
			m_webviewEventSource = webviewEventSource;
			m_controllerEventSource = controllerEventSource;
			m_webViewEnvironment = webViewEnvironment;

			m_callback = callback;
			if (!m_webviewEventSource.try_query_to<ICoreWebView2_2>(&m_webviewEventSource2))
				return ERROR_INVALID_DATA;
			if (!m_webviewEventSource.try_query_to<ICoreWebView2_10>(&m_webviewEventSource3))
				return ERROR_INVALID_DATA;

			if (!m_webviewEventSource.try_query_to<ICoreWebView2_5>(&m_webviewEventSource5))
				return ERROR_INVALID_DATA;
			RETURN_IF_FAILED(enable_webresource_navigation_starting_event());
			RETURN_IF_FAILED(enable_webresource_navigation_complete_event());
			RETURN_IF_FAILED(enable_webresource_response_received_event());
			RETURN_IF_FAILED(enable_webresource_request_event());
			RETURN_IF_FAILED(enable_client_certificate_request_event());
			return S_OK;
		}
	private:
		void trace_webresource_response_received_event(wil::com_ptr<ICoreWebView2WebResourceRequest> webResourceRequest)
		{
			//LOG_TRACE << __FUNCTION__;
		}
		void raise_webresource_response_received_event(wil::com_ptr<ICoreWebView2WebResourceRequest> webResourceRequest)
		{
			if (m_callback == nullptr)
				return;

			wil::unique_cotaskmem_string method;
			webResourceRequest->get_Method(&method);
			std::wstring methodText = method.get();

			wil::unique_cotaskmem_string uri;
			webResourceRequest->get_Uri(&uri);
			std::wstring uriText = uri.get();

			auto asyncResult = std::async(std::launch::async, [=, this]()
				{
					UIFunctor functor([&, this]()
						{
							m_callback->ResponseReceivedEvent(methodText, uriText);
						});

					functor.PostToQueue(m_callback->GetHWnd(), RUN_EVENT);
				});

			// Need to keep alive future<void> instance until fire-and-forget async operation completes.
			// See Herb Sutter's paper: https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2012/n3451.pdf
			m_callback->KeepAliveAsyncResult(std::move(asyncResult));
		}
		void trace_webresource_navigation_complete_event(wil::com_ptr <ICoreWebView2NavigationCompletedEventArgs> args)
		{
			BOOL is_success = FALSE;
			UINT64 inavigationid = 0;

			args->get_IsSuccess(&is_success);
			args->get_NavigationId(&inavigationid);

			LOG_TRACE << __FUNCTION__ << " IsSuccess:" << is_success << " NavigationId:" << inavigationid;
		}		
		void raise_webresource_navigation_complete_event(wil::com_ptr <ICoreWebView2NavigationCompletedEventArgs> args)
		{
			if (m_callback == nullptr)
				return;

			BOOL isSuccess = FALSE;
			args->get_IsSuccess(&isSuccess);

			unsigned long long navigationId = 0ull;
			args->get_NavigationId(&navigationId);

			COREWEBVIEW2_WEB_ERROR_STATUS errorStatus;
			args->get_WebErrorStatus(&errorStatus);


			auto asyncResult = std::async(std::launch::async, [=, this]()
				{
					UIFunctor functor([&, this]()
						{
							m_callback->NavigationCompleteEvent(isSuccess ? true : false, navigationId,
								errorStatus);
						});

					functor.PostToQueue(m_callback->GetHWnd(), RUN_EVENT);
				});

			// Need to keep alive future<void> instance until fire-and-forget async operation completes.
			// See Herb Sutter's paper: https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2012/n3451.pdf
			m_callback->KeepAliveAsyncResult(std::move(asyncResult));
		}
		void trace_webresource_navigation_starting_event(wil::com_ptr <ICoreWebView2NavigationStartingEventArgs> args)
		{
			BOOL is_redirected = FALSE;
			BOOL is_userInitiated = FALSE;
			UINT64 inavigationid = 0;
			args->get_IsRedirected(&is_redirected);
			args->get_IsUserInitiated(&is_userInitiated);
			args->get_NavigationId(&inavigationid);

			wil::unique_cotaskmem_string navigationTargetUri;
			if (SUCCEEDED(args->get_Uri(&navigationTargetUri)))
			{
				LOG_TRACE << __FUNCTION__ << " uri:" << navigationTargetUri.get() << " redirected:" << is_redirected
						  << " is user initiated:" << is_userInitiated << "NavigationId" << inavigationid;
			}
		}
		void raise_webresource_navigation_starting_event(wil::com_ptr <ICoreWebView2NavigationStartingEventArgs> args)
		{
			if (m_callback == nullptr)
				return;

			wil::unique_cotaskmem_string uri;
			args->get_Uri(&uri);
			std::wstring uriText = uri.get();

			unsigned long long navigationId = 0ull;
			args->get_NavigationId(&navigationId);

			BOOL isRedirected = FALSE;
			args->get_IsRedirected(&isRedirected);

			BOOL isUserInitiated = FALSE;
			args->get_IsUserInitiated(&isUserInitiated);

			auto asyncResult = std::async(std::launch::async, [=, this]()
				{
					UIFunctor functor([&, this]()
						{
							m_callback->NavigationStartingEvent(uriText, navigationId, 
								isRedirected ? true : false, isUserInitiated ? true : false);
						});
			
					functor.PostToQueue(m_callback->GetHWnd(), RUN_EVENT);
				});

			// Need to keep alive future<void> instance until fire-and-forget async operation completes.
			// See Herb Sutter's paper: https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2012/n3451.pdf
			m_callback->KeepAliveAsyncResult(std::move(asyncResult));
		}

		template <typename T>
		bool starts_with(const std::basic_string<T>& str, const std::basic_string<T>& prefix)
		{
			return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
		}

		void trace_webresource_request_event(wil::com_ptr<ICoreWebView2WebResourceRequestedEventArgs> args)
		{
			wil::com_ptr<ICoreWebView2Deferral> deferral;
			args->GetDeferral(&deferral);

			auto asyncResult = std::async(std::launch::async, [=, this]()
				{
					UIFunctor functor([&, this]()
						{						
							std::string						page;
							wil::unique_cotaskmem_string	uri;
							std::wstring					path			= Utility::GetExecutablePath();
							std::wstring					uri_string;
							std::wstring					finalcontenttype = L"Content-Type:";

							wil::com_ptr<ICoreWebView2WebResourceRequest> webResourceRequest;

							RETURN_IF_FAILED(args->get_Request(&webResourceRequest));							
							RETURN_IF_FAILED(webResourceRequest->get_Uri(&uri));
						
							uri_string = uri.get();
							auto file_uri_string = Utility::ConvertUriToFile(uri_string, path);
							auto contenttype = Utility::GetMimeMapping(uri_string);

							std::ifstream file(file_uri_string, std::ios::binary);
							if (file.is_open())
							{
								std::stringstream buffer;
								buffer << file.rdbuf();
								page = buffer.str();
								
							}
							else
							{
								LOG_TRACE << "unable to open the file";
								page = "<html><body><h1>404 Not Found</h1></body></html>";
							}
							// create a stream from the string
							Microsoft::WRL::ComPtr<IStream> responseStream;
							responseStream = SHCreateMemStream((const BYTE*)page.c_str(),static_cast<UINT>(page.length()));

							finalcontenttype += contenttype;

							Microsoft::WRL::ComPtr<ICoreWebView2WebResourceResponse> response;
							RETURN_IF_FAILED(m_webViewEnvironment->CreateWebResourceResponse(responseStream.Get(), 200, L"OK", finalcontenttype.c_str(), &response));
							RETURN_IF_FAILED(args->put_Response(response.Get()));
							RETURN_IF_FAILED(deferral->Complete());
							return S_OK;
						});
					functor.PostToQueue(m_callback->GetHWnd(), RUN_EVENT);
				});
			m_callback->KeepAliveAsyncResult(std::move(asyncResult));
		}


		void raise_webresource_request_event(wil::com_ptr<ICoreWebView2WebResourceRequestedEventArgs> args)
		{
			if (m_callback == nullptr)
				return;

			COREWEBVIEW2_WEB_RESOURCE_CONTEXT resourceContext;
			args->get_ResourceContext(&resourceContext);

			wil::com_ptr<ICoreWebView2WebResourceRequest> webResourceRequest;
			std::wstring methodText;
			std::wstring uriText;
			
			if (SUCCEEDED(args->get_Request(&webResourceRequest)) && webResourceRequest)
			{
				wil::unique_cotaskmem_string method;
				webResourceRequest->get_Method(&method);
				methodText = method.get();

				wil::unique_cotaskmem_string uri;
				webResourceRequest->get_Uri(&uri);
				uriText = uri.get();
			}

			auto asyncResult = std::async(std::launch::async, [=, this]()
				{
					UIFunctor functor([&, this]()
						{
							m_callback->RequestEvent(methodText, uriText, resourceContext);
						});

					functor.PostToQueue(m_callback->GetHWnd(), RUN_EVENT);
				});

			// Need to keep alive future<void> instance until fire-and-forget async operation completes.
			// See Herb Sutter's paper: https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2012/n3451.pdf
			m_callback->KeepAliveAsyncResult(std::move(asyncResult));
		}

		static PCWSTR NameOfCertificateKind(COREWEBVIEW2_CLIENT_CERTIFICATE_KIND kind)
		{
			switch (kind)
			{
			case COREWEBVIEW2_CLIENT_CERTIFICATE_KIND_SMART_CARD:
				return L"Smart Card";
			case COREWEBVIEW2_CLIENT_CERTIFICATE_KIND_PIN:
				return L"PIN";
			default:
				return L"Other";
			}
		}

		void HandleCertificate()
		{

		}


		HRESULT enable_client_certificate_request_event() noexcept
		{
			LOG_TRACE << __FUNCTION__;
			HRESULT hr = S_OK;

			m_webviewEventSource5->add_ClientCertificateRequested(
				Microsoft::WRL::Callback<ICoreWebView2ClientCertificateRequestedEventHandler>([this](ICoreWebView2* webview, ICoreWebView2ClientCertificateRequestedEventArgs* args)
					-> HRESULT
					{
						
						auto showDialog = [this, args]
						{
							wil::com_ptr<ICoreWebView2ClientCertificateCollection> certificateCollection;
							args->get_MutuallyTrustedCertificates(&certificateCollection);

							m_clientCertificates.clear();

							RETURN_IF_FAILED(args->get_MutuallyTrustedCertificates(&certificateCollection));
							wil::unique_cotaskmem_string host;
							RETURN_IF_FAILED(args->get_Host(&host));
							INT port = FALSE;
							RETURN_IF_FAILED(args->get_Port(&port));
							UINT certificateCollectionCount;
							RETURN_IF_FAILED(certificateCollection->get_Count(&certificateCollectionCount));
							wil::com_ptr<ICoreWebView2ClientCertificate> certificate = nullptr;
							if (certificateCollectionCount > 0)
							{
								ClientCertificate clientCertificate;
								for (UINT i = 0; i < certificateCollectionCount; i++)
								{
									RETURN_IF_FAILED(certificateCollection->GetValueAtIndex(i, &certificate));
									RETURN_IF_FAILED(certificate->get_Subject(&clientCertificate.Subject));
									RETURN_IF_FAILED(certificate->get_DisplayName(&clientCertificate.DisplayName));
									RETURN_IF_FAILED(certificate->get_Issuer(&clientCertificate.Issuer));
									COREWEBVIEW2_CLIENT_CERTIFICATE_KIND Kind;
									RETURN_IF_FAILED(certificate->get_Kind(&Kind));
									clientCertificate.CertificateKind = NameOfCertificateKind(Kind);
									RETURN_IF_FAILED(certificate->get_ValidFrom(&clientCertificate.ValidFrom));
									RETURN_IF_FAILED(certificate->get_ValidTo(&clientCertificate.ValidTo));
									m_clientCertificates.push_back(clientCertificate);
								}
								std::wstring host_name = host.get();
								host_name += L":";
								host_name += std::to_wstring(port);

								CCertificateDlg dlg(m_clientCertificates, host_name, m_hwnd_parent);
								if (dlg.DoModal() == IDOK)
								{
									if (dlg.get_selectedItem() >= 0)
									{
										RETURN_IF_FAILED(certificateCollection->GetValueAtIndex(dlg.get_selectedItem(), &certificate));
										RETURN_IF_FAILED(args->put_SelectedCertificate(certificate.get()));
										args->put_Handled(TRUE);
									}
									else
									{
										args->put_Handled(TRUE);
									}
								}
								else
								{
									args->put_Handled(TRUE);
								}
							}
							return S_OK;
						};

						wil::com_ptr<ICoreWebView2Deferral> deferral;
						args->GetDeferral(&deferral);

						auto asyncResult = std::async(std::launch::async, [deferral, showDialog]()
						{
							showDialog();
							deferral->Complete();
						});										
						m_callback->ClientCertificateRequestedEvent(this->m_clientCertificates, deferral);
						return S_OK;
					}).Get(),
					&m_clientCertificateRequestedToken);
			return (hr);
		}

		HRESULT enable_webresource_response_received_event()
		{
			LOG_TRACE << __FUNCTION__;
			m_webviewEventSource2->add_WebResourceResponseReceived(
					Microsoft::WRL::Callback<ICoreWebView2WebResourceResponseReceivedEventHandler>(
					[this](ICoreWebView2* webview, ICoreWebView2WebResourceResponseReceivedEventArgs* args)
					-> HRESULT {
						wil::com_ptr<ICoreWebView2WebResourceRequest> webResourceRequest;
						RETURN_IF_FAILED(args->get_Request(&webResourceRequest));
						wil::com_ptr<ICoreWebView2WebResourceResponseView> webResourceResponse;
						RETURN_IF_FAILED(args->get_Response(&webResourceResponse));
						webResourceResponse->GetContent(Microsoft::WRL::Callback<ICoreWebView2WebResourceResponseViewGetContentCompletedHandler>(
														[this, webResourceRequest,
														webResourceResponse](HRESULT result, IStream* content)
						{
							trace_webresource_response_received_event(webResourceRequest);
							raise_webresource_response_received_event(webResourceRequest);
							return S_OK;
					})
				.Get());
				return S_OK;})
				.Get(), &m_webResourceResponseReceivedToken);
			return S_OK;
		}
		HRESULT enable_webresource_navigation_complete_event()
		{
			LOG_TRACE << __FUNCTION__;
			RETURN_IF_FAILED(m_webviewEventSource->add_NavigationCompleted(Microsoft::WRL::Callback<ICoreWebView2NavigationCompletedEventHandler>([this](
				ICoreWebView2* core_web_view2,
				ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT
				{
					trace_webresource_navigation_complete_event(args);
					raise_webresource_navigation_complete_event(args);
					return S_OK;
				}).Get(), &m_navigationCompletedToken));

			return S_OK;
		}
		HRESULT enable_webresource_navigation_starting_event()
		{
			LOG_TRACE << __FUNCTION__;
			RETURN_IF_FAILED(m_webviewEventSource->add_NavigationStarting(Microsoft::WRL::Callback<ICoreWebView2NavigationStartingEventHandler>([this](
				ICoreWebView2* core_web_view2,
				ICoreWebView2NavigationStartingEventArgs* args) -> HRESULT
				{			
					trace_webresource_navigation_starting_event(args);
					raise_webresource_navigation_starting_event(args);
					return S_OK;
				}).Get(), &m_navigationStartingToken));
			return S_OK;
		}
		HRESULT enable_webresource_request_event()
		{
			LOG_TRACE << __FUNCTION__;
			RETURN_IF_FAILED(m_webviewEventSource->AddWebResourceRequestedFilter(L"*", COREWEBVIEW2_WEB_RESOURCE_CONTEXT_ALL));
			RETURN_IF_FAILED(m_webviewEventSource->add_WebResourceRequested(Microsoft::WRL::Callback<ICoreWebView2WebResourceRequestedEventHandler>([this](
				ICoreWebView2* core_web_view2,
				ICoreWebView2WebResourceRequestedEventArgs* args) -> HRESULT
				{
					trace_webresource_request_event(args);
					raise_webresource_request_event(args);
					return S_OK;
				}).Get(), &m_webResourceRequestedToken));
			return S_OK;
		}				
	};
}