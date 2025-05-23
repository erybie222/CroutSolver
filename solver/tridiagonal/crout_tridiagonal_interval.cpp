#pragma once
#include <tuple>
#include <QVector>
#include "interval.hpp"   // Interval<mpfr::mpreal>
#include <stdexcept>

namespace solver {
namespace tridiagonal {

// Returns L (subdiag, length n-1), D (diag, length n), U (supdiag, length n-1), y, x
std::tuple<
    QVector<interval_arithmetic::Interval<mpfr::mpreal>>,
    QVector<interval_arithmetic::Interval<mpfr::mpreal>>,
    QVector<interval_arithmetic::Interval<mpfr::mpreal>>,
    QVector<interval_arithmetic::Interval<mpfr::mpreal>>,
    QVector<interval_arithmetic::Interval<mpfr::mpreal>>
>
solveCroutTridiagonal(
    const QVector<interval_arithmetic::Interval<mpfr::mpreal>> &a,
    const QVector<interval_arithmetic::Interval<mpfr::mpreal>> &b,
    const QVector<interval_arithmetic::Interval<mpfr::mpreal>> &c,
    const QVector<interval_arithmetic::Interval<mpfr::mpreal>> &rhs)
{
    int n = b.size();
    if (a.size() != n-1 || c.size() != n-1 || rhs.size() != n)
        throw std::invalid_argument("Invalid sizes in tridiagonal interval");

    using I = interval_arithmetic::Interval<mpfr::mpreal>;
    QVector<I> L(n-1), D(n), U(n-1), y(n), x(n);

    // Crout decomposition (identycznie jak w mpreal, tylko na przedziałach)
    D[0] = b[0];
    U[0] = c[0];
    for (int i = 1; i < n; ++i) {
        L[i-1] = a[i-1] / D[i-1];               // <-- TU była pomyłka
        D[i]   = b[i] - L[i-1] * c[i-1];
        if (i < n-1)
            U[i] = c[i];
    }

    // forward substitution Ly = rhs
    y[0] = rhs[0];
    for (int i = 1; i < n; ++i) {
        y[i] = rhs[i] - L[i-1] * y[i-1];
    }

    // back substitution Ux = y (obie strony dzielone przez D w trakcie Ux=y/D)
    x[n-1] = y[n-1] / D[n-1];
    for (int i = n-2; i >= 0; --i) {
        x[i] = (y[i] - U[i] * x[i+1]) / D[i];
    }

    return {L, D, U, y, x};
}

} // namespace tridiagonal
} // namespace solver
