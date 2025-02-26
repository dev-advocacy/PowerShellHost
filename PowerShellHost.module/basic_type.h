#pragma once

#ifdef _WIN32
#define _UTF16_STRINGS
#endif

#ifndef _WIN32
typedef uint32_t HRESULT; 
#endif


#ifdef _UTF16_STRINGS
//
// On Windows, all strings are wide
//
typedef wchar_t char_t;
typedef std::wstring string_t;
typedef std::wstring_view string_view_t;
#define _XPLATSTR(x) L##x
typedef std::wostringstream ostringstream_t;
typedef std::wofstream ofstream_t;
typedef std::wostream ostream_t;
typedef std::wistream istream_t;
typedef std::wifstream ifstream_t;
typedef std::wistringstream istringstream_t;
typedef std::wstringstream stringstream_t;
#define ucout std::wcout
#define ucin std::wcin
#define ucerr std::wcerr
#else
//
// On Linux platforms, all strings are narrow
//
typedef char char_t;
typedef std::string string_t;
typedef std::string_view string_view_t;
#define _XPLATSTR(x) x
typedef std::ostringstream ostringstream_t;
typedef std::ofstream ofstream_t;
typedef std::ostream ostream_t;
typedef std::istream istream_t;
typedef std::ifstream ifstream_t;
typedef std::istringstream istringstream_t;
typedef std::stringstream stringstream_t;
#define ucout std::cout
#define ucin std::cin
#define ucerr std::cerr
#endif // endif _UTF16_STRINGS