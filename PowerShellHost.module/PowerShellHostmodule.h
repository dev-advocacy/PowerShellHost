// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the POWERSHELLHOSTMODULE_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// POWERSHELLHOSTMODULE_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.



#ifdef POWERSHELLHOSTMODULE_EXPORTS
#define POWERSHELLHOSTMODULE_API __declspec(dllexport)
#else
#define POWERSHELLHOSTMODULE_API __declspec(dllimport)

#endif

#include "HostConsolePowerShell.h"
#include "ExecutionPolicy.h"

// This class is exported from the dll
class PowerShellHostmodule {
public:
	POWERSHELLHOSTMODULE_API PowerShellHostmodule(void);	
	POWERSHELLHOSTMODULE_API string_t run_pwsh_lib(string_t& script_file, ExecutionPolicy policy, PowerShellLibType type);
	POWERSHELLHOSTMODULE_API string_t llmconvert(string_t& script_file, string_t& llmconvert);

private:
	HRESULT InitPowerShellVariables();
private:
	HostConsolePowerShell _powershell_host;
};

