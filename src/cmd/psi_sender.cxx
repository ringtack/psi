#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>

#include "../../include-shared/logger.hpp"
#include "../../include-shared/util.hpp"
#include "../../include/pkg/psi_sender.hpp"

// std::vector<std::string> parse_input(std::string input_file);

/*
 * Usage: ./psi_sender <input file> <psi_type> <address> <port> <output file>
 */
int main(int argc, char *argv[]) {
  // Initialize logger

  // Parse args
  if (argc != 6) {
    std::cout << "Usage: ./psi_sender <input file> <psi_type: I CA> <address> "
                 "<port> <output file>"
              << std::endl;
    return 1;
  }
  std::string input_file = argv[1];
  std::string type_str = argv[2];
  PSIType type;

  if (type_str == "I") {
    type = PSIType::PSI_Intersection;
  } else if (type_str == "CA") {
    type = PSIType::PSI_CA;
  } else {
    std::cout << "Invalid type" << std::endl;
    return 1;
  }

  std::string address = argv[3];
  int port = atoi(argv[4]);

  // Parse input.
  auto string_set = parse_input(input_file);

  // Save output file directory
  std::string out_file = argv[5];

  // Connect to network driver.
  std::shared_ptr<NetworkDriver> network_driver =
      std::make_shared<NetworkDriverImpl>();
  network_driver->listen(port);
  std::shared_ptr<CryptoDriver> crypto_driver =
      std::make_shared<CryptoDriver>();

  // Create sender then run.
  PSISender sender(string_set, network_driver, crypto_driver);

  // Time
  auto start = std::chrono::high_resolution_clock::now();

  sender.run(type, out_file);

  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> diff = end - start;
  // Print time in milliseconds
  std::cout << "Time: " << diff.count() * 1000 << "ms" << std::endl;

  return 0;
}
