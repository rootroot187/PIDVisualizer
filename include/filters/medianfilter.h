#ifndef MEDIANFILTER_H
#define MEDIANFILTER_H

#include "../core/Constants.h"
#include "filterbase.h"
#include <cstddef>
#include <deque>

/**
 * @brief медианный фильтр
 */
class MedianFilter final
    : public FilterBase // девиртуализирую класс -> оптимизация
{
public:
  /**
   * @brief конструктор
   * @param windowSize размер окна
   */
  explicit MedianFilter(
      size_t windowSize = Constants::Filters::DEFAULT_MEDIAN_WINDOW);

  virtual ~MedianFilter() = default;

  DataPoint filter(const DataPoint &input) override;
  void reset() override;
  bool isReady() const override;

  void setWindowSize(size_t windowSize);
  size_t getWindowSize() const;

  size_t getMemoryUsage() const override;

private:
  /** @brief возвращает медиану текущего окна без изменения окна */
  float computeMedian() const;

  size_t m_windowSize;
  std::deque<float> m_window;
};

#endif // MEDIANFILTER_H
