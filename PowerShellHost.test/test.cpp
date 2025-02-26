#include "pch.h"
#include "../PowerShellHost.module/ExecutionPolicy.h"
#include "../PowerShellHost.module/PowerShellHostmodule.h"

std::wstring GetExecutablePath()
{
    wchar_t buffer[MAX_PATH];
    if (GetModuleFileName(nullptr, buffer, MAX_PATH) > 0)
    {
        auto dir = fs::path(buffer).parent_path();
        return dir.wstring();
    }
    return std::wstring();
}
std::list<std::wstring> GetScripts(std::string folder)
{
	fs::path p = GetExecutablePath();

	// get the scripts folder parent
	p = p.parent_path().parent_path();
	p.append("scripts");
	p.append(folder);

	// get all files in the folder
	std::list<std::wstring> scripts;
	for (const auto& entry : fs::directory_iterator(p))
	{
		scripts.push_back(entry.path().wstring());
	}
	return scripts;
}
TEST(HostConsolePowerShellCOMTestRestricted, LoadHostfxr) {
	PowerShellHostmodule console;

	auto scripts = GetScripts("ComInterop");

	for (auto script : scripts)
	{
		auto out = console.run_pwsh_lib(script, ExecutionPolicy::Restricted, PowerShellLibType::File);
		EXPECT_FALSE(out.empty());
	}
}
TEST(HostConsolePowerShellCOMUnrestricted, LoadHostfxr) {
	PowerShellHostmodule console;

	auto scripts = GetScripts("ComInterop");

	for (auto script : scripts)
	{
		auto out = console.run_pwsh_lib(script, ExecutionPolicy::Unrestricted , PowerShellLibType::File);
		EXPECT_FALSE(out.empty());
	}
}
TEST(HostConsolePowerShellSystemUnrestricted, LoadHostfxr) {
	PowerShellHostmodule console;

	auto scripts = GetScripts("System");

	for (auto script : scripts)
	{
		auto out = console.run_pwsh_lib(script, ExecutionPolicy::Unrestricted, PowerShellLibType::File);
		EXPECT_FALSE(out.empty());
	}
}