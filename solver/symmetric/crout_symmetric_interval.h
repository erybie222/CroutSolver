#ifndef CROUT_SYMMETRIC_INTERVAL_H
#define CROUT_SYMMETRIC_INTERVAL_H

#include <QVector>
#include <tuple>
#include "interval.hpp"

using namespace interval_arithmetic;

std::tuple<
    QVector<QVector<Interval<mpfr::mpreal>>>,
    QVector<QVector<Interval<mpfr::mpreal>>>,
    QVector<Interval<mpfr::mpreal>>,
    QVector<Interval<mpfr::mpreal>>
>
solveCroutSymmetric(const QVector<QVector<Interval<mpfr::mpreal>>> &A, const QVector<Interval<mpfr::mpreal>> &b);

#endif // CROUT_SYMMETRIC_INTERVAL_H
