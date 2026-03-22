cmake_minimum_required(VERSION 3.16)
project(AMSE_CPP)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6 (Optional for CLI core)
find_package(Qt6 COMPONENTS Widgets QUIET)

include_directories(include)

# Source files
set(SOURCES
    src/main.cpp
    src/collector/LinuxMetrics.cpp
    src/features/Window.cpp
    src/features/Processor.cpp
    src/structures/Trie.cpp
    src/structures/AVL.cpp
    src/structures/SegmentTree.cpp
    src/ml/MLEngine.cpp
    src/engine/Scorer.cpp
    src/engine/Controller.cpp
    src/ui/Dashboard.cpp
)

add_executable(amse_cpp ${SOURCES})

if(Qt6_FOUND)
    target_link_libraries(amse_cpp PRIVATE Qt6::Widgets)
    target_compile_definitions(amse_cpp PRIVATE HAS_QT)
endif()
