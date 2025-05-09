#ifndef CROUT_GENERAL_DOUBLE_H
#define CROUT_GENERAL_DOUBLE_H

#include <QVector>
#include <tuple>
namespace solver {
    namespace general {
    
std::tuple<QVector<QVector<double>>, QVector<QVector<double>>, QVector<double>, QVector<double>>
solveCroutGeneral(const QVector<QVector<double>> &A, const QVector<double> &b);
    }
}
#endif // CROUT_GENERAL_DOUBLE_H
