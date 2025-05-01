#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QLineEdit>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QTextEdit>
#include <QString>
#include <mpreal.h>
#include "interval.hpp"

using interval_arithmetic::Interval;
using mpfr::mpreal;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void createMatrixInputs(int size);
    void solveSystem();

private:
    QGridLayout *matrixLayout;
    QVBoxLayout *vectorLayout;
    QComboBox *dataTypeComboBox;
    QSpinBox *matrixSizeSpinBox;
    QPushButton *solveButton;
    QTextEdit *solutionTextEdit;

    QVector<QVector<QLineEdit *>> matrixInputs;
    QVector<QLineEdit *> vectorInputs;

    QString selectedType = "interval";

    QVector<double> solveCrout(const QVector<QVector<double>> &A, const QVector<double> &b);
    QVector<mpreal> solveCrout(const QVector<QVector<mpreal>> &A, const QVector<mpreal> &b);
    QVector<Interval<mpreal>> solveCrout(const QVector<QVector<Interval<mpreal>>> &A, const QVector<Interval<mpreal>> &b);
};

#endif // MAINWINDOW_H
