#include "crout_tridiagonal_interval.h"
#include "interval.hpp"        // upewnij się, że definiuje Interval<mpreal>
#include <tuple>

namespace solver {
namespace tridiagonal {

std::tuple<
    QVector<Interval<mpreal>>, // L (rozmiar n-1)
    QVector<Interval<mpreal>>, // D (rozmiar n)
    QVector<Interval<mpreal>>, // U (rozmiar n-1)
    QVector<Interval<mpreal>>, // y (rozmiar n)
    QVector<Interval<mpreal>>  // x (rozmiar n)
>
solveCroutTridiagonal(
    const QVector<Interval<mpreal>> &a,   // podprzekątna, rozmiar n-1
    const QVector<Interval<mpreal>> &b,   // przekątna, rozmiar n
    const QVector<Interval<mpreal>> &c,   // nadprzekątna, rozmiar n-1
    const QVector<Interval<mpreal>> &rhs) // wektor prawej strony, rozmiar n
{
    int n = b.size();
    QVector<Interval<mpreal>> L(n-1), D(n), U(n-1);
    QVector<Interval<mpreal>> y(n), x(n);

    // 1) dekompozycja Crouta:
    D[0] = b[0];
    U[0] = c[0] / D[0];

    for (int i = 1; i < n-1; ++i) {
        // **TU popra­wka**: dzielimy przez poprzedni D
        L[i-1] = a[i-1] / D[i-1];
        D[i]   = b[i] - L[i-1] * U[i-1];
        U[i]   = c[i] / D[i];
    }

    // ostatni element
    L[n-2]   = a[n-2] / D[n-2];
    D[n-1]   = b[n-1] - L[n-2] * U[n-2];

    // 2) forward substitution Ly = rhs
    y[0] = rhs[0];
    for (int i = 1; i < n; ++i) {
        y[i] = rhs[i] - L[i-1] * y[i-1];
    }

    // 3) rozwiązanie Dz = y  (zastępujemy y)
    for (int i = 0; i < n; ++i) {
        y[i] = y[i] / D[i];
    }

    // 4) backward substitution Ux = z
    x[n-1] = y[n-1];
    for (int i = n-2; i >= 0; --i) {
        x[i] = y[i] - U[i] * x[i+1];
    }

    return {L, D, U, y, x};
}

} // namespace tridiagonal
} // namespace solver
