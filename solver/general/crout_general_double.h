#pragma once
#include <tuple>
#include <QVector>
#include <stdexcept>

namespace solver {
namespace general {

/**
 * Rozwiązuje A x = b metodą Crout–Doolittle (A = L·U, L[i][i]=1).
 * Zwraca krotkę (L, U, y, x):
 *   L·y = b,
 *   U·x = y.
 */
std::tuple<
    QVector<QVector<double>>,
    QVector<QVector<double>>,
    QVector<double>,
    QVector<double>
>
solveCroutGeneral(const QVector<QVector<double>> &A,
                  const QVector<double>         &b);

} // namespace general
} // namespace solver
