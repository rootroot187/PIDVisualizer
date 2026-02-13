#include "../../include/ui/mainwindow.h"
#include "../../include/ui/cyclictargetcontroller.h"
#include "../../include/ui/graphmanager.h"
#include "../../include/ui/statusbarmanager.h"
#include "ui_mainwindow.h"

#include "../../include/filters/exponentialfilter.h"
#include "../../include/filters/kalmanfilter.h"
#include "../../include/filters/medianfilter.h"
#include "../../include/filters/movingaveragefilter.h"

#include <QApplication>
#include <QDebug>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QTabWidget>
#include <QVBoxLayout>
#include <cstdlib>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_movingAvgBuffer(nullptr), m_medianBuffer(nullptr),
      m_exponentialBuffer(nullptr), m_kalmanBuffer(nullptr),
      m_updateTimer(nullptr), m_isRunning(false),
      m_maxSamples(Constants::DEFAULT_DISPLAY_SAMPLES), ui(new Ui::MainWindow) {
  if (!ui) {
    qFatal("Failed to create UI object");
    return;
  }

  ui->setupUi(this);
  setWindowTitle("PID Visualizer");
  setupComponents();
}

MainWindow::~MainWindow() {
  if (m_dataProcessor && m_dataProcessor->isRunning()) {
    m_dataProcessor->stop();
  }

  if (m_networkController) {
    m_networkController->stopReceiver();
  }

  if (m_updateTimer) {
    m_updateTimer->stop();
  }

  delete ui;
}

void MainWindow::setupComponents() {
  // создаем буферы для данных
  m_rawDataDisplayBuffer = std::make_unique<ThreadSafeRingBuffer<DataPoint>>(
      Constants::DEFAULT_BUFFER_SIZE);
  m_rawBufferMovingAvg = std::make_unique<ThreadSafeRingBuffer<DataPoint>>(
      Constants::DEFAULT_BUFFER_SIZE);
  m_rawBufferMedian = std::make_unique<ThreadSafeRingBuffer<DataPoint>>(
      Constants::DEFAULT_BUFFER_SIZE);
  m_rawBufferExponential = std::make_unique<ThreadSafeRingBuffer<DataPoint>>(
      Constants::DEFAULT_BUFFER_SIZE);
  m_rawBufferKalman = std::make_unique<ThreadSafeRingBuffer<DataPoint>>(
      Constants::DEFAULT_BUFFER_SIZE);

  // рассылаем каждую точку во все буферы
  auto fanOutToBuffers = [this](const DataPoint &point) {
    if (m_rawDataDisplayBuffer) {
      m_rawDataDisplayBuffer->push(point);
    }
    if (m_rawBufferMovingAvg) {
      m_rawBufferMovingAvg->push(point);
    }
    if (m_rawBufferMedian) {
      m_rawBufferMedian->push(point);
    }
    if (m_rawBufferExponential) {
      m_rawBufferExponential->push(point);
    }
    if (m_rawBufferKalman) {
      m_rawBufferKalman->push(point);
    }
  };

  // создаем NetworkController
  m_networkController = std::make_unique<NetworkController>();
  m_networkController->initialize(m_rawDataDisplayBuffer.get(),
                                  fanOutToBuffers);

  // создаем GraphManager
  m_graphManager = std::make_unique<GraphManager>(this);

  setupFilters();

  // создаем CyclicTargetController
  m_cyclicTargetController = std::make_unique<CyclicTargetController>(this);
  m_cyclicTargetController->initialize(
      m_networkController.get(), ui->lineEdit, ui->doubleSpinBoxCyclicMin,
      ui->doubleSpinBoxCyclicMax, ui->doubleSpinBoxCyclicStep,
      ui->spinBoxCyclicPeriod, ui->comboBoxCyclicSignalType,
      ui->checkBoxCyclicTargetEnable, ui->lineEdit_3, ui->spinBox_3);

  // настраиваем графики через GraphManager
  if (ui->horizontalLayout_3) {
    // используем MainWindow как контейнер для графиков
    m_graphManager->setupGraphs(this);

    // добавляем tabWidget в layout
    QWidget *tabWidget = m_graphManager->getTabWidget();
    if (tabWidget) {
      ui->horizontalLayout_3->addWidget(tabWidget);
      tabWidget->show(); // явно показываем виджет
    } else {
      qWarning("Failed to get tabWidget from GraphManager");
    }

    // устанавливаем начальную видимость серий по чекбоксам
    if (m_graphManager) {
      m_graphManager->setSeriesVisible(
          "MovingAverage", ui->checkBoxMovingAverageEnable->isChecked());
      m_graphManager->setSeriesVisible("Median",
                                       ui->checkBoxMedianEnable->isChecked());
      m_graphManager->setSeriesVisible(
          "Exponential", ui->checkBoxExponentialEnable->isChecked());
      if (ui->checkBoxKalmanEnable) {
        m_graphManager->setSeriesVisible("Kalman",
                                         ui->checkBoxKalmanEnable->isChecked());
      }
    }
  }

  // создаем StatusBarManager
  m_statusBarManager = std::make_unique<StatusBarManager>(this);
  m_statusBarManager->initialize(ui->statusBar, m_networkController.get(),
                                 m_dataProcessor.get());
  m_statusBarManager->setFilterStatsLabels(ui->labelMovingAverageStats,
                                           ui->labelMedianStats,
                                           ui->labelExponentialStats);
  m_statusBarManager->setFilterMemoryLabels(
      ui->labelMovingAverageMemory, ui->labelMedianMemory,
      ui->labelExponentialMemory, ui->labelKalmanMemory);
  m_statusBarManager->setFilters(m_movingAvgFilter.get(), m_medianFilter.get(),
                                 m_exponentialFilter.get(),
                                 m_kalmanFilter.get());
  m_statusBarManager->setFilterBuffers(
      m_rawBufferMovingAvg.get(), m_movingAvgBuffer, m_rawBufferMedian.get(),
      m_medianBuffer, m_rawBufferExponential.get(), m_exponentialBuffer,
      m_rawBufferKalman.get(), m_kalmanBuffer);

  setupTimer();
  connectSignals();

  // устанавливаем начальные значения в ui
  ui->lineEdit_2->setText(
      QString::fromStdString(Constants::Network::DEFAULT_SEND_IP));
  ui->spinBox_2->setValue(Constants::Network::DEFAULT_SEND_PORT);

  ui->lineEdit_3->setText(
      QString::fromStdString(Constants::Network::DEFAULT_RECEIVE_IP));
  ui->spinBox_3->setValue(Constants::Network::DEFAULT_RECEIVE_PORT);

  ui->spinBox->setValue(static_cast<int>(m_maxSamples));

  ui->doubleSpinBox->setValue(Constants::Filters::DEFAULT_EXPONENTIAL_ALPHA);

  if (m_statusBarManager) {
    m_statusBarManager->updateStatus(m_isRunning);
  }
}

void MainWindow::setupFilters() {
  // создаем data processor
  m_dataProcessor = std::make_unique<DataProcessor>();

  // создаем фильтры
  m_movingAvgFilter = std::make_unique<MovingAverageFilter>(
      Constants::Filters::DEFAULT_MOVING_AVERAGE_WINDOW);
  m_medianFilter =
      std::make_unique<MedianFilter>(Constants::Filters::DEFAULT_MEDIAN_WINDOW);
  m_exponentialFilter = std::make_unique<ExponentialFilter>(
      Constants::Filters::DEFAULT_EXPONENTIAL_ALPHA);
  m_kalmanFilter =
      std::make_unique<KalmanFilter>(Constants::Filters::DEFAULT_KALMAN_Q,
                                     Constants::Filters::DEFAULT_KALMAN_R,
                                     Constants::Filters::DEFAULT_KALMAN_P);

  // добавляем фильтры в data processor
  m_movingAvgBuffer = m_dataProcessor->addFilter(
      m_movingAvgFilter.get(), m_rawBufferMovingAvg.get(), "MovingAverage");
  m_medianBuffer = m_dataProcessor->addFilter(
      m_medianFilter.get(), m_rawBufferMedian.get(), "Median");
  m_exponentialBuffer = m_dataProcessor->addFilter(
      m_exponentialFilter.get(), m_rawBufferExponential.get(), "Exponential");
  m_kalmanBuffer = m_dataProcessor->addFilter(
      m_kalmanFilter.get(), m_rawBufferKalman.get(), "Kalman");
}

void MainWindow::setupTimer() {
  // создаем таймер для обновления графика
  m_updateTimer = new QTimer(this);
  connect(m_updateTimer, &QTimer::timeout, this, &MainWindow::updateGraph);
  m_updateTimer->setInterval(100);
}

void MainWindow::connectSignals() {
  connect(ui->pushButton, &QPushButton::clicked, this,
          &MainWindow::onSendButtonClicked);

  connect(ui->pushButton_3, &QPushButton::clicked, this,
          &MainWindow::onStartStopButtonClicked);

  connect(ui->spinBox, QOverload<int>::of(&QSpinBox::valueChanged),
          [this](int value) { m_maxSamples = static_cast<size_t>(value); });

  // включение/выключение фильтров
  connect(ui->checkBoxMovingAverageEnable, &QCheckBox::toggled,
          [this](bool checked) {
            if (m_graphManager)
              m_graphManager->setSeriesVisible("MovingAverage", checked);
            if (checked && m_isRunning && m_dataProcessor)
              m_dataProcessor->startFilter("MovingAverage");
            else if (!checked && m_dataProcessor)
              m_dataProcessor->stopFilter("MovingAverage");
          });
  connect(ui->checkBoxMedianEnable, &QCheckBox::toggled, [this](bool checked) {
    if (m_graphManager)
      m_graphManager->setSeriesVisible("Median", checked);
    if (checked && m_isRunning && m_dataProcessor)
      m_dataProcessor->startFilter("Median");
    else if (!checked && m_dataProcessor)
      m_dataProcessor->stopFilter("Median");
  });
  connect(ui->checkBoxExponentialEnable, &QCheckBox::toggled,
          [this](bool checked) {
            if (m_graphManager)
              m_graphManager->setSeriesVisible("Exponential", checked);
            if (checked && m_isRunning && m_dataProcessor)
              m_dataProcessor->startFilter("Exponential");
            else if (!checked && m_dataProcessor)
              m_dataProcessor->stopFilter("Exponential");
          });
  if (ui->checkBoxKalmanEnable) {
    connect(ui->checkBoxKalmanEnable, &QCheckBox::toggled,
            [this](bool checked) {
              if (m_graphManager)
                m_graphManager->setSeriesVisible("Kalman", checked);
              if (checked && m_isRunning && m_dataProcessor)
                m_dataProcessor->startFilter("Kalman");
              else if (!checked && m_dataProcessor)
                m_dataProcessor->stopFilter("Kalman");
            });
  }

  // параметры фильтров: при изменении — setter + reset
  connect(ui->spinBoxMovingAverageWindow,
          QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
            if (m_movingAvgFilter) {
              m_movingAvgFilter->setWindowSize(static_cast<size_t>(value));
              m_movingAvgFilter->reset();
            }
          });
  connect(ui->spinBoxMedianWindow, QOverload<int>::of(&QSpinBox::valueChanged),
          [this](int value) {
            if (m_medianFilter) {
              m_medianFilter->setWindowSize(static_cast<size_t>(value));
              m_medianFilter->reset();
            }
          });
  connect(ui->doubleSpinBox,
          QOverload<double>::of(&QDoubleSpinBox::valueChanged),
          [this](double value) {
            if (m_exponentialFilter) {
              m_exponentialFilter->setAlpha(value);
              m_exponentialFilter->reset();
            }
          });

  // параметры Kalman фильтра: Q, R, P
  if (ui->doubleSpinBoxKalmanQ) {
    connect(ui->doubleSpinBoxKalmanQ,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            [this](double value) {
              if (m_kalmanFilter) {
                m_kalmanFilter->setQ(value);
                m_kalmanFilter->reset();
              }
            });
  }
  if (ui->doubleSpinBoxKalmanR) {
    connect(ui->doubleSpinBoxKalmanR,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            [this](double value) {
              if (m_kalmanFilter) {
                m_kalmanFilter->setR(value);
                m_kalmanFilter->reset();
              }
            });
  }
  if (ui->doubleSpinBoxKalmanP) {
    connect(ui->doubleSpinBoxKalmanP,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            [this](double value) {
              if (m_kalmanFilter) {
                m_kalmanFilter->setP(value);
                m_kalmanFilter->reset();
              }
            });
  }
}

void MainWindow::onSendButtonClicked() {
  if (!m_networkController) {
    return;
  }

  bool ok;
  float targetValue = ui->lineEdit->text().toFloat(&ok);

  if (!ok) {
    QMessageBox::warning(this, "Ошибка",
                         "Введите корректное число для целевого значения");
    return;
  }

  QString ip = ui->lineEdit_3->text().isEmpty()
                   ? QString::fromStdString(Constants::Network::DEFAULT_SEND_IP)
                   : ui->lineEdit_3->text();

  uint16_t port = static_cast<uint16_t>(ui->spinBox_3->value());

  if (m_networkController->sendCommand(targetValue, ip.toStdString(), port)) {
    if (m_statusBarManager) {
      m_statusBarManager->updateStatus(m_isRunning);
    }
  } else {
    QMessageBox::warning(this, "Ошибка", "Не удалось отправить команду");
  }
}

void MainWindow::onStartStopButtonClicked() {
  if (!m_networkController) {
    return;
  }

  if (!m_isRunning) {
    QString ip =
        ui->lineEdit_2->text().isEmpty()
            ? QString::fromStdString(Constants::Network::DEFAULT_RECEIVE_IP)
            : ui->lineEdit_2->text();

    uint16_t port = static_cast<uint16_t>(ui->spinBox_2->value());

    if (m_networkController->startReceiver(ip.toStdString(), port)) {
      if (m_dataProcessor) {
        if (ui->checkBoxMovingAverageEnable->isChecked())
          m_dataProcessor->startFilter("MovingAverage");
        if (ui->checkBoxMedianEnable->isChecked())
          m_dataProcessor->startFilter("Median");
        if (ui->checkBoxExponentialEnable->isChecked())
          m_dataProcessor->startFilter("Exponential");
        if (ui->checkBoxKalmanEnable && ui->checkBoxKalmanEnable->isChecked())
          m_dataProcessor->startFilter("Kalman");
      }

      m_isRunning = true;

      if (m_updateTimer && !m_updateTimer->isActive()) {
        m_updateTimer->start();
      }

      ui->pushButton_3->setText("Стоп");
      if (m_statusBarManager) {
        m_statusBarManager->updateStatus(m_isRunning);
      }
    } else {
      QMessageBox::warning(this, "Ошибка", "Не удалось запустить прием данных");
    }
  } else {
    m_isRunning = false;
    ui->pushButton_3->setText("Старт");

    if (m_updateTimer && m_updateTimer->isActive()) {
      m_updateTimer->stop();
    }

    QTimer::singleShot(0, this, [this]() {
      if (m_dataProcessor) {
        m_dataProcessor->stop();
      }
      if (m_networkController) {
        m_networkController->stopReceiver();
      }
    });

    if (m_statusBarManager) {
      m_statusBarManager->updateStatus(m_isRunning);
    }
  }
}

void MainWindow::updateGraph() {
  if (!m_graphManager || !m_isRunning) {
    return;
  }

  if (!m_rawDataDisplayBuffer || !m_movingAvgBuffer || !m_medianBuffer ||
      !m_exponentialBuffer || !m_kalmanBuffer) {
    return;
  }

  // подготавливаем структуры для работы с графиками и буферами
  std::vector<GraphSeries> graphSeries = {
      {m_graphManager->getRawDataGraph(), m_rawDataDisplayBuffer.get(), "Raw"},
      {m_graphManager->getMovingAvgGraph(), m_movingAvgBuffer, "MovingAverage"},
      {m_graphManager->getMedianGraph(), m_medianBuffer, "Median"},
      {m_graphManager->getExponentialGraph(), m_exponentialBuffer,
       "Exponential"},
      {m_graphManager->getKalmanGraph(), m_kalmanBuffer, "Kalman"}};

  // обновляем график через GraphManager
  m_graphManager->updateGraph(graphSeries, m_maxSamples);
  if (m_statusBarManager) {
    m_statusBarManager->updateStatus(m_isRunning);
  }

  // обновляем спектр реже, чем график сигнала
  static int spectrumUpdateCounter = 0;
  if (++spectrumUpdateCounter >=
      Constants::Performance::SPECTRUM_UPDATE_INTERVAL) {
    spectrumUpdateCounter = 0;
    m_graphManager->updateSpectrum(graphSeries, m_isRunning);
  }
}
