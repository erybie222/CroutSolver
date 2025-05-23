#include "crout_general_double.h"

namespace solver {
namespace general {

std::tuple<QVector<QVector<double>>,
           QVector<QVector<double>>,
           QVector<double>,
           QVector<double>>
solveCroutGeneral(const QVector<QVector<double>> &A,
                  const QVector<double>         &b)
{
    int n = A.size();
    if (b.size() != n)
        throw std::invalid_argument("Vector size does not match matrix dimension.");

    QVector<QVector<double>> L(n, QVector<double>(n, 0.0));
    QVector<QVector<double>> U(n, QVector<double>(n, 0.0));
    QVector<double> y(n), x(n);

    // Crout–Doolittle
    for (int i = 0; i < n; ++i) {
        L[i][i] = 1.0;
        // U row
        for (int j = i; j < n; ++j) {
            double sum = 0;
            for (int k = 0; k < i; ++k) sum += L[i][k] * U[k][j];
            U[i][j] = A[i][j] - sum;
        }
        // L column
        for (int j = i+1; j < n; ++j) {
            double sum = 0;
            for (int k = 0; k < i; ++k) sum += L[j][k] * U[k][i];
            if (U[i][i] == 0.0) throw std::runtime_error("Zero pivot");
            L[j][i] = (A[j][i] - sum) / U[i][i];
        }
    }

    // forward: L·y = b
    for (int i = 0; i < n; ++i) {
        double sum = 0;
        for (int k = 0; k < i; ++k) sum += L[i][k] * y[k];
        y[i] = b[i] - sum; // L[i][i]==1
    }
    // back: U·x = y
    for (int i = n-1; i >= 0; --i) {
        double sum = 0;
        for (int k = i+1; k < n; ++k) sum += U[i][k] * x[k];
        if (U[i][i] == 0.0) throw std::runtime_error("Zero pivot");
        x[i] = (y[i] - sum) / U[i][i];
    }

    return {L, U, y, x};
}

} // namespace general
} // namespace solver
