cmake_minimum_required(VERSION 3.31)
project(eeprom_24lc32a_storage LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(eeprom_app
    src/main.cpp
)

target_compile_options(eeprom_app PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GPIODXX REQUIRED libgpiodcxx)
target_include_directories(eeprom_app PRIVATE ${GPIODXX_INCLUDE_DIRS})
target_link_libraries(eeprom_app PRIVATE ${GPIODXX_LIBRARIES})
target_link_libraries(eeprom_app PRIVATE gpiod)