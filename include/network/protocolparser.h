#ifndef PROTOCOLPARSER_H
#define PROTOCOLPARSER_H

#include "../core/datapoint.h"
#include <array>
#include <cstdint>
#include <cstring>
#include <vector>

/**
 * @brief парсер бинарных UDP пакетов
 */
class ProtocolParser {
public:
  /**
   * @brief распарсить пакет с данными от модели
   *
   * @param data указатель на сырые байты
   * @param size размер данных в байтах
   * @return DataPoint с распарсенными данными
   * @throws std::runtime_error если размер данных некорректный
   */
  static DataPoint parseDataPacket(const uint8_t *data, size_t size);

  /**
   * @brief распарсить команду (целевое значение)
   *
   * @param data указатель на сырые байты
   * @param size размер данных в байтах
   * @return целевое значение (float)
   * @throws std::runtime_error если размер данных некорректный
   */
  static float parseCommandPacket(const uint8_t *data, size_t size);

  /**
   * @brief создать пакет команды
   *
   * @param targetValue целевое значение (float)
   * @return вектор байт (4 байта)
   */
  static std::vector<uint8_t> createCommandPacket(float targetValue);

  /**
   * @brief проверить корректность размера пакета данных
   *
   * @param size размер в байтах
   * @return true если размер корректен (8 байт)
   */
  static bool isValidDataPacketSize(size_t size);

  /**
   * @brief проверить корректность размера пакета команды
   *
   * @param size размер в байтах
   * @return true если размер корректен (4 байта)
   */
  static bool isValidCommandPacketSize(size_t size);

private:
  /**
   * @brief конвертировать байты в uint32_t (little-endian)
   *
   * @param bytes указатель на 4 байта
   * @return uint32_t значение
   */
  static uint32_t bytesToUint32(const uint8_t *bytes);

  /**
   * @brief конвертировать байты в float (little-endian)
   *
   * @param bytes указатель на 4 байта
   * @return float значение
   */
  static float bytesToFloat(const uint8_t *bytes);

  /**
   * @brief конвертировать float в байты (little-endian)
   *
   * @param value float значение
   * @return массив из 4 байт
   */
  static std::array<uint8_t, 4> floatToBytes(float value);
};

#endif // PROTOCOLPARSER_H
