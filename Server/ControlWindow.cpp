#include "ControlWindow.h"

static const TCHAR *className = TEXT("HiddenDesktop_ControlWindow");
static const TCHAR *titlePattern = TEXT("Desktop@%S | HVNC - Tinynuke Clone [Melted@HF]");

BOOL CW_Register(WNDPROC lpfnWndProc)
{
   WNDCLASSEX wndClass;
   wndClass.cbSize        = sizeof(WNDCLASSEX);
   wndClass.style         = CS_DBLCLKS;
   wndClass.lpfnWndProc   = lpfnWndProc;
   wndClass.cbClsExtra    = 0;
   wndClass.cbWndExtra    = 0;
   wndClass.hInstance     = NULL;
   wndClass.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
   wndClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
   wndClass.hbrBackground = (HBRUSH) COLOR_WINDOW;
   wndClass.lpszMenuName  = NULL;
   wndClass.lpszClassName = className;
   wndClass.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);
   return RegisterClassEx(&wndClass);
}

HWND CW_Create(DWORD uhid, DWORD width, DWORD height)
{
   TCHAR title[100];
   IN_ADDR addr;
   addr.S_un.S_addr = uhid;

   wsprintf(title, titlePattern, inet_ntoa(addr));

   HWND hWnd = CreateWindow(className, 
      title, 
      WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SIZEBOX | WS_SYSMENU, 
      CW_USEDEFAULT, 
      CW_USEDEFAULT, 
      width, 
      height, 
      NULL,
      NULL, 
      GetModuleHandle(NULL), 
      NULL);

   if(hWnd == NULL)
      return NULL;

   ShowWindow(hWnd, SW_SHOW); 
   return hWnd;
}