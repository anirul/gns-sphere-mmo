#include "common/gns/sphere_mmo_client.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <steam/isteamnetworkingSockets.h>
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

SphereMmoClient* SphereMmoClient::s_instance_ = nullptr;

SphereMmoClient::SphereMmoClient() : GameNetworkingSockets() {}

void SphereMmoClient::Run(std::string_view server_address) {
  steam_networking_sockets_ = SteamNetworkingSockets();
  char szAddr[SteamNetworkingIPAddr::k_cchMaxString];
  SteamNetworkingIPAddr server_ip_address{};
  server_ip_address.ParseString(server_address.data());
  server_ip_address.ToString(szAddr, sizeof(szAddr), true);
  printf("Connecting to %s\n", szAddr);
  SteamNetworkingConfigValue_t opt;
  opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged,
             (void*)SteamNetConnectionStatusChangedCallback);
  h_connection_ =
      steam_networking_sockets_->ConnectByIPAddress(server_ip_address, 1, &opt);
  if (h_connection_ == k_HSteamNetConnection_Invalid) {
    throw std::runtime_error("Failed to connect to server.");
  }

  while (!the_end) {
    PollIncomingMessages();
    PollConnectionStateChanges();
    PollLocalUserInput();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

void SphereMmoClient::PollIncomingMessages() {
  while (!the_end) {
    ISteamNetworkingMessage* pIncomingMsg = nullptr;
    int numMsgs = steam_networking_sockets_->ReceiveMessagesOnConnection(
        h_connection_, &pIncomingMsg, 1);
    if (numMsgs == 0) break;
    if (numMsgs < 0) {
      throw std::runtime_error("Error checking for messages");
    }

    // Just echo anything we get from the server
    fwrite(pIncomingMsg->m_pData, 1, pIncomingMsg->m_cbSize, stdout);
    fputc('\n', stdout);

    // We don't need this anymore.
    pIncomingMsg->Release();
  }
}

void SphereMmoClient::PollLocalUserInput() {
  // TODO(anirul): Fill me!
}

void SphereMmoClient::OnSteamNetConnectionStatusChanged(
    SteamNetConnectionStatusChangedCallback_t* pInfo) {
  assert(pInfo->m_hConn == h_connection_ ||
         h_connection_ == k_HSteamNetConnection_Invalid);

  // What's the state of the connection?
  switch (pInfo->m_info.m_eState) {
    case k_ESteamNetworkingConnectionState_None:
      // NOTE: We will get callbacks here when we destroy connections. You can
      // ignore these.
      break;

    case k_ESteamNetworkingConnectionState_ClosedByPeer:
      [[fallthrough]];
    case k_ESteamNetworkingConnectionState_ProblemDetectedLocally: {
      the_end = true;

      // Print an appropriate message
      if (pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connecting) {
        // Note: we could distinguish between a timeout, a rejected
        // connection, or some other transport problem.
        printf(
            "We sought the remote host, yet our efforts were met with "
            "defeat.  (%s)",
            pInfo->m_info.m_szEndDebug);
      } else if (pInfo->m_info.m_eState ==
                 k_ESteamNetworkingConnectionState_ProblemDetectedLocally) {
        printf(
            "Alas, troubles beset us; we have lost contact with the host.  "
            "(%s)",
            pInfo->m_info.m_szEndDebug);
      } else {
        // NOTE: We could check the reason code for a normal disconnection
        printf("The host hath bidden us farewell.  (%s)",
               pInfo->m_info.m_szEndDebug);
      }

      // Clean up the connection.  This is important!
      // The connection is "closed" in the network sense, but
      // it has not been destroyed.  We must close it on our end, too
      // to finish up.  The reason information do not matter in this case,
      // and we cannot linger because it's already closed on the other end,
      // so we just pass 0's.
      steam_networking_sockets_->CloseConnection(pInfo->m_hConn, 0, nullptr,
                                                 false);
      h_connection_ = k_HSteamNetConnection_Invalid;
      break;
    }
    case k_ESteamNetworkingConnectionState_Connecting:
      // We will get this callback when we start connecting.
      // We can ignore this.
      break;

    case k_ESteamNetworkingConnectionState_Connected:
      printf("Connected to server OK");
      break;

    default:
      // Silences '-Wswitch'.
      break;
  }
}

void SphereMmoClient::PollConnectionStateChanges() {
  s_instance_ = this;
  steam_networking_sockets_->RunCallbacks();
}

}  // namespace gns