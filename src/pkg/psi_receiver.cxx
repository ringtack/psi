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

  // Send g^b
  DHPublicValue_Message public_value_msg;
  public_value_msg.public_value = std::get<2>(dh_values);
  std::vector<unsigned char> public_value_data;
  public_value_msg.serialize(public_value_data);
  network_driver->send(public_value_data);

  // Listen for g^a
  public_value_data = network_driver->read();
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
void PSIReceiver::run() {
  // Key exchange
  auto keys = this->HandleKeyExchange();
  std::tie(this->AES_key, this->HMAC_key) = keys;

  std::vector<unsigned char> data;
  bool res;

  // Read message from sender
  data = this->network_driver->read();
  // Decrypt and verify
  std::tie(data, res) = this->crypto_driver->decrypt_and_verify(
      this->AES_key, this->HMAC_key, data);
  if (!res) {
    throw std::runtime_error("dumbass failed to decrypt/verify");
  }

  // Deserialize
  SenderToReceiver_PSI_Message psi_msg;
  psi_msg.deserialize(data);

  // Sample k_a <- Z_q
  CryptoPP::AutoSeededRandomPool prng;
  CryptoPP::Integer k_a(prng, 0, DL_Q - 1);

  // Hash all inputs, and raise to power
  std::vector<CryptoPP::SecByteBlock> hashed_receiver_inputs;
  hashed_receiver_inputs.reserve(this->input_set.size());
  for (auto &&input : this->input_set) {
    auto bb = string_to_byteblock(input);
    auto h = this->crypto_driver->hash(bb);
    auto exp_h =
        CryptoPP::ModularExponentiation(byteblock_to_integer(h), k_a, DL_Q);
    hashed_receiver_inputs.push_back(integer_to_byteblock(exp_h));
  }

  // Raise all of received values to power as well
  std::vector<CryptoPP::SecByteBlock> hashed_sender_inputs;
  for (auto &&input : psi_msg.hashed_sender_inputs) {
    auto exp_input =
        CryptoPP::ModularExponentiation(byteblock_to_integer(input), k_a, DL_Q);
    hashed_sender_inputs.push_back(integer_to_byteblock(exp_input));
  }

  // If PSI_CA, shuffle sender inputs
  if (psi_msg.type == PSIType::PSI_CA) {
    std::random_shuffle(hashed_sender_inputs.begin(),
                        hashed_sender_inputs.end());
  }

  // Send back to sender
  ReceiverToSender_PSI_Message receiver_msg;
  receiver_msg.hashed_sender_inputs = hashed_sender_inputs;
  receiver_msg.hashed_receiver_inputs = hashed_receiver_inputs;
  data = this->crypto_driver->encrypt_and_tag(this->AES_key, this->HMAC_key,
                                              &receiver_msg);
  this->network_driver->send(data);

  this->network_driver->disconnect();
}
