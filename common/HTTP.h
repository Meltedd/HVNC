#pragma once
#include "Common.h"

struct HttpRequestData
{
   BOOL        post;
   int         port;
   const char *host;
   const char *path;
   BYTE       *inputBody;
   int         inputBodySize;
   BYTE       *outputBody;
   int         outputBodySize;
};

BOOL HttpSubmitRequest(HttpRequestData &httpRequestData);