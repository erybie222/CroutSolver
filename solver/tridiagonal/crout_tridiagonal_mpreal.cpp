#include "crout_tridiagonal_mpreal.h"
namespace solver {
    namespace tridiagonal {
std::tuple<
    QVector<mpreal>,
    QVector<mpreal>,
    QVector<mpreal>,
    QVector<mpreal>,
    QVector<mpreal>
> solveCroutTridiagonal(
    const QVector<mpreal> &a,
    const QVector<mpreal> &b,
    const QVector<mpreal> &c,
    const QVector<mpreal> &rhs)
{
    int n = b.size();
    QVector<mpreal> L(n), D(n), U(n - 1);
    QVector<mpreal> y(n), x(n);

    // Dla Crouta: A = L * U, przy czym L ma jedynki na przekątnej.
    // D[i] to przekątna U, L[i] to współczynniki podprzekątnej

    D[0] = b[0];
    U[0] = c[0];
    for (int i = 1; i < n; ++i)
    {
        L[i] = a[i - 1] / D[i - 1];
        D[i] = b[i] - L[i] * c[i - 1];
        if (i < n - 1)
            U[i] = c[i];
    }

    // Rozwiązanie Ly = rhs (L ma jedynki na przekątnej)
    y[0] = rhs[0];
    for (int i = 1; i < n; ++i)
        y[i] = rhs[i] - L[i] * y[i - 1];

    // Rozwiązanie Ux = y
    x[n - 1] = y[n - 1] / D[n - 1];
    for (int i = n - 2; i >= 0; --i)
        x[i] = (y[i] - U[i] * x[i + 1]) / D[i];

    return {L, D, U, y, x};
}
    }
}