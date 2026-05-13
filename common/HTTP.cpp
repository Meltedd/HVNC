#include "HTTP.h"
#include <limits.h>

static BOOL RecvAll(SOCKET s, void *buffer, int size)
{
   char *data = (char *) buffer;
   int received = 0;
   while(received != size)
   {
      int ret = Funcs::pRecv(s, data + received, size - received, 0);
      if(ret <= 0)
         return FALSE;
      received += ret;
   }
   return TRUE;
}

BOOL HttpSubmitRequest(HttpRequestData &httpRequestData)
{
   BOOL ret = FALSE;
   WSADATA wsa;
   BOOL wsaStarted = FALSE;
   SOCKET s = INVALID_SOCKET;
   hostent *he = NULL;
   struct sockaddr_in addr = { 0 };
   char header[1024] = { 0 };
   int contentLength = -1;
   int lastPos = 0;
   BOOL firstLine = TRUE;
   BOOL transferChunked = FALSE;

   char request[1024] = { 0 };

   httpRequestData.outputBody = NULL;
   httpRequestData.outputBodySize = 0;
   Funcs::pLstrcpyA(request, (httpRequestData.post ? Strs::postSpace : Strs::getSpace));
   Funcs::pLstrcatA(request, httpRequestData.path);
   Funcs::pLstrcatA(request, Strs::httpReq1);
   Funcs::pLstrcatA(request, Strs::httpReq2);
   Funcs::pLstrcatA(request, httpRequestData.host);
   Funcs::pLstrcatA(request, Strs::httpReq3);

   if(httpRequestData.post && httpRequestData.inputBody)
   {
      Funcs::pLstrcatA(request, Strs::httpReq4);
      char sizeStr[10];
      Funcs::pWsprintfA(sizeStr, Strs::sprintfIntEscape, httpRequestData.inputBodySize);
      Funcs::pLstrcatA(request, sizeStr);
      Funcs::pLstrcatA(request, Strs::winNewLine);
   }
   Funcs::pLstrcatA(request, Strs::winNewLine);

   if(Funcs::pWSAStartup(MAKEWORD(2, 2), &wsa) != 0)
      goto exit;
   wsaStarted = TRUE;

   if((s = Funcs::pSocket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
      goto exit;

   he = Funcs::pGethostbyname(httpRequestData.host);
   if(!he)
      goto exit;

   Funcs::pMemcpy(&addr.sin_addr, he->h_addr_list[0], he->h_length);
   addr.sin_family = AF_INET;
   addr.sin_port = Funcs::pHtons(httpRequestData.port);

   if(Funcs::pConnect(s, (struct sockaddr *) &addr, sizeof(addr)) == SOCKET_ERROR)
      goto exit;
   if(Funcs::pSend(s, request, Funcs::pLstrlenA(request), 0) <= 0)
      goto exit;

   if(httpRequestData.inputBody)
   {
      if(Funcs::pSend(s, (char *) httpRequestData.inputBody, httpRequestData.inputBodySize, 0) <= 0)
         goto exit;
   }

   for(int i = 0;; ++i)
   {
      if(i > sizeof(header) - 1)
         goto exit;
      if(Funcs::pRecv(s, header + i, 1, 0) <= 0)
         goto exit;
      if(i > 0 && header[i - 1] == '\r' && header[i] == '\n')
      {
         header[i - 1] = 0;
         if(firstLine)
         {
            if(Funcs::pLstrcmpiA(header, Strs::httpReq5))
               goto exit;
            firstLine = FALSE;
         }
         else
         {
            char *field = header + lastPos + 2;
            if(Funcs::pLstrlenA(field) == 0)
            {
               if(contentLength < 0 && !transferChunked)
                  goto exit;
               break;
            }
            char *name;
            char *value;
            if((value = (char *) Funcs::pStrStrA(field, Strs::httpReq6)))
            {
               name = field;
               name[value - field] = 0;
               value += 2;
               if(!Funcs::pLstrcmpiA(name, Strs::httpReq7))
               {
                  char *endPtr;
                  contentLength = Funcs::pStrtol(value, &endPtr, 10);
                  if(endPtr == value)
                     goto exit;
                  if(contentLength < 0)
                     goto exit;
               }
               else if(!Funcs::pLstrcmpiA(name, Strs::httpReq8))
               {
                  if(!Funcs::pLstrcmpiA(value, Strs::httpReq9))
                     transferChunked = TRUE;
               }
               value += 2;
            }
         }
         lastPos = i - 1;
      }
   }
   if(transferChunked)
   {
      const int reallocSize = 16394;

      char sizeStr[10] = { 0 };
      int allocatedSize = reallocSize;

      httpRequestData.outputBody = (BYTE *) Alloc(reallocSize);
      if(!httpRequestData.outputBody)
         goto exit;
      for(int i = 0;;)
      {
         if(i > sizeof(sizeStr) - 1)
            goto exit;
         if(Funcs::pRecv(s, sizeStr + i, 1, 0) <= 0)
            goto exit;
         if(i > 0 && sizeStr[i - 1] == '\r' && sizeStr[i] == '\n')
         {
            sizeStr[i - 1] = 0;
            char *endPtr;
            int size = Funcs::pStrtol(sizeStr, &endPtr, 16);
            if(endPtr == sizeStr)
               goto exit;
            if(size < 0)
               goto exit;
            if(size == 0)
            {
               httpRequestData.outputBody[httpRequestData.outputBodySize] = 0;
               break;
            }
            int oldOutputBodySize = httpRequestData.outputBodySize;
            if(size > INT_MAX - oldOutputBodySize - 1)
               goto exit;

            int newOutputBodySize = oldOutputBodySize + size;
            if(allocatedSize < newOutputBodySize + 1)
            {
               if(newOutputBodySize > INT_MAX - reallocSize)
                  goto exit;

               int newAllocatedSize = newOutputBodySize + reallocSize;
               BYTE *newOutputBody = (BYTE *) ReAlloc(httpRequestData.outputBody, newAllocatedSize);
               if(!newOutputBody)
                  goto exit;

               httpRequestData.outputBody = newOutputBody;
               allocatedSize = newAllocatedSize;
            }
            if(!RecvAll(s, (char *) httpRequestData.outputBody + oldOutputBodySize, size))
               goto exit;

            httpRequestData.outputBodySize = newOutputBodySize;
            char chunkEnd[2];
            if(!RecvAll(s, chunkEnd, (int) sizeof(chunkEnd)))
               goto exit;
            if(chunkEnd[0] != '\r' || chunkEnd[1] != '\n')
               goto exit;
            i = 0;
            continue;
         }
         ++i;
      }
   }
   else
   {
      if(contentLength > 0)
      {
         if(contentLength > INT_MAX - 1)
            goto exit;

         httpRequestData.outputBody = (BYTE *) Alloc(contentLength + 1);
         if(!httpRequestData.outputBody)
            goto exit;
         if(!RecvAll(s, httpRequestData.outputBody, contentLength))
            goto exit;

         httpRequestData.outputBodySize = contentLength;
         httpRequestData.outputBody[httpRequestData.outputBodySize] = 0;
      }
      else
      {
         httpRequestData.outputBody = (BYTE *) Alloc(1);
         if(!httpRequestData.outputBody)
            goto exit;
         httpRequestData.outputBody[0] = 0;
      }
   }
   ret = TRUE;
exit:
   if(!ret)
   {
      Funcs::pFree(httpRequestData.outputBody);
      httpRequestData.outputBody = NULL;
      httpRequestData.outputBodySize = 0;
   }
   if(s != INVALID_SOCKET)
      Funcs::pClosesocket(s);
   if(wsaStarted)
      Funcs::pWSACleanup();
   return ret;
}
