#pragma once

#include "../../include-shared/circuit.hpp"
#include "../../include/drivers/cli_driver.hpp"
#include "../../include/drivers/crypto_driver.hpp"
#include "../../include/drivers/network_driver.hpp"

class PSISender {
public:
  PSISender(std::vector<std::string> input,
            std::shared_ptr<NetworkDriver> network_driver,
            std::shared_ptr<CryptoDriver> crypto_driver) {
    this->input_set = input;
    this->network_driver = network_driver;
    this->crypto_driver = crypto_driver;
    this->cli_driver = std::make_shared<CLIDriver>();
  }

  void run(PSIType type);
  std::pair<CryptoPP::SecByteBlock, CryptoPP::SecByteBlock> HandleKeyExchange();

private:
  std::shared_ptr<CryptoDriver> crypto_driver;
  std::shared_ptr<NetworkDriver> network_driver;
  std::shared_ptr<CLIDriver> cli_driver;
  std::vector<std::string> input_set;

  CryptoPP::SecByteBlock AES_key;
  CryptoPP::SecByteBlock HMAC_key;
};

// class EvaluatorClient {
// public:
//   EvaluatorClient(Circuit circuit,
//                   std::shared_ptr<NetworkDriver> network_driver,
//                   std::shared_ptr<CryptoDriver> crypto_driver);
//   std::pair<CryptoPP::SecByteBlock, CryptoPP::SecByteBlock>
//   HandleKeyExchange(); std::string run(std::vector<int> input); GarbledWire
//   evaluate_gate(GarbledGate gate, GarbledWire lhs, GarbledWire rhs); bool
//   verify_decryption(CryptoPP::SecByteBlock decryption);
//   CryptoPP::SecByteBlock snip_decryption(CryptoPP::SecByteBlock decryption);

// private:
//   Circuit circuit;
//   std::shared_ptr<NetworkDriver> network_driver;
//   std::shared_ptr<CryptoDriver> crypto_driver;
//   std::shared_ptr<OTDriver> ot_driver;
//   std::shared_ptr<CLIDriver> cli_driver;

//   CryptoPP::SecByteBlock AES_key;
//   CryptoPP::SecByteBlock HMAC_key;

//   void print_and_throw(const std::string &s) {
//     this->cli_driver->print_warning(s);
//     throw std::runtime_error(s);
//   }
// };
