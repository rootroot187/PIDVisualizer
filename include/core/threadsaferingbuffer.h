#ifndef THREADSAFERINGBUFFER_H
#define THREADSAFERINGBUFFER_H

#include "ringbuffer.h"
#include <mutex>
#include <vector>

/**
 * @brief потокобезопасный кольцевой буфер
 * @tparam T тип данных для хранения
 */
template <typename T> class ThreadSafeRingBuffer : public RingBuffer<T> {
public:
  /**
   * @brief конструктор
   * @param capacity размер буфера
   */
  explicit ThreadSafeRingBuffer(size_t capacity) : RingBuffer<T>(capacity) {}

  /**
   * @brief добавить элемент потокобезопасно
   */
  void push(const T &item) override {
    std::unique_lock<std::mutex> lock(m_mutex);
    RingBuffer<T>::push(item);
  }

  /**
   * @brief получить элемент по индексу потокобезопасно
   * @param index индекс элемента
   * @return константная ссылка на элемент
   */
  const T &at(size_t index) const override {
    std::unique_lock<std::mutex> lock(m_mutex);
    return RingBuffer<T>::at(index);
  }

  /**
   * @brief получить элемент по индексу неконстантная версия
   */
  T &at(size_t index) override {
    std::unique_lock<std::mutex> lock(m_mutex);
    return RingBuffer<T>::at(index);
  }

  /**
   * @brief получить самый новый элемент потокобезопасно
   */
  const T &back() const override {
    std::unique_lock<std::mutex> lock(m_mutex);
    return RingBuffer<T>::back();
  }

  /**
   * @brief получить самый старый элемент потокобезопасно
   */
  const T &front() const override {
    std::unique_lock<std::mutex> lock(m_mutex);
    return RingBuffer<T>::front();
  }

  /**
   * @brief удалить самый старый элемент потокобезопасно
   */
  void pop() override {
    std::unique_lock<std::mutex> lock(m_mutex);
    RingBuffer<T>::pop();
  }

  /**
   * @brief очистить буфер потокобезопасно
   */
  void clear() override {
    std::unique_lock<std::mutex> lock(m_mutex);
    RingBuffer<T>::clear();
  }

  /**
   * @brief проверить, пуст ли буфер потокобезопасно
   */
  bool empty() const override {
    std::unique_lock<std::mutex> lock(m_mutex);
    return RingBuffer<T>::empty();
  }

  /**
   * @brief проверить, полон ли буфер потокобезопасно
   */
  bool full() const override {
    std::unique_lock<std::mutex> lock(m_mutex);
    return RingBuffer<T>::full();
  }

  /**
   * @brief получить текущий размер потокобезопасно
   */
  size_t size() const override {
    std::unique_lock<std::mutex> lock(m_mutex);
    return RingBuffer<T>::size();
  }

  /**
   * @brief получить максимальную вместимость потокобезопасно
   */
  size_t capacity() const override {
    std::unique_lock<std::mutex> lock(m_mutex);
    return RingBuffer<T>::capacity();
  }

  std::vector<T> getAll() const override {
    std::unique_lock<std::mutex> lock(m_mutex);
    return RingBuffer<T>::getAll();
  }

  /**
   * @brief получить последние N элементов потокобезопасно
   * @param count количество элементов
   * @return вектор с элементами в порядке от старого к новому
   */
  std::vector<T> getLast(size_t count) const override {
    std::unique_lock<std::mutex> lock(m_mutex);
    return RingBuffer<T>::getLast(count);
  }

  /**
   * @brief получить все элементы и очистить буфер потокобезопасно
   * @return вектор с элементами в порядке от старого к новому
   */
  std::vector<T> popAll() override {
    std::unique_lock<std::mutex> lock(m_mutex);
    std::vector<T> result = RingBuffer<T>::getAll();
    RingBuffer<T>::clear();
    return result;
  }

  /**
   * @brief получить размер памяти, используемой буфером потокобезопасно
   * @return размер памяти в байтах включая мьютекс
   */
  size_t getMemoryUsage() const override {
    std::unique_lock<std::mutex> lock(m_mutex);
    return RingBuffer<T>::getMemoryUsage() + sizeof(m_mutex);
  }

private:
  /**
   * @brief мьютекс для синхронизации доступа
   */
  mutable std::mutex m_mutex;
};

#endif // THREADSAFERINGBUFFER_H
