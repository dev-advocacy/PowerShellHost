// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H



#define BOOST_USE_WINAPI_VERSION 0x0601
#define _WIN32_WINNT	_WIN32_WINNT_WINBLUE
#define NTDDI_VERSION	0x0A000004

#define USE_WEBVIEW2_WIN10

#define IDM_CREATION_MODE_TARGET_DCOMP  195

#define BLUE_COLOR RGB(0, 108, 190)
#define DIALOG_BACKGROUD_DARK_COLOR  RGB(74, 74, 74)
#define WHITE_COLOR RGB(255, 255, 255)

#define COMPFOLDER 	L"HostPowerShell"

// add headers that you want to pre-compile here
#include "framework.h"

#endif //PCH_H

#define WIDE2(x) L##x
#define WIDECHAR(x) WIDE2(x)
#define WIDE_FUNCTION WIDECHAR(__FUNCTION__)


#if defined _M_IX86
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
