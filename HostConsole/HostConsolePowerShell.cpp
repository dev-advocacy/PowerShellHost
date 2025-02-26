#include "pch.h"
#include "HostConsolePowerShell.h"
#include "utility.h"
#include "logger.h"



#ifdef _WIN32
#define PATH_SEPARATOR_CHR  '\\'
#define PATH_SEPARATOR_STR  "\\"
#define HOSTFXR_LIB_NAME "hostfxr.dll"
#define CORECLR_LIB_NAME "coreclr.dll"
#else
#define PATH_SEPARATOR_CHR  '/'
#define PATH_SEPARATOR_STR  "//"
#define HOSTFXR_LIB_NAME "libhostfxr.so"
#define CORECLR_LIB_NAME "libcoreclr.so"
#endif


bool HostConsolePowerShell::get_powershell_env()
{
    _pwsh_base_path = utility::get_powershell_path();
    _pwsh_host_dll = utility::get_hostlibrary_path();

	if (_pwsh_base_path.empty() || _pwsh_host_dll.empty())
	{
        LOG_ERROR << "unable to resolved powershell path or " << HOSTLIBRARY_LIB_NAME  " path" << std::endl;
		return false;
	}
    return true;
}

int32_t HostConsolePowerShell::hostfxr_initialize_for_runtime_config(const char* runtime_config_path,
    const HOSTFXR_INIT_PARAMS* params, hostfxr_handle* host_context_handle)
{
    HOSTFXR_CONTEXT* hostfxr = &g_HOSTFXR_CONTEXT;
#ifdef _WIN32
    int32_t status;
    WCHAR* runtime_config_path_w = NULL;
    struct hostfxr_initialize_parameters params_w;
    struct hostfxr_initialize_parameters* p_params = NULL;

    runtime_config_path_w = convert_string_to_utf16(runtime_config_path);

    if (params) {
        params_w.size = sizeof(params_w);
        params_w.host_path = convert_string_to_utf16(params->host_path);
        params_w.dotnet_root = convert_string_to_utf16(params->dotnet_root);
        p_params = &params_w;
    }

    status = hostfxr->initialize_for_runtime_config(runtime_config_path_w, p_params, host_context_handle);

    if (params) {
        free((void*)params_w.host_path);
        free((void*)params_w.dotnet_root);
    }

    free(runtime_config_path_w);

    return status;
#else
    return hostfxr->initialize_for_runtime_config(runtime_config_path,
        (const struct hostfxr_initialize_parameters*)params, host_context_handle);
#endif
}




void* HostConsolePowerShell::load_library(const char* path) {
#ifdef _WIN32
    HMODULE hModule = LoadLibraryA(path);
    return (void*)hModule;
#else
    void* handle = dlopen(path, RTLD_LAZY | RTLD_LOCAL);
    return handle;
#endif
}

void* HostConsolePowerShell::get_proc_address(void* handle, const char* name) {
#ifdef _WIN32
    HMODULE hModule = (HMODULE)handle;
    void* symbol = GetProcAddress(hModule, name);
    return symbol;
#else
    void* symbol = dlsym(handle, name);
    return symbol;
#endif
}

uint8_t* HostConsolePowerShell::load_file(const char* filename, size_t* size) {
    if (!filename || !size)
        return nullptr;

    *size = 0;

    FILE* fp = nullptr;
    if (fopen_s(&fp, filename, "rb") != 0 || !fp)
        return nullptr;

    std::unique_ptr<FILE, decltype(&fclose)> filePtr(fp, fclose);

    fseek(filePtr.get(), 0, SEEK_END);
    *size = ftell(filePtr.get());
    fseek(filePtr.get(), 0, SEEK_SET);

    uint8_t* data = (uint8_t*)malloc(*size + 1);
    if (!data)
        return nullptr;

    if (fread(data, 1, *size, filePtr.get()) != *size) {
        free(data);
        *size = 0;
        return nullptr;
    }

    data[*size] = '\0';
    return data;
}

#ifdef _WIN32
WCHAR* HostConsolePowerShell::convert_string_to_utf16(const char* lpMultiByteStr) {
    if (!lpMultiByteStr)
        return nullptr;

    int cchWideChar = MultiByteToWideChar(CP_UTF8, 0, lpMultiByteStr, -1, NULL, 0);
    WCHAR* lpWideCharStr = (LPWSTR)calloc(cchWideChar + 1, sizeof(WCHAR));
    MultiByteToWideChar(CP_UTF8, 0, lpMultiByteStr, -1, lpWideCharStr, cchWideChar);

    return lpWideCharStr;
}
#endif

bool HostConsolePowerShell::load_function_impl(void* lib_handle, auto& func, const char* name)
{
    void* p = nullptr;
    p = get_proc_address(lib_handle, name);
    if (p == nullptr)
    {
        LOG_TRACE << "could not load function " << name;
        return false;
    }
    func = reinterpret_cast<decltype(func)>(p);
    return true;
}

bool HostConsolePowerShell::load_hostfxr(HOSTFXR_CONTEXT* hostfxr, const char* hostfxr_path)
{
    void* lib_handle = load_library(hostfxr_path);
    memset(hostfxr, 0, sizeof(HOSTFXR_CONTEXT));

    if (!lib_handle) {
        LOG_TRACE << "could not load " << hostfxr_path;
        return false;
    }

    if (!load_function_impl(lib_handle, hostfxr->initialize_for_dotnet_command_line, "hostfxr_initialize_for_dotnet_command_line") ||
        !load_function_impl(lib_handle, hostfxr->initialize_for_runtime_config, "hostfxr_initialize_for_runtime_config") ||
        !load_function_impl(lib_handle, hostfxr->get_runtime_property_value, "hostfxr_get_runtime_property_value") ||
        !load_function_impl(lib_handle, hostfxr->set_runtime_property_value, "hostfxr_set_runtime_property_value") ||
        !load_function_impl(lib_handle, hostfxr->get_runtime_properties, "hostfxr_get_runtime_properties") ||
        !load_function_impl(lib_handle, hostfxr->run_app, "hostfxr_run_app") ||
        !load_function_impl(lib_handle, hostfxr->get_runtime_delegate, "hostfxr_get_runtime_delegate") ||
        !load_function_impl(lib_handle, hostfxr->close, "hostfxr_close")) {
        return false;
    }
    return true;
}

bool HostConsolePowerShell::load_runtime(HOSTFXR_CONTEXT* hostfxr, const char* config_path) {
    hostfxr_handle ctx = nullptr;
    load_assembly_and_get_function_pointer_fn load_assembly_and_get_function_pointer = nullptr;

    int rc = hostfxr_initialize_for_runtime_config(config_path, nullptr, &ctx);

    if ((rc != 0) || (ctx == nullptr)) {
        std::cout << "initialize_for_runtime_config(" << config_path << ") failure" << std::endl;
        return false;
    }

    rc = hostfxr->get_runtime_delegate(ctx,
        hdt_load_assembly_and_get_function_pointer,
        (void**)&load_assembly_and_get_function_pointer);

    if ((rc != 0) || (load_assembly_and_get_function_pointer == nullptr)) {
        std::cout << "get_runtime_delegate failure" << std::endl;
        return false;
    }

    hostfxr->close(ctx);

    hostfxr->load_assembly_and_get_function_pointer = load_assembly_and_get_function_pointer;

    return true;
}

int32_t HostConsolePowerShell::hostfxr_load_assembly_and_get_function_pointer(const char* assembly_path,
    const char* type_name, const char* method_name, const char* delegate_type_name,
    void* reserved, void** delegate)
{
    HOSTFXR_CONTEXT* hostfxr = &g_HOSTFXR_CONTEXT;
#ifdef _WIN32
    int32_t status;
    const WCHAR* assembly_path_w;
    const WCHAR* type_name_w;
    const WCHAR* method_name_w;
    const WCHAR* delegate_type_name_w;

    assembly_path_w = convert_string_to_utf16(assembly_path);
    type_name_w = convert_string_to_utf16(type_name);
    method_name_w = convert_string_to_utf16(method_name);

    if (delegate_type_name != UNMANAGEDCALLERSONLY_METHOD_A) {
        delegate_type_name_w = convert_string_to_utf16(delegate_type_name);
    }
    else {
        delegate_type_name_w = UNMANAGEDCALLERSONLY_METHOD;
    }

    status = hostfxr->load_assembly_and_get_function_pointer(assembly_path_w,
        type_name_w, method_name_w, delegate_type_name_w,
        reserved, delegate);

    free((void*)assembly_path_w);
    free((void*)type_name_w);
    free((void*)method_name_w);

    if (delegate_type_name != UNMANAGEDCALLERSONLY_METHOD_A)
        free((void*)delegate_type_name_w);

    return status;
#else
    return hostfxr->load_assembly_and_get_function_pointer(assembly_path,
        type_name, method_name, delegate_type_name,
        reserved, delegate);
#endif
}


typedef int32_t(CORECLR_DELEGATE_CALLTYPE* fnLoadAssemblyFromNativeMemory)(uint8_t* bytes, int32_t size);
static fnLoadAssemblyFromNativeMemory g_LoadAssemblyFromNativeMemory = NULL;

bool HostConsolePowerShell::load_assembly_helper(HOSTFXR_CONTEXT* hostfxr, const char* helper_path, const char* type_name) {
    int rc;

    rc = hostfxr_load_assembly_and_get_function_pointer(helper_path,
        type_name, "LoadAssemblyFromNativeMemory",
        UNMANAGEDCALLERSONLY_METHOD_A, nullptr, (void**)&g_LoadAssemblyFromNativeMemory);

    if (rc != 0) {
        std::cout << "load_assembly_and_get_function_pointer(LoadAssemblyFromNativeMemory): 0x" << std::hex << rc << std::endl;
        return false;
    }

    return true;
}

int32_t HostConsolePowerShell::hostfxr_get_function_pointer(const char* type_name,
    const char* method_name, const char* delegate_type_name,
    void* load_context, void* reserved, void** delegate)
{
    HOSTFXR_CONTEXT* hostfxr = &g_HOSTFXR_CONTEXT;
#ifdef _WIN32
    int32_t status;
    const WCHAR* type_name_w;
    const WCHAR* method_name_w;
    const WCHAR* delegate_type_name_w;

    type_name_w = convert_string_to_utf16(type_name);
    method_name_w = convert_string_to_utf16(method_name);

    if (delegate_type_name != UNMANAGEDCALLERSONLY_METHOD_A) {
        delegate_type_name_w = convert_string_to_utf16(delegate_type_name);
    }
    else {
        delegate_type_name_w = UNMANAGEDCALLERSONLY_METHOD;
    }

    status = hostfxr->get_function_pointer(type_name_w,
        method_name_w, delegate_type_name_w,
        load_context, reserved, delegate);

    free((void*)type_name_w);
    free((void*)method_name_w);

    if (delegate_type_name != UNMANAGEDCALLERSONLY_METHOD_A)
        free((void*)delegate_type_name_w);

    return status;
#else
    return hostfxr->get_function_pointer(type_name,
        method_name, delegate_type_name,
        load_context, reserved, delegate);
#endif
}


bool HostConsolePowerShell::load_pwsh_sdk(HOSTFXR_CONTEXT* hostfxr, const char* assembly_path, iPowerShell* iface) {
    int rc;
    size_t assembly_size = 0;
    std::unique_ptr<uint8_t[]> assembly_data(load_file(assembly_path, &assembly_size));

    LOG_TRACE << __FUNCTION__;


    if (!assembly_data) {
        LOG_FATAL << "couldn't load " << assembly_path << std::endl;
        return false;
    }

    LOG_TRACE << "loaded " << assembly_path << " (" << assembly_size << " bytes)" << std::endl;

    g_LoadAssemblyFromNativeMemory(assembly_data.get(), static_cast<int32_t>(assembly_size));

    rc = hostfxr_get_function_pointer(
        "NativeHost.Bindings, NativeHost", "PowerShell_Create",
        UNMANAGEDCALLERSONLY_METHOD_A, nullptr, nullptr, (void**)&iface->Create);

    if (rc != 0) {
        LOG_FATAL << "get_function_pointer failure: 0x" << std::hex << rc << std::endl;
        return false;
    }

    rc = hostfxr_get_function_pointer(
        "NativeHost.Bindings, NativeHost", "PowerShell_AddScript",
        UNMANAGEDCALLERSONLY_METHOD_A, nullptr, nullptr, (void**)&iface->AddScript);

    if (rc != 0) {
        LOG_FATAL << "get_function_pointer failure: 0x" << std::hex << rc << std::endl;
        return false;
    }

    rc = hostfxr_get_function_pointer(
        "NativeHost.Bindings, NativeHost", "PowerShell_Invoke",
        UNMANAGEDCALLERSONLY_METHOD_A, nullptr, nullptr, (void**)&iface->Invoke);

    if (rc != 0) {
        LOG_FATAL << "get_function_pointer failure: 0x" << std::hex << rc << std::endl;
        return false;
    }

    rc = hostfxr_get_function_pointer(
        "NativeHost.Bindings, NativeHost", "PowerShell_test",
        UNMANAGEDCALLERSONLY_METHOD_A, nullptr, nullptr, (void**)&iface->PowerShell_test);

    if (rc != 0) {
        LOG_FATAL << "get_function_pointer failure: 0x" << std::hex << rc << std::endl;
        return false;
    }

    

    return true;
}

bool HostConsolePowerShell::call_pwsh_sdk(HOSTFXR_CONTEXT* hostfxr, const char* assembly_path, std::string powershellscript) {
    iPowerShell iface;

    USES_CONVERSION;

    if (!std::filesystem::exists(powershellscript))
    {
        LOG_FATAL << "File does not exist: " << powershellscript;
        return false;
    }

    load_pwsh_sdk(hostfxr, assembly_path, &iface);

    hPowerShell handle = iface.Create();
    std::ifstream file(powershellscript);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    iface.AddScript(handle, content.c_str());

    auto hCommand = iface.Invoke(handle);
    if (hCommand != nullptr)
	{
        const wchar_t* wstr = reinterpret_cast<const wchar_t*>(hCommand);
        auto powershelloutput = CW2A(wstr);
		std::string output(powershelloutput);
		LOG_DEBUG << "PowerShell invoke done output" << output.data();

	}
    return true;
}

int32_t HostConsolePowerShell::hostfxr_initialize_for_dotnet_command_line(int argc, const char** argv,
    const HOSTFXR_INIT_PARAMS* params, hostfxr_handle* host_context_handle)
{
    HOSTFXR_CONTEXT* hostfxr = &g_HOSTFXR_CONTEXT;
#ifdef _WIN32
    int32_t status;
    int32_t index;
    WCHAR** argv_w = NULL;
    struct hostfxr_initialize_parameters params_w;
    struct hostfxr_initialize_parameters* p_params = NULL;

    if (params) {
        params_w.size = sizeof(params_w);
        params_w.host_path = convert_string_to_utf16(params->host_path);
        params_w.dotnet_root = convert_string_to_utf16(params->dotnet_root);
        p_params = &params_w;
    }

    argv_w = (WCHAR**)calloc(argc, sizeof(WCHAR*));

    if (!argv_w)
        return -1;

    for (index = 0; index < argc; index++) {
        argv_w[index] = convert_string_to_utf16(argv[index]);
    }

    status = hostfxr->initialize_for_dotnet_command_line(argc, (const char_t**)argv_w, p_params, host_context_handle);

    for (index = 0; index < argc; index++) {
        free(argv_w[index]);
    }
    free(argv_w);

    if (params) {
        free((void*)params_w.host_path);
        free((void*)params_w.dotnet_root);
    }

    return status;
#else
    return hostfxr->initialize_for_dotnet_command_line(argc, argv,
        (const struct hostfxr_initialize_parameters*)params, host_context_handle);
#endif
}


bool HostConsolePowerShell::load_command(HOSTFXR_CONTEXT* hostfxr, int argc, const char** argv, bool close_handle) {
    hostfxr_handle ctx = nullptr;
    load_assembly_and_get_function_pointer_fn load_assembly_and_get_function_pointer = nullptr;
    get_function_pointer_fn get_function_pointer = nullptr;

    int rc = hostfxr_initialize_for_dotnet_command_line(argc, argv, nullptr, &ctx);

    if ((rc != 0) || (ctx == nullptr)) {
        std::cout << "hostfxr->initialize_for_dotnet_command_line() failure: 0x" << std::hex << rc << std::endl;
        return false;
    }

    rc = hostfxr->get_runtime_delegate(ctx,
        hdt_load_assembly_and_get_function_pointer,
        (void**)&load_assembly_and_get_function_pointer);

    if ((rc != 0) || (load_assembly_and_get_function_pointer == nullptr)) {
        std::cout << "get_runtime_delegate failure: 0x" << std::hex << rc << std::endl;
        return false;
    }

    rc = hostfxr->get_runtime_delegate(ctx,
        hdt_get_function_pointer,
        (void**)&get_function_pointer);

    if ((rc != 0) || (get_function_pointer == nullptr)) {
        std::cout << "get_runtime_delegate failure: 0x" << std::hex << rc << std::endl;
        return false;
    }

    hostfxr->context_handle = ctx;

    if (close_handle) {
        hostfxr->close(ctx);
    }

    hostfxr->load_assembly_and_get_function_pointer = load_assembly_and_get_function_pointer;
    hostfxr->get_function_pointer = get_function_pointer;

    return true;
}

bool HostConsolePowerShell::run_pwsh_app(const std::string& file)
{
    std::filesystem::path base_path = _pwsh_base_path;
    std::filesystem::path hostfxr_path = base_path;
    hostfxr_path.append(HOSTFXR_LIB_NAME);
    std::filesystem::path runtime_config_path = base_path;
    runtime_config_path.append("pwsh.runtimeconfig.json");
    std::filesystem::path assembly_path = base_path;
    assembly_path.append("pwsh.dll");

    LOG_TRACE << __FUNCTION__;


    HOSTFXR_CONTEXT* hostfxr = &g_HOSTFXR_CONTEXT;

	if (!load_hostfxr(hostfxr, hostfxr_path.string().c_str())) {
		std::cout << "failed to load hostfxr!" << std::endl;
		return false;
	}

    const char* command_args[] = {
         assembly_path.string().c_str(),
         "-NoLogo",
         "-NoExit",
         "-Command",
         "Write-Host 'Hello PowerShell Host'"
    };
    int command_argc = sizeof(command_args) / sizeof(char*);

    if (!load_command(hostfxr, command_argc, command_args, false)) {
        std::cout << "failed to load runtime!" << std::endl;
        return false;
    }

    hostfxr->run_app(hostfxr->context_handle);

    return true;
}

bool HostConsolePowerShell::run_pwsh_lib(const std::string& powershellfile)
{
    std::filesystem::path base_path = _pwsh_base_path;
    std::filesystem::path hostfxr_path = base_path;
    hostfxr_path.append(HOSTFXR_LIB_NAME);
    std::filesystem::path coreclr_path = base_path;
    coreclr_path.append(CORECLR_LIB_NAME);
    std::filesystem::path runtime_config_path = base_path;
    runtime_config_path.append("pwsh.runtimeconfig.json");
    std::filesystem::path assembly_path = base_path;
    assembly_path.append("pwsh.dll");
    HOSTFXR_CONTEXT* hostfxr = &g_HOSTFXR_CONTEXT;

    LOG_TRACE << __FUNCTION__;


    if (!load_hostfxr(hostfxr, hostfxr_path.string().c_str()))
    {
        LOG_FATAL << "failed to load hostfxr!";
        return false;
    }
    LOG_TRACE << "loading " << runtime_config_path;
    std::string assembly_path_str = assembly_path.string();
    const char* command_args[] = {
        assembly_path_str.c_str()
    };
    int command_argc = sizeof(command_args) / sizeof(char*);


    if (!load_command(hostfxr, command_argc, command_args, false)) {
        LOG_FATAL << "failed to load runtime!" << std::endl;
        return false;
    }

    std::filesystem::path helper_assembly_path = base_path;
    helper_assembly_path.append("System.Management.Automation.dll");
    if (!load_assembly_helper(hostfxr, helper_assembly_path.string().c_str(),
        "System.Management.Automation.PowerShellUnsafeAssemblyLoad, System.Management.Automation")) {
        LOG_FATAL << "failed to load PowerShellUnsafeAssemblyLoad helper function!" << std::endl;
        return false;
    }

    LOG_TRACE << "loading " << helper_assembly_path;

    std::filesystem::path host_assembly_path = _pwsh_host_dll;
    call_pwsh_sdk(hostfxr, host_assembly_path.string().c_str(), powershellfile);

    return true;
}