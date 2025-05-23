#pragma once
#include <tuple>
#include <QVector>
#include "interval.hpp"

namespace solver {
namespace symmetric {

using I = interval_arithmetic::Interval<mpfr::mpreal>;

/**
 * Crout–LDLᵀ dla macierzy symetrycznej w precyzji przedziałowej.
 * Zwraca (L, U, y, x), gdzie U = D·Lᵀ.
 */
std::tuple<
    QVector<QVector<I>>,  // L
    QVector<QVector<I>>,  // U
    QVector<I>,           // y
    QVector<I>            // x
>
solveCroutSymmetric(
    const QVector<QVector<I>>& A,
    const QVector<I>&          b
);

} // namespace symmetric
} // namespace solver
