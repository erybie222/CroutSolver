#include "crout_general_mpreal.h"
#include <stdexcept>

namespace solver {
namespace general {

auto solveCroutGeneral(
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
    QVector<QVector<mpfr::mpreal>> U(n, QVector<mpfr::mpreal>(n, 0));
    QVector<mpfr::mpreal> y(n), x(n);

    // --- dekompozycja Crout ---
    for (int i = 0; i < n; ++i) {
        L[i][i] = 1;
        // U[i][j]
        for (int j = i; j < n; ++j) {
            mpfr::mpreal sum = 0;
            for (int k = 0; k < i; ++k)
                sum += L[i][k] * U[k][j];
            U[i][j] = A[i][j] - sum;
        }
        // L[j][i]
        for (int j = i + 1; j < n; ++j) {
            mpfr::mpreal sum = 0;
            for (int k = 0; k < i; ++k)
                sum += L[j][k] * U[k][i];
            if (U[i][i] == 0)
                throw std::runtime_error("Zero pivot in Crout general mpreal");
            L[j][i] = (A[j][i] - sum) / U[i][i];
        }
    }

    // --- podstawianie przód (L·y = b) ---
    for (int i = 0; i < n; ++i) {
        mpfr::mpreal sum = 0;
        for (int k = 0; k < i; ++k)
            sum += L[i][k] * y[k];
        y[i] = (b[i] - sum) / L[i][i];
    }

    // --- podstawianie tył (U·x = y) ---
    for (int i = n - 1; i >= 0; --i) {
        mpfr::mpreal sum = 0;
        for (int k = i + 1; k < n; ++k)
            sum += U[i][k] * x[k];
        if (U[i][i] == 0)
            throw std::runtime_error("Zero pivot in Crout general mpreal");
        x[i] = (y[i] - sum) / U[i][i];
    }

    return {L, U, y, x};
}

} // namespace general
} // namespace solver
