cmake_minimum_required(VERSION 3.10)

# Nazwa projektu
project(CroutSolver LANGUAGES CXX)

# Ustaw standard C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Znajdź Qt5 z komponentem Widgets
find_package(Qt5 REQUIRED COMPONENTS Widgets)

# Włącz automatyczne przetwarzanie MOC, UIC i RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Dodaj pliki źródłowe
set(SOURCES
    main.cpp
    mainwindow.cpp
)

set(HEADERS
    mainwindow.h
)

# Utwórz plik wykonywalny
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
)

# Powiąż Qt5::Widgets z naszym programem
target_link_libraries(${PROJECT_NAME} Qt5::Widgets)
