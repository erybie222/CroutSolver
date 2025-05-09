#include "crout_symmetric_interval.h"
namespace solver {
    namespace symmetric {
    
std::tuple<
    QVector<QVector<Interval<mpfr::mpreal>>>,
    QVector<QVector<Interval<mpfr::mpreal>>>,
    QVector<Interval<mpfr::mpreal>>,
    QVector<Interval<mpfr::mpreal>>
>
solveCroutSymmetric(const QVector<QVector<Interval<mpfr::mpreal>>> &A, const QVector<Interval<mpfr::mpreal>> &b)
{
    using Interval = Interval<mpfr::mpreal>;
    int n = A.size();

    QVector<QVector<Interval>> L(n, QVector<Interval>(n));
    QVector<QVector<Interval>> U(n, QVector<Interval>(n));
    QVector<Interval> y(n), x(n);

    for (int i = 0; i < n; ++i)
    {
        L[i][i] = Interval(1, 1);
        for (int j = i; j < n; ++j)
        {
            Interval sum(0, 0);
            for (int k = 0; k < i; ++k)
                sum = sum + L[i][k] * U[k][j];
            U[i][j] = A[i][j] - sum;
            U[j][i] = U[i][j]; // Symmetry
        }

        for (int j = i + 1; j < n; ++j)
        {
            Interval sum(0, 0);
            for (int k = 0; k < i; ++k)
                sum = sum + L[j][k] * U[k][i];
            L[j][i] = (A[j][i] - sum) / U[i][i];
        }
    }

    for (int i = 0; i < n; ++i)
    {
        Interval sum(0, 0);
        for (int k = 0; k < i; ++k)
            sum = sum + L[i][k] * y[k];
        y[i] = (b[i] - sum) / L[i][i];
    }

    for (int i = n - 1; i >= 0; --i)
    {
        Interval sum(0, 0);
        for (int k = i + 1; k < n; ++k)
            sum = sum + U[i][k] * x[k];
        x[i] = (y[i] - sum) / U[i][i];
    }

    return {L, U, y, x};
}
}
}
