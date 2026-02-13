#ifndef PROCESSMEMORY_H
#define PROCESSMEMORY_H

#include <cstddef>
#include <cstdint>

/**
 * @brief Чтение фактического потребления памяти процессом из ос
 */
namespace ProcessMemory {

/**
 * @brief Получить текущее потребление оперативной памяти процессом
 * @return Размер в байтах или 0 при ошибке / неподдерживаемой ОС
 */
uint64_t getCurrentProcessRSSBytes();

/**
 * @brief Получить текущее потребление памяти в мегабайтах (удобно для
 * отображения)
 *
 * @return Размер в МБ (дробное значение), или -1.0 при ошибке
 */
double getCurrentProcessRSSMegabytes();

} // namespace ProcessMemory

#endif // PROCESSMEMORY_H
