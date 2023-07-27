#pragma once

#include <string>

// Game Networking Sockets class.
namespace gns {

class GameNetworkingSockets {
 public:
  GameNetworkingSockets();
  virtual ~GameNetworkingSockets();

 public:
  virtual void Print(std::string_view message);
  virtual void FatalError(std::string_view message);

 private:
  void PrintStackTrace();
};

}  // namespace gns