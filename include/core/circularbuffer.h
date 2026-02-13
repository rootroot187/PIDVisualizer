#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <vector>
#include <algorithm>
#include <cstddef>
#include <cassert> 

/**
 * @brief простой кольцевой буфер
 */
template<typename T>
class RingBuffer
{
public:
    /**
     * @brief конструктор
     * @param capacity размер буфера
     */
    explicit RingBuffer(size_t capacity)
        : m_buffer(capacity)
        , m_capacity(capacity)
        , m_size(0)
        , m_write(0)
        , m_read(0)
    {
        assert(capacity > 0 && "Capacity must be greater than 0");
    }
    
    /**
     * @brief добавить элемент в буфер
     * @param item элемент для добавления
     */
    virtual void push(const T& item)
    {
        // записываем элемент в текущую позицию записи
        m_buffer[m_write] = item;
        
        m_write = (m_write + 1) % m_capacity;
        
        if (m_size < m_capacity) {
            m_size++;
        } else {
            m_read = (m_read + 1) % m_capacity;
        }
    }
    
    /**
     * @brief получить элемент по индексу
     * @return константная ссылка на элемент
     */
    virtual const T& at(size_t index) const
    {
        assert(index < m_size && "Index out of range");
        
        //реальный индекс в массиве
        size_t actualIndex = (m_read + index) % m_capacity;
        
        //возвращаем элемент
        return m_buffer[actualIndex];
    }
    
    /**
     * @brief получить элемент по индексу (неконстантная версия)
     * @return неконстантная ссылка на элемент
     */
    virtual T& at(size_t index)
    {
        assert(index < m_size && "Index out of range");
        size_t actualIndex = (m_read + index) % m_capacity;
        return m_buffer[actualIndex];
    }
    
    /**
     * @brief получить самый новый элемент
     * @return константная ссылка на элемент
     */
    virtual const T& back() const
    {
        assert(m_size > 0 && "Buffer is empty");
        size_t lastIndex = (m_write - 1 + m_capacity) % m_capacity;
        return m_buffer[lastIndex];
    }
    
    /**
     * @brief получить самый старый элемент
     * @return константная ссылка на элемент
     */
    virtual const T& front() const
    {
        assert(m_size > 0 && "Buffer is empty");
        return m_buffer[m_read];
    }
    
    /**
     * @brief удалить самый старый элемент
     */
    virtual void pop()
    {
        assert(m_size > 0 && "Cannot pop from empty buffer");
        m_read = (m_read + 1) % m_capacity;
        m_size--;
    }
    
    /**
     * @brief очистить буфер
     */
    virtual void clear()
    {
        m_size = 0;
        m_write = 0;
        m_read = 0;
    }
    
    /**
     * @brief проверить, пуст ли буфер
     */
    virtual bool empty() const
    {
        return m_size == 0;
    }
    
    /**
     * @brief проверить, полон ли буфер
     */
    virtual bool full() const
    {
        return m_size == m_capacity;
    }
    
    /**
     * @brief получить текущий размер (количество элементов)
     */
    virtual size_t size() const
    {
        return m_size;
    }
    
    /**
    * @brief получить максимальную вместимость
     */
    virtual size_t capacity() const
    {
        return m_capacity;
    }
    
    /**
     * @brief получить все элементы в правильном порядке (от старого к новому)
     * @return вектор с элементами в порядке от старого к новому
     */
    virtual std::vector<T> getAll() const
    {
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
    
    /**
     * @brief получить все элементы и очистить буфер
     * @return вектор с элементами в порядке от старого к новому
     */
    virtual std::vector<T> popAll()
    {
        std::vector<T> result = getAll();  // читаем все данные
        clear();                            // очищаем буфер
        return result;
    }
    
    /**
     * @brief получить последние N элементов
     * @param count количество элементов
     * @return вектор с элементами в порядке от старого к новому
     */
    virtual std::vector<T> getLast(size_t count) const
    {
        //проверка граничных случаев
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

private:
    std::vector<T> m_buffer;  // массив для хранения данных
    size_t m_capacity;         // максимальная вместимость
    size_t m_size;            // текущий размер
    size_t m_write;           // индекс следующей позиции для записи
    size_t m_read;            // индекс старого элемента для чтения
};

#endif // RINGBUFFER_H
