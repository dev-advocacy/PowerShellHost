// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H


#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#define BOOST_USE_WINAPI_VERSION	0x0601
#define _WIN32_WINNT				_WIN32_WINNT_WINBLUE
#define NTDDI_VERSION				0x0A000004
#define COMPFOLDER 					L"HostPowerShell"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <string>

#include <cstdlib>
#include <cstdio>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <string_view>
#include <filesystem>
#include <map>
#include <functional>
#include <regex>
#include <chrono>
#include <future>
#include <mutex>
#include <unordered_set>

// clr headers
#include <nethost.h>
#include <coreclr_delegates.h>
#include <hostfxr.h>

#include <shlobj_core.h>

#include <atlconv.h>

// command line parser
#include <boost/program_options.hpp>

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

//namespaces

namespace	fs = std::filesystem;
namespace	logging = boost::log;
namespace	src = boost::log::sources;
namespace	attrs = boost::log::attributes;
namespace	sinks = boost::log::sinks;
namespace	expr = boost::log::expressions;
namespace	keywords = boost::log::keywords;
namespace	po = boost::program_options;

#ifdef _WIN32
#include <windows.h>
#include <atlbase.h>
#include <atlconv.h>
#else
#include <dlfcn.h>
#include <limits.h>
#endif


#ifdef _WIN32
#define HOSTFXR_LIB_NAME _T("hostfxr.dll")
#define CORECLR_LIB_NAME _T("coreclr.dll")
#define HOSTLIBRARY_LIB_NAME _T("NativeHost.dll")
#else
#define HOSTFXR_LIB_NAME "libhostfxr.so"
#define CORECLR_LIB_NAME "libcoreclr.so"
#define HOSTLIBRARY_LIB_NAME "NativeHost.so"
#endif

#endif //PCH_H
