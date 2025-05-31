cmake_minimum_required(VERSION 3.16)
project(CroutSolver LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# pkg-config do MPFR i GMP
find_package(PkgConfig REQUIRED)
pkg_check_modules(MPFR REQUIRED IMPORTED_TARGET mpfr)
pkg_check_modules(GMP  REQUIRED IMPORTED_TARGET gmp)

# Qt6 
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# Boost (tylko nagłówki)
find_package(Boost REQUIRED)

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

# Ścieżki do własnych i zewnętrznych nagłówków
target_include_directories(CroutSolver PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${Boost_INCLUDE_DIRS}
    PkgConfig::MPFR
    PkgConfig::GMP
)

target_include_directories(CroutSolver PRIVATE
    ${CMAKE_SOURCE_DIR}/ścieżka/do/interval_rounding_fix
)

# Definicje wymagane przez MPFR
target_compile_definitions(CroutSolver PRIVATE
    MPFR_USE_NO_MACRO
    MPFR_USE_INTMAX_T
)

# Linkowanie
target_link_libraries(CroutSolver PRIVATE
    Qt6::Core Qt6::Gui Qt6::Widgets
    Boost::boost
    PkgConfig::MPFR
    PkgConfig::GMP
)
