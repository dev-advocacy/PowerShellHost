#include "pch.h"
#include "logger.h"
#include "utility.h"

#ifdef _WIN32
#define PATH_SEPARATOR_CHR  _T('\\')

#else
#define PATH_SEPARATOR_CHR  '/'
#endif

constexpr const wchar_t* POWER_SHELL_REGISTRY_PATH = _T("SOFTWARE\\Microsoft\\PowerShellCore\\InstalledVersions\\31ab5147-9a97-4452-8443-d9709f0516e1");

/// <summary>
/// get the environment variable
/// </summary>
/// <param name="name"></param>
/// <returns></returns>
string_t utility::get_env(string_t name)
{
    LOG_TRACE << __FUNCTION__;
    wchar_t* str = nullptr;
    size_t len = 0;
    if (_wdupenv_s(&str, &len, name.data()) != 0 || len == 0 || str == nullptr)
    {
        return string_t();
    }
    string_t result(str);
    return result;
}

/// <summary>
/// get powershell path from the registry
/// </summary>
/// <returns></returns>
string_t utility::get_env_registry()
{
    LOG_TRACE << __FUNCTION__;
    USES_CONVERSION;

    CRegKey reg;
    LONG lRes = reg.Open(HKEY_LOCAL_MACHINE, POWER_SHELL_REGISTRY_PATH, KEY_READ);
    if (lRes != ERROR_SUCCESS)
    {
        return string_t();
    }
    ULONG nChars = 0;
    lRes = reg.QueryStringValue(L"InstallDir", nullptr, &nChars);
    if (lRes != ERROR_SUCCESS && lRes != ERROR_MORE_DATA)
    {
        return string_t();
    }
    std::unique_ptr<wchar_t[]> installpath(new wchar_t[nChars]);

    lRes = reg.QueryStringValue(L"InstallDir", installpath.get(), &nChars);
    if (lRes != ERROR_SUCCESS)
    {
        return string_t();
    }
    string_t powershellpath(installpath.get());

	if (!path_exists(powershellpath))
	{
		return string_t();
	}
	return powershellpath;
}

/// <summary>
/// Get the executable directory
/// </summary>
/// <returns></returns>
string_t utility::get_assembly_from_executable_directory()
{
    LOG_TRACE << __FUNCTION__;

    wchar_t path[MAX_PATH];
	GetModuleFileName(nullptr, path, MAX_PATH);
    string_t result(path);
	return result.substr(0, result.find_last_of(PATH_SEPARATOR_CHR));
}

string_t utility::get_powershell_path()
{
    LOG_TRACE << __FUNCTION__;
    string_t powershell = get_env(_T("PWSH_BASE_PATH"));
    if (powershell.empty())
    {
        LOG_TRACE << "powershell path is not set using env variable PWSH_BASE_PATH try to use the registry";
        fs::path assembly_module = get_env_registry();
        if (path_exists(assembly_module.wstring()))
        {
            LOG_TRACE << "assembly path is valid using registry, path:" << assembly_module.string();
            return assembly_module.wstring();
        }
    }
    else
    {
        LOG_TRACE << "assembly path is valid using env, path:" << powershell.data();
    }
    return string_t();
}
/// <summary>
/// get the path of NativeHost.dll
/// </summary>
/// <returns>the path of an empty string</returns>
string_t utility::get_hostlibrary_path()
{
    LOG_TRACE << __FUNCTION__;

    string_t hostLibrary = get_env(_T("HOSTLIBRARY_LIB_NAME"));
    if (hostLibrary.empty())
    {
        LOG_TRACE << HOSTLIBRARY_LIB_NAME << " path is not set using env variable HOSTLIBRARY_LIB_NAME try to the application directory";
        fs::path assembly_module = get_assembly_from_executable_directory();
        assembly_module.append(HOSTLIBRARY_LIB_NAME);

        if (path_exists(assembly_module.wstring()))
        {
            LOG_TRACE << "assembly path is valid using executable directory, path:" << assembly_module.string();
            return assembly_module.wstring();
        }
    }
    else
    {
        LOG_TRACE << "assembly path is valid using env, path:" << hostLibrary.data();
    }
    return string_t();
}
bool utility::path_exists(const string_t& path)
{
    return std::filesystem::exists(path);
}


