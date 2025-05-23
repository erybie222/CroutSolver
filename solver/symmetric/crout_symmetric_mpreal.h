#pragma once
#include <tuple>
#include <QVector>
#include <mpreal.h>

namespace solver {
namespace symmetric {

/**
 * Crout–LDLᵀ dla macierzy symetrycznej w precyzji mpfr::mpreal.
 * Zwraca (L, U, y, x), gdzie U = D·Lᵀ.
 */
std::tuple<
    QVector<QVector<mpfr::mpreal>>,  // L
    QVector<QVector<mpfr::mpreal>>,  // U
    QVector<mpfr::mpreal>,           // y
    QVector<mpfr::mpreal>            // x
>
solveCroutSymmetric(
    const QVector<QVector<mpfr::mpreal>>& A,
    const QVector<mpfr::mpreal>&         b
);

} // namespace symmetric
} // namespace solver
