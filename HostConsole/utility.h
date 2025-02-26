#pragma once
class utility
{
public:
	static std::string get_powershell_path();
	static std::string get_hostlibrary_path();
private:
	static std::string get_assembly_from_executable_directory();
	static std::string get_env(std::string name);
	static std::string get_env_registry();
	static bool path_exists(const std::string& path);

};

