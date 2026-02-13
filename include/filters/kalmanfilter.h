#ifndef KALMANFILTER_H
#define KALMANFILTER_H

#include "../core/Constants.h"
#include "filterbase.h"
#include <cstddef>
#include <cstdint>

/**
 * @brief фильтр Калмана
 */
class KalmanFilter final
    : public FilterBase // девиртуализирую класс -> оптимизация
{
public:
  /**
   * @brief конструктор
   *
   * @param q шум процесса
   * @param r шум измерения
   * @param p начальная ковариация
   */
  explicit KalmanFilter(double q = Constants::Filters::DEFAULT_KALMAN_Q,
                        double r = Constants::Filters::DEFAULT_KALMAN_R,
                        double p = Constants::Filters::DEFAULT_KALMAN_P);

  virtual ~KalmanFilter() = default;

  DataPoint filter(const DataPoint &input) override;
  void reset() override;
  bool isReady() const override;

  // сеттеры и геттеры для параметров
  void setQ(double q);
  double getQ() const;

  void setR(double r);
  double getR() const;

  void setP(double p);
  double getP() const;

  // установить все параметры сразу
  void setParameters(double q, double r, double p);

  size_t getMemoryUsage() const override;

private:
  double m_q; // шум процесса
  double m_r; // шум измерения
  double m_p; // ковариация (текущая)

  // состояние фильтра (модель с ускорением)
  float m_x; // Оценка состояния (текущее значение)
  float m_v; // оценка скорости изменения
  float m_a; // оценка ускорения

  // Предыдущие значения для оценки скорости и ускорения
  float m_prevX;            // предыдущее значение
  float m_prevV;            // предыдущая скорость
  uint32_t m_prevTimestamp; // предыдущий timestamp

  bool m_initialized; // флаг инициализации (для первого отсчета)
};

#endif // KALMANFILTER_H
