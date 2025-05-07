#ifndef CROUT_GENERAL_MPREAL_H
#define CROUT_GENERAL_MPREAL_H

#include <QVector>
#include <tuple>
#include "mpreal.h"

using namespace mpfr;

std::tuple<QVector<QVector<mpreal>>, QVector<QVector<mpreal>>, QVector<mpreal>, QVector<mpreal>>
solveCroutGeneral(const QVector<QVector<mpreal>> &A, const QVector<mpreal> &b);

#endif // CROUT_GENERAL_MPREAL_H
