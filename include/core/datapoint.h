#ifndef DATAPOINT_H
#define DATAPOINT_H

#include <cstdint>

/**
 * @brief структура для хранения одной точки данных
 * timestamp uint32_t 4 байта - время в мс
 * value float 4 байта - значение
 */
struct DataPoint {
  uint32_t timestamp; // время в мс
  float value;        // значение

  DataPoint() : timestamp(0), value(0.0f) {}

  DataPoint(uint32_t ts, float val) : timestamp(ts), value(val) {}

  bool operator==(const DataPoint &other) const {
    return timestamp == other.timestamp && value == other.value;
  }
};

#endif // DATAPOINT_H
