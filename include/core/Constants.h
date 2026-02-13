#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <cstddef>
#include <cstdint>
#include <cstring>

/**
 * @brief пространство имен для всех констант проекта
 */
namespace Constants {

/**
 * @brief размеры буферов для хранения данных
 */
constexpr size_t DEFAULT_BUFFER_SIZE = 1000;
constexpr size_t MAX_BUFFER_SIZE = 10000;

/**
 * @brief количество отсчетов для отображения на графике
 */
constexpr size_t MIN_DISPLAY_SAMPLES = 50;
constexpr size_t MAX_DISPLAY_SAMPLES = 1000;
constexpr size_t DEFAULT_DISPLAY_SAMPLES = 500;

/**
 * @brief настройки сети по умолчанию
 */
namespace Network {
// IP адреса
constexpr const char *DEFAULT_RECEIVE_IP = "127.0.0.1";
constexpr const char *DEFAULT_SEND_IP = "127.0.0.1";

// порты
constexpr uint16_t DEFAULT_RECEIVE_PORT = 50005;
constexpr uint16_t DEFAULT_SEND_PORT = 50006;

// размеры буферов для UDP
constexpr size_t UDP_BUFFER_SIZE = 1024;
constexpr int SOCKET_TIMEOUT_MS = 100;
constexpr size_t DATA_PACKET_SIZE = 8;
} // namespace Network

/**
 * @brief параметры фильтров
 */
namespace Filters {
/**
 * @brief размер окна для скользящего среднего
 */
constexpr size_t DEFAULT_MOVING_AVERAGE_WINDOW = 10;
constexpr size_t MIN_MOVING_AVERAGE_WINDOW = 2;
constexpr size_t MAX_MOVING_AVERAGE_WINDOW = 100;

/**
 * @brief размер окна для медианного фильтра
 */
constexpr size_t DEFAULT_MEDIAN_WINDOW = 5;
constexpr size_t MIN_MEDIAN_WINDOW = 3;  // минимум
constexpr size_t MAX_MEDIAN_WINDOW = 51; // максимум

/**
 * @brief коэффициент сглаживания (alpha)
 */
constexpr double DEFAULT_EXPONENTIAL_ALPHA = 0.3;
constexpr double MIN_EXPONENTIAL_ALPHA = 0.0;
constexpr double MAX_EXPONENTIAL_ALPHA = 1.0;

/**
 * @brief параметры фильтра Калмана
 */
constexpr double DEFAULT_KALMAN_Q = 0.1; // шум процесса
constexpr double DEFAULT_KALMAN_R = 0.5; // шум измерения
constexpr double DEFAULT_KALMAN_P = 1.0; // начальная ковариация
} // namespace Filters

/**
 * @brief перечисление типов фильтров
 */
enum class FilterType {
  MovingAverage, // КИХ: Скользящее среднее
  Median,        // КИХ: Медианный фильтр
  Exponential,   // БИХ: Экспоненциальное сглаживание
  Kalman         // БИХ: Фильтр Калмана
};

/**
 * @brief цвета для отображения разных серий на графике
 */
namespace Colors {
constexpr const char *RAW_DATA = "#FF0000";
constexpr const char *MOVING_AVERAGE = "#00FF00";
constexpr const char *MEDIAN = "#0000FF";
constexpr const char *EXPONENTIAL = "#FF00FF";
constexpr const char *KALMAN = "#00FFFF";
} // namespace Colors

/**
 * @brief настройки для мониторинга производительности
 */
namespace Performance {
constexpr size_t MONITORING_WINDOW_SIZE = 1000;
constexpr int UPDATE_INTERVAL_MS = 100;
constexpr int SPECTRUM_UPDATE_INTERVAL = 5;
}

/**
 * @brief настройки для автоматического циклического задания целевого значения
 */
namespace CyclicTarget {
/**
 * @brief типы сигналов для циклического задания
 */
enum class SignalType {
  Triangle, // пила
  Sine,     // синусоида
  Square,   // прямоугольная волна
  Random    // случайные значения
};

constexpr float DEFAULT_MIN_VALUE = 0.0f;  // минимальное значение по умолчанию
constexpr float DEFAULT_MAX_VALUE = 10.0f; // максимальное значение по умолчанию
constexpr float DEFAULT_STEP = 1.0f;       // шаг изменения по умолчанию
constexpr int DEFAULT_PERIOD_MS = 1000;    // период отправки по умолчанию
constexpr SignalType DEFAULT_SIGNAL_TYPE =
    SignalType::Triangle;            // тип сигнала по умолчанию
constexpr float MIN_STEP = 0.1f;     // минимальный шаг
constexpr float MAX_STEP = 100.0f;   // максимальный шаг
constexpr int MIN_PERIOD_MS = 100;   // минимальный период
constexpr int MAX_PERIOD_MS = 60000; // максимальный период
} // namespace CyclicTarget
} // namespace Constants
#endif // CONSTANTS_H
