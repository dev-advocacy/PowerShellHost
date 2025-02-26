#pragma once
class utility
{
public:
	static string_t get_powershell_path();
	static string_t get_hostlibrary_path();
private:
	static string_t get_assembly_from_executable_directory();
	static string_t get_env(string_t name);
	static string_t get_env_registry();
	static bool path_exists(const string_t& path);

};

