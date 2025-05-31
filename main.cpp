#include <QApplication>
#include "mainwindow.h"
#include "interval_rounding_fix.hpp"
#include "interval.hpp"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 1. precyzja / outDigits (nic nie boli, gdy wywołasz dwa razy)
    interval_arithmetic::Interval<mpfr::mpreal>::Initialize();
    // 2. przełączamy tryb na dual-interval
    interval_arithmetic::Interval<mpfr::mpreal>::SetMode(interval_arithmetic::DINT_MODE);

    MainWindow w;
    w.show();
    return app.exec();
}

