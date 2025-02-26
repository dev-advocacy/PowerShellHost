//
// pch.h
//

#pragma once

#include "gtest/gtest.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>

#include <cstdlib>
#include <cstdio>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <string_view>
#include <filesystem>
#include <map>
#include <functional>
#include <regex>
#include <chrono>
#include <future>
#include <mutex>
#include <unordered_set>

#include <wchar.h>

// clr headers
#include <nethost.h>
#include <coreclr_delegates.h>
#include <hostfxr.h>

#ifdef _WIN32
	#include <Windows.h>
#endif

namespace	fs = std::filesystem;