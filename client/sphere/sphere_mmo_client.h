#pragma once

#include <steam/isteamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>

#include <memory>

#include "common/gns/game_networking_sockets.h"

namespace gns {

class SphereMmoClient : public GameNetworkingSockets {
 public:
  SphereMmoClient();
  void Run(std::string_view server_address);

 private:
  ISteamNetworkingSockets* steam_networking_sockets_ = nullptr;
  HSteamNetConnection h_connection_ = k_HSteamNetConnection_Invalid;
  bool the_end = false;

 private:
  void PollIncomingMessages();
  void PollLocalUserInput();
  void OnSteamNetConnectionStatusChanged(
      SteamNetConnectionStatusChangedCallback_t* pInfo);
  void PollConnectionStateChanges();

 private:
  static SphereMmoClient* s_instance_;
  static void SteamNetConnectionStatusChangedCallback(
      SteamNetConnectionStatusChangedCallback_t* pInfo) {
    s_instance_->OnSteamNetConnectionStatusChanged(pInfo);
  }
};

}  // namespace gns
