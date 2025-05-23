#include "crout_symmetric_interval.h"
#include <stdexcept>

namespace solver {
namespace symmetric {

auto solveCroutSymmetric(
    const QVector<QVector<I>>& A,
    const QVector<I>&          b
) -> std::tuple<
         QVector<QVector<I>>,
         QVector<QVector<I>>,
         QVector<I>,
         QVector<I>
     >
{
    int n = A.size();
    if (b.size() != n)
        throw std::invalid_argument("Vector size does not match matrix dimension.");

    QVector<QVector<I>> L(n, QVector<I>(n));
    QVector<I> D(n);
    QVector<QVector<I>> U(n, QVector<I>(n));
    QVector<I> y(n), x(n);

    // Crout–LDLᵀ
    for (int j = 0; j < n; ++j) {
        I sum = A[j][j];
        for (int k = 0; k < j; ++k)
            sum = sum - (L[j][k] * D[k] * L[j][k]);
        D[j] = sum;
        // pivot = [0;0]?
        if (D[j].a == mpfr::mpreal(0) && D[j].b == mpfr::mpreal(0))
            throw std::runtime_error("Zero pivot in interval LDLᵀ");

        L[j][j] = I(mpfr::mpreal(1), mpfr::mpreal(1));
        for (int i = j + 1; i < n; ++i) {
            sum = A[i][j];
            for (int k = 0; k < j; ++k)
                sum = sum - (L[i][k] * D[k] * L[j][k]);
            L[i][j] = sum / D[j];
        }
    }

    // U = D * Lᵀ
    for (int i = 0; i < n; ++i)
        for (int j = i; j < n; ++j)
            U[i][j] = D[i] * L[j][i];

    // forward: L·y = b
    for (int i = 0; i < n; ++i) {
        I s = b[i];
        for (int k = 0; k < i; ++k)
            s = s - (L[i][k] * y[k]);
        y[i] = s;  // L[i][i] == 1
    }

    // middle: D·z = y
    QVector<I> z(n);
    for (int i = 0; i < n; ++i)
        z[i] = y[i] / D[i];

    // backward: Lᵀ·x = z
    for (int i = n - 1; i >= 0; --i) {
        I s = z[i];
        for (int k = i + 1; k < n; ++k)
            s = s - (L[k][i] * x[k]);
        x[i] = s;  // L[i][i] == 1
    }

    return {L, U, y, x};
}

} // namespace symmetric
} // namespace solver
