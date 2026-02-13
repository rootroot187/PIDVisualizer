#ifndef FILTERTHREAD_H
#define FILTERTHREAD_H

#include "../core/datapoint.h"
#include "../core/threadsaferingbuffer.h"
#include "../filters/ifilter.h"
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>

/**
 * @brief поток для выполнения фильтра
 */
class FilterThread {
public:
  /**
   * @brief конструктор
   *
   * @param filter фильтр для применения (IFilter*)
   * @param inputBuffer буфер входных данных
   * @param outputBuffer буфер выходных данных
   * @param name имя потока
   */
  FilterThread(IFilter *filter, ThreadSafeRingBuffer<DataPoint> *inputBuffer,
               ThreadSafeRingBuffer<DataPoint> *outputBuffer,
               const char *name = "FilterThread");

  /**
   * @brief деструктор
   */
  ~FilterThread();

  /**
   * @brief запустить поток фильтрации
   */
  void start();

  /**
   * @brief остановить поток фильтрации
   */
  void stop();

  /**
   * @brief проверить, работает ли поток
   * @return true или false
   */
  bool isRunning() const;

  /**
   * @brief получить имя потока
   *
   * @return имя потока
   */
  const std::string &getName() const;

  /**
   * @brief получить статистику
   * @return количество обработанных точек
   */
  size_t getProcessedCount() const;

private:
  /**
   * @brief основная функция потока
   *
   * выполняется в отдельном потоке.
   * чтение из inputBuffer, применение фильтра + сохранение в outputBuffer.
   */
  void run();

  /**
   * @brief обработать одну точку данных
   *
   * применение фильтра сохранение результата.
   */
  void processPoint(const DataPoint &point);

  IFilter *m_filter;                               // фильтр (IFilter*)
  ThreadSafeRingBuffer<DataPoint> *m_inputBuffer;  // буфер входных данных
  ThreadSafeRingBuffer<DataPoint> *m_outputBuffer; // буфер выходных данных
  std::thread m_thread;                            // поток выполнения
  std::atomic<bool> m_running;          // потокобезопасный флаг работы
  std::atomic<size_t> m_processedCount; // счетчик точек
  std::string m_name;                   // имя потока
};

#endif // FILTERTHREAD_H
