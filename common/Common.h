#pragma once
#define SECURITY_WIN32
#pragma warning(disable: 4267)
#pragma warning(disable: 4244)
#pragma warning(disable: 4533)
#include <winsock.h>
#include <windows.h>
#include <stdio.h>
#include <ntdef.h>
#include <security.h>
#include <sddl.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <wininet.h>
#include <urlmon.h>
#include "Api.h"
#include "Utils.h"
#include "Inject.h"
#include "HTTP.h"
#include "Panel.h"

#define HOST (char*)"127.0.0.1"
#define PATH Strs::path
#define PORT 80
#define POLL 60000
