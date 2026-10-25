cmake_minimum_required(VERSION 3.10)
project(SnowLight VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")

add_executable(SnowLight ${SOURCES})

# SFML 3 wymaga komponentów pisanych wielką literą
find_package(SFML 3 COMPONENTS Graphics Window System REQUIRED)
target_link_libraries(SnowLight PRIVATE SFML::Graphics SFML::Window SFML::System)