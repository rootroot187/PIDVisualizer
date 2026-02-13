#ifndef STATUSBARMANAGER_H
#define STATUSBARMANAGER_H

#include "../core/datapoint.h"
#include "../core/threadsaferingbuffer.h"
#include <QObject>
#include <QString>
#include <memory>

class QStatusBar;
class QLabel;
class NetworkController;
class DataProcessor;
class MovingAverageFilter;
class MedianFilter;
class ExponentialFilter;
class KalmanFilter;

/**
 * @brief менеджер для управления статусной строкой и статистикой фильтров
 */
class StatusBarManager : public QObject {
  Q_OBJECT

public:
  explicit StatusBarManager(QObject *parent = nullptr);
  ~StatusBarManager();

  /**
   * @brief инициализация менеджера
   * @param statusBar статусная строка для отображения основного статуса
   * @param networkController контроллер сети для получения статистики
   * @param dataProcessor процессор данных для получения статистики фильтров
   */
  void initialize(QStatusBar *statusBar, NetworkController *networkController,
                  DataProcessor *dataProcessor);

  /**
   * @brief установить ui элементы для статистики фильтров
   */
  void setFilterStatsLabels(QLabel *movingAvgStats, QLabel *medianStats,
                            QLabel *exponentialStats);

  /**
   * @brief установить ui элементы для памяти фильтров
   */
  void setFilterMemoryLabels(QLabel *movingAvgMemory, QLabel *medianMemory,
                             QLabel *exponentialMemory, QLabel *kalmanMemory);

  /**
   * @brief установить фильтры для расчета памяти
   */
  void setFilters(MovingAverageFilter *movingAvgFilter,
                  MedianFilter *medianFilter,
                  ExponentialFilter *exponentialFilter,
                  KalmanFilter *kalmanFilter);

  /**
   * @brief установить буферы для расчета памяти фильтров
   */
  void setFilterBuffers(ThreadSafeRingBuffer<DataPoint> *rawBufferMovingAvg,
                        ThreadSafeRingBuffer<DataPoint> *movingAvgBuffer,
                        ThreadSafeRingBuffer<DataPoint> *rawBufferMedian,
                        ThreadSafeRingBuffer<DataPoint> *medianBuffer,
                        ThreadSafeRingBuffer<DataPoint> *rawBufferExponential,
                        ThreadSafeRingBuffer<DataPoint> *exponentialBuffer,
                        ThreadSafeRingBuffer<DataPoint> *rawBufferKalman,
                        ThreadSafeRingBuffer<DataPoint> *kalmanBuffer);

  /**
   * @brief обновить статус
   * @param isRunning Флаг работы приложения
   */
  void updateStatus(bool isRunning);

private:
  /**
   * @brief форматирование размера в КБ
   */
  QString formatKb(size_t bytes) const;

  /**
   * @brief обновить статусную строку
   */
  void updateStatusBar(bool isRunning);

  /**
   * @brief обновить статистику фильтров
   */
  void updateFilterStats();

  /**
   * @brief обновить информацию о памяти фильтров
   */
  void updateFilterMemory();

  QStatusBar *m_statusBar;
  NetworkController *m_networkController;
  DataProcessor *m_dataProcessor;

  // ui элементы для статистики фильтров
  QLabel *m_movingAvgStatsLabel;
  QLabel *m_medianStatsLabel;
  QLabel *m_exponentialStatsLabel;

  // ui элементы для памяти фильтров
  QLabel *m_movingAvgMemoryLabel;
  QLabel *m_medianMemoryLabel;
  QLabel *m_exponentialMemoryLabel;
  QLabel *m_kalmanMemoryLabel;

  // фильтры
  MovingAverageFilter *m_movingAvgFilter;
  MedianFilter *m_medianFilter;
  ExponentialFilter *m_exponentialFilter;
  KalmanFilter *m_kalmanFilter;

  // буферы для фильтров
  ThreadSafeRingBuffer<DataPoint> *m_rawBufferMovingAvg;
  ThreadSafeRingBuffer<DataPoint> *m_movingAvgBuffer;
  ThreadSafeRingBuffer<DataPoint> *m_rawBufferMedian;
  ThreadSafeRingBuffer<DataPoint> *m_medianBuffer;
  ThreadSafeRingBuffer<DataPoint> *m_rawBufferExponential;
  ThreadSafeRingBuffer<DataPoint> *m_exponentialBuffer;
  ThreadSafeRingBuffer<DataPoint> *m_rawBufferKalman;
  ThreadSafeRingBuffer<DataPoint> *m_kalmanBuffer;
};

#endif // STATUSBARMANAGER_H
