#include "crout_symmetric_double.h"
#include <stdexcept>

namespace solver {
namespace symmetric {

std::tuple<QVector<QVector<double>>,
           QVector<QVector<double>>,
           QVector<double>,
           QVector<double>>
solveCroutSymmetric(const QVector<QVector<double>> &A,
                    const QVector<double>         &b)
{
    int n = A.size();
    if (b.size() != n)
        throw std::invalid_argument("Vector size does not match matrix dimension.");

    // L: dolna trójkątna z jedynkami na diag., D: diag vector, U = D * Lᵀ
    QVector<QVector<double>> L(n, QVector<double>(n, 0.0));
    QVector<double>         D(n, 0.0);
    QVector<QVector<double>> U(n, QVector<double>(n, 0.0));
    QVector<double> y(n), x(n);

    // Crout–LDLᵀ
    for (int j = 0; j < n; ++j) {
        // obliczamy D[j] i kolumnę L[i][j], i>=j
        double sum;
        // najpierw D[j] = A[j][j] - Σ_{k<j} L[j][k]*D[k]*L[j][k]
        sum = A[j][j];
        for (int k = 0; k < j; ++k) {
            sum -= L[j][k] * D[k] * L[j][k];
        }
        D[j] = sum;
        if (D[j] == 0.0)
            throw std::runtime_error("Zero pivot in LDLT decomposition");

        // pozostałe wiersze i kolumny L[i][j] = (A[i][j] - Σ L[i][k]*D[k]*L[j][k]) / D[j]
        L[j][j] = 1.0; // schemat unit lower
        for (int i = j+1; i < n; ++i) {
            sum = A[i][j];
            for (int k = 0; k < j; ++k) {
                sum -= L[i][k] * D[k] * L[j][k];
            }
            L[i][j] = sum / D[j];
        }
    }

    // Budujemy U = D * Lᵀ
    for (int i = 0; i < n; ++i) {
        for (int j = i; j < n; ++j) {
            U[i][j] = D[i] * L[j][i];
        }
    }

    // Forward: L * y = b
    for (int i = 0; i < n; ++i) {
        double s = b[i];
        for (int k = 0; k < i; ++k) {
            s -= L[i][k] * y[k];
        }
        // L[i][i] == 1
        y[i] = s;
    }
    // Middle: D * z = y  (z = Lᵀ x)
    QVector<double> z(n);
    for (int i = 0; i < n; ++i) {
        z[i] = y[i] / D[i];
    }
    // Backward: Lᵀ * x = z
    for (int i = n-1; i >= 0; --i) {
        double s = z[i];
        for (int k = i+1; k < n; ++k) {
            s -= L[k][i] * x[k];
        }
        // L[i][i] == 1 in Lᵀ too
        x[i] = s;
    }

    return {L, U, y, x};
}

} // namespace symmetric
} // namespace solver
