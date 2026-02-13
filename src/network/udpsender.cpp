#include "../../include/network/udpsender.h"
#include "../../include/core/Constants.h"
#include "../../include/network/protocolparser.h"

#ifdef _WIN32
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include <QDebug>
#include <cstring>

UdpSender::UdpSender()
#ifdef _WIN32
    : m_socket(INVALID_SOCKET)
#else
    : m_socket(-1)
#endif
      ,
      m_packetsSent(0) {
}

UdpSender::~UdpSender() { closeSocket(); }

bool UdpSender::sendCommand(float targetValue, const std::string &ip,
                            uint16_t port) {
  // инициализация сокета
  if (
#ifdef _WIN32
      m_socket == INVALID_SOCKET
#else
      m_socket < 0
#endif
  ) {
    // сокет не создан
    if (!initializeSocket()) {
      // ошибка создания сокета
      return false;
    }
  }

  // создание пакета из float значения
  std::vector<uint8_t> packet =
      ProtocolParser::createCommandPacket(targetValue);

  // подготовка адреса получателя
  struct sockaddr_in destAddr;
  std::memset(&destAddr, 0, sizeof(destAddr));

  destAddr.sin_family = AF_INET;                    // IPv4
  destAddr.sin_port = htons(port);                  // порт
  destAddr.sin_addr.s_addr = inet_addr(ip.c_str()); // IP

  // правильный ли IP?
  if (destAddr.sin_addr.s_addr == INADDR_NONE) {
    // неправильный IP адрес
    return false;
  }

#ifdef _WIN32
  int sent =
      sendto(m_socket,                                      // сокет
             reinterpret_cast<const char *>(packet.data()), // данные (байты)
             static_cast<int>(packet.size()),               // размер данных
             0,                                             // флаги
             (struct sockaddr *)&destAddr,                  // адрес получателя
             sizeof(destAddr)); // размер структуры адреса
#else
  ssize_t sent =
      sendto(m_socket,                                      // сокет
             reinterpret_cast<const char *>(packet.data()), // данные (байты)
             packet.size(),                                 // размер данных
             0,                                             // флаги
             (struct sockaddr *)&destAddr,                  // адрес получателя
             sizeof(destAddr)); // размер структуры адреса
#endif

  // успешно ли отправлено?
  if (sent < 0) {
    return false;
  }

  // отправили ли все байты?
  if (static_cast<size_t>(sent) != packet.size()) {
    // отправили не все байты
    return false;
  }

  // обновление статистики
  m_packetsSent++;
  return true;
}

bool UdpSender::isInitialized() const {
#ifdef _WIN32
  return m_socket != INVALID_SOCKET;
#else
  return m_socket >= 0;
#endif
}

size_t UdpSender::getPacketsSent() const {
  // количество успешно отправленных пакетов
  return m_packetsSent;
}

void UdpSender::resetStatistics() { m_packetsSent = 0; }

bool UdpSender::initializeSocket() {
#ifdef _WIN32
  static bool winsockInitialized = false;
  if (!winsockInitialized) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
      return false;
    }
    winsockInitialized = true;
  }
  m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (m_socket == INVALID_SOCKET) {
    return false;
  }

  // настройки сокета
  BOOL reuse = TRUE;
  if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR,
                 reinterpret_cast<const char *>(&reuse),
                 sizeof(reuse)) == SOCKET_ERROR) {
    closesocket(m_socket);
    m_socket = INVALID_SOCKET;
    return false;
  }
#else
  m_socket = socket(AF_INET, SOCK_DGRAM, 0);
  if (m_socket < 0) {
    // ошибка: не удалось создать сокет
    return false;
  }

  int reuse = 1;
  if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) <
      0) {
    // ошибка настройки
    close(m_socket);
    m_socket = -1;
    return false;
  }
#endif

  // сокет создан
  return true;
}

void UdpSender::closeSocket() {
  if (
#ifdef _WIN32
      m_socket != INVALID_SOCKET
#else
      m_socket >= 0
#endif
  ) {
#ifdef _WIN32
    closesocket(m_socket);
    m_socket = INVALID_SOCKET;
#else
    close(m_socket);
    m_socket = -1;
#endif
  }
}
