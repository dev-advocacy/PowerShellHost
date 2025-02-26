// HostConsole.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


#include "pch.h"
#include "logger.h"

#include "../PowerShellHost.module/ExecutionPolicy.h"
#include "../PowerShellHost.module/PowerShellHostmodule.h"


int main(int argc, char** argv)
{
	USES_CONVERSION;

	try {
		ExecutionPolicy policy = ExecutionPolicy::Restricted;

		po::options_description desc("Allowed options");
		desc.add_options()
			("help", "produce help message")
			("file", po::value<std::string>(), "set PowerShell file you run")
			("script", "execute as script")
			("executionPolicy", po::value<int>(), "set execution policy (0: Unrestricted, 1: RemoteSigned, 2: AllSigned, 3: Restricted, 4: Bypass, 5: Undefined)");


		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);

		if (vm.count("help")) {
			std::cout << desc << "\n";
			return 0;
		}

		int executionPolicy = 3; // Default to Restricted
		if (vm.count("executionPolicy") && executionPolicy > 0 && executionPolicy < 6)
		{
			executionPolicy = vm["executionPolicy"].as<int>();
			policy = static_cast<ExecutionPolicy>(executionPolicy);
		}

		PowerShellHostmodule console;
		if (vm.count("file"))
		{
			std::string file = vm["file"].as<std::string>();
			std::wstring wfile = A2W(file.c_str());
			if (vm.count("script")) {
				console.run_pwsh_lib(wfile, policy, PowerShellLibType::Script);
			}
			else
			{
				console.run_pwsh_lib(wfile, policy, PowerShellLibType::File);
			}

		}
		else {
			std::cout << "Please specify the --file option." << std::endl;
			return -1;
		}
	}
	catch (std::exception& e) {
		std::cerr << "error: " << e.what() << "\n";
		return 1;
	}
	catch (...) {
		std::cerr << "Exception of unknown type!\n";
	}
}

