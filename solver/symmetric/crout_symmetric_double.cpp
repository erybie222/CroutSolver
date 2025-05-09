#include "crout_symmetric_double.h"
namespace solver {
    namespace symmetric {
    
std::tuple<QVector<QVector<double>>, QVector<QVector<double>>, QVector<double>, QVector<double>>
solveCroutSymmetric(const QVector<QVector<double>> &A, const QVector<double> &b)
{
    int n = A.size();
    QVector<QVector<double>> L(n, QVector<double>(n, 0));
    QVector<QVector<double>> U(n, QVector<double>(n, 0));
    QVector<double> y(n), x(n);

    // Symetryczna wersja Crouta (macierz symetryczna, więc U = L^T)
    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j <= i; ++j)
        {
            double sum = 0;
            for (int k = 0; k < j; ++k)
                sum += L[i][k] * L[j][k];

            if (i == j)
                L[i][j] = std::sqrt(A[i][i] - sum);  // wymaga dodatniej określoności
            else
                L[i][j] = (A[i][j] - sum) / L[j][j];

            U[j][i] = L[i][j]; // L transponowane
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