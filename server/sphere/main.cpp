
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <steam/isteamnetworkingutils.h>
#include <steam/steamnetworkingsockets.h>
#include <string.h>

#include <algorithm>
#include <cctype>
#include <chrono>
#include <map>
#include <mutex>
#include <queue>
#include <random>
#include <string>
#include <thread>

static void DebugOutput(ESteamNetworkingSocketsDebugOutputType eType,
                           const char* pszMsg) {
  printf("%s\n", pszMsg);
}

int main(int ac, char** av) {
  {
    SteamDatagramErrMsg errMsg;
    if (!GameNetworkingSockets_Init(nullptr, errMsg))
      throw std::runtime_error("GameNetworkingSockets_Init failed.  %s" +
                               std::string(errMsg));
  }
  SteamNetworkingUtils()->SetDebugOutputFunction(
      k_ESteamNetworkingSocketsDebugOutputType_Msg, DebugOutput);

  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  GameNetworkingSockets_Kill();
  return 0;
}