#include "pch.h"
#include "logger.h"
#include "utility.h"


std::string utility::get_env(std::string name)
{
    LOG_TRACE << __FUNCTION__;
    char* str = nullptr;
    size_t len = 0;
    if (_dupenv_s(&str, &len, name.data()) != 0 || len == 0 || str == nullptr)
    {
        return std::string();
    }
    std::string result(str);
    return result;
}


std::string utility::get_env_registry()
{
    LOG_TRACE << __FUNCTION__;
    USES_CONVERSION;

    CRegKey reg;
    LONG lRes = reg.Open(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\PowerShellCore\\InstalledVersions\\31ab5147-9a97-4452-8443-d9709f0516e1", KEY_READ);
    if (lRes != ERROR_SUCCESS)
    {
        return std::string();
    }
    ULONG nChars = 0;
    lRes = reg.QueryStringValue(L"InstallDir", nullptr, &nChars);
    if (lRes != ERROR_SUCCESS && lRes != ERROR_MORE_DATA)
    {
        return std::string();
    }
    std::unique_ptr<wchar_t[]> installpath(new wchar_t[nChars]);

    lRes = reg.QueryStringValue(L"InstallDir", installpath.get(), &nChars);
    if (lRes != ERROR_SUCCESS)
    {
        return std::string();
    }

    std::string powershellpath(CW2A(installpath.get()));

	if (!path_exists(powershellpath))
	{
		return std::string();
	}
	return powershellpath;
}

/// <summary>
/// Get the executable directory
/// </summary>
/// <returns></returns>
std::string utility::get_assembly_from_executable_directory()
{
    LOG_TRACE << __FUNCTION__;

	char path[MAX_PATH];
	GetModuleFileNameA(NULL, path, MAX_PATH);
	std::string result(path);
	return result.substr(0, result.find_last_of(PATH_SEPARATOR_CHR));
}

std::string utility::get_powershell_path()
{
    LOG_TRACE << __FUNCTION__;
    std::string powershell = get_env("PWSH_BASE_PATH");
    if (powershell.empty())
    {
        LOG_TRACE << "powershell path is not set using env variable PWSH_BASE_PATH try to use the registry";
        fs::path assembly_module = get_env_registry();
        if (path_exists(assembly_module.string()))
        {
            LOG_TRACE << "assembly path is valid using registry, path:" << assembly_module.string();
            return assembly_module.string();
        }
    }
    else
    {
        LOG_TRACE << "assembly path is valid using env, path:" << powershell.data();
    }
    return std::string();
}
std::string utility::get_hostlibrary_path()
{
    LOG_TRACE << __FUNCTION__;

    std::string hostLibrary = get_env("HOSTLIBRARY_LIB_NAME");
    if (hostLibrary.empty())
    {
        LOG_TRACE << HOSTLIBRARY_LIB_NAME << " path is not set using env variable HOSTLIBRARY_LIB_NAME try to the application directory";
        fs::path assembly_module = get_assembly_from_executable_directory();
        assembly_module.append(HOSTLIBRARY_LIB_NAME);

        if (path_exists(assembly_module.string()))
        {
            LOG_TRACE << "assembly path is valid using executable directory, path:" << assembly_module.string();
            return assembly_module.string();
        }
    }
    else
    {
        LOG_TRACE << "assembly path is valid using env, path:" << hostLibrary.data();
    }
    return std::string();
}


bool utility::path_exists(const std::string& path)
{
    return std::filesystem::exists(path);
}