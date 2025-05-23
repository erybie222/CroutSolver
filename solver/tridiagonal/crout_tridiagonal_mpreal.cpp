#include "crout_tridiagonal_mpreal.h"
#include <tuple>
#include <mpreal.h>  // lub odpowiedni nagłówek mpfr::mpreal

namespace solver {
namespace tridiagonal {

std::tuple<
    QVector<mpreal>, // L (rozmiar n-1)
    QVector<mpreal>, // D (rozmiar n)
    QVector<mpreal>, // U (rozmiar n-1)
    QVector<mpreal>, // y (rozmiar n)
    QVector<mpreal>  // x (rozmiar n)
>
solveCroutTridiagonal(
    const QVector<mpreal> &a,    // podprzekątna, rozmiar n-1
    const QVector<mpreal> &b,    // przekątna, rozmiar n
    const QVector<mpreal> &c,    // nadprzekątna, rozmiar n-1
    const QVector<mpreal> &rhs)  // wektor prawej strony, rozmiar n
{
    int n = b.size();
    QVector<mpreal> L(n-1), D(n), U(n-1);
    QVector<mpreal> y(n), x(n);

    // 1) dekompozycja Crouta:
    D[0] = b[0];
    U[0] = c[0];
    for (int i = 1; i < n; ++i) {
        // **poprawka**: skalujemy podprzekątną przez poprzednie D
        L[i-1] = a[i-1] / D[i-1];
        D[i]   = b[i] - L[i-1] * c[i-1];
        if (i < n-1)
            U[i] = c[i];
    }

    // 2) forward substitution Ly = rhs
    y[0] = rhs[0];
    for (int i = 1; i < n; ++i) {
        y[i] = rhs[i] - L[i-1] * y[i-1];
    }

    // 3) back substitution Ux = y
    x[n-1] = y[n-1] / D[n-1];
    for (int i = n-2; i >= 0; --i) {
        x[i] = (y[i] - U[i] * x[i+1]) / D[i];
    }

    return {L, D, U, y, x};
}

} // namespace tridiagonal
} // namespace solver
