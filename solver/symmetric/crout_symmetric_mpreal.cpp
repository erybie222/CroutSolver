#include "crout_symmetric_mpreal.h"
#include <stdexcept>

namespace solver {
namespace symmetric {

auto solveCroutSymmetric(
    const QVector<QVector<mpfr::mpreal>>& A,
    const QVector<mpfr::mpreal>&         b
) -> std::tuple<
         QVector<QVector<mpfr::mpreal>>,
         QVector<QVector<mpfr::mpreal>>,
         QVector<mpfr::mpreal>,
         QVector<mpfr::mpreal>
     >
{
    int n = A.size();
    if (b.size() != n)
        throw std::invalid_argument("Vector size does not match matrix dimension.");

    QVector<QVector<mpfr::mpreal>> L(n, QVector<mpfr::mpreal>(n, 0));
    QVector<mpfr::mpreal> D(n, 0);
    QVector<QVector<mpfr::mpreal>> U(n, QVector<mpfr::mpreal>(n, 0));
    QVector<mpfr::mpreal> y(n), x(n);

    // Crout–LDLᵀ
    for (int j = 0; j < n; ++j) {
        // D[j]
        mpfr::mpreal sum = A[j][j];
        for (int k = 0; k < j; ++k)
            sum -= L[j][k] * D[k] * L[j][k];
        D[j] = sum;
        if (D[j] == 0)
            throw std::runtime_error("Zero pivot in LDLᵀ decomposition");

        L[j][j] = 1;
        // L[i][j], i>j
        for (int i = j + 1; i < n; ++i) {
            sum = A[i][j];
            for (int k = 0; k < j; ++k)
                sum -= L[i][k] * D[k] * L[j][k];
            L[i][j] = sum / D[j];
        }
    }

    // U = D * Lᵀ
    for (int i = 0; i < n; ++i)
        for (int j = i; j < n; ++j)
            U[i][j] = D[i] * L[j][i];

    // forward: L·y = b
    for (int i = 0; i < n; ++i) {
        mpfr::mpreal s = b[i];
        for (int k = 0; k < i; ++k)
            s -= L[i][k] * y[k];
        y[i] = s;  // L[i][i] == 1
    }

    // middle: D·z = y
    QVector<mpfr::mpreal> z(n);
    for (int i = 0; i < n; ++i)
        z[i] = y[i] / D[i];

    // backward: Lᵀ·x = z
    for (int i = n - 1; i >= 0; --i) {
        mpfr::mpreal s = z[i];
        for (int k = i + 1; k < n; ++k)
            s -= L[k][i] * x[k];
        x[i] = s;  // L[i][i] == 1
    }

    return {L, U, y, x};
}

} // namespace symmetric
} // namespace solver
