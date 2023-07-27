#include "sphere_mmo_client.h"

int main(int ac, char** av) {
  gns::SphereMmoClient client{};
  client.Run("127.0.0.1:27015");
  return 0;
}