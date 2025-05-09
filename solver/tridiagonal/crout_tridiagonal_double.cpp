#include "crout_tridiagonal_double.h"
#include <stdexcept>
namespace solver {
    namespace tridiagonal {
std::tuple<
    QVector<double>, // a (subdiagonal)
    QVector<double>, // b (main diagonal)
    QVector<double>, // c (superdiagonal)
    QVector<double>, // y (Ly = b)
    QVector<double>  // x (Ux = y)
>
solveCroutTridiagonal(const QVector<double> &a_in, const QVector<double> &b_in, const QVector<double> &c_in, const QVector<double> &rhs)
{
    int n = b_in.size();
    if (a_in.size() != n - 1 || c_in.size() != n - 1 || rhs.size() != n)
        throw std::invalid_argument("Invalid vector sizes");

    QVector<double> L(n);
    QVector<double> U(n);
    QVector<double> a = a_in;
    QVector<double> b = b_in;
    QVector<double> c = c_in;

    QVector<double> y(n), x(n);

    // Crout's LU decomposition for tridiagonal matrix
    L[0] = b[0];
    U[0] = c[0] / L[0];

    for (int i = 1; i < n - 1; ++i)
    {
        L[i] = b[i] - a[i - 1] * U[i - 1];
        U[i] = c[i] / L[i];
    }
    L[n - 1] = b[n - 1] - a[n - 2] * U[n - 2];

    // Forward substitution (Ly = b)
    y[0] = rhs[0] / L[0];
    for (int i = 1; i < n; ++i)
        y[i] = (rhs[i] - a[i - 1] * y[i - 1]) / L[i];

    // Backward substitution (Ux = y)
    x[n - 1] = y[n - 1];
    for (int i = n - 2; i >= 0; --i)
        x[i] = y[i] - U[i] * x[i + 1];

    return {L, b, c, y, x};
}
    }
}