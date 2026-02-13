#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <vector>

/**
 * @brief простой кольцевой буфер (шаблон)
 * @tparam T тип данных для хранения
 */
template <typename T> class RingBuffer {
public:
  explicit RingBuffer(size_t capacity)
      : m_buffer(capacity), m_capacity(capacity), m_size(0), m_write(0),
        m_read(0) {
    assert(capacity > 0 && "Capacity must be greater than 0");
  }

  virtual ~RingBuffer() = default;

  virtual void push(const T &item) {
    m_buffer[m_write] = item;
    m_write = (m_write + 1) % m_capacity;

    if (m_size < m_capacity) {
      m_size++;
    } else {
      m_read = (m_read + 1) % m_capacity;
    }
  }

  virtual const T &at(size_t index) const {
    assert(index < m_size && "Index out of range");
    size_t actualIndex = (m_read + index) % m_capacity;
    return m_buffer[actualIndex];
  }

  virtual T &at(size_t index) {
    assert(index < m_size && "Index out of range");
    size_t actualIndex = (m_read + index) % m_capacity;
    return m_buffer[actualIndex];
  }

  virtual const T &back() const {
    assert(m_size > 0 && "Buffer is empty");
    size_t lastIndex = (m_write - 1 + m_capacity) % m_capacity;
    return m_buffer[lastIndex];
  }

  virtual const T &front() const {
    assert(m_size > 0 && "Buffer is empty");
    return m_buffer[m_read];
  }

  virtual void pop() {
    assert(m_size > 0 && "Cannot pop from empty buffer");
    m_read = (m_read + 1) % m_capacity;
    m_size--;
  }

  virtual void clear() {
    m_size = 0;
    m_write = 0;
    m_read = 0;
  }

  virtual bool empty() const { return m_size == 0; }

  virtual bool full() const { return m_size == m_capacity; }

  virtual size_t size() const { return m_size; }

  virtual size_t capacity() const { return m_capacity; }

  /**
   * @brief получить размер памяти, используемой буфером
   * @return размер памяти в байтах
   */
  virtual size_t getMemoryUsage() const {

    return m_capacity * sizeof(T) + sizeof(m_capacity) + sizeof(m_size) +
           sizeof(m_write) + sizeof(m_read);
  }

  virtual std::vector<T> getAll() const {
    if (m_size == 0) {
      return {};
    }

    std::vector<T> result;
    result.reserve(m_size);

    for (size_t i = 0; i < m_size; ++i) {
      size_t index = (m_read + i) % m_capacity;
      result.push_back(m_buffer[index]);
    }

    return result;
  }

  virtual std::vector<T> getLast(size_t count) const {
    if (count == 0 || m_size == 0) {
      return {};
    }

    count = std::min(count, m_size);

    std::vector<T> result;
    result.reserve(count);

    size_t startIndex = m_size - count;

    for (size_t i = 0; i < count; ++i) {
      size_t index = (m_read + startIndex + i) % m_capacity;
      result.push_back(m_buffer[index]);
    }

    return result;
  }

  virtual std::vector<T> popAll() {
    std::vector<T> result = getAll();
    clear();
    return result;
  }

protected:
  std::vector<T> m_buffer;
  size_t m_capacity;
  size_t m_size;
  size_t m_write;
  size_t m_read;
};

#endif // RINGBUFFER_H
