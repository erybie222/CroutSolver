#include "crout_symmetric_double.h"
#include <QVector>
#include <stdexcept>

namespace solver {
namespace symmetric {

std::tuple<
    QVector<QVector<double>>, // L
    QVector<QVector<double>>, // U
    QVector<double>,          // y
    QVector<double>           // x
>
solveCroutSymmetric(const QVector<QVector<double>> &A, const QVector<double> &b)
{
    const int n = A.size();
    if (b.size() != n)
        throw std::invalid_argument("Vector size does not match matrix dimension.");

    QVector<QVector<double>> L(n, QVector<double>(n, 0.0));
    QVector<QVector<double>> U(n, QVector<double>(n, 0.0));
    QVector<double> y(n), x(n);

    // Crout's decomposition for symmetric matrix A = LU (U is transpose of L)
    for (int j = 0; j < n; ++j) {
        for (int i = j; i < n; ++i) {
            double sum = A[i][j];
            for (int k = 0; k < j; ++k)
                sum -= L[i][k] * L[j][k];
            L[i][j] = sum / L[j][j == 0 ? 0 : j];
        }
        L[j][j] = std::sqrt(A[j][j] - [&]() {
            double s = 0.0;
            for (int k = 0; k < j; ++k)
                s += L[j][k] * L[j][k];
            return s;
        }());
    }

    // U = L^T
    for (int i = 0; i < n; ++i)
        for (int j = 0; j <= i; ++j)
            U[j][i] = L[i][j];

    // Forward substitution: L * y = b
    for (int i = 0; i < n; ++i) {
        double sum = b[i];
        for (int j = 0; j < i; ++j)
            sum -= L[i][j] * y[j];
        y[i] = sum / L[i][i];
    }

    // Backward substitution: U * x = y
    for (int i = n - 1; i >= 0; --i) {
        double sum = y[i];
        for (int j = i + 1; j < n; ++j)
            sum -= U[i][j] * x[j];
        x[i] = sum / U[i][i];
    }

    return {L, U, y, x};
}

} // namespace symmetric
} // namespace solver
