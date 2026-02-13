#ifndef UDPRECEIVER_H
#define UDPRECEIVER_H

#include "../core/Constants.h"
#include "../core/datapoint.h"
#include "../core/threadsaferingbuffer.h"
#include <atomic>
#include <functional>
#include <string>
#include <thread>

#ifdef _WIN32
// Для шиндовса используем WinSock2
#include <winsock2.h>
#include <ws2tcpip.h>
using SocketHandle = SOCKET;
#else
// Для линуха используем posix-сокеты
using SocketHandle = int;
#endif

/**
 * @brief класс для приема данных от модели по UDP
 */
class UdpReceiver {
public:
  /**
   * @brief конструктор
   *
   * @param buffer потокобезопасный буфер для сохранения данных
   * @param onDataReceived callback функция, вызываемая при получении данных
   */
  explicit UdpReceiver(
      ThreadSafeRingBuffer<DataPoint> *buffer,
      std::function<void(const DataPoint &)> onDataReceived = nullptr);

  /**
   * @brief деструктор
   * автоматически останавливает прием и закрывает сокет
   * ждет завершения потока приема
   */
  ~UdpReceiver();

  /**
   * @brief начать прием данных
   * создает сокет, привязывает к адресу и порту, запускает поток приема
   *
   * @param ip IP адрес для приема (по умолчанию из Constants)
   * @param port порт для приема (по умолчанию из Constants)
   * @return true если успешно, false при ошибке
   */
  bool start(const std::string &ip = Constants::Network::DEFAULT_SEND_IP,
             uint16_t port = Constants::Network::DEFAULT_SEND_PORT);

  /**
   * @brief Остановить прием данных
   */
  void stop();

  /**
   * @brief Проверить, работает ли прием
   * @return true если прием активен
   */
  bool isRunning() const;

  /**
   * @brief Получить статистику
   * @return Количество принятых пакетов
   */
  size_t getPacketsReceived() const;

  /**
   * @brief Сбросить статистику
   */
  void resetStatistics();

private:
  /**
   * @brief основной цикл приема данных
   */
  void receiveLoop();

  /**
   * @brief инициализация сокета
   *
   * создает UDP сокет и привязывает к адресу
   *
   * @param ip IP адрес
   * @param port порт
   * @return true если успешно
   */
  bool initializeSocket(const std::string &ip, uint16_t port);

  /**
   * @brief закрытие сокета
   */
  void closeSocket();

  ThreadSafeRingBuffer<DataPoint> *m_buffer;               // буфер для данных
  std::function<void(const DataPoint &)> m_onDataReceived; // callback

  std::thread m_receiveThread; // поток приема
  std::atomic<bool> m_running; // флаг

  SocketHandle m_socket;                 // дескриптор сокета
  std::atomic<size_t> m_packetsReceived; // количество принятых пакетов
};

#endif // UDPRECEIVER_H
