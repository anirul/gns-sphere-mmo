#include "game_networking_sockets.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <steam/isteamnetworkingutils.h>
#include <steam/steamnetworkingsockets.h>
#include <steam/steamnetworkingtypes.h>
#include <string.h>
#include <absl/debugging/symbolize.h>
#include <absl/debugging/stacktrace.h>
#include <absl/strings/str_cat.h>

#include <algorithm>
#include <cctype>
#include <chrono>
#include <iostream>
#include <map>
#include <mutex>
#include <queue>
#include <random>
#include <string>
#include <thread>
#include <format>

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

void GameNetworkingSockets::Print(std::string_view message) {
  std::cout << message << std::endl;
}

void GameNetworkingSockets::FatalError(std::string_view message) {
  std::cerr << message << std::endl;
    PrintStackTrace();
  throw std::runtime_error(std::string(message));
}

void GameNetworkingSockets::PrintStackTrace() {
  void* result[30];
  int depth = absl::GetStackTrace(result, 30, 1);
  for (int i = 0; i < depth; i++) {
    std::string tmp;
    const char* symbol = "(unknown)";
    if (absl::Symbolize(result[i], &tmp[0], sizeof(tmp))) {
      symbol = tmp.c_str();
    }
    Print(std::format("    #{} {}", absl::StrCat(i), symbol));
  }
}

}  // namespace gns
