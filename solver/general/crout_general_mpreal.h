#pragma once
#include <tuple>
#include <QVector>
#include <mpreal.h>

namespace solver {
namespace general {

/**
 * Crout (LU) dla dowolnej macierzy w precyzji mpfr::mpreal.
 * Zwraca (L, U, y, x).
 */
std::tuple<
    QVector<QVector<mpfr::mpreal>>,  // L
    QVector<QVector<mpfr::mpreal>>,  // U
    QVector<mpfr::mpreal>,           // y
    QVector<mpfr::mpreal>            // x
>
solveCroutGeneral(
    const QVector<QVector<mpfr::mpreal>>& A,
    const QVector<mpfr::mpreal>&         b
);

} // namespace general
} // namespace solver
