#ifndef CROUT_GENERAL_INTERVAL_H
#define CROUT_GENERAL_INTERVAL_H

#include <QVector>
#include <tuple>
#include "interval.hpp"
#include "mpreal.h"

using namespace mpfr;
using namespace interval_arithmetic;
namespace solver {
    namespace general {
    
std::tuple<QVector<QVector<Interval<mpreal>>>, QVector<QVector<Interval<mpreal>>>, QVector<Interval<mpreal>>, QVector<Interval<mpreal>>>
solveCroutGeneral(const QVector<QVector<Interval<mpreal>>> &A, const QVector<Interval<mpreal>> &b);
   }
}
#endif // CROUT_GENERAL_INTERVAL_H
 