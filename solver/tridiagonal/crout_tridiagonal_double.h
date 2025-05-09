#ifndef CROUT_TRIDIAGONAL_DOUBLE_H
#define CROUT_TRIDIAGONAL_DOUBLE_H

#include <QVector>
#include <tuple>
namespace solver {
    namespace tridiagonal {
std::tuple<
    QVector<double>, // a (subdiagonal)
    QVector<double>, // b (main diagonal)
    QVector<double>, // c (superdiagonal)
    QVector<double>, // y (Ly = b)
    QVector<double>  // x (Ux = y)
>
solveCroutTridiagonal(const QVector<double> &a, const QVector<double> &b, const QVector<double> &c, const QVector<double> &rhs);
  }
}
#endif // CROUT_TRIDIAGONAL_DOUBLE_H
  