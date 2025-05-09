#ifndef CROUT_SYMMETRIC_DOUBLE_H
#define CROUT_SYMMETRIC_DOUBLE_H

#include <QVector>
#include <tuple>
namespace solver {
    namespace symmetric {
    
std::tuple<QVector<QVector<double>>, QVector<QVector<double>>, QVector<double>, QVector<double>>
solveCroutSymmetric(const QVector<QVector<double>> &A, const QVector<double> &b);
   }
}
#endif // CROUT_SYMMETRIC_DOUBLE_H
 