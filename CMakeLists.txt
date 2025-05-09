cmake_minimum_required(VERSION 3.16)
project(CroutSolver)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui)

# Główne źródła
add_executable(CroutSolver
    main.cpp
    mainwindow.cpp
    mainwindow.h
    crout_solver.hpp
    interval.hpp
    mpreal.h

    solver/general/crout_general_double.cpp
    solver/general/crout_general_mpreal.cpp
    solver/general/crout_general_interval.cpp

    solver/symmetric/crout_symmetric_double.cpp
    solver/symmetric/crout_symmetric_mpreal.cpp
    solver/symmetric/crout_symmetric_interval.cpp

    solver/tridiagonal/crout_tridiagonal_double.cpp
    solver/tridiagonal/crout_tridiagonal_mpreal.cpp
    solver/tridiagonal/crout_tridiagonal_interval.cpp
)

# Ścieżki do własnych plików nagłówkowych
target_include_directories(CroutSolver PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Linkowanie bibliotek
target_link_libraries(CroutSolver
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    mpfr
    gmp
)

add_definitions(-DMPFR_USE_NO_MACRO -DMPFR_USE_INTMAX_T)
