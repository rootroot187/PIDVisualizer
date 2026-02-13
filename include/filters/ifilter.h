#ifndef IFILTER_H
#define IFILTER_H

#include "../core/datapoint.h"
#include <cstddef>
#include <string>

/**
 * @brief интерфейс для всех фильтров
 * @details люто следуем принципам SOLID, используем интерфейсы и абстракции,
 * чтобы не было зависимостей между классами
 */
class IFilter {
public:
  /**
   * @brief виртуальный деструктор
   * @details должен быть виртуальным чтобы правильно вызывался деструктор
   * дочерних классов
   */
  virtual ~IFilter() = default;

  /**
   * @brief применить фильтр к данным
   * @param input входные данные
   * @return отфильтрованные данные
   */
  virtual DataPoint filter(const DataPoint &input) = 0;

  /**
   * @brief сбросить состояние фильтра
   *
   * очищает внутреннее состояние фильтра
   * после вызова reset фильтр возвращается в начальное состояние
   */
  virtual void reset() = 0;

  /**
   * @brief проверить, готов ли фильтр к работе
   *
   * @return true если фильтр готов давать корректные результаты
   */
  virtual bool isReady() const = 0;

  /**
   * @brief получить имя фильтра
   * @return строка с именем фильтра
   */
  virtual std::string getName() const = 0;

  /**
   * @brief получить размер памяти, используемой фильтром
   * @return размер памяти в байтах
   */
  virtual size_t getMemoryUsage() const = 0;
};

#endif // IFILTER_H
