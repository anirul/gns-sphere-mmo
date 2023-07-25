#include "common/gns/sphere_mmo_server.h"

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
#include <format>
#include <iostream>
#include <map>
#include <mutex>
#include <queue>
#include <random>
#include <string>
#include <thread>

namespace gns {

SphereMmoServer* SphereMmoServer::s_instance_ = nullptr;

SphereMmoServer::SphereMmoServer() : GameNetworkingSockets() {}

void SphereMmoServer::SendStringToClient(HSteamNetConnection conn,
                                         const char* str) {
  steam_networking_sockets_->SendMessageToConnection(
      conn, str, (uint32)strlen(str), k_nSteamNetworkingSend_Reliable, nullptr);
}

void SphereMmoServer::SendStringToAllClients(
    const char* str,
    HSteamNetConnection except /*= k_HSteamNetConnection_Invalid*/) {
  for (auto& c : map_sphere_clients_) {
    if (c.first != except) SendStringToClient(c.first, str);
  }
}

void SphereMmoServer::SetClientNick(HSteamNetConnection hConn,
                                    const char* nick) {
  // Remember their nick
  map_sphere_clients_[hConn].nick = nick;

  // Set the connection name, too, which is useful for debugging
  steam_networking_sockets_->SetConnectionName(hConn, nick);
}

void SphereMmoServer::PollIncomingMessages() {
  char temp[1024];

  while (!the_end) {
    ISteamNetworkingMessage* pIncomingMsg = nullptr;
    int numMsgs = steam_networking_sockets_->ReceiveMessagesOnPollGroup(
        h_poll_group_, &pIncomingMsg, 1);
    if (numMsgs == 0) break;
    if (numMsgs < 0) FatalError("Error checking for messages");
    assert(numMsgs == 1 && pIncomingMsg);
    auto itClient = map_sphere_clients_.find(pIncomingMsg->m_conn);
    assert(itClient != map_sphere_clients_.end());

    // '\0'-terminate it to make it easier to parse
    std::string sCmd;
    sCmd.assign((const char*)pIncomingMsg->m_pData, pIncomingMsg->m_cbSize);
    const char* cmd = sCmd.c_str();

    // We don't need this anymore.
    pIncomingMsg->Release();

    // Check for known commands.  None of this example code is secure or robust.
    // Don't write a real server like this, please.

    if (strncmp(cmd, "/nick", 5) == 0) {
      const char* nick = cmd + 5;
      while (isspace(*nick)) ++nick;

      // Let everybody else know they changed their name
      sprintf(temp, "%s shall henceforth be known as %s",
              itClient->second.nick.c_str(), nick);
      SendStringToAllClients(temp, itClient->first);

      // Respond to client
      sprintf(temp, "Ye shall henceforth be known as %s", nick);
      SendStringToClient(itClient->first, temp);

      // Actually change their name
      SetClientNick(itClient->first, nick);
      continue;
    }

    // Assume it's just a ordinary chat message, dispatch to everybody else
    sprintf(temp, "%s: %s", itClient->second.nick.c_str(), cmd);
    SendStringToAllClients(temp, itClient->first);
  }
}

void SphereMmoServer::PollLocalUserInput() {
  std::string cmd;
  while (!the_end && std::getline(std::cin, cmd)) {
    if (strcmp(cmd.c_str(), "/quit") == 0) {
      the_end = true;
      Print("Shutting down server");
      break;
    }

    // That's the only command we support
    Print("The server only knows one command: '/quit'");
  }
}

void SphereMmoServer::OnSteamNetConnectionStatusChanged(
    SteamNetConnectionStatusChangedCallback_t* pInfo) {
  char temp[1024];

  // What's the state of the connection?
  switch (pInfo->m_info.m_eState) {
    case k_ESteamNetworkingConnectionState_None:
      // NOTE: We will get callbacks here when we destroy connections.  You can
      // ignore these.
      break;

    case k_ESteamNetworkingConnectionState_ClosedByPeer:
    case k_ESteamNetworkingConnectionState_ProblemDetectedLocally: {
      // Ignore if they were not previously connected.  (If they disconnected
      // before we accepted the connection.)
      if (pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connected) {
        // Locate the client.  Note that it should have been found, because this
        // is the only codepath where we remove clients (except on shutdown),
        // and connection change callbacks are dispatched in queue order.
        auto itClient = map_sphere_clients_.find(pInfo->m_hConn);
        assert(itClient != map_sphere_clients_.end());

        // Select appropriate log messages
        const char* pszDebugLogAction;
        if (pInfo->m_info.m_eState ==
            k_ESteamNetworkingConnectionState_ProblemDetectedLocally) {
          pszDebugLogAction = "problem detected locally";
          sprintf(temp, "Alas, %s hath fallen into shadow.  (%s)",
                  itClient->second.nick.c_str(), pInfo->m_info.m_szEndDebug);
        } else {
          // Note that here we could check the reason code to see if
          // it was a "usual" connection or an "unusual" one.
          pszDebugLogAction = "closed by peer";
          sprintf(temp, "%s hath departed", itClient->second.nick.c_str());
        }

        // Spew something to our own log.  Note that because we put their nick
        // as the connection description, it will show up, along with their
        // transport-specific data (e.g. their IP address)
        Print(std::format("Connection {} {}, reason {}: {}\n",
                          pInfo->m_info.m_szConnectionDescription,
                          pszDebugLogAction, pInfo->m_info.m_eEndReason,
                          pInfo->m_info.m_szEndDebug));

        map_sphere_clients_.erase(itClient);

        // Send a message so everybody else knows what happened
        SendStringToAllClients(temp);
      } else {
        assert(pInfo->m_eOldState ==
               k_ESteamNetworkingConnectionState_Connecting);
      }

      // Clean up the connection.  This is important!
      // The connection is "closed" in the network sense, but
      // it has not been destroyed.  We must close it on our end, too
      // to finish up.  The reason information do not matter in this case,
      // and we cannot linger because it's already closed on the other end,
      // so we just pass 0's.
      steam_networking_sockets_->CloseConnection(pInfo->m_hConn, 0, nullptr,
                                                 false);
      break;
    }

    case k_ESteamNetworkingConnectionState_Connecting: {
      // This must be a new connection
      assert(map_sphere_clients_.find(pInfo->m_hConn) ==
             map_sphere_clients_.end());

      Print(std::format("Connection request from {}",
                        pInfo->m_info.m_szConnectionDescription));

      // A client is attempting to connect
      // Try to accept the connection.
      if (steam_networking_sockets_->AcceptConnection(pInfo->m_hConn) !=
          k_EResultOK) {
        // This could fail.  If the remote host tried to connect, but then
        // disconnected, the connection may already be half closed.  Just
        // destroy whatever we have on our side.
        steam_networking_sockets_->CloseConnection(pInfo->m_hConn, 0, nullptr,
                                                   false);
        Print("Can't accept connection.  (It was already closed?)");
        break;
      }

      // Assign the poll group
      if (!steam_networking_sockets_->SetConnectionPollGroup(pInfo->m_hConn,
                                                             h_poll_group_)) {
        steam_networking_sockets_->CloseConnection(pInfo->m_hConn, 0, nullptr,
                                                   false);
        Print("Failed to set poll group?");
        break;
      }

      // Generate a random nick.  A random temporary nick
      // is really dumb and not how you would write a real chat server.
      // You would want them to have some sort of signon message,
      // and you would keep their client in a state of limbo (connected,
      // but not logged on) until them.  I'm trying to keep this example
      // code really simple.
      char nick[64];
      sprintf(nick, "BraveWarrior%d", 10000 + (rand() % 100000));

      // Send them a welcome message
      sprintf(temp,
              "Welcome, stranger.  Thou art known to us for now as '%s'; upon "
              "thine command '/nick' we shall know thee otherwise.",
              nick);
      SendStringToClient(pInfo->m_hConn, temp);

      // Also send them a list of everybody who is already connected
      if (map_sphere_clients_.empty()) {
        SendStringToClient(pInfo->m_hConn, "Thou art utterly alone.");
      } else {
        sprintf(temp,
                "%d companions greet you:", (int)map_sphere_clients_.size());
        for (auto& c : map_sphere_clients_)
          SendStringToClient(pInfo->m_hConn, c.second.nick.c_str());
      }

      // Let everybody else know who they are for now
      sprintf(temp,
              "Hark!  A stranger hath joined this merry host.  For now we "
              "shall call them '%s'",
              nick);
      SendStringToAllClients(temp, pInfo->m_hConn);

      // Add them to the client list, using std::map wacky syntax
      map_sphere_clients_[pInfo->m_hConn];
      SetClientNick(pInfo->m_hConn, nick);
      break;
    }

    case k_ESteamNetworkingConnectionState_Connected:
      // We will get a callback immediately after accepting the connection.
      // Since we are the server, we can ignore this, it's not news to us.
      break;

    default:
      // Silences -Wswitch
      break;
  }
}

void SphereMmoServer::PollConnectionStateChanges() {
  s_instance_ = this;
  steam_networking_sockets_->RunCallbacks();
}

void SphereMmoServer::Run(std::int16_t port) {
  // Select instance to use.  For now we'll always use the default.
  // But we could use SteamGameServerNetworkingSockets() on Steam.
  steam_networking_sockets_ = SteamNetworkingSockets();

  // Start listening
  SteamNetworkingIPAddr serverLocalAddr;
  serverLocalAddr.Clear();
  serverLocalAddr.m_port = port;
  SteamNetworkingConfigValue_t opt;
  opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged,
             (void*)SteamNetConnectionStatusChangedCallback);
  h_listen_socket_ =
      steam_networking_sockets_->CreateListenSocketIP(serverLocalAddr, 1, &opt);
  if (h_listen_socket_ == k_HSteamListenSocket_Invalid)
    FatalError(std::format("Failed to listen on port {}", port));
  h_poll_group_ = steam_networking_sockets_->CreatePollGroup();
  if (h_poll_group_ == k_HSteamNetPollGroup_Invalid)
    FatalError(std::format("Failed to listen on port {}", port));
  Print(std::format("Server listening on port {}\n", port));

  while (!the_end) {
    PollIncomingMessages();
    PollConnectionStateChanges();
    PollLocalUserInput();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  // Close all the connections
  Print("Closing connections...\n");
  for (auto it : map_sphere_clients_) {
    // Send them one more goodbye message.  Note that we also have the
    // connection close reason as a place to send final data.  However,
    // that's usually best left for more diagnostic/debug text not actual
    // protocol strings.
    SendStringToClient(it.first, "Server is shutting down.  Goodbye.");

    // Close the connection.  We use "linger mode" to ask
    // SteamNetworkingSockets to flush this out and close gracefully.
    steam_networking_sockets_->CloseConnection(it.first, 0, "Server Shutdown",
                                               true);
  }
  map_sphere_clients_.clear();

  steam_networking_sockets_->CloseListenSocket(h_listen_socket_);
  h_listen_socket_ = k_HSteamListenSocket_Invalid;

  steam_networking_sockets_->DestroyPollGroup(h_poll_group_);
  h_poll_group_ = k_HSteamNetPollGroup_Invalid;
}

}  // namespace gns
