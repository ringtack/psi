#pragma once

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <boost/chrono.hpp>
#include <boost/thread.hpp>
#include <crypto++/cryptlib.h>
#include <crypto++/dsa.h>
#include <crypto++/filters.h>
#include <crypto++/hex.h>
#include <crypto++/integer.h>
#include <crypto++/nbtheory.h>

// ================================================
// MESSAGE TYPES
// ================================================

namespace MessageType {
enum T {
  HMACTagged_Wrapper = 1,
  DHPublicValue_Message = 2,
  SenderToReceiver_PSI_Message = 3,
  ReceiverToSender_PSI_Message = 4,
};
};

MessageType::T get_message_type(std::vector<unsigned char> &data);

// ================================================
// SERIALIZABLE
// ================================================

struct Serializable {
  virtual void serialize(std::vector<unsigned char> &data) = 0;
  virtual int deserialize(std::vector<unsigned char> &data) = 0;
};

// serializers.
int put_bool(bool b, std::vector<unsigned char> &data);
int put_string(std::string s, std::vector<unsigned char> &data);
int put_integer(CryptoPP::Integer i, std::vector<unsigned char> &data);

// deserializers
int get_bool(bool *b, std::vector<unsigned char> &data, int idx);
int get_string(std::string *s, std::vector<unsigned char> &data, int idx);
int get_integer(CryptoPP::Integer *i, std::vector<unsigned char> &data,
                int idx);

// ================================================
// WRAPPERS
// ================================================

struct HMACTagged_Wrapper : public Serializable {
  std::vector<unsigned char> payload;
  CryptoPP::SecByteBlock iv;
  std::string mac;

  void serialize(std::vector<unsigned char> &data);
  int deserialize(std::vector<unsigned char> &data);
};

// ================================================
// KEY EXCHANGE
// ================================================

struct DHPublicValue_Message : public Serializable {
  CryptoPP::SecByteBlock public_value;

  void serialize(std::vector<unsigned char> &data);
  int deserialize(std::vector<unsigned char> &data);
};

// ================================================
// PSI
// ================================================

enum class PSIType {
  PSI_Intersection = 1,
  PSI_CA = 2,
};

struct SenderToReceiver_PSI_Message : public Serializable {
  PSIType type;
  std::vector<CryptoPP::SecByteBlock> hashed_sender_inputs;

  void serialize(std::vector<unsigned char> &data);
  int deserialize(std::vector<unsigned char> &data);
};

struct ReceiverToSender_PSI_Message : public Serializable {
  std::vector<CryptoPP::SecByteBlock> hashed_sender_inputs;
  std::vector<CryptoPP::SecByteBlock> hashed_receiver_inputs;

  void serialize(std::vector<unsigned char> &data);
  int deserialize(std::vector<unsigned char> &data);
};