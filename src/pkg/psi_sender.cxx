#include <algorithm>
#include <crypto++/misc.h>

#include "../../include-shared/constants.hpp"
#include "../../include-shared/logger.hpp"
#include "../../include-shared/util.hpp"
#include "../../include/pkg/psi_sender.hpp"

/*
Syntax to use logger:
  CUSTOM_LOG(lg, debug) << "your message"
See logger.hpp for more modes besides 'debug'
*/
namespace {
src::severity_logger<logging::trivial::severity_level> lg;
}

/**
 * Handle key exchange with evaluator
 */
std::pair<CryptoPP::SecByteBlock, CryptoPP::SecByteBlock>
PSISender::HandleKeyExchange() {
  // Generate private/public DH keys
  auto dh_values = this->crypto_driver->DH_initialize();

  // Send g^b
  DHPublicValue_Message public_value_msg;
  public_value_msg.public_value = std::get<2>(dh_values);
  std::vector<unsigned char> public_value_data;
  public_value_msg.serialize(public_value_data);
  network_driver->send(public_value_data);

  // Listen for g^a
  std::vector<unsigned char> public_value_data = network_driver->read();
  DHPublicValue_Message public_value_s;
  public_value_s.deserialize(public_value_data);

  // Recover g^ab
  CryptoPP::SecByteBlock DH_shared_key = crypto_driver->DH_generate_shared_key(
      std::get<0>(dh_values), std::get<1>(dh_values),
      public_value_s.public_value);
  CryptoPP::SecByteBlock AES_key =
      this->crypto_driver->AES_generate_key(DH_shared_key);
  CryptoPP::SecByteBlock HMAC_key =
      this->crypto_driver->HMAC_generate_key(DH_shared_key);
  auto keys = std::make_pair(AES_key, HMAC_key);
  return keys;
}

/**
 */
std::string PSISender::run(std::vector<int> input) {
  // Key exchange
  auto keys = this->HandleKeyExchange();
  std::tie(this->AES_key, this->HMAC_key) = keys;

  // hash each of the strings
  CryptoPP::AutoSeededRandomPool prng;
  CryptoPP::Integer k(prng, 0, DL_Q - 1);

  SenderToReceiver_PSI_Message sender_msg;
  for (auto curr_input : this->input_set) {
    auto hashed_value = string_to_byteblock(curr_input);
    sender_msg.hashed_sender_inputs.push_back(
        CryptoPP::ModularExponentiation(hashed_value, k, DL_Q));
  }

  this->crypto_driver->encrypt_and_tag()
}