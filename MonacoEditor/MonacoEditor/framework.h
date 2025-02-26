#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#include <dwmapi.h>

#include <cstdlib>
#include <cstdio>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <string>
#include <string_view>
#include <filesystem>
#include <map>
#include <functional>
#include <regex>
#include <chrono>
#include <future>
#include <mutex>
#include <unordered_set>
#include <fstream>

// ATL/WTL
#include <atlbase.h>
#include <atlapp.h>
#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlctrlw.h>
#include <atlsplit.h>
#include <atlmisc.h>
#include <atlctrlx.h>
#include <atlcrack.h>
#include <atlddx.h>
#include <atlimage.h>
#include <atlwin.h>


extern CAppModule _Module;

#include <wil/com.h>
#include <wil/resource.h>
#include <wil/result.h>
#include <wrl.h>

// WebView
#include <WebView2.h>
#include <WebView2EnvironmentOptions.h>


// logs
#include <boost/log/trivial.hpp>
#include <boost/log/core.hpp>
#include <boost/log/common.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/expressions/keyword.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/log/sinks/text_ipc_message_queue_backend.hpp>
#include <boost/log/utility/ipc/reliable_message_queue.hpp>
#include <boost/log/utility/ipc/object_name.hpp>

//Windows
#include <ppl.h>
#include <concurrent_unordered_map.h>
#include <d2d1_3.h>
#include <dcomp.h>
#include <wincodec.h>
#include <DispatcherQueue.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <windows.ui.composition.interop.h>
#include <winrt/Windows.UI.Composition.Desktop.h>
#include <winrt/Windows.UI.Composition.h>
#include <winrt/Windows.UI.ViewManagement.h>
#include <dpapi.h>

#include <nethost.h>
#include <coreclr_delegates.h>
#include <hostfxr.h>

#include "basic_type.h"

//namespaces

namespace	fs = std::filesystem;
namespace	logging = boost::log;
namespace	src = boost::log::sources;
namespace	attrs = boost::log::attributes;
namespace	sinks = boost::log::sinks;
namespace	expr = boost::log::expressions;
namespace	keywords = boost::log::keywords;


// custom messages
inline constexpr UINT MSG_NAVIGATE = WM_APP + 123;
inline constexpr UINT MSG_RUN_ASYNC_CALLBACK = WM_APP + 124;
inline constexpr UINT MSG_NAVIGATE_CALLBACK = WM_APP + 125;
inline constexpr UINT MSG_GETCOOKIES_CALLBACK = WM_APP + 126;
inline constexpr UINT MSG_CALLBACK = WM_APP + 127;
inline constexpr UINT WM_RUN_FUNCTOR = WM_APP + 128;
inline constexpr int TEXT_SIZE = 1024;
inline constexpr int ERR_WEBVIEW_NOT_INSTALLED = -1024;
inline constexpr int ERR_RESOURCE_NOT_FOUND = -1025;

#define WM_FILELIST_REFRESH			(WM_APP+150)

#define RUN_SCRIPT		0x0001
#define RUN_EVENT		0x0002

enum ColorMode
{
	Dark,
	Light,
	none
};

#if defined _M_IX86
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
