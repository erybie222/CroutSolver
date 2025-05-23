#pragma once
#include <tuple>
#include <QVector>

namespace solver {
namespace symmetric {

/**
 * Rozkład LDLᵀ dla macierzy symetrycznej (niekoniecznie SPD).
 * Zwraca (L, U=D·Lᵀ, y, x).
 */
std::tuple<
    QVector<QVector<double>>,
    QVector<QVector<double>>,
    QVector<double>,
    QVector<double>
>
solveCroutSymmetric(const QVector<QVector<double>> &A,
                    const QVector<double>         &b);

} // namespace symmetric
} // namespace solver
