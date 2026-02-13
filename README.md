# PID Visualizer

![C++](https://img.shields.io/badge/C++-17-blue.svg)
![Qt](https://img.shields.io/badge/Qt-6.5+-green.svg)
![CMake](https://img.shields.io/badge/CMake-3.16+-blue.svg)
![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux-lightgrey.svg)

Приложение для фильтрации и визуализации данных, полученных от PID-модели в реальном времени.

## Описание

PID Visualizer - это оконное приложение на C++/Qt, которое:
- Принимает данные от PID-модели по UDP
- Применяет различные фильтры (КИХ и БИХ) к данным в отдельных потоках
- Отображает исходные и отфильтрованные данные на графике в реальном времени
- Позволяет управлять целевым значением для модели
- Вычисляет и отображает спектр сигналов (FFT)

## Требования

### Windows:
- Windows 10 или новее (64-bit)
- CMake 3.16+
- Qt 5.15+ или Qt 6.0+
- Компилятор: MSVC 2017+, MinGW-w64, или Clang
- Git

### Linux:
- Ubuntu 18.04+ / Debian 10+ / Fedora 30+ / Arch Linux
- CMake 3.16+
- Qt 5.15+ или Qt 6.0+
- Компилятор: Clang 8+
- Git

## Установка зависимостей

### Windows

1. **CMake**: https://cmake.org/download/
2. **Qt**: https://www.qt.io/download-qt-installer
   - Выберите Qt 6.5.x → MSVC 2019 64-bit или MinGW 11.2.0 64-bit
3. **Компилятор**: Visual Studio 2022 или MinGW-w64

### Linux

#### Ubuntu/Debian:
```bash
sudo apt update
sudo apt install -y build-essential cmake git qt6-base-dev qt6-tools-dev qt6-tools-dev-tools
```

#### Fedora:
```bash
sudo dnf install -y gcc-c++ cmake git qt6-qtbase-devel qt6-qttools-devel
```

#### Arch Linux:
```bash
sudo pacman -S base-devel cmake git qt6-base qt6-tools
```

## Сборка проекта

### Linux

```bash
git clone <URL_РЕПОЗИТОРИЯ>
cd PIDVisualizer
mkdir build && cd build
CC=clang CXX=clang++ cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

Или через Qt Creator:
1. Откройте Qt Creator
2. **File** → **Open File or Project** → выберите `CMakeLists.txt`
3. Выберите Kit и нажмите **Configure Project**
4. Нажмите **Build** (Ctrl+B)

### Windows

```powershell
mkdir build
cd build
$env:PATH = "C:\Qt\Tools\mingw1310_64\bin;C:\Qt\{your-version}\mingw_64\bin;" + $env:PATH
cmake .. -G "Ninja" -DCMAKE_PREFIX_PATH="C:/Qt/{your-version}/mingw_64" -DCMAKE_BUILD_TYPE=Release
ninja
windeployqt PIDVisualizer.exe
```

**Примечания:**
- Замените `{your-version}` на вашу версию Qt (например, `6.10.2`)
- Если `ninja` не найден, скачайте с https://github.com/ninja-build/ninja/releases

Или через Qt Creator:
1. Откройте Qt Creator
2. **File** → **Open File or Project** → выберите `CMakeLists.txt`
3. Выберите Kit и нажмите **Configure Project**
4. Нажмите **Build** (Ctrl+B)

## Запуск приложения

### Linux:
```bash
cd build
./PIDVisualizer
```

### Windows:
```cmd
cd build
PIDVisualizer.exe
```

**Важно для Windows:** После сборки запустите `windeployqt PIDVisualizer.exe` для копирования необходимых DLL.

## Использование

### Запуск модели

1. Перейдите в папку `PIDemulator/`
2. Запустите модель: `python model.py`

### Настройка приложения

1. **Сетевые параметры:**
   - IP приема: `127.0.0.1` (по умолчанию)
   - Порт приема: `50006` (по умолчанию)
   - IP отправки: `127.0.0.1` (по умолчанию)
   - Порт отправки: `50005` (по умолчанию)

2. **Запуск:** Нажмите кнопку **"Старт"** для начала приема данных

3. **Отправка целевого значения:** Введите значение и нажмите **"Отправить"**

4. **Настройка фильтров:**
   - Включите/выключите фильтры через чекбоксы
   - Настройте параметры:
     - **Moving Average**: размер окна
     - **Median**: размер окна (нечетный)
     - **Exponential**: коэффициент альфа (0.0 - 1.0)
     - **Kalman**: параметры Q, R, P

5. **Настройка графика:** Количество отсчетов: 50-1000

6. **Циклическое задание:** Включите опцию и выберите тип сигнала (Треугольный, Синусоида, Прямоугольный, Случайный)

## Архитектура

Проект использует модульную архитектуру:
- **Core**: Буферы данных, константы
- **Network**: UDP прием/отправка
- **Filters**: Реализации фильтров (IFilter интерфейс)
- **Processing**: Управление потоками обработки
- **UI**: Графический интерфейс (Qt)
