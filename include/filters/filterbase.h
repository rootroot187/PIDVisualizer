#ifndef FILTERBASE_H
#define FILTERBASE_H

#include "../core/datapoint.h"
#include "ifilter.h"
#include <string>

/**
 * @brief базовый класс для всех фильтров
 */
class FilterBase : public IFilter {
public:
  /**
   * @brief конструктор
   *
   * @param name имя фильтра
   */
  explicit FilterBase(const std::string &name);

  /**
   * @brief виртуальный деструктор чтоб правильно вызывался деструктор дочерних
   * классов
   */
  virtual ~FilterBase() = default;

  /**
   * @brief получить имя фильтра
   *
   * реализация виртуальной функции из IFilter
   *
   * @return имя фильтра
   */
  std::string getName() const override;

protected:
  /**
   * @brief имя фильтра
   */
  std::string m_name;
};

#endif // FILTERBASE_H
