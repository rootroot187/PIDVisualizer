#include "../../include/network/protocolparser.h"
#include <array>
#include <stdexcept>

namespace {
constexpr size_t DATA_PACKET_SIZE = 8;
constexpr size_t COMMAND_PACKET_SIZE = 4;
} // namespace

DataPoint ProtocolParser::parseDataPacket(const uint8_t *data, size_t size) {
  // проверка размера пакета
  if (!isValidDataPacketSize(size)) {
    throw std::runtime_error(
        "Invalid data packet size: expected 8 bytes, got " +
        std::to_string(size));
  }

  // парсим timestamp первые 4 байта
  // data указатель на начало пакета
  // bytesToUint32 читает 4 байта начиная с data
  uint32_t timestamp = bytesToUint32(data);

  // парсим value следующие 4 байта
  // data + 4 - сдвигаем указатель на 4 байта вперёд
  // bytesToFloat читает 4 байта начиная с data + 4
  float value = bytesToFloat(data + 4);

  // создаем и возвращаем DataPoint
  return DataPoint(timestamp, value);
}

float ProtocolParser::parseCommandPacket(const uint8_t *data, size_t size) {
  if (!isValidCommandPacketSize(size)) {
    throw std::runtime_error(
        "Invalid command packet size: expected 4 bytes, got " +
        std::to_string(size));
  }

  // парсим float значение
  return bytesToFloat(data);
}

std::vector<uint8_t> ProtocolParser::createCommandPacket(float targetValue) {
  auto bytes = floatToBytes(targetValue);
  return std::vector<uint8_t>(bytes.begin(), bytes.end());
}

bool ProtocolParser::isValidDataPacketSize(size_t size) {
  return size == DATA_PACKET_SIZE;
}

bool ProtocolParser::isValidCommandPacketSize(size_t size) {
  return size == COMMAND_PACKET_SIZE;
}

uint32_t ProtocolParser::bytesToUint32(const uint8_t *bytes) {
  // Little-endian: младший байт первым
  uint32_t result;
  std::memcpy(&result, bytes, sizeof(uint32_t));
  return result;
}

float ProtocolParser::bytesToFloat(const uint8_t *bytes) {
  float result;
  std::memcpy(&result, bytes, sizeof(float));

  return result;
}

std::array<uint8_t, 4> ProtocolParser::floatToBytes(float value) {
  std::array<uint8_t, 4> bytes;
  std::memcpy(bytes.data(), &value, sizeof(float));

  return bytes;
}
