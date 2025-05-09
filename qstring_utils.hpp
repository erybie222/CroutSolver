#ifndef QSTRING_UTILS_HPP
#define QSTRING_UTILS_HPP

#include <QString>
#include <QList>
#include <QVector>
#include <sstream>
#include "interval.hpp"
#include "mpreal.h"

class QStringUtils {
public:
    template <typename T>
    static QString toQString(const T &val);

    template <typename T>
    static QString toQStringList(const QList<T> &list);

    template <typename T>
    static QString toQStringVector(const QVector<T> &vec);
};

// szablon ogólny
template <typename T>
QString QStringUtils::toQString(const T &val) {
    std::ostringstream oss;
    oss << val;
    return QString::fromStdString(oss.str());
}

// specjalizacje
template <>
inline QString QStringUtils::toQString(const double &val) {
    std::ostringstream oss;
    oss << val;
    return QString::fromStdString(oss.str());
}

template <>
inline QString QStringUtils::toQString(const mpfr::mpreal &val) {
    std::ostringstream oss;
    oss << val;
    return QString::fromStdString(oss.str());
}

template <>
inline QString QStringUtils::toQString(const interval_arithmetic::Interval<mpfr::mpreal> &val) {
    std::ostringstream oss;
    oss << val;
    return QString::fromStdString(oss.str());
}

template <typename T>
QString QStringUtils::toQStringList(const QList<T> &list) {
    std::ostringstream oss;
    oss << "[";
    for (int i = 0; i < list.size(); ++i) {
        oss << toQString(list[i]).toStdString();
        if (i < list.size() - 1)
            oss << ", ";
    }
    oss << "]";
    return QString::fromStdString(oss.str());
}

template <typename T>
QString QStringUtils::toQStringVector(const QVector<T> &vec) {
    std::ostringstream oss;
    oss << "[";
    for (int i = 0; i < vec.size(); ++i) {
        oss << toQString(vec[i]).toStdString();
        if (i < vec.size() - 1)
            oss << ", ";
    }
    oss << "]";
    return QString::fromStdString(oss.str());
}

#endif // QSTRING_UTILS_HPP
