#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>

#include "interval.hpp"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void solveSystem();
    void createMatrixInputs(int size);

    // GETTERY
    QVector<QVector<double>> getMatrixDouble() const;
    QVector<double> getVectorDouble() const;

    QVector<QVector<mpfr::mpreal>> getMatrixMpreal() const;
    QVector<mpfr::mpreal> getVectorMpreal() const;

    QVector<QVector<interval_arithmetic::Interval<mpfr::mpreal>>> getMatrixInterval() const;
    QVector<interval_arithmetic::Interval<mpfr::mpreal>> getVectorInterval() const;

    // WYPISYWANIE ROZWIĄZAŃ
    void displaySolutionDetails(const QVector<QVector<double>> &L,
                                const QVector<QVector<double>> &U,
                                const QVector<double> &y,
                                const QVector<double> &x);

    void displaySolutionDetails(const QVector<QVector<mpfr::mpreal>> &L,
                                const QVector<QVector<mpfr::mpreal>> &U,
                                const QVector<mpfr::mpreal> &y,
                                const QVector<mpfr::mpreal> &x);

    void displaySolutionDetails(const QVector<QVector<interval_arithmetic::Interval<mpfr::mpreal>>> &L,
                                const QVector<QVector<interval_arithmetic::Interval<mpfr::mpreal>>> &U,
                                const QVector<interval_arithmetic::Interval<mpfr::mpreal>> &y,
                                const QVector<interval_arithmetic::Interval<mpfr::mpreal>> &x);

    // WIDGETY
    QVBoxLayout *mainLayout;
    QHBoxLayout *controlsLayout;
    QGridLayout *matrixLayout;
    QVBoxLayout *vectorLayout;

    QSpinBox *matrixSizeSpinBox;
    QComboBox *dataTypeComboBox;
    QComboBox *matrixTypeComboBox;
    QPushButton *solveButton;
    QTextEdit *solutionTextEdit;

    QString selectedType;

    QVector<QVector<QLineEdit *>> matrixInputs;
    QVector<QLineEdit *> vectorInputs;
};

#endif // MAINWINDOW_H
