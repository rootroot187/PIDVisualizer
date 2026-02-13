#ifndef EXPONENTIALFILTER_H
#define EXPONENTIALFILTER_H

#include "../core/Constants.h"
#include "filterbase.h"
#include <cstddef>

/**
 * @brief экспоненциальный фильтр
 */
class ExponentialFilter final
    : public FilterBase // девиртуализирую класс -> оптимизация
{
public:
  /**
   * @brief конструктор
   * @param alpha коэффициент сглаживания
   */
  explicit ExponentialFilter(
      double alpha = Constants::Filters::DEFAULT_EXPONENTIAL_ALPHA);

  /**
   * @brief деструктор
   */
  virtual ~ExponentialFilter() = default;

  /**
   * @brief фильтр
   */

  DataPoint filter(const DataPoint &input) override;

  /**
   * @brief сброс
   */
  void reset() override;

  /**
   * @brief готов ли фильтр
   */
  bool isReady() const override;

  /**
   * @brief установить коэффициент сглаживания
   * @param alpha коэффициент сглаживания
   */
  void setAlpha(double alpha);

  /**
   * @brief получить коэффициент сглаживания
   */
  double getAlpha() const;

  /**
   * @brief получить размер памяти, используемой фильтром
   * @return размер памяти в байтах
   */
  size_t getMemoryUsage() const override;

private:
  /**
   * @brief коэффициент сглаживания
   */
  double m_alpha;

  /**
   * @brief предыдущее значение
   */
  float m_prevOutput;
};

#endif // EXPONENTIALFILTER_H
