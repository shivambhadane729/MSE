cmake_minimum_required(VERSION 3.16)
project(AMSE_CPP)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt6 Configuration
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

include_directories(include)

# Source files
set(SOURCES
    src/main.cpp
    src/collector/LinuxMetrics.cpp
    src/ui/Dashboard.cpp
    include/ui/Dashboard.hpp
)

add_executable(amse_cpp ${SOURCES})

target_link_libraries(amse_cpp PRIVATE Qt6::Widgets)
target_compile_definitions(amse_cpp PRIVATE HAS_QT)
