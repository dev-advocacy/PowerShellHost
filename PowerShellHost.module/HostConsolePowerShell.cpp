#include "pch.h"
#include "ExecutionPolicy.h"
#include "HostConsolePowerShell.h"
#include "HostfxrContextSingleton.h"
#include "utility.h"
#include "logger.h"

#ifdef _WIN32
#define PATH_SEPARATOR_CHR  _T('\\')
#define PATH_SEPARATOR_STR  _T("\\")
#else
#define PATH_SEPARATOR_CHR  '/'
#define PATH_SEPARATOR_STR  "//"
#define HOSTFXR_LIB_NAME "libhostfxr.so"
#define CORECLR_LIB_NAME "libcoreclr.so"
#endif


HostConsolePowerShell::HostConsolePowerShell()
{
    _hostfxr_context = HostfxrContextSingleton::getInstance().getContext();
};

/// <summary>
/// resolve the powershell path and the hostlibrary path
/// </summary>
/// <returns> ERROR_FILE_NOT_FOUND or S_OK </returns>
HRESULT HostConsolePowerShell::get_powershell_env()
{
    _pwsh_base_path = utility::get_powershell_path();
    _pwsh_host_dll = utility::get_hostlibrary_path();

	if (_pwsh_base_path.empty() || _pwsh_host_dll.empty())
	{
        LOG_ERROR << "unable to resolved powershell path or " << HOSTLIBRARY_LIB_NAME  " path" << std::endl;
		return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
	}
    return S_OK;
}

/// <summary>
/// Initialize the hostfxr context
/// </summary>
/// <param name="runtime_config_path"></param>
/// <param name="params"></param>
/// <param name="host_context_handle"></param>
/// <returns></returns>
int32_t HostConsolePowerShell::hostfxr_initialize_for_runtime_config(string_t runtime_config_path,const HOSTFXR_INIT_PARAMS* params, hostfxr_handle* host_context_handle)
{

#ifdef _WIN32
    int32_t status;
    WCHAR* runtime_config_path_w = nullptr;
    struct hostfxr_initialize_parameters params_w;
    struct hostfxr_initialize_parameters* p_params = nullptr;

    runtime_config_path_w = runtime_config_path.data();

    if (params) {
        params_w.size = sizeof(params_w);
        params_w.host_path = params->host_path;
        params_w.dotnet_root = params->dotnet_root;
        p_params = &params_w;
    }

    status = _hostfxr_context->initialize_for_runtime_config(runtime_config_path_w, p_params, host_context_handle);

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


/// <summary>
/// load the library
/// </summary>
/// <param name="path"></param>
/// <returns></returns>
HRESULT HostConsolePowerShell::load_library(const wchar_t* path, HMODULE& hModule)
{
#ifdef _WIN32
    hModule = LoadLibrary(path);
    if (hModule == nullptr)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    return S_OK;
#else
    void* handle = dlopen(path, RTLD_LAZY | RTLD_LOCAL);
    return handle;
#endif
}

/// <summary>
/// get the proc address of a function
/// </summary>
/// <param name="handle"></param>
/// <param name="name"></param>
/// <returns></returns>
HRESULT HostConsolePowerShell::get_proc_address(void* handle, const char* name, void*& symbol)
{
#ifdef _WIN32
    HMODULE hModule = (HMODULE)handle;
    symbol = GetProcAddress(hModule, name);
    if (symbol == nullptr) 
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    return S_OK;
#else
    symbol = dlsym(handle, name);
	if (symbol == nullptr)
	{
		return dlerror();
	}
    return S_OK;
#endif
}



/// <summary>
/// Load the assembly and get the function pointer
/// </summary>
/// <param name="filename"></param>
/// <param name="size"></param>
/// <returns></returns>
uint8_t* HostConsolePowerShell::load_file(string_t filename, size_t* size)
{

    if (filename.empty() || !size)
        return nullptr;

    *size = 0;

    FILE* fp = nullptr;
    if (_wfopen_s(&fp, filename.data(), _T("rb") ) != 0 || !fp)
        return nullptr;

    std::unique_ptr<FILE, decltype(&fclose)> filePtr(fp, fclose);

    fseek(filePtr.get(), 0, SEEK_END);
    *size = ftell(filePtr.get());
    fseek(filePtr.get(), 0, SEEK_SET);

    std::unique_ptr<uint8_t[]> data(new uint8_t[*size + 1]);
    if (fread(data.get(), 1, *size, filePtr.get()) != *size) {
        *size = 0;
        return nullptr;
    }
    data[*size] = '\0';
    return data.release();
 }
/// <summary>
/// getprocaddress for the function
/// </summary>
/// <param name="lib_handle"></param>
/// <param name="func"></param>
/// <param name="name"></param>
/// <returns></returns>
HRESULT HostConsolePowerShell::load_function_impl(void* lib_handle, auto& func, const char* name)
{
    void* p = nullptr;
    auto hr = get_proc_address(lib_handle, name, p);
	if (FAILED(hr))
    {
        LOG_TRACE << "could not load function " << name;
        return hr;
    }
    LOG_DEBUG << "Loading " << name << std::endl;
    func = reinterpret_cast<decltype(func)>(p);
    return hr;
}

/// <summary>
/// Load the hostfxr library entry points
/// </summary>
/// <param name="hostfxr_path"></param>
/// <returns></returns>
HRESULT HostConsolePowerShell::load_hostfxr(const wchar_t* hostfxr_path)
{
	HMODULE lib_handle = nullptr;
    auto hr = load_library(hostfxr_path, lib_handle);
	if (FAILED(hr))
    {
        LOG_TRACE << "could not load " << hostfxr_path;
        return hr;
    }
    RETURN_IF_FAILED(load_function_impl(lib_handle, _hostfxr_context->initialize_for_dotnet_command_line, "hostfxr_initialize_for_dotnet_command_line"));
    RETURN_IF_FAILED(load_function_impl(lib_handle, _hostfxr_context->initialize_for_runtime_config, "hostfxr_initialize_for_runtime_config"));
    RETURN_IF_FAILED(load_function_impl(lib_handle, _hostfxr_context->get_runtime_property_value, "hostfxr_get_runtime_property_value"));
    RETURN_IF_FAILED(load_function_impl(lib_handle, _hostfxr_context->set_runtime_property_value, "hostfxr_set_runtime_property_value"));
    RETURN_IF_FAILED(load_function_impl(lib_handle, _hostfxr_context->get_runtime_properties, "hostfxr_get_runtime_properties"));
    RETURN_IF_FAILED(load_function_impl(lib_handle, _hostfxr_context->run_app, "hostfxr_run_app"));
    RETURN_IF_FAILED(load_function_impl(lib_handle, _hostfxr_context->get_runtime_delegate, "hostfxr_get_runtime_delegate"));
    RETURN_IF_FAILED(load_function_impl(lib_handle, _hostfxr_context->close, "hostfxr_close"));

	return S_OK;
}

/// <summary>
/// Initialize the runtime, load the assembly and get the function pointer
/// </summary>
/// <param name="config_path"></param>
/// <returns></returns>
HRESULT HostConsolePowerShell::load_runtime(string_t config_path)
{
    hostfxr_handle ctx = nullptr;
    load_assembly_and_get_function_pointer_fn load_assembly_and_get_function_pointer = nullptr;

    int rc = hostfxr_initialize_for_runtime_config(config_path, nullptr, &ctx);

    if ((rc != 0) || (ctx == nullptr)) 
    {
        LOG_TRACE << "initialize_for_runtime_config(" << config_path << ") failure" << std::endl;
        return static_cast<int>(rc);
    }

    rc = _hostfxr_context->get_runtime_delegate(ctx,
        hdt_load_assembly_and_get_function_pointer,
        (void**)&load_assembly_and_get_function_pointer);

    if ((rc != 0) || (load_assembly_and_get_function_pointer == nullptr))
    {
        LOG_TRACE << "get_runtime_delegate failure" << "rc=" << rc << std::endl;
        return static_cast<int>(rc);;
    }
    _hostfxr_context->close(ctx);
    _hostfxr_context->load_assembly_and_get_function_pointer = load_assembly_and_get_function_pointer;
    return static_cast<int>(rc);
}

/// <summary>
/// Load the assembly and get the function pointer
/// </summary>
/// <param name="assembly_path"></param>
/// <param name="type_name"></param>
/// <param name="method_name"></param>
/// <param name="delegate_type_name"></param>
/// <param name="reserved"></param>
/// <param name="delegate"></param>
/// <returns></returns>
int32_t HostConsolePowerShell::hostfxr_load_assembly_and_get_function_pointer(const string_t assembly_path,
    const string_t type_name, const string_t method_name, const wchar_t *delegate_type_name,void* reserved, void** delegate)
{    
#ifdef _WIN32
    int32_t status;
    

    status = _hostfxr_context->load_assembly_and_get_function_pointer(assembly_path.data(),
        type_name.data(), method_name.data(), delegate_type_name,
        reserved, delegate);

    return status;
#else
    return hostfxr->load_assembly_and_get_function_pointer(assembly_path,
        type_name, method_name, delegate_type_name,
        reserved, delegate);
#endif
}

HRESULT HostConsolePowerShell::load_assembly_helper(const string_t helper_path, const string_t type_name)
{
    int rc = 0;

	string_t methodname = _T("LoadAssemblyFromNativeMemory");

    rc = hostfxr_load_assembly_and_get_function_pointer(helper_path,
        type_name, methodname,
        UNMANAGEDCALLERSONLY_METHOD, nullptr, (void**)&_LoadAssemblyFromNativeMemory);

    if (rc != 0) {
        LOG_TRACE << "load_assembly_and_get_function_pointer(LoadAssemblyFromNativeMemory): 0x" << std::hex << rc << std::endl;
        return E_FAIL;
    }
    return S_OK;
}

/// <summary>
/// Generic implementation of get function pointer
/// </summary>
/// <param name="type_name"></param>
/// <param name="method_name"></param>
/// <param name="delegate_type_name"></param>
/// <param name="load_context"></param>
/// <param name="reserved"></param>
/// <param name="delegate"></param>
/// <returns></returns>
int32_t HostConsolePowerShell::hostfxr_get_function_pointer(const string_t type_name,
    const string_t method_name, const wchar_t* delegate_type_name,
    void* load_context, void* reserved, void** delegate)
{

#ifdef _WIN32
    int32_t status;

    status = _hostfxr_context->get_function_pointer(type_name.data(),
        method_name.data(), UNMANAGEDCALLERSONLY_METHOD,
        load_context, reserved, delegate);

    return status;
#else
    return hostfxr->get_function_pointer(type_name,
        method_name, delegate_type_name,
        load_context, reserved, delegate);
#endif
}


/// <summary>
/// Load the powershell sdk using the hostfxr and the manage assembly
/// </summary>
/// <param name="assembly_path"></param>
/// <param name="iface"></param>
/// <returns></returns>
HRESULT HostConsolePowerShell::load_pwsh_sdk(const string_t assembly_path, iPowerShell* iface, LoadOption optionload)
{
    int rc;
    size_t assembly_size = 0;
    

    if (optionload == LoadOption::Load)
    {
        std::unique_ptr<uint8_t[]> assembly_data(load_file(assembly_path, &assembly_size));
        LOG_TRACE << __FUNCTION__;

        if (!assembly_data) {
            LOG_FATAL << "couldn't load " << assembly_path << std::endl;
            return false;
        }

        LOG_TRACE << "loaded " << assembly_path << " (" << assembly_size << " bytes)" << std::endl;

        _LoadAssemblyFromNativeMemory(assembly_data.get(), static_cast<int32_t>(assembly_size));
    }

    rc = hostfxr_get_function_pointer(_T("NativeHost.Bindings, NativeHost"),
                                      _T("PowerShell_Create"),
                                      UNMANAGEDCALLERSONLY_METHOD,
                                      nullptr,
                                      nullptr,
                                      (void**)&iface->Create);

    if (rc != 0)
    {
        LOG_FATAL << "get_function_pointer failure: 0x" << std::hex << rc << std::endl;
        return rc;
    }

    rc = hostfxr_get_function_pointer(_T("NativeHost.Bindings, NativeHost"),
                                      _T("PowerShell_Execute_File"),
                                      UNMANAGEDCALLERSONLY_METHOD,
                                      nullptr,
                                      nullptr,
                                      (void**)&iface->Execute_File);
    if (rc != 0)
    {
        LOG_FATAL << "get_function_pointer failure: 0x" << std::hex << rc << std::endl;
        return rc;
    }

    rc = hostfxr_get_function_pointer(_T("NativeHost.Bindings, NativeHost"),
                                      _T("PowerShell_Execute_Script"),
                                      UNMANAGEDCALLERSONLY_METHOD,
                                      nullptr,
                                      nullptr,
                                      (void**)&iface->Execute_Script);
    if (rc != 0) {
        LOG_FATAL << "get_function_pointer failure: 0x" << std::hex << rc << std::endl;
        return rc;
    }

    rc = hostfxr_get_function_pointer(_T("NativeHost.Bindings, NativeHost"),
        _T("RunCommandLLM"),
        UNMANAGEDCALLERSONLY_METHOD,
        nullptr,
        nullptr,
        (void**)&iface->RunCommandLLM);
    if (rc != 0) {
        LOG_FATAL << "get_function_pointer failure: 0x" << std::hex << rc << std::endl;
        return rc;
    }

    return rc;
}

/// <summary>
/// Execute the powershell script
/// </summary>
/// <param name="type"></param>
/// <param name="assembly_path"></param>
/// <param name="powershellscriptcommand"></param>
/// <param name="policy"></param>
/// <returns></returns>
string_t HostConsolePowerShell::call_pwsh_sdk(PowerShellLibType type,
                                              string_t assembly_path,
                                              string_t powershellscriptcommand,
                                              string_t connectionstring,  
                                              ExecutionPolicy policy,
                                              LoadOption optionload)
{
    iPowerShell iface;

    USES_CONVERSION;

    if (!std::filesystem::exists(powershellscriptcommand) && (type == PowerShellLibType::File || type == PowerShellLibType::LLM))
    {
        LOG_FATAL << "File does not exist: " << powershellscriptcommand;
        return string_t();
    }
    HRESULT hr = load_pwsh_sdk(assembly_path, &iface, optionload);
	if (FAILED(hr))
	{
		LOG_FATAL << "failed to load the powershell sdk, hr=" << hr;
		return string_t();
	}

	int p = static_cast<int>(policy);

    if (type == PowerShellLibType::LLM && !connectionstring.empty())
	{
        hOutput pOutput = iface.RunCommandLLM(powershellscriptcommand.data(), connectionstring.data());
        if (pOutput != nullptr)
        {
            const wchar_t* wstr = reinterpret_cast<const wchar_t*>(pOutput);
            string_t output(wstr);
            return output;
        }
	}


    hPowerShell handle = iface.Create(p);

    if (type == PowerShellLibType::File)
    {
        hOutput pOutput =  iface.Execute_File(handle, powershellscriptcommand.data());
		if (pOutput != nullptr)
		{
			const wchar_t* wstr = reinterpret_cast<const wchar_t*>(pOutput);			
            string_t output(wstr);
            return output;
		}
    }
	else if (type == PowerShellLibType::Script)
	{
        hOutput pOutput = iface.Execute_Script(handle, powershellscriptcommand.data());
		if (pOutput != nullptr)
		{
			const wchar_t* wstr = reinterpret_cast<const wchar_t*>(pOutput);
			string_t output(wstr);
            return output;
		}
	}
    return string_t();
}

/// <summary>
/// Execute the powershell script using the hostfxr
/// </summary>
/// <param name="argc"></param>
/// <param name="argv"></param>
/// <param name="params"></param>
/// <param name="host_context_handle"></param>
/// <returns></returns>
int32_t HostConsolePowerShell::hostfxr_initialize_for_dotnet_command_line(int argc, const wchar_t** argv,
    const HOSTFXR_INIT_PARAMS* params, hostfxr_handle* host_context_handle)
{
    {
#ifdef _WIN32
        int32_t status;
        struct hostfxr_initialize_parameters* p_params = nullptr;
        status = _hostfxr_context->initialize_for_dotnet_command_line(argc, (const char_t**)argv, p_params, host_context_handle);
        return status;
#else
        return hostfxr->initialize_for_dotnet_command_line(argc, argv,
            (const struct hostfxr_initialize_parameters*)params, host_context_handle);
#endif
    }
}

/// <summary>
/// get the function pointer
/// </summary>
/// <param name="argc"></param>
/// <param name="argv"></param>
/// <param name="close_handle"></param>
/// <returns></returns>
HRESULT HostConsolePowerShell::load_command(int argc, const  wchar_t** argv, bool close_handle) {
    hostfxr_handle ctx = nullptr;
    load_assembly_and_get_function_pointer_fn load_assembly_and_get_function_pointer = nullptr;
    get_function_pointer_fn get_function_pointer = nullptr;

    int rc = hostfxr_initialize_for_dotnet_command_line(argc, argv, nullptr, &ctx);

    if ((rc != 0) || (ctx == nullptr)) {
        std::cout << "hostfxr->initialize_for_dotnet_command_line() failure: 0x" << std::hex << rc << std::endl;
        return rc;
    }

    rc = _hostfxr_context->get_runtime_delegate(ctx,
        hdt_load_assembly_and_get_function_pointer,
        (void**)&load_assembly_and_get_function_pointer);

    if ((rc != 0) || (load_assembly_and_get_function_pointer == nullptr)) {
        std::cout << "get_runtime_delegate failure: 0x" << std::hex << rc << std::endl;
        return rc;
    }

    rc = _hostfxr_context->get_runtime_delegate(ctx,
        hdt_get_function_pointer,
        (void**)&get_function_pointer);

    if ((rc != 0) || (get_function_pointer == nullptr)) {
        std::cout << "get_runtime_delegate failure: 0x" << std::hex << rc << std::endl;
        return rc;
    }

    _hostfxr_context->context_handle = ctx;

    if (close_handle) {
        _hostfxr_context->close(ctx);
    }

    _hostfxr_context->load_assembly_and_get_function_pointer = load_assembly_and_get_function_pointer;
    _hostfxr_context->get_function_pointer = get_function_pointer;

    return S_OK;
}

HRESULT HostConsolePowerShell::run_pwsh_app_script(const string_t& file)
{
    std::filesystem::path base_path = _pwsh_base_path;
    std::filesystem::path hostfxr_path = base_path;
    hostfxr_path.append(HOSTFXR_LIB_NAME);
    std::filesystem::path runtime_config_path = base_path;
    runtime_config_path.append(_T("pwsh.runtimeconfig.json"));
    std::filesystem::path assembly_path = base_path;
    assembly_path.append(_T("pwsh.dll"));

    LOG_TRACE << __FUNCTION__;

    if (!load_hostfxr(hostfxr_path.wstring().c_str())) {
        std::cout << "failed to load hostfxr!" << std::endl;
        return false;
    }

    string_t assembly_path_str = assembly_path.wstring();

    // for debugging add the parameter "-NoExit",
    const wchar_t* command_args[] = {
         assembly_path_str.data(),
         _T("-NoLogo"),
         _T("-Command"),
         file.data(),
    };
    int command_argc = sizeof(command_args) / sizeof(char*);

    auto hr = load_command(command_argc, command_args, false);
    if (FAILED(hr))
    {
        LOG_TRACE << "failed to load runtime, hr=" << hr;
        RETURN_IF_FAILED(hr);
    }

    hr  = _hostfxr_context->run_app(_hostfxr_context->context_handle);
    if (FAILED(hr))
    {
        LOG_TRACE << "failed to load the context, hr=" << hr;
        RETURN_IF_FAILED(hr);
    }
    return S_OK;
}

/// <summary>
/// Call the powershell using powershell command line
/// </summary>
/// <param name="file"></param>
/// <returns></returns>
HRESULT HostConsolePowerShell::run_pwsh_app_file(const string_t& file)
{
    std::filesystem::path base_path = _pwsh_base_path;
    std::filesystem::path hostfxr_path = base_path;
    hostfxr_path.append(HOSTFXR_LIB_NAME);
    std::filesystem::path runtime_config_path = base_path;
    runtime_config_path.append("pwsh.runtimeconfig.json");
    std::filesystem::path assembly_path = base_path;
    assembly_path.append("pwsh.dll");

    LOG_TRACE << __FUNCTION__;

    auto hr = load_hostfxr(hostfxr_path.wstring().c_str());
    if (FAILED(hr))
    {
        LOG_TRACE << "failed to load hostfxr, hr=" << hr;
        RETURN_IF_FAILED(hr);
    }
	string_t assembly_path_str = assembly_path.wstring();

    if (!fs::exists(file))
    {
        LOG_TRACE << "File does not exist: " << file << std::endl;
		return E_FAIL;
    }
	// for debugging add the parameter "-NoExit",
    const wchar_t* command_args[] = {
         assembly_path_str.data(),
         _T("-NoLogo"),
         _T("-File"),
         file.data(),
    };
    int command_argc = sizeof(command_args) / sizeof(char*);

    hr = load_command(command_argc, command_args, false);
    if (FAILED(hr))
    {
        LOG_TRACE << "failed to load runtime, hr=" << hr;
        RETURN_IF_FAILED(hr);
    }
    hr = _hostfxr_context->run_app(_hostfxr_context->context_handle);
    if (FAILED(hr))
    {
        LOG_TRACE << "failed to run app, hr=" << hr;
        RETURN_IF_FAILED(hr);
    }
    return S_OK;
}

/// <summary>
/// run the powershell script
/// </summary>
/// <param name="powershellcommand"></param>
/// <returns></returns>
string_t HostConsolePowerShell::run_pwsh_lib_script(const string_t& powershellcommand )
{
    return run_pwsh_lib_internal(PowerShellLibType::Script, powershellcommand, _T(""), ExecutionPolicy::Undefined);
}

/// <summary>
/// run the powershell file using the policy
/// </summary>
/// <param name="powershellfile"></param>
/// <param name="policy"></param>
/// <returns></returns>
string_t HostConsolePowerShell::run_pwsh_lib_file(const string_t& powershellfile, ExecutionPolicy policy)
{
    return run_pwsh_lib_internal(PowerShellLibType::File, powershellfile, _T(""), policy);
}

/// <summary>
/// run the powershell using script or file
/// </summary>
/// <param name="type"></param>
/// <param name="powershellfilecommand"></param>
/// <param name="policy"></param>
/// <returns></returns>
string_t HostConsolePowerShell::run_pwsh_lib_internal(PowerShellLibType type, const string_t& powershellfilecommand, const string_t& connectionstring, ExecutionPolicy policy)
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

    LOG_TRACE << __FUNCTION__;
	
    if (_hostfxr_context->run_app == nullptr)
    {
        auto hr = load_hostfxr(hostfxr_path.wstring().c_str());
		if (FAILED(hr))
        {
            LOG_FATAL << "failed to load hostfxr!";
            return string_t();
        }
        LOG_TRACE << "loading " << runtime_config_path;
        string_t assembly_path_str = assembly_path.wstring();
        const wchar_t* command_args[] = {
            assembly_path_str.c_str()
        };
        int command_argc = sizeof(command_args) / sizeof(char*);


        hr = load_command(command_argc, command_args, false);
		if (FAILED(hr))
        {
            LOG_FATAL << "failed to load runtime!" << std::endl;
            return string_t();
        }

        std::filesystem::path helper_assembly_path = base_path;
        helper_assembly_path.append("System.Management.Automation.dll");
        hr = load_assembly_helper(helper_assembly_path.wstring().c_str(),
            _T("System.Management.Automation.PowerShellUnsafeAssemblyLoad, System.Management.Automation"));		
        if (FAILED(hr))           
        {
            LOG_FATAL << "failed to load PowerShellUnsafeAssemblyLoad helper function!" << std::endl;
            return string_t();
        }

        LOG_TRACE << "loading " << helper_assembly_path;
		
        std::filesystem::path host_assembly_path = _pwsh_host_dll;
        return (call_pwsh_sdk(type, host_assembly_path.wstring().c_str(), powershellfilecommand, connectionstring, policy, LoadOption::Load));
    }
	// if the hostfxr context is already loaded, we can use it to run the powershell script
    else
    {
        std::filesystem::path host_assembly_path = _pwsh_host_dll;
        return (call_pwsh_sdk(type, host_assembly_path.wstring().c_str(),powershellfilecommand, connectionstring, policy, LoadOption::NoLoad));
    }
    return string_t();
}

string_t HostConsolePowerShell::llmconvert(string_t& script_file, const string_t& connectionstring)
{
    return (run_pwsh_lib_internal(PowerShellLibType::LLM, script_file, connectionstring));
}