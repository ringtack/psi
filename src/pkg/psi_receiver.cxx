#include "../../include/pkg/psi_receiver.hpp"
#include "../../include-shared/constants.hpp"
#include "../../include-shared/logger.hpp"
#include "../../include-shared/util.hpp"

/*
Syntax to use logger:
  CUSTOM_LOG(lg, debug) << "your message"
See logger.hpp for more modes besides 'debug'
*/

/**
 * Handle key exchange with evaluator
 */
std::pair<CryptoPP::SecByteBlock, CryptoPP::SecByteBlock>
PSIReceiver::HandleKeyExchange() {
  // Generate private/public DH keys
  auto dh_values = this->crypto_driver->DH_initialize();

  // Listen for g^b
  std::vector<unsigned char> public_value_data = network_driver->read();
  DHPublicValue_Message public_value_msg;
  public_value_msg.deserialize(public_value_data);

  // Send g^a
  DHPublicValue_Message evaluator_public_value_s;
  evaluator_public_value_s.public_value = std::get<2>(dh_values);
  std::vector<unsigned char> evaluator_public_value_data;
  evaluator_public_value_s.serialize(evaluator_public_value_data);
  network_driver->send(evaluator_public_value_data);

  // Recover g^ab
  CryptoPP::SecByteBlock DH_shared_key = crypto_driver->DH_generate_shared_key(
      std::get<0>(dh_values), std::get<1>(dh_values),
      garbler_public_value_s.public_value);
  CryptoPP::SecByteBlock AES_key =
      this->crypto_driver->AES_generate_key(DH_shared_key);
  CryptoPP::SecByteBlock HMAC_key =
      this->crypto_driver->HMAC_generate_key(DH_shared_key);
  auto keys = std::make_pair(AES_key, HMAC_key);
  return keys;
}

/**

 */
std::string PSI_Receiver::run(std::vector<int> input) {
  // Key exchange
  auto keys = this->HandleKeyExchange();
  std::tie(this->AES_key, this->HMAC_key) = keys;

  std::vector<unsigned char> data;
  bool res;
  // Read message from sender
  std::vector<unsigned char> = this->network_driver->read();
  // Deserialize and verify
  std::tie(data, res) = this->crypto_driver->decrypt_and_verify(
      this->AES_key, this->HMAC_key, ciphertext_data);
  if (!res) {
    this->cli_driver->print_warning("dumbass failed to decrypt/verify");
    exit(1);
  }
  SenderToReceiver_PSI_Message psi_msg;
  psi_msg.deserialize(data);

  // Hash all inputs
  // std::vector<CryptoPP::SecByteBlock>
  // Sample k_a <- Z_q
  CryptoPP::AutoSeededRandomPool prng;
  CryptoPP::Integer k_a(prng, 0, DL_Q - 1);
}
