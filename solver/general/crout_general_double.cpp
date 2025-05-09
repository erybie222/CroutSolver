#include "crout_general_double.h"
namespace solver {
    namespace general {
    
std::tuple<QVector<QVector<double>>, QVector<QVector<double>>, QVector<double>, QVector<double>>
solveCroutGeneral(const QVector<QVector<double>> &A, const QVector<double> &b)
{
    int n = A.size();
    QVector<QVector<double>> L(n, QVector<double>(n, 0));
    QVector<QVector<double>> U(n, QVector<double>(n, 0));
    QVector<double> y(n), x(n);

    for (int i = 0; i < n; ++i)
    {
        L[i][i] = 1.0;
        for (int j = i; j < n; ++j)
        {
            double sum = 0;
            for (int k = 0; k < i; ++k)
                sum += L[i][k] * U[k][j];
            U[i][j] = A[i][j] - sum;
        }
        for (int j = i + 1; j < n; ++j)
        {
            double sum = 0;
            for (int k = 0; k < i; ++k)
                sum += L[j][k] * U[k][i];
            L[j][i] = (A[j][i] - sum) / U[i][i];
        }
    }

    for (int i = 0; i < n; ++i)
    {
        double sum = 0;
        for (int k = 0; k < i; ++k)
            sum += L[i][k] * y[k];
        y[i] = (b[i] - sum) / L[i][i];
    }

    for (int i = n - 1; i >= 0; --i)
    {
        double sum = 0;
        for (int k = i + 1; k < n; ++k)
            sum += U[i][k] * x[k];
        x[i] = (y[i] - sum) / U[i][i];
    }

    return {L, U, y, x};
}
    }
}