cmake_minimum_required(VERSION 3.11)
project(AtwoodSimulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui)

add_subdirectory(libs/imgui-sfml)

file(GLOB IMPLOT_SOURCES
    "libs/implot/implot.cpp"
    "libs/implot/implot_items.cpp"
    "libs/implot/implot_demo.cpp"
)

add_executable(Atwood
    src/main.cpp
    src/physics.cpp
    libs/implot/implot.cpp
    libs/implot/implot_items.cpp
    libs/implot/implot_demo.cpp
    libs/imgui/imgui_demo.cpp  
)

target_include_directories(Atwood PRIVATE 
    libs/implot
    libs/imgui
)

target_link_libraries(Atwood PRIVATE ImGui-SFML::ImGui-SFML)