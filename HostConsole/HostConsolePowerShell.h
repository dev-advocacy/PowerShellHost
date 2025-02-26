#pragma once

#define UNMANAGEDCALLERSONLY_METHOD_A ((const char*)-1)


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

static HOSTFXR_CONTEXT g_HOSTFXR_CONTEXT;

typedef void* hPowerShell;

typedef void* hCommand;

typedef hPowerShell(CORECLR_DELEGATE_CALLTYPE* fnPowerShell_Create)(void);

typedef hPowerShell(CORECLR_DELEGATE_CALLTYPE* fnPowerShell_test)(void);

typedef void (CORECLR_DELEGATE_CALLTYPE* fnPowerShell_AddScript)(hPowerShell handle, const char* script);

typedef void (CORECLR_DELEGATE_CALLTYPE* fnPowerShell_RunCommand)(hPowerShell handle, const char* script);

typedef hCommand(CORECLR_DELEGATE_CALLTYPE* fnPowerShell_Invoke)(hPowerShell handle);

typedef struct
{
    fnPowerShell_Create Create;
    fnPowerShell_AddScript AddScript;
    fnPowerShell_Invoke Invoke;
    fnPowerShell_test PowerShell_test;
    //fnPowerShell_RunCommand RunCommand;


} iPowerShell;

struct hostfxr_init_params
{
    size_t size;
    const char* host_path;
    const char* dotnet_root;
};
typedef struct hostfxr_init_params HOSTFXR_INIT_PARAMS;



class HostConsolePowerShell {
public:
    HostConsolePowerShell() {};

    bool run_pwsh_app(const std::string& file);
    bool run_pwsh_lib(const std::string& file);
    

    bool get_powershell_env();
private:
    std::string _pwsh_base_path;
    std::string _pwsh_host_dll;
    HOSTFXR_CONTEXT g_hostfxr_context;

    void* load_library(const char* path);
    void* get_proc_address(void* handle, const char* name);
    uint8_t* load_file(const char* filename, size_t* size);
    WCHAR* convert_string_to_utf16(const char* lpMultiByteStr);
    bool load_hostfxr(HOSTFXR_CONTEXT* hostfxr, const char* hostfxr_path);
    bool load_runtime(HOSTFXR_CONTEXT* hostfxr, const char* config_path);
    bool load_assembly_helper(HOSTFXR_CONTEXT* hostfxr, const char* helper_path, const char* type_name);
    bool load_pwsh_sdk(HOSTFXR_CONTEXT* hostfxr, const char* assembly_path, iPowerShell* iface);
    bool call_pwsh_sdk(HOSTFXR_CONTEXT* hostfxr, const char* assembly_path, std::string powershellscript);
    bool load_command(HOSTFXR_CONTEXT* hostfxr, int argc, const char** argv, bool close_handle);
    int32_t hostfxr_initialize_for_runtime_config(const char* runtime_config_path, const HOSTFXR_INIT_PARAMS* params, hostfxr_handle* host_context_handle);
    int32_t hostfxr_load_assembly_and_get_function_pointer(const char* assembly_path,
        const char* type_name, const char* method_name, const char* delegate_type_name,
        void* reserved, void** delegate);


    int32_t hostfxr_get_function_pointer(const char* type_name,
        const char* method_name, const char* delegate_type_name,
        void* load_context, void* reserved, void** delegate);

    int32_t hostfxr_initialize_for_dotnet_command_line(int argc, const char** argv,
        const HOSTFXR_INIT_PARAMS* params, hostfxr_handle* host_context_handle);

    bool load_function_impl(void* lib_handle, auto& func, const char* name);
};