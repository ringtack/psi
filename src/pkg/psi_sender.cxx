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
void PSISender::run(PSIType type) {
  // Key exchange
  auto keys = this->HandleKeyExchange();
  std::tie(this->AES_key, this->HMAC_key) = keys;

  // hash each of the strings
  CryptoPP::AutoSeededRandomPool prng;
  CryptoPP::Integer k(prng, 0, DL_Q - 1);

  // initialize the hashed inputs for the sender
  SenderToReceiver_PSI_Message sender_msg;
  for (auto curr_input : this->input_set) {
    auto bb = string_to_byteblock(curr_input);
    auto hashed_value = this->crypto_driver->hash(bb);
    auto res_value = CryptoPP::ModularExponentiation(
        byteblock_to_integer(hashed_value), k, DL_Q);
    sender_msg.hashed_sender_inputs.push_back(integer_to_byteblock(res_value));
  }
  sender_msg.type = type;

  std::vector<unsigned char> hashed_set = this->crypto_driver->encrypt_and_tag(
      this->AES_key, this->HMAC_key, &sender_msg);
  this->network_driver->send(hashed_set);

  // read the data from the receiver
  std::vector<unsigned char> two_hashed_set_data = this->network_driver->read();
  ReceiverToSender_PSI_Message receiver_msg;
  auto data_and_tag = this->crypto_driver->decrypt_and_verify(
      this->AES_key, this->HMAC_key, two_hashed_set_data);
  if (!data_and_tag.second) {
    throw std::runtime_error("invalid tag");
  }
  receiver_msg.deserialize(data_and_tag.first);

  std::set<std::string> receiver_inputs;
  for (auto &&hashed_input : receiver_msg.hashed_receiver_inputs) {
    receiver_inputs.insert(byteblock_to_string(hashed_input));
  }

  if (type == PSIType::PSI_Intersection) {
    std::vector<std::string> res;

    for (int i = 0; i < receiver_msg.hashed_sender_inputs.size(); i++) {
      if (receiver_inputs.contains(
              byteblock_to_string(receiver_msg.hashed_sender_inputs[i]))) {
        res.push_back(this->input_set[i]);
      }
    }
    // verify the output to make sure that it's correct
    for (auto str : res) {
      this->cli_driver->print_info(str);
    }
  } else {
    std::set<std::string> sender_inputs;
    for (auto &&hashed_input : sender_msg.hashed_sender_inputs) {
      sender_inputs.insert(byteblock_to_string(hashed_input));
    }

    std::vector<std::string> intersection;
    std::set_intersection(sender_inputs.begin(), sender_inputs.end(),
                          receiver_inputs.begin(), receiver_inputs.end(),
                          std::inserter(intersection, intersection.begin()));

    this->cli_driver->print_info("Intersection size: " +
                                 std::to_string(intersection.size()));
  }

  this->network_driver->disconnect();
}