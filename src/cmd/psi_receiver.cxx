#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>

#include "../../include-shared/logger.hpp"
#include "../../include-shared/util.hpp"
#include "../../include/pkg/psi_receiver.hpp"

// std::vector<std::string> parse_input(std::string input_file);

/*
 * Usage: ./psi_receiver <input file> <address> <port>
 */
int main(int argc, char *argv[]) {
  // Initialize logger

  // Parse args
  if (argc != 4) {
    std::cout << "Usage: ./psi_receiver <input file> <address> <port>"
              << std::endl;
    return 1;
  }
  std::string input_file = argv[1];
  std::string address = argv[2];
  int port = atoi(argv[3]);

  auto string_set = parse_input(input_file);

  // Connect to network driver.
  std::shared_ptr<NetworkDriver> network_driver =
      std::make_shared<NetworkDriverImpl>();
  network_driver->connect(address, port);
  std::shared_ptr<CryptoDriver> crypto_driver =
      std::make_shared<CryptoDriver>();

  // Create PSI receiver then run.
  PSIReceiver receiver(string_set, network_driver, crypto_driver);
  receiver.run();
  return 0;
}
