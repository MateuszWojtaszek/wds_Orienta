cmake_minimum_required(VERSION 3.30)
project(wds_Orienta)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_PREFIX_PATH "/Users/mateuszwojtaszek/Qt/6.10.0/macos/")

find_package(Qt6 COMPONENTS
  Core
  Gui
  Widgets
  Charts
  3DCore
  3DRender
  3DExtras
  WebEngineWidgets
  SerialPort
  REQUIRED)

add_executable(wds_Orienta main.cpp
        MainWindow.cpp
        MainWindow.h
        ImuDataHandler.cpp
        ImuDataHandler.h
        GpsDataHandler.cpp
        GpsDataHandler.h
        SerialPortHandler.cpp
        SerialPortHandler.h
        SensorGraph.h
        SensorGraph.cpp)
target_link_libraries(wds_Orienta
        Qt6::Widgets
        Qt6::Charts
        Qt6::3DCore
        Qt6::3DRender
        Qt6::3DExtras
        Qt6::WebEngineWidgets
        Qt6::SerialPort
)

