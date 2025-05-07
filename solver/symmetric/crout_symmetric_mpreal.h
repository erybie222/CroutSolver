#ifndef CROUT_SYMMETRIC_MPREAL_H
#define CROUT_SYMMETRIC_MPREAL_H

#include <QVector>
#include <tuple>
#include <mpreal.h>

std::tuple<QVector<QVector<mpfr::mpreal>>, QVector<QVector<mpfr::mpreal>>, QVector<mpfr::mpreal>, QVector<mpfr::mpreal>>
solveCroutSymmetric(const QVector<QVector<mpfr::mpreal>> &A, const QVector<mpfr::mpreal> &b);

#endif // CROUT_SYMMETRIC_MPREAL_H
