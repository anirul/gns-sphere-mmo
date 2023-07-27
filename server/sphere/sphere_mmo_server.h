#pragma once

#include <steam/isteamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>

#include <memory>
#include <map>

#include "common/gns/game_networking_sockets.h"
#include "common/gns/sphere_client.h"

namespace gns {

class SphereMmoServer : public GameNetworkingSockets {
 public:
  SphereMmoServer();
  void Run(std::int16_t);

 private:
  ISteamNetworkingSockets* steam_networking_sockets_ = nullptr;
  HSteamListenSocket h_listen_socket_;
  HSteamNetPollGroup h_poll_group_;

  std::map<HSteamNetConnection, SphereClient> map_sphere_clients_;
  bool the_end = false;

  void SendStringToClient(HSteamNetConnection conn, const char* str);
  void SendStringToAllClients(
      const char* str,
      HSteamNetConnection except = k_HSteamNetConnection_Invalid);
  void SetClientNick( HSteamNetConnection hConn, const char *nick );
  void PollIncomingMessages();
  void PollLocalUserInput();
  void OnSteamNetConnectionStatusChanged(
      SteamNetConnectionStatusChangedCallback_t* pInfo);
  void PollConnectionStateChanges();

  static SphereMmoServer* s_instance_;
  static void SteamNetConnectionStatusChangedCallback(
      SteamNetConnectionStatusChangedCallback_t* pInfo) {
    s_instance_->OnSteamNetConnectionStatusChanged(pInfo);
  };
};

}  // namespace gns
