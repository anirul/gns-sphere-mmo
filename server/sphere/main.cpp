
#include "sphere_mmo_server.h"

int main(int ac, char** av) {
  gns::SphereMmoServer server{};
  server.Run(27015);
  return 0;
}