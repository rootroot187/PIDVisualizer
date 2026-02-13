#ifndef DATAPROCESSOR_H
#define DATAPROCESSOR_H

#include "../core/datapoint.h"
#include "../core/threadsaferingbuffer.h"
#include "../filters/ifilter.h"
#include "filterthread.h"
#include <memory>
#include <string>
#include <vector>

/**
 * @brief менеджер обработки данных
 */
class DataProcessor {
public:
  /**
   * @brief конструктор
   */
  DataProcessor();

  /**
   * @brief деструктор
   * останавливает все потоки и освобождает ресурсы.
   */
  ~DataProcessor();

  /**
   * @brief Добавить фильтр
   *
   * создает FilterThread для фильтра и запускает его.
   *
   * @param filter фильтр
   * @param inputBuffer входной буфер сырых данных именно для этого фильтра
   * @param name имя фильтра
   * @return указатель на output buffer для отфильтрованных данных
   */
  ThreadSafeRingBuffer<DataPoint> *
  addFilter(IFilter *filter, ThreadSafeRingBuffer<DataPoint> *inputBuffer,
            const std::string &name = "");

  /**
   * @brief Удалить фильтр
   *
   * @param name имя фильтра для удаления
   * @return true или false
   */
  bool removeFilter(const std::string &name);

  /**
   * @brief получить output buffer для фильтра
   *
   * @param name имя фильтра
   * @return указатель на output buffer или nullptr если фильтр не найден
   */
  ThreadSafeRingBuffer<DataPoint> *
  getFilterOutputBuffer(const std::string &name) const;

  /**
   * @brief запустить все потоки обработки
   *
   * запускает все FilterThread'ы.
   */
  void start();

  /**
   * @brief остановить все потоки обработки
   *
   * останавливает все FilterThread'ы.
   */
  void stop();

  /**
   * @brief запустить поток одного фильтра по имени
   */
  void startFilter(const std::string &name);

  /**
   * @brief остановить поток одного фильтра по имени
   */
  void stopFilter(const std::string &name);

  /**
   * @brief проверить, работают ли потоки
   *
   * @return true или false
   */
  bool isRunning() const;

  /**
   * @brief получить список имен фильтров
   *
   * @return вектор имен всех добавленных фильтров
   */
  std::vector<std::string> getFilterNames() const;

  /**
   * @brief получить статистику
   *
   * @param name имя фильтра
   * @return количество обработанных точек или 0 если фильтр не найден
   */
  size_t getFilterProcessedCount(const std::string &name) const;

private:
  /**
   * @brief Структура для хранения информации о фильтре
   */
  struct FilterInfo {
    IFilter *filter;                      // фильтр (не владеем)
    std::unique_ptr<FilterThread> thread; // поток обработки
    std::unique_ptr<ThreadSafeRingBuffer<DataPoint>>
        outputBuffer; // буфер выходных данных
    std::string name; // имя фильтра

    FilterInfo(IFilter *f, std::unique_ptr<FilterThread> t,
               std::unique_ptr<ThreadSafeRingBuffer<DataPoint>> buf,
               const std::string &n)
        : filter(f), thread(std::move(t)), outputBuffer(std::move(buf)),
          name(n) {}
  };

  std::vector<std::unique_ptr<FilterInfo>> m_filters; // список фильтров
};

#endif // DATAPROCESSOR_H
