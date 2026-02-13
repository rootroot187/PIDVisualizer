#include "../../include/processing/dataprocessor.h"
#include "../../include/core/Constants.h"
#include <QDebug>
#include <algorithm>
#include <stdexcept>

DataProcessor::DataProcessor() {}

DataProcessor::~DataProcessor() { stop(); }

ThreadSafeRingBuffer<DataPoint> *
DataProcessor::addFilter(IFilter *filter,
                         ThreadSafeRingBuffer<DataPoint> *inputBuffer,
                         const std::string &name) {
  if (!filter || !inputBuffer) {
    return nullptr;
  }

  // создаем output buffer для отфильтрованных данных
  auto outputBuffer = std::make_unique<ThreadSafeRingBuffer<DataPoint>>(
      Constants::DEFAULT_BUFFER_SIZE);

  // сохраняем указатель на output buffer
  ThreadSafeRingBuffer<DataPoint> *outputBufferPtr = outputBuffer.get();

  // FilterThread. для каждого фильтра свой входной буфер
  std::string filterName = name.empty() ? filter->getName() : name;
  auto thread =
      std::make_unique<FilterThread>(filter,
                                     inputBuffer,       // входной буфер
                                     outputBufferPtr,   // выходной буфер
                                     filterName.c_str() // имя фильтра
      );

  // создаем FilterInfo и сохраняем
  auto filterInfo = std::make_unique<FilterInfo>(
      filter, std::move(thread), std::move(outputBuffer),
      name.empty() ? filter->getName() : name);

  m_filters.push_back(std::move(filterInfo));

  // запускаем поток, если DataProcessor уже запущен
  if (isRunning()) {
    m_filters.back()->thread->start();
  }

  return outputBufferPtr;
}

bool DataProcessor::removeFilter(const std::string &name) {
  auto it = std::find_if(m_filters.begin(), m_filters.end(),
                         [&name](const std::unique_ptr<FilterInfo> &info) {
                           return info->name == name;
                         });

  if (it != m_filters.end()) {
    // останавливаем поток перед удалением
    (*it)->thread->stop();
    m_filters.erase(it);
    return true;
  }

  return false;
}

ThreadSafeRingBuffer<DataPoint> *
DataProcessor::getFilterOutputBuffer(const std::string &name) const {

  auto it = std::find_if(m_filters.begin(), m_filters.end(),
                         [&name](const std::unique_ptr<FilterInfo> &info) {
                           return info->name == name;
                         });

  if (it != m_filters.end()) {
    return (*it)->outputBuffer.get();
  }

  return nullptr;
}

void DataProcessor::start() {
  for (auto &filterInfo : m_filters) {
    if (filterInfo->thread && !filterInfo->thread->isRunning()) {
      filterInfo->thread->start();
    }
  }
}

void DataProcessor::stop() {
  for (auto &filterInfo : m_filters) {
    if (filterInfo->thread && filterInfo->thread->isRunning()) {
      filterInfo->thread->stop();
    }
  }
}

void DataProcessor::startFilter(const std::string &name) {
  // находим фильтр по имени
  auto it = std::find_if(m_filters.begin(), m_filters.end(),
                         [&name](const std::unique_ptr<FilterInfo> &info) {
                           return info->name == name;
                         });
  // если фильтр найден и поток не запущен, то запускаем его
  if (it != m_filters.end() && (*it)->thread && !(*it)->thread->isRunning()) {
    (*it)->thread->start();
  }
}

void DataProcessor::stopFilter(const std::string &name) {
  // находим фильтр по имени
  auto it = std::find_if(m_filters.begin(), m_filters.end(),
                         [&name](const std::unique_ptr<FilterInfo> &info) {
                           return info->name == name;
                         });
  // если фильтр найден и поток запущен, то останавливаем его
  if (it != m_filters.end() && (*it)->thread && (*it)->thread->isRunning()) {
    (*it)->thread->stop();
  }
}

bool DataProcessor::isRunning() const {
  // проверяем, запущены ли все потоки
  for (const auto &filterInfo : m_filters) {
    if (filterInfo->thread && filterInfo->thread->isRunning()) {
      return true;
    }
  }
  return false;
}

std::vector<std::string> DataProcessor::getFilterNames() const {
  // создаем вектор имен фильтров
  std::vector<std::string> names;
  names.reserve(m_filters.size());
  // добавляем имена фильтров в вектор
  for (const auto &filterInfo : m_filters) {
    names.push_back(filterInfo->name);
  }

  return names;
}

size_t DataProcessor::getFilterProcessedCount(const std::string &name) const {
  // находим фильтр по имени
  auto it = std::find_if(m_filters.begin(), m_filters.end(),
                         [&name](const std::unique_ptr<FilterInfo> &info) {
                           return info->name == name;
                         });
  // если фильтр найден и поток существует, то возвращаем количество
  // обработанных точек
  if (it != m_filters.end() && (*it)->thread) {
    return (*it)->thread->getProcessedCount();
  }

  return 0;
}
