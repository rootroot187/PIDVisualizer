#include "../../include/ui/cyclictargetcontroller.h"
#include "../../include/core/Constants.h"
#include "../../include/ui/networkcontroller.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QSpinBox>
#include <cmath>
#include <cstdlib>

CyclicTargetController::CyclicTargetController(QObject *parent)
    : QObject(parent), m_networkController(nullptr),
      m_targetValueLineEdit(nullptr), m_minSpinBox(nullptr),
      m_maxSpinBox(nullptr), m_stepSpinBox(nullptr), m_periodSpinBox(nullptr),
      m_signalTypeComboBox(nullptr), m_enableCheckBox(nullptr),
      m_ipLineEdit(nullptr), m_portSpinBox(nullptr), m_timer(new QTimer(this)),
      m_enabled(false),
      m_currentValue(Constants::CyclicTarget::DEFAULT_MIN_VALUE),
      m_step(Constants::CyclicTarget::DEFAULT_STEP), m_directionUp(true),
      m_signalType(Constants::CyclicTarget::DEFAULT_SIGNAL_TYPE),
      m_phase(0.0f) {
  connect(m_timer, &QTimer::timeout, this, &CyclicTargetController::onTimer);
  m_timer->setInterval(Constants::CyclicTarget::DEFAULT_PERIOD_MS);
}

CyclicTargetController::~CyclicTargetController() {
  if (m_timer) {
    m_timer->stop();
  }
}

void CyclicTargetController::initialize(
    NetworkController *networkController, QLineEdit *targetValueLineEdit,
    QDoubleSpinBox *minSpinBox, QDoubleSpinBox *maxSpinBox,
    QDoubleSpinBox *stepSpinBox, QSpinBox *periodSpinBox,
    QComboBox *signalTypeComboBox, QCheckBox *enableCheckBox,
    QLineEdit *ipLineEdit, QSpinBox *portSpinBox) {
  m_networkController = networkController;
  m_targetValueLineEdit = targetValueLineEdit;
  m_minSpinBox = minSpinBox;
  m_maxSpinBox = maxSpinBox;
  m_stepSpinBox = stepSpinBox;
  m_periodSpinBox = periodSpinBox;
  m_signalTypeComboBox = signalTypeComboBox;
  m_enableCheckBox = enableCheckBox;
  m_ipLineEdit = ipLineEdit;
  m_portSpinBox = portSpinBox;

  // подключаем сигналы от ui элементов
  if (m_enableCheckBox) {
    connect(m_enableCheckBox, &QCheckBox::toggled, this,
            &CyclicTargetController::onEnableToggled);
  }

  if (m_periodSpinBox) {
    connect(m_periodSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this,
            &CyclicTargetController::onPeriodChanged);
  }

  if (m_stepSpinBox) {
    connect(m_stepSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &CyclicTargetController::onStepChanged);
  }
}

void CyclicTargetController::setEnabled(bool enabled) {
  m_enabled = enabled;
  if (m_enableCheckBox && m_enableCheckBox->isChecked() != enabled) {
    m_enableCheckBox->setChecked(enabled);
  }
  onEnableToggled(enabled);
}

bool CyclicTargetController::isEnabled() const { return m_enabled; }

void CyclicTargetController::setPeriod(int periodMs) {
  if (m_timer) {
    m_timer->setInterval(periodMs);
  }
  if (m_periodSpinBox && m_periodSpinBox->value() != periodMs) {
    m_periodSpinBox->setValue(periodMs);
  }
}

void CyclicTargetController::setStep(float step) {
  m_step = step;
  if (m_stepSpinBox && std::abs(m_stepSpinBox->value() - step) > 0.001) {
    m_stepSpinBox->setValue(step);
  }
}

void CyclicTargetController::onTimer() {
  if (!m_enabled || !m_networkController) {
    return;
  }

  float minValue, maxValue, step;
  int period;
  Constants::CyclicTarget::SignalType signalType;
  getParametersFromUI(minValue, maxValue, step, period, signalType);

  if (m_timer && m_timer->interval() != period) {
    m_timer->setInterval(period);
  }

  // проверяем корректность параметров
  if (minValue >= maxValue) {
    qWarning() << "Некорректные параметры циклического задания: min="
               << minValue << ", max=" << maxValue;
    return;
  }

  // генерируем новое значение
  float newValue = generateValue(minValue, maxValue, step, signalType);

  // ограничиваем значение диапазоном
  if (newValue < minValue)
    newValue = minValue;
  if (newValue > maxValue)
    newValue = maxValue;

  m_currentValue = newValue;

  // отправляем новое целевое значение модели
  QString ip =
      m_ipLineEdit && !m_ipLineEdit->text().isEmpty()
          ? m_ipLineEdit->text()
          : QString::fromStdString(Constants::Network::DEFAULT_SEND_IP);

  uint16_t port = m_portSpinBox ? static_cast<uint16_t>(m_portSpinBox->value())
                                : Constants::Network::DEFAULT_SEND_PORT;

  qDebug() << "Циклическое задание [" << static_cast<int>(signalType)
           << "]: отправка targetValue=" << m_currentValue << ", IP=" << ip
           << ", port=" << port;

  if (m_networkController->sendCommand(m_currentValue, ip.toStdString(),
                                       port)) {
    if (m_targetValueLineEdit) {
      m_targetValueLineEdit->setText(QString::number(m_currentValue, 'f', 2));
    }
  } else {
    qWarning() << "Ошибка отправки циклического целевого значения";
  }
}

void CyclicTargetController::onEnableToggled(bool enabled) {
  m_enabled = enabled;

  if (enabled) {
    // инициализируем начальное значение
    float minValue = m_minSpinBox ? static_cast<float>(m_minSpinBox->value())
                                  : Constants::CyclicTarget::DEFAULT_MIN_VALUE;
    m_currentValue = minValue;
    m_directionUp = true;
    m_phase = 0.0f;

    // обновляем период таймера
    if (m_periodSpinBox && m_timer) {
      m_timer->setInterval(m_periodSpinBox->value());
    }

    // запускаем таймер
    if (m_timer) {
      m_timer->start();
    }
    qDebug() << "Циклическое задание включено";
  } else {
    // останавливаем таймер
    if (m_timer) {
      m_timer->stop();
    }
    qDebug() << "Циклическое задание выключено";
  }
}

void CyclicTargetController::onPeriodChanged(int periodMs) {
  if (m_timer) {
    m_timer->setInterval(periodMs);
  }
}

void CyclicTargetController::onStepChanged(double step) {
  m_step = static_cast<float>(step);
}

void CyclicTargetController::getParametersFromUI(
    float &minValue, float &maxValue, float &step, int &period,
    Constants::CyclicTarget::SignalType &signalType) {
  minValue = m_minSpinBox ? static_cast<float>(m_minSpinBox->value())
                          : Constants::CyclicTarget::DEFAULT_MIN_VALUE;

  maxValue = m_maxSpinBox ? static_cast<float>(m_maxSpinBox->value())
                          : Constants::CyclicTarget::DEFAULT_MAX_VALUE;

  step = m_stepSpinBox ? static_cast<float>(m_stepSpinBox->value()) : m_step;

  period = m_periodSpinBox ? m_periodSpinBox->value()
                           : Constants::CyclicTarget::DEFAULT_PERIOD_MS;

  signalType = m_signalTypeComboBox
                   ? static_cast<Constants::CyclicTarget::SignalType>(
                         m_signalTypeComboBox->currentIndex())
                   : m_signalType;
}

float CyclicTargetController::generateValue(
    float minValue, float maxValue, float step,
    Constants::CyclicTarget::SignalType signalType) {
  float amplitude = (maxValue - minValue) / 2.0f; // амплитуда
  float offset = (maxValue + minValue) / 2.0f;    // смещение

  switch (signalType) {
  case Constants::CyclicTarget::SignalType::Triangle: {
    // треугольная волна (пила): линейное изменение от min до max
    if (step <= 0.0f) {
      qWarning() << "Некорректный шаг для треугольной волны: step=" << step;
      return m_currentValue;
    }

    if (m_directionUp) {
      m_currentValue += step;
      if (m_currentValue >= maxValue) {
        m_currentValue = maxValue;
        m_directionUp = false;
      }
    } else {
      m_currentValue -= step;
      if (m_currentValue <= minValue) {
        m_currentValue = minValue;
        m_directionUp = true;
      }
    }
    return m_currentValue;
  }

  case Constants::CyclicTarget::SignalType::Sine: {
    // синусоида: sin(phase) * amplitude + offset
    constexpr float TWO_PI = 2.0f * 3.14159265359f;
    // около 40 шагов на один полный период синуса
    float phaseIncrement = TWO_PI / 40.0f;

    m_phase += phaseIncrement;
    if (m_phase >= TWO_PI) {
      m_phase -= TWO_PI;
    }

    return amplitude * std::sin(m_phase) + offset;
  }

  case Constants::CyclicTarget::SignalType::Square: {
    // прямоугольная волна: мгновенные переключения min - max
    m_directionUp = !m_directionUp;

    return m_directionUp ? maxValue : minValue;
  }

  case Constants::CyclicTarget::SignalType::Random: {
    // случайные значения из диапазона [min, max]
    float range = maxValue - minValue;
    return minValue +
           (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)) *
               range;
  }

  default:
    qWarning() << "Неизвестный тип сигнала: " << static_cast<int>(signalType);
    return m_currentValue;
  }
}
