#include "crout_tridiagonal_interval.h"

std::tuple<
    QVector<Interval<mpreal>>, // L
    QVector<Interval<mpreal>>, // D
    QVector<Interval<mpreal>>, // U
    QVector<Interval<mpreal>>, // y
    QVector<Interval<mpreal>>  // x
>
solveCroutTridiagonal(
    const QVector<Interval<mpreal>> &a,
    const QVector<Interval<mpreal>> &b,
    const QVector<Interval<mpreal>> &c,
    const QVector<Interval<mpreal>> &rhs)
{
    int n = b.size();
    QVector<Interval<mpreal>> L(n - 1), D(n), U(n - 1);
    QVector<Interval<mpreal>> y(n), x(n);

    // D[0]
    D[0] = b[0];
    U[0] = c[0] / D[0];

    // L[0] is unused, start from i = 1
    for (int i = 1; i < n - 1; ++i) {
        L[i - 1] = a[i - 1];
        D[i] = b[i] - L[i - 1] * U[i - 1];
        U[i] = c[i] / D[i];
    }

    L[n - 2] = a[n - 2];
    D[n - 1] = b[n - 1] - L[n - 2] * U[n - 2];

    // Forward substitution: Ly = rhs
    y[0] = rhs[0];
    for (int i = 1; i < n; ++i) {
        y[i] = rhs[i] - L[i - 1] * y[i - 1];
    }

    // Solve Dz = y
    for (int i = 0; i < n; ++i) {
        y[i] = y[i] / D[i];
    }

    // Backward substitution: Ux = z (we reuse y as z)
    x[n - 1] = y[n - 1];
    for (int i = n - 2; i >= 0; --i) {
        x[i] = y[i] - U[i] * x[i + 1];
    }

    return {L, D, U, y, x};
}
