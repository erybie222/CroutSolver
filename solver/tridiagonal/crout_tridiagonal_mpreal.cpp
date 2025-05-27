#include "crout_tridiagonal_mpreal.h"
#include <tuple>
#include <mpreal.h>  // lub odpowiedni nagłówek mpfr::mpreal

namespace solver {
namespace tridiagonal {

std::tuple<
    QVector<mpreal>, // L (n-1)
    QVector<mpreal>, // D (n)
    QVector<mpreal>, // U (n-1)
    QVector<mpreal>, // y (n)
    QVector<mpreal>  // x (n)
>
solveCroutTridiagonal(
    const QVector<mpreal> &a,
    const QVector<mpreal> &b,
    const QVector<mpreal> &c,
    const QVector<mpreal> &rhs)
{
    int n = b.size();
    QVector<mpreal> L(n-1), D(n), U(n-1);
    QVector<mpreal> y(n), x(n);

    // 1) dekompozycja Crouta:
    D[0] = b[0];
    U[0] = c[0];

    // jeżeli już na samym początku przekątna jest zero => osobliwy
    if (D[0] == 0) {
        // zapełniamy y i x NaN-ami,
        // następnie w solveSystem() wyłapiesz NaN i ustawisz st=3
        for (auto &v : y) v = std::numeric_limits<mpreal>::quiet_NaN();
        for (auto &v : x) v = std::numeric_limits<mpreal>::quiet_NaN();
        return {L, D, U, y, x};
    }

    for (int i = 1; i < n; ++i) {
        // Crout: L[i-1] = a[i-1]/D[i-1]
        L[i-1] = a[i-1] / D[i-1];
        // obliczamy kolejny D[i]
        D[i]   = b[i] - L[i-1] * c[i-1];

        // TU WSTAWIAMY SPRAWDZENIE, CZY PIVOT JEST ZERO:
        if (D[i] == 0) {
            // macierz jest osobliwa
            for (auto &v : y) v = std::numeric_limits<mpreal>::quiet_NaN();
            for (auto &v : x) v = std::numeric_limits<mpreal>::quiet_NaN();
            return {L, D, U, y, x};
        }

        if (i < n-1)
            U[i] = c[i];
    }

    // 2) forward substitution Ly = rhs
    y[0] = rhs[0];
    for (int i = 1; i < n; ++i) {
        y[i] = rhs[i] - L[i-1] * y[i-1];
    }

    // 3) back substitution Dx = y (tutaj nazwane D i U ale
    //    w tej wersji U to tylko nadprzekątna, a D pełni rolę głównej)
    x[n-1] = y[n-1] / D[n-1];
    for (int i = n-2; i >= 0; --i) {
        x[i] = (y[i] - U[i] * x[i+1]) / D[i];
    }

    return {L, D, U, y, x};
}


} // namespace tridiagonal
} // namespace solver
