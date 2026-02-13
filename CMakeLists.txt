cmake_minimum_required(VERSION 3.16)

project(PIDVisualizer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets PrintSupport)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets PrintSupport)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/core/)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/core/)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/network/)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/processing/)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/core/)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/filters/)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/network/)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/processing/)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ui/)

set(PROJECT_SOURCES




)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(PIDVisualizer
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        src/main.cpp
        include/ui/mainwindow.h 
        src/ui/mainwindow.cpp
        include/ui/graphmanager.h
        src/ui/graphmanager.cpp
        include/ui/networkcontroller.h
        src/ui/networkcontroller.cpp
        include/ui/cyclictargetcontroller.h
        src/ui/cyclictargetcontroller.cpp
        include/ui/statusbarmanager.h
        src/ui/statusbarmanager.cpp
        include/core/datapoint.h
        include/core/ringbuffer.h
        include/core/circularbuffer.h
        include/network/protocolparser.h
        include/network/udpreceiver.h
        include/network/udpsender.h
        include/core/Constants.h
        include/core/threadsaferingbuffer.h
        include/core/processmemory.h
        include/core/fft.h

        src/core/processmemory.cpp
        src/core/fft.cpp
        src/network/protocolparser.cpp
        src/network/udpreceiver.cpp
        src/network/udpsender.cpp
        include/filters/ifilter.h
        include/filters/filterbase.h
        include/filters/movingaveragefilter.h
        include/filters/medianfilter.h
        include/filters/exponentialfilter.h
        include/filters/kalmanfilter.h
        src/filters/filterbase.cpp
        src/filters/movingaveragefilter.cpp
        src/filters/medianfilter.cpp
        src/filters/exponentialfilter.cpp
        src/filters/kalmanfilter.cpp
        include/processing/dataprocessor.h
        include/processing/filterthread.h
        src/processing/dataprocessor.cpp
        src/processing/filterthread.cpp
        third_party/qcustomplot/qcustomplot.cpp
        third_party/qcustomplot/qcustomplot.h

    )
    
    qt6_wrap_ui(PIDVisualizer_UI_HEADERS
        src/ui/mainwindow.ui
    )
    
    target_sources(PIDVisualizer PRIVATE ${PIDVisualizer_UI_HEADERS})
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET PIDVisualizer APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(PIDVisualizer SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(PIDVisualizer
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(PIDVisualizer PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::PrintSupport)
if(WIN32)    
    target_link_libraries(PIDVisualizer PRIVATE ws2_32)   
    target_link_libraries(PIDVisualizer PRIVATE psapi)
endif()

target_include_directories(PIDVisualizer PRIVATE third_party/qcustomplot)

if(MINGW)
    set_source_files_properties(third_party/qcustomplot/qcustomplot.cpp
        PROPERTIES
        COMPILE_FLAGS "-Wa,-mbig-obj"
    )

    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        set_source_files_properties(third_party/qcustomplot/qcustomplot.cpp
            PROPERTIES
            COMPILE_FLAGS "-Wa,-mbig-obj -g0"
        )
    endif()
endif()

get_target_property(AUTOGEN_BUILD_DIR PIDVisualizer AUTOGEN_BUILD_DIR)
if(AUTOGEN_BUILD_DIR)
    target_include_directories(PIDVisualizer PRIVATE ${AUTOGEN_BUILD_DIR}/include)
endif()

target_include_directories(PIDVisualizer PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.PIDVisualizer)
endif()
set_target_properties(PIDVisualizer PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS PIDVisualizer
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(PIDVisualizer)
endif()
