#include "crout_symmetric_mpreal.h"

using namespace mpfr;
namespace solver {
    namespace symmetric {
std::tuple<QVector<QVector<mpreal>>, QVector<QVector<mpreal>>, QVector<mpreal>, QVector<mpreal>>
solveCroutSymmetric(const QVector<QVector<mpreal>> &A, const QVector<mpreal> &b)
{
    int n = A.size();
    QVector<QVector<mpreal>> L(n, QVector<mpreal>(n));
    QVector<QVector<mpreal>> U(n, QVector<mpreal>(n));
    QVector<mpreal> y(n), x(n);

    for (int i = 0; i < n; ++i)
    {
        L[i][i] = 1;
        for (int j = i; j < n; ++j)
        {
            mpreal sum = 0;
            for (int k = 0; k < i; ++k)
                sum += L[i][k] * U[k][j];
            U[i][j] = A[i][j] - sum;
        }
        for (int j = i + 1; j < n; ++j)
        {
            mpreal sum = 0;
            for (int k = 0; k < i; ++k)
                sum += L[j][k] * U[k][i];
            L[j][i] = (A[j][i] - sum) / U[i][i];
        }
    }

    for (int i = 0; i < n; ++i)
    {
        mpreal sum = 0;
        for (int k = 0; k < i; ++k)
            sum += L[i][k] * y[k];
        y[i] = (b[i] - sum) / L[i][i];
    }

    for (int i = n - 1; i >= 0; --i)
    {
        mpreal sum = 0;
        for (int k = i + 1; k < n; ++k)
            sum += U[i][k] * x[k];
        x[i] = (y[i] - sum) / U[i][i];
    }

    return {L, U, y, x};
}
    }
}