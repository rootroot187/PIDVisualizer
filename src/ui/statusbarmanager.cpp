#include "../../include/ui/statusbarmanager.h"
#include "../../include/core/datapoint.h"
#include "../../include/core/processmemory.h"
#include "../../include/core/threadsaferingbuffer.h"
#include "../../include/filters/exponentialfilter.h"
#include "../../include/filters/kalmanfilter.h"
#include "../../include/filters/medianfilter.h"
#include "../../include/filters/movingaveragefilter.h"
#include "../../include/processing/dataprocessor.h"
#include "../../include/ui/networkcontroller.h"

#include <QLabel>
#include <QStatusBar>

StatusBarManager::StatusBarManager(QObject *parent)
    : QObject(parent), m_statusBar(nullptr), m_networkController(nullptr),
      m_dataProcessor(nullptr), m_movingAvgStatsLabel(nullptr),
      m_medianStatsLabel(nullptr), m_exponentialStatsLabel(nullptr),
      m_movingAvgMemoryLabel(nullptr), m_medianMemoryLabel(nullptr),
      m_exponentialMemoryLabel(nullptr), m_kalmanMemoryLabel(nullptr),
      m_movingAvgFilter(nullptr), m_medianFilter(nullptr),
      m_exponentialFilter(nullptr), m_kalmanFilter(nullptr),
      m_rawBufferMovingAvg(nullptr), m_movingAvgBuffer(nullptr),
      m_rawBufferMedian(nullptr), m_medianBuffer(nullptr),
      m_rawBufferExponential(nullptr), m_exponentialBuffer(nullptr),
      m_rawBufferKalman(nullptr), m_kalmanBuffer(nullptr) {}

StatusBarManager::~StatusBarManager() {}

void StatusBarManager::initialize(QStatusBar *statusBar,
                                  NetworkController *networkController,
                                  DataProcessor *dataProcessor) {
  m_statusBar = statusBar;
  m_networkController = networkController;
  m_dataProcessor = dataProcessor;
}

void StatusBarManager::setFilterStatsLabels(QLabel *movingAvgStats,
                                            QLabel *medianStats,
                                            QLabel *exponentialStats) {
  m_movingAvgStatsLabel = movingAvgStats;
  m_medianStatsLabel = medianStats;
  m_exponentialStatsLabel = exponentialStats;
}

void StatusBarManager::setFilterMemoryLabels(QLabel *movingAvgMemory,
                                             QLabel *medianMemory,
                                             QLabel *exponentialMemory,
                                             QLabel *kalmanMemory) {
  m_movingAvgMemoryLabel = movingAvgMemory;
  m_medianMemoryLabel = medianMemory;
  m_exponentialMemoryLabel = exponentialMemory;
  m_kalmanMemoryLabel = kalmanMemory;
}

void StatusBarManager::setFilters(MovingAverageFilter *movingAvgFilter,
                                  MedianFilter *medianFilter,
                                  ExponentialFilter *exponentialFilter,
                                  KalmanFilter *kalmanFilter) {
  m_movingAvgFilter = movingAvgFilter;
  m_medianFilter = medianFilter;
  m_exponentialFilter = exponentialFilter;
  m_kalmanFilter = kalmanFilter;
}

void StatusBarManager::setFilterBuffers(
    ThreadSafeRingBuffer<DataPoint> *rawBufferMovingAvg,
    ThreadSafeRingBuffer<DataPoint> *movingAvgBuffer,
    ThreadSafeRingBuffer<DataPoint> *rawBufferMedian,
    ThreadSafeRingBuffer<DataPoint> *medianBuffer,
    ThreadSafeRingBuffer<DataPoint> *rawBufferExponential,
    ThreadSafeRingBuffer<DataPoint> *exponentialBuffer,
    ThreadSafeRingBuffer<DataPoint> *rawBufferKalman,
    ThreadSafeRingBuffer<DataPoint> *kalmanBuffer) {
  m_rawBufferMovingAvg = rawBufferMovingAvg;
  m_movingAvgBuffer = movingAvgBuffer;
  m_rawBufferMedian = rawBufferMedian;
  m_medianBuffer = medianBuffer;
  m_rawBufferExponential = rawBufferExponential;
  m_exponentialBuffer = exponentialBuffer;
  m_rawBufferKalman = rawBufferKalman;
  m_kalmanBuffer = kalmanBuffer;
}

void StatusBarManager::updateStatus(bool isRunning) {
  updateStatusBar(isRunning);
  updateFilterStats();
  updateFilterMemory();
}

QString StatusBarManager::formatKb(size_t bytes) const {
  double kb = static_cast<double>(bytes) / 1024.0;
  return QString("%1 КБ").arg(kb, 0, 'f', 1);
}

void StatusBarManager::updateStatusBar(bool isRunning) {
  if (!m_statusBar) {
    return;
  }

  QString status;

  if (isRunning) {
    status = "Работает";
    if (m_networkController) {
      status += QString(" | Пакетов получено: %1")
                    .arg(m_networkController->getPacketsReceived());
    }
  } else {
    status = "Остановлено";
  }

  if (m_networkController) {
    status +=
        QString(" | Отправлено: %1").arg(m_networkController->getPacketsSent());
  }

  // память процесса из монитора ОС (фактическая ОЗУ: буферы, расчёты, Qt и
  // т.д.)
  double rssMb = ProcessMemory::getCurrentProcessRSSMegabytes();
  if (rssMb >= 0.0) {
    status += QString(" | Память процесса: %1 МБ").arg(rssMb, 0, 'f', 2);
  }

  m_statusBar->showMessage(status);
}

void StatusBarManager::updateFilterStats() {
  if (!m_dataProcessor) {
    return;
  }

  // статистика по фильтрам
  if (m_movingAvgStatsLabel) {
    m_movingAvgStatsLabel->setText(
        QString("Обработано: %1")
            .arg(m_dataProcessor->getFilterProcessedCount("MovingAverage")));
  }

  if (m_medianStatsLabel) {
    m_medianStatsLabel->setText(
        QString("Обработано: %1")
            .arg(m_dataProcessor->getFilterProcessedCount("Median")));
  }

  if (m_exponentialStatsLabel) {
    m_exponentialStatsLabel->setText(
        QString("Обработано: %1")
            .arg(m_dataProcessor->getFilterProcessedCount("Exponential")));
  }
}

void StatusBarManager::updateFilterMemory() {
  // оценка потребления памяти фильтрами + их буферами

  // moving average
  if (m_movingAvgMemoryLabel && m_movingAvgFilter && m_rawBufferMovingAvg &&
      m_movingAvgBuffer) {
    size_t bytes = m_movingAvgFilter->getMemoryUsage();
    bytes += m_rawBufferMovingAvg->capacity() * sizeof(DataPoint);
    bytes += m_movingAvgBuffer->capacity() * sizeof(DataPoint);
    m_movingAvgMemoryLabel->setText(QString("Память: %1").arg(formatKb(bytes)));
  }

  // median
  if (m_medianMemoryLabel && m_medianFilter && m_rawBufferMedian &&
      m_medianBuffer) {
    size_t bytes = m_medianFilter->getMemoryUsage();
    bytes += m_rawBufferMedian->capacity() * sizeof(DataPoint);
    bytes += m_medianBuffer->capacity() * sizeof(DataPoint);
    m_medianMemoryLabel->setText(QString("Память: %1").arg(formatKb(bytes)));
  }

  // exponential
  if (m_exponentialMemoryLabel && m_exponentialFilter &&
      m_rawBufferExponential && m_exponentialBuffer) {
    size_t bytes = m_exponentialFilter->getMemoryUsage();
    bytes += m_rawBufferExponential->capacity() * sizeof(DataPoint);
    bytes += m_exponentialBuffer->capacity() * sizeof(DataPoint);
    m_exponentialMemoryLabel->setText(
        QString("Память: %1").arg(formatKb(bytes)));
  }

  // kalman
  if (m_kalmanMemoryLabel && m_kalmanFilter && m_rawBufferKalman &&
      m_kalmanBuffer) {
    size_t bytes = m_kalmanFilter->getMemoryUsage();
    bytes += m_rawBufferKalman->capacity() * sizeof(DataPoint);
    bytes += m_kalmanBuffer->capacity() * sizeof(DataPoint);
    m_kalmanMemoryLabel->setText(QString("Память: %1").arg(formatKb(bytes)));
  }
}
