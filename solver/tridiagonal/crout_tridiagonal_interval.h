#ifndef CROUT_TRIDIAGONAL_INTERVAL_H
#define CROUT_TRIDIAGONAL_INTERVAL_H

#include "interval.hpp"
#include <QVector>
#include <tuple>

using namespace mpfr;
using namespace interval_arithmetic;

std::tuple<
    QVector<Interval<mpreal>>, // L
    QVector<Interval<mpreal>>, // D
    QVector<Interval<mpreal>>, // U
    QVector<Interval<mpreal>>, // y
    QVector<Interval<mpreal>>  // x
>
solveCroutTridiagonal(
    const QVector<Interval<mpreal>> &a,
    const QVector<Interval<mpreal>> &b,
    const QVector<Interval<mpreal>> &c,
    const QVector<Interval<mpreal>> &rhs);

#endif // CROUT_TRIDIAGONAL_INTERVAL_H
