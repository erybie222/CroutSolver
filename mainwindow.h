#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <mpreal.h>
#include "interval.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

using interval_arithmetic::Interval;
using mpfr::mpreal;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void createMatrixInputs(int size);
    void solveSystem();

private:
    QString selectedType;

    QGridLayout *matrixLayout;
    QVBoxLayout *vectorLayout;

    QSpinBox *matrixSizeSpinBox;
    QComboBox *dataTypeComboBox;
    QPushButton *solveButton;
    QTextEdit *solutionTextEdit;

    QVector<QVector<QLineEdit *>> matrixInputs;
    QVector<QLineEdit *> vectorInputs;

    // Rozwiązania metodą Crouta
    QVector<double> solveCrout(const QVector<QVector<double>> &A, const QVector<double> &b);
    QVector<mpreal> solveCrout(const QVector<QVector<mpreal>> &A, const QVector<mpreal> &b);
    QVector<Interval<mpreal>> solveCrout(const QVector<QVector<Interval<mpreal>>> &A, const QVector<Interval<mpreal>> &b);

    std::tuple<QVector<QVector<double>>, QVector<QVector<double>>, QVector<double>, QVector<double>>
    solveCroutFull(const QVector<QVector<double>> &A, const QVector<double> &b);

    std::tuple<QVector<QVector<mpreal>>, QVector<QVector<mpreal>>, QVector<mpreal>, QVector<mpreal>>
    solveCroutFull(const QVector<QVector<mpreal>> &A, const QVector<mpreal> &b);

    std::tuple<QVector<QVector<Interval<mpreal>>>, QVector<QVector<Interval<mpreal>>>, QVector<Interval<mpreal>>, QVector<Interval<mpreal>>>
    solveCroutFull(const QVector<QVector<Interval<mpreal>>> &A, const QVector<Interval<mpreal>> &b);

    // Wyświetlanie wyników
    // Wyświetlanie wyników
    
    void displaySolutionDetails(const QVector<QVector<double>> &L, const QVector<QVector<double>> &U, const QVector<double> &y, const QVector<double> &x);
    void displaySolutionDetails(const QVector<QVector<mpreal>> &L, const QVector<QVector<mpreal>> &U, const QVector<mpreal> &y, const QVector<mpreal> &x);
    void displaySolutionDetails(const QVector<QVector<Interval<mpreal>>> &L, const QVector<QVector<Interval<mpreal>>> &U, const QVector<Interval<mpreal>> &y, const QVector<Interval<mpreal>> &x);

    QVBoxLayout *mainLayout;


    template <typename T>
    QString toQString(const T &val);

    QComboBox *matrixTypeComboBox;
    QHBoxLayout *controlsLayout;

};

#endif // MAINWINDOW_H
