#ifndef CROUT_TRIDIAGONAL_MPREAL_H
#define CROUT_TRIDIAGONAL_MPREAL_H

#include <QVector>
#include <tuple>
#include "mpreal.h"

using namespace mpfr;
namespace solver {
    namespace tridiagonal {
std::tuple<
    QVector<mpreal>, // L (subdiagonal with diagonal elements)
    QVector<mpreal>, // b (main diagonal)
    QVector<mpreal>, // c (superdiagonal)
    QVector<mpreal>, // y (Ly = b)
    QVector<mpreal>  // x (Ux = y)
>
solveCroutTridiagonal(
    const QVector<mpreal> &a,  // subdiagonal (n - 1)
    const QVector<mpreal> &b,  // main diagonal (n)
    const QVector<mpreal> &c,  // superdiagonal (n - 1)
    const QVector<mpreal> &rhs // right-hand side (n)
);
 }
}
#endif // CROUT_TRIDIAGONAL_MPREAL_H
   