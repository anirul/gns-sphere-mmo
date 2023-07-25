#include "game_networking_sockets.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <steam/isteamnetworkingutils.h>
#include <steam/steamnetworkingsockets.h>
#include <steam/steamnetworkingtypes.h>
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

namespace gns {

namespace {
static void DebugOutput(ESteamNetworkingSocketsDebugOutputType eType,
                        const char* pszMsg) {
  printf("%s\n", pszMsg);
}
}  // namespace

GameNetworkingSockets::GameNetworkingSockets() {
  SteamDatagramErrMsg errMsg;
  if (!GameNetworkingSockets_Init(nullptr, errMsg)) {
    std::size_t size = sizeof(errMsg);
    errMsg[size - 1] = '\0';  // Ensure null-terminated
    throw std::runtime_error("GameNetworkingSockets_Init failed.  %s" +
                             std::string(errMsg));
  }
  SteamNetworkingUtils()->SetDebugOutputFunction(
      k_ESteamNetworkingSocketsDebugOutputType_Msg, DebugOutput);
}

GameNetworkingSockets::~GameNetworkingSockets() {
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  GameNetworkingSockets_Kill();
}

void GameNetworkingSockets::Print(std::string_view message) {}

void GameNetworkingSockets::FatalError(std::string_view message) {}

}  // namespace gns
