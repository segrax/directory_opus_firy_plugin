// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include "strsafe.h"

#include <string>
#include <list>
#include <regex>

#include <algorithm>
#include <iostream>
#include <iterator>
#include <sstream>
#include <vector>
#include <queue>
#include <map>
#include <cctype>
#include <fstream>
#include <direct.h>
#include <memory>

#include <functional>
#include <mutex>

typedef const BYTE *LPCBYTE;

#include "headers/vfs plugins.h"
#include "headers/plugin support.h"

using tLockGuard = std::lock_guard<std::mutex>;
extern DOpusPluginHelperFunction DOpus;
extern DOpusPluginHelperUtil DOpusUtil;

#include "safecontainer.hpp"

#include "firy.hpp"

#include "opusoptions.hpp"
#include "firyopus.hpp"

std::wstring s2ws(const std::string& str);
