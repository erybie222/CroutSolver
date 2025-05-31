#ifndef QSTRING_UTILS_HPP
#define QSTRING_UTILS_HPP

#include <QString>
#include <QList>
#include <QVector>
#include <sstream>

// wyciszamy tylko w tym pliku warningi narrowing z interval.hpp
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnarrowing"
#include "interval.hpp"
#pragma GCC diagnostic pop

class QStringUtils {
public:
    template <typename T>
    static QString toQString(const T &val);

    template <typename T>
    static QString toQStringList(const QList<T> &list);

    template <typename T>
    static QString toQStringVector(const QVector<T> &vec);
};

// ogólny szablon
template <typename T>
QString QStringUtils::toQString(const T &val) {
    std::ostringstream oss;
    oss << val;
    return QString::fromStdString(oss.str());
}

// specjalizacja dla Interval<mpreal>
template <>
inline QString QStringUtils::toQString<interval_arithmetic::Interval<mpfr::mpreal>>(const interval_arithmetic::Interval<mpfr::mpreal> &ival) {
    // zrób nie-const kopię, żeby wywołać IEndsToStrings
    auto tmp = ival;
    std::string left, right;
    tmp.IEndsToStrings(left, right);
    return QString("[%1;%2]")
        .arg(QString::fromStdString(left).toUpper())
        .arg(QString::fromStdString(right).toUpper());
}

template <typename T>
QString QStringUtils::toQStringList(const QList<T> &list) {
    std::ostringstream oss;
    oss << "[";
    for (int i = 0; i < list.size(); ++i) {
        oss << toQString(list[i]).toStdString();
        if (i < list.size() - 1) oss << ", ";
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
        if (i < vec.size() - 1) oss << ", ";
    }
    oss << "]";
    return QString::fromStdString(oss.str());
}

#endif // QSTRING_UTILS_HPP
