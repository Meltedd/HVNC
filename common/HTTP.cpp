#include "HTTP.h"

BOOL HttpSubmitRequest(HttpRequestData &httpRequestData)
{
   BOOL ret = FALSE;
   WSADATA wsa;
   SOCKET s;

   char request[1024] = { 0 };

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

   if((s = Funcs::pSocket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
      goto exit;

   hostent *he = Funcs::pGethostbyname(httpRequestData.host);
   if(!he)
      goto exit;

   struct sockaddr_in addr;
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
   
   char header[1024] = { 0 };
   int contentLength = -1;
   int lastPos = 0;
   BOOL firstLine = TRUE;
   BOOL transferChunked = FALSE;

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
                  if(value < 0)
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
      int read = 0;

      httpRequestData.outputBody = (BYTE *) Alloc(reallocSize);
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
            httpRequestData.outputBodySize += size;
            if(allocatedSize < httpRequestData.outputBodySize + 1)
            {
               allocatedSize += httpRequestData.outputBodySize + reallocSize;
               httpRequestData.outputBody = (BYTE *) ReAlloc(httpRequestData.outputBody, allocatedSize);
            }
            int chunkRead = 0;
            do
            {
               int read2 = Funcs::pRecv(s, (char *) httpRequestData.outputBody + read + chunkRead, size - chunkRead, 0);
               if(read2 <= 0)
                  goto exit;
               chunkRead += read2;
            } while(chunkRead != size);
            if(Funcs::pRecv(s, sizeStr, 2, 0) <= 0)
               goto exit;
            read += size;
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
         httpRequestData.outputBody = (BYTE *) Alloc(contentLength + 1);
         httpRequestData.outputBodySize = contentLength;
         httpRequestData.outputBody[httpRequestData.outputBodySize] = 0;
         int totalRead = 0;
         do
         {
            int read = Funcs::pRecv(s, (char *) httpRequestData.outputBody + totalRead, contentLength - totalRead, 0);
            if(read <= 0) goto exit;
            totalRead += read;
         }
         while(totalRead != contentLength);
      }
      else
      {
         httpRequestData.outputBody = (BYTE *) Alloc(1);
         httpRequestData.outputBody[0] = 0;
      }
   }
   ret = TRUE;
exit:
   if(!ret)
   {
      httpRequestData.outputBody = NULL;
      Funcs::pFree(httpRequestData.outputBody);
   }
   Funcs::pClosesocket(s);
   Funcs::pWSACleanup();
   return ret;
}