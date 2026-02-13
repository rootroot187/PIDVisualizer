#include "../../include/processing/filterthread.h"
#include <chrono>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <thread>

FilterThread::FilterThread(IFilter *filter,
                           ThreadSafeRingBuffer<DataPoint> *inputBuffer,
                           ThreadSafeRingBuffer<DataPoint> *outputBuffer,
                           const char *name)
    : m_filter(filter), m_inputBuffer(inputBuffer),
      m_outputBuffer(outputBuffer), m_running(false), m_processedCount(0),
      m_name(name ? name : "FilterThread") {
  // конструктор вызывается из GUI потока
  if (!m_filter) {
    throw std::invalid_argument("Filter cannot be nullptr");
  }
  if (!m_inputBuffer) {
    throw std::invalid_argument("Input buffer cannot be nullptr");
  }
  if (!m_outputBuffer) {
    throw std::invalid_argument("Output buffer cannot be nullptr");
  }
}

FilterThread::~FilterThread() { stop(); }

void FilterThread::start() {
  // start() вызывается из GUI потока, можно использовать qDebug
  if (m_running.load()) {
    return; // Уже запущен
  }

  // ВАЖНО: Проверяем, что старый поток завершился перед созданием нового
  // Это предотвращает ошибку "terminate called without an active exception"
  if (m_thread.joinable()) {
    m_thread.join();
  }

  m_running.store(true);
  m_thread = std::thread(&FilterThread::run, this);
}

void FilterThread::stop() {
  // stop() вызывается из GUI потока, можно использовать qDebug
  if (!m_running.load()) {
    return; // Уже остановлен
  }

  m_running.store(false);

  if (m_thread.joinable()) {
    m_thread.join(); // Ждем завершения потока
  }
}

bool FilterThread::isRunning() const { return m_running.load(); }

const std::string &FilterThread::getName() const { return m_name; }

size_t FilterThread::getProcessedCount() const {
  return m_processedCount.load();
}

void FilterThread::run() {
  // run() выполняется в отдельном потоке
  int iteration = 0;

  while (m_running.load()) {
    iteration++;
    // проверяем, что буферы валидны
    if (!m_inputBuffer || !m_outputBuffer) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      continue;
    }

    // читаем все доступные данные из входного буфера
    std::vector<DataPoint> inputData;
    try {
      inputData = m_inputBuffer->popAll();
    } catch (...) {

      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      continue;
    }

    if (inputData.empty()) {
      // если данных нет, то засыпаем на 100 мс
      std::this_thread::sleep_for(std::chrono::milliseconds(10));

      continue;
    }

    // обрабатываем все полученные данные
    for (const auto &point : inputData) {
      if (!m_running.load()) {
        break; // если поток остановлен, то прерываем обработку
      }
      processPoint(point);
    }
  }
}

void FilterThread::processPoint(const DataPoint &point) {
  // применяем фильтр к точке данных и сохраняем результат
  DataPoint filtered = m_filter->filter(point);

  // если фильтр готов (накопил достаточно данных), то сохраняем результат
  if (m_filter->isReady()) {
    m_outputBuffer->push(filtered);
    m_processedCount.fetch_add(1);
  }
}
