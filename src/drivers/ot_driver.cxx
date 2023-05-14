#include <iostream>
#include <stdexcept>
#include <string>

#include "crypto++/base64.h"
#include "crypto++/dsa.h"
#include "crypto++/osrng.h"
#include "crypto++/rsa.h"
#include <crypto++/cryptlib.h>
#include <crypto++/elgamal.h>
#include <crypto++/files.h>
#include <crypto++/hkdf.h>
#include <crypto++/nbtheory.h>
#include <crypto++/queue.h>
#include <crypto++/sha.h>

#include "../../include-shared/constants.hpp"
#include "../../include-shared/messages.hpp"
#include "../../include-shared/util.hpp"
#include "../../include/drivers/ot_driver.hpp"

/*
 * Constructor
 */
OTDriver::OTDriver(
    std::shared_ptr<NetworkDriver> network_driver,
    std::shared_ptr<CryptoDriver> crypto_driver,
    std::pair<CryptoPP::SecByteBlock, CryptoPP::SecByteBlock> keys) {
  this->network_driver = network_driver;
  this->crypto_driver = crypto_driver;
  this->AES_key = keys.first;
  this->HMAC_key = keys.second;
  this->cli_driver = std::make_shared<CLIDriver>();
}

/*
 * Send either m0 or m1 using OT. This function should:
 * 1) Sample a public DH value and send it to the receiver
 * 2) Receive the receiver's public value
 * 3) Encrypt m0 and m1 using different keys
 * 4) Send the encrypted values
 * You may find `byteblock_to_integer` and `integer_to_byteblock` useful
 * Disconnect and throw errors only for invalid MACs
 */
void OTDriver::OT_send(std::string m0, std::string m1) {
  // TODO: implement me!

  // Store results for Enc/Dec
  std::vector<unsigned char> data;
  bool res;

  // Generate a public DH value and send it to the receiver
  auto [DH_obj, sk, pk] = this->crypto_driver->DH_initialize();
  SenderToReceiver_OTPublicValue_Message pk_msg;
  pk_msg.public_value = pk;
  data = this->crypto_driver->encrypt_and_tag(this->AES_key, this->HMAC_key,
                                              &pk_msg);
  this->network_driver->send(data);

  // Receive the receiver's public value B = A^c * g^b
  data = this->network_driver->read();
  std::tie(data, res) = this->crypto_driver->decrypt_and_verify(
      this->AES_key, this->HMAC_key, data);
  if (!res) {
    this->network_driver->disconnect();
    this->print_and_throw("Failed to verify receiver message.");
  }
  // Deserialize public key
  ReceiverToSender_OTPublicValue_Message recv_pk_msg;
  recv_pk_msg.deserialize(data);

  // Generate shared DH keys for both choices
  auto k0_shared = this->crypto_driver->DH_generate_shared_key(
      DH_obj, sk, recv_pk_msg.public_value);

  CryptoPP::ModularArithmetic mod_p(DL_P);
  auto k1_pk = integer_to_byteblock(
      mod_p.Divide(byteblock_to_integer(recv_pk_msg.public_value),
                   byteblock_to_integer(pk)));
  auto k1_shared =
      this->crypto_driver->DH_generate_shared_key(DH_obj, sk, k1_pk);

  // Generate k0, k1
  auto k0 = this->crypto_driver->AES_generate_key(k0_shared);
  auto k1 = this->crypto_driver->AES_generate_key(k1_shared);

  // Encrypt m0, m1
  auto [e0, iv0] = this->crypto_driver->AES_encrypt(k0, m0);
  auto [e1, iv1] = this->crypto_driver->AES_encrypt(k1, m1);

  // Send encrypted values
  SenderToReceiver_OTEncryptedValues_Message enc_msg;
  enc_msg.e0 = e0;
  enc_msg.e1 = e1;
  enc_msg.iv0 = iv0;
  enc_msg.iv1 = iv1;
  data = this->crypto_driver->encrypt_and_tag(this->AES_key, this->HMAC_key,
                                              &enc_msg);
  this->network_driver->send(data);
}

/*
 * Receive m_c using OT. This function should:
 * 1) Read the sender's public value
 * 2) Respond with our public value that depends on our choice bit
 * 3) Generate the appropriate key and decrypt the appropriate ciphertext
 * You may find `byteblock_to_integer` and `integer_to_byteblock` useful
 * Disconnect and throw errors only for invalid MACs
 */
std::string OTDriver::OT_recv(int choice_bit) {
  // TODO: implement me!

  // Store results for Enc/Dec
  std::vector<unsigned char> data;
  bool res;

  // Read the sender's public value
  data = this->network_driver->read();
  std::tie(data, res) = this->crypto_driver->decrypt_and_verify(
      this->AES_key, this->HMAC_key, data);
  if (!res) {
    this->network_driver->disconnect();
    this->print_and_throw("Failed to verify sender message.");
  }
  // Deserialize public key
  SenderToReceiver_OTPublicValue_Message pk_msg;
  pk_msg.deserialize(data);

  // Generate DH keypair, then create appropriate public key: B = A^c * g^b
  auto [DH_obj, sk, pk] = this->crypto_driver->DH_initialize();
  CryptoPP::ModularArithmetic mod_p(DL_P);
  auto pk_c = pk;
  if (choice_bit) {
    pk_c = integer_to_byteblock(mod_p.Multiply(
        byteblock_to_integer(pk_msg.public_value), byteblock_to_integer(pk)));
  }

  // Send back to sender
  ReceiverToSender_OTPublicValue_Message recv_pk_msg;
  recv_pk_msg.public_value = pk_c;
  data = this->crypto_driver->encrypt_and_tag(this->AES_key, this->HMAC_key,
                                              &recv_pk_msg);
  this->network_driver->send(data);

  // Generate shared key k_c
  auto k_shared = this->crypto_driver->DH_generate_shared_key(
      DH_obj, sk, pk_msg.public_value);
  auto k_c = this->crypto_driver->AES_generate_key(k_shared);

  // Receive encrypted values from sender
  data = this->network_driver->read();
  std::tie(data, res) = this->crypto_driver->decrypt_and_verify(
      this->AES_key, this->HMAC_key, data);
  if (!res) {
    this->network_driver->disconnect();
    this->print_and_throw("Failed to verify sender message.");
  }
  SenderToReceiver_OTEncryptedValues_Message enc_msg;
  enc_msg.deserialize(data);

  std::string dec_msg =
      choice_bit
          ? this->crypto_driver->AES_decrypt(k_c, enc_msg.iv1, enc_msg.e1)
          : this->crypto_driver->AES_decrypt(k_c, enc_msg.iv0, enc_msg.e0);

  return dec_msg;
}