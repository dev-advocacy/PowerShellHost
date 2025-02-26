#pragma once

#include "ExecutionPolicy.h"
#include "basic_type.h"

#define UNMANAGEDCALLERSONLY_METHOD_A ((const char*)-1)

enum class LoadOption {
    Load,
	NoLoad
};

enum class PowerShellLibType {
    File,
    Script,
    LLM
};

struct hostfxr_context
{
    hostfxr_initialize_for_dotnet_command_line_fn initialize_for_dotnet_command_line;
    hostfxr_initialize_for_runtime_config_fn initialize_for_runtime_config;
    hostfxr_get_runtime_property_value_fn get_runtime_property_value;
    hostfxr_set_runtime_property_value_fn set_runtime_property_value;
    hostfxr_get_runtime_properties_fn get_runtime_properties;
    hostfxr_run_app_fn run_app;
    hostfxr_get_runtime_delegate_fn get_runtime_delegate;
    hostfxr_close_fn close;

    load_assembly_and_get_function_pointer_fn load_assembly_and_get_function_pointer;
    get_function_pointer_fn get_function_pointer;
    hostfxr_handle context_handle;
};
typedef struct hostfxr_context HOSTFXR_CONTEXT;

//static HOSTFXR_CONTEXT g_HOSTFXR_CONTEXT;

typedef void* hPowerShell;
typedef void* hOutput;

typedef hPowerShell(CORECLR_DELEGATE_CALLTYPE* fnPowerShell_Create)(int policy);
typedef hOutput(CORECLR_DELEGATE_CALLTYPE* fnPowerShell_Execute_File)(hPowerShell handle, wchar_t* script);
typedef void (CORECLR_DELEGATE_CALLTYPE* fnPowerShell_RunCommand)(hPowerShell handle, wchar_t* script);
typedef hOutput(CORECLR_DELEGATE_CALLTYPE* fnPowerShell_Execute_Script)(hPowerShell handle, wchar_t* script);
typedef hOutput(CORECLR_DELEGATE_CALLTYPE* fnPowerShell_RunCommandLLM)(wchar_t* script, wchar_t* connection_string);

typedef struct
{
    fnPowerShell_Create Create;
    fnPowerShell_Execute_File Execute_File;
    fnPowerShell_Execute_Script Execute_Script;
	fnPowerShell_RunCommandLLM RunCommandLLM;
} iPowerShell;

struct hostfxr_init_params
{
    size_t size;
    const wchar_t* host_path;
    const wchar_t* dotnet_root;
};
typedef struct hostfxr_init_params HOSTFXR_INIT_PARAMS;

typedef int32_t(CORECLR_DELEGATE_CALLTYPE* fnLoadAssemblyFromNativeMemory)(uint8_t* bytes, int32_t size);

class HostConsolePowerShell {
public:
    HostConsolePowerShell();
    HRESULT run_pwsh_app_file(const string_t& file);
    HRESULT run_pwsh_app_script(const string_t& file);
    string_t run_pwsh_lib_file(const string_t& file, ExecutionPolicy policy);
    string_t run_pwsh_lib_script(const string_t& powershellcommand);
    string_t llmconvert(string_t& script_file, const string_t& connectionstring);
    HRESULT get_powershell_env();
private:
    string_t _pwsh_base_path;
    string_t _pwsh_host_dll;
    HOSTFXR_CONTEXT    *_hostfxr_context;
    fnLoadAssemblyFromNativeMemory      _LoadAssemblyFromNativeMemory = nullptr;
private:
    string_t run_pwsh_lib_internal(PowerShellLibType type, const string_t& powershellfilecommand, const string_t& connectionstring, ExecutionPolicy policy = ExecutionPolicy::Undefined);
    HRESULT load_library(const wchar_t* path, HMODULE& hModule);
    HRESULT load_hostfxr(const wchar_t* hostfxr_path);
    HRESULT load_runtime(const string_t config_path);
    HRESULT get_proc_address(void* handle, const char* name, void*& symbol);
    uint8_t* load_file(string_t filename, size_t* size);
    HRESULT load_assembly_helper(const string_t helper_path, const string_t type_name);
    HRESULT load_pwsh_sdk(const string_t assembly_path, iPowerShell* iface, LoadOption optionload);
    string_t call_pwsh_sdk(PowerShellLibType type, string_t assembly_path, string_t powershellscriptcommand, string_t connectionstring, ExecutionPolicy policy, LoadOption optionload);
    HRESULT load_command(int argc, const  wchar_t** argv, bool close_handle);
    int32_t hostfxr_initialize_for_runtime_config(string_t runtime_config_path, const HOSTFXR_INIT_PARAMS* params, hostfxr_handle* host_context_handle);
    int32_t hostfxr_load_assembly_and_get_function_pointer(const string_t assembly_path, const string_t type_name, const  string_t method_name, const wchar_t* delegate_type_name,void* reserved, void** delegate);
    int32_t hostfxr_get_function_pointer(const string_t type_name,const string_t method_name, const wchar_t* delegate_type_name,void* load_context, void* reserved, void** delegate); 
    int32_t hostfxr_initialize_for_dotnet_command_line(int argc, const wchar_t** argv,const HOSTFXR_INIT_PARAMS* params, hostfxr_handle* host_context_handle);
    HRESULT load_function_impl(void* lib_handle, auto& func, const char* name);
};