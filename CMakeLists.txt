cmake_minimum_required(VERSION 3.21)
project(CurrencyConverter VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Qt 6.11.0 (MSVC 2022 64-bit) is the target; 6.4 is the lowest version the code is written against.
find_package(Qt6 6.4 REQUIRED COMPONENTS Widgets Network)
qt_standard_project_setup()

qt_add_executable(CurrencyConverter WIN32
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/WiseClient.cpp
    src/WiseClient.h
    src/Currency.h
    src/FallbackCurrencies.h
   app.rc
)

# qt_standard_project_setup() does not enable AUTORCC, so the resource is added explicitly.
qt_add_resources(CurrencyConverter "app_resources"
    PREFIX "/"
    BASE resources
    FILES resources/arrow-down.png
)

target_link_libraries(CurrencyConverter PRIVATE Qt6::Widgets Qt6::Network)

if(MSVC)
    # /utf-8: source files contain Chinese text and accented currency names.
    target_compile_options(CurrencyConverter PRIVATE /utf-8 /W4 /permissive-)
else()
    target_compile_options(CurrencyConverter PRIVATE -Wall -Wextra -Wpedantic)
endif()
