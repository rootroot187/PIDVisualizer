#ifndef CYCLICTARGETCONTROLLER_H
#define CYCLICTARGETCONTROLLER_H

#include "../core/Constants.h"
#include <QObject>
#include <QString>
#include <QTimer>
#include <memory>

class NetworkController;
class QLineEdit;
class QDoubleSpinBox;
class QSpinBox;
class QComboBox;
class QCheckBox;

/**
 * @brief контроллер для управления циклическим заданием целевых значений
 */
class CyclicTargetController : public QObject {
  Q_OBJECT

public:
  explicit CyclicTargetController(QObject *parent = nullptr);
  ~CyclicTargetController();

  /**
   * @brief инициализация контроллера
   * @param networkController указатель на NetworkController для отправки команд
   * @param targetValueLineEdit поле ввода для отображения текущего целевого
   * значения
   * @param minSpinBox спинбокс для минимального значения
   * @param maxSpinBox спинбокс для максимального значения
   * @param stepSpinBox спинбокс для шага (для треугольной волны)
   * @param periodSpinBox спинбокс для периода таймера
   * @param signalTypeComboBox комбобокс для выбора типа сигнала
   * @param enableCheckBox чекбокс для включения/выключения
   * @param ipLineEdit поле ввода IP адреса для отправки
   * @param portSpinBox спинбокс для порта отправки
   */
  void initialize(NetworkController *networkController,
                  QLineEdit *targetValueLineEdit, QDoubleSpinBox *minSpinBox,
                  QDoubleSpinBox *maxSpinBox, QDoubleSpinBox *stepSpinBox,
                  QSpinBox *periodSpinBox, QComboBox *signalTypeComboBox,
                  QCheckBox *enableCheckBox, QLineEdit *ipLineEdit,
                  QSpinBox *portSpinBox);

  /**
   * @brief включить/выключить циклическое задание
   */
  void setEnabled(bool enabled);

  /**
   * @brief проверить, включено ли циклическое задание
   */
  bool isEnabled() const;

  /**
   * @brief установить период таймера
   */
  void setPeriod(int periodMs);

  /**
   * @brief установить шаг для треугольной волны
   */
  void setStep(float step);

private slots:
  /**
   * @brief обработчик таймера - генерирует новое значение и отправляет его
   */
  void onTimer();

  /**
   * @brief обработчик изменения состояния чекбокса
   */
  void onEnableToggled(bool enabled);

  /**
   * @brief обработчик изменения периода
   */
  void onPeriodChanged(int periodMs);

  /**
   * @brief обработчик изменения шага
   */
  void onStepChanged(double step);

private:
  /**
   * @brief генерация нового значения в зависимости от типа сигнала
   * @param minValue минимальное значение
   * @param maxValue максимальное значение
   * @param step шаг (для треугольной волны)
   * @param signalType тип сигнала
   * @return сгенерированное значение
   */
  float generateValue(float minValue, float maxValue, float step,
                      Constants::CyclicTarget::SignalType signalType);

  /**
   * @brief получить текущие параметры из UI
   */
  void getParametersFromUI(float &minValue, float &maxValue, float &step,
                           int &period,
                           Constants::CyclicTarget::SignalType &signalType);

  NetworkController *m_networkController;

  // ui элементы
  QLineEdit *m_targetValueLineEdit;
  QDoubleSpinBox *m_minSpinBox;
  QDoubleSpinBox *m_maxSpinBox;
  QDoubleSpinBox *m_stepSpinBox;
  QSpinBox *m_periodSpinBox;
  QComboBox *m_signalTypeComboBox;
  QCheckBox *m_enableCheckBox;
  QLineEdit *m_ipLineEdit;
  QSpinBox *m_portSpinBox;

  QTimer *m_timer;

  // состояние генератора
  bool m_enabled;
  float m_currentValue;
  float m_step;
  bool m_directionUp;
  Constants::CyclicTarget::SignalType m_signalType;
  float m_phase;
};

#endif // CYCLICTARGETCONTROLLER_H
