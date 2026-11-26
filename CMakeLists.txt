cmake_minimum_required(VERSION 3.16)
project(VHAppVectorDim VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Widgets OpenGLWidgets OpenGL)

add_executable(vhappvectordim
    src/main.cpp
    src/FileValidator.cpp
    src/CubeLayout.cpp
    src/gui/CubeView.cpp
    src/gui/CubeVisualizer.cpp
)

target_compile_definitions(vhappvectordim
    PRIVATE
        VHAPPVECTORDIM_VERSION="${PROJECT_VERSION}"
)

target_include_directories(vhappvectordim
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/gui
)

target_link_libraries(vhappvectordim
    PRIVATE
        Qt6::Widgets
        Qt6::OpenGLWidgets
        Qt6::OpenGL
)
