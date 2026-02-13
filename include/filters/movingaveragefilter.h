#ifndef MOVINGAVERAGEFILTER_H
#define MOVINGAVERAGEFILTER_H

#include "../core/Constants.h"
#include "filterbase.h"
#include <cstddef>
#include <deque> // Для хранения окна значений

/**
 * @brief фильтр скользящего среднего
 * @details фильтр скользящего среднего
 */
class MovingAverageFilter final
    : public FilterBase { // девиртуализирую класс -> оптимизация
public:
  /**
   * @brief конструктор
   *
   * @param windowSize размер окна
   */
  explicit MovingAverageFilter(
      size_t windowSize = Constants::Filters::DEFAULT_MOVING_AVERAGE_WINDOW);

  /**
   * @brief деструктор
   */
  virtual ~MovingAverageFilter() = default;

  DataPoint filter(const DataPoint &input) override;

  /**
   * @brief сбросить состояние фильтра
   */
  void reset() override;
  bool isReady() const override;

  /**
   * @brief установить размер окна
   *
   * изменяет размер окна. старые данные очищаются.
   *
   * @param windowSize новый размер окна
   */
  void setWindowSize(size_t windowSize);

  /**
   * @brief получить размер окна
   *
   * @return текущий размер окна
   */
  size_t getWindowSize() const;

  /**
   * @brief Размер памяти фильтра (окно + служебные)
   */
  size_t getMemoryUsage() const override;

private:
  /**
   * @brief вычислить среднее арифметическое значений в окне
   *
   * @return среднее значение
   */
  float calculateAverage() const;

  size_t m_windowSize; // размер окна
  std::deque<float>
      m_window; // окно значений, deque потому что быстрее чем vector
};

#endif // MOVINGAVERAGEFILTER_H
