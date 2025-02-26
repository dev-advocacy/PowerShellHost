#include "pch.h"
#include "logger.h"
#include "HostConsolePowerShell.h"
#include "PowerShellHostmodule.h"

string_t read_file(const string_t& file_path)
{
	std::wifstream file(file_path);
	if (!file.is_open()) {
		throw std::runtime_error("Could not open file");
	}
	string_t content((std::istreambuf_iterator<wchar_t>(file)), std::istreambuf_iterator<wchar_t>());
	return content;
}
PowerShellHostmodule::PowerShellHostmodule(void){}

HRESULT PowerShellHostmodule::InitPowerShellVariables()
{
	auto hr = _powershell_host.get_powershell_env();
	if (FAILED(hr))
	{
		LOG_FATAL << "Set PWSH_BASE_PATH environment variable to point to PowerShell installation path" << std::endl;
		LOG_FATAL << "Set PWSH_HOST_DLL environment variable to point to NativeHost.dll bindings" << std::endl;
	}
	return (hr);
}

string_t PowerShellHostmodule::run_pwsh_lib(string_t& script_file, ExecutionPolicy policy, PowerShellLibType type)
{
	if (FAILED(InitPowerShellVariables()))
	{
		return string_t();
	}

	if (type == PowerShellLibType::Script)
	{
		return (_powershell_host.run_pwsh_lib_script(script_file));
	}
	else

	if (!script_file.empty()) 
	{		
		return (_powershell_host.run_pwsh_lib_file(script_file, policy ));
	}
	else
	{
		return string_t();
	}
	return string_t();
}
string_t PowerShellHostmodule::llmconvert(string_t& script_file, string_t& connectionstring)
{
	if (FAILED(InitPowerShellVariables()))
	{
		return string_t();
	}
	if (!script_file.empty())
	{
		return (_powershell_host.llmconvert(script_file, connectionstring));
	}
	return string_t();
}