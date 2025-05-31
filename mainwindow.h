#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QRadioButton>
#include <QButtonGroup>
#include <QGridLayout>    // <-- nowy
#include <QVBoxLayout>    // <-- nowy
#include <QHBoxLayout>
#include <QGroupBox>

#include "interval.hpp"


class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void createMatrixInputs(int size);
    void solveSystem();

private:
    // Gettery
    QVector<QVector<double>> getMatrixDouble() const;
    QVector<double>          getVectorDouble() const;
    QVector<QVector<mpfr::mpreal>> getMatrixMpreal() const;
    QVector<mpfr::mpreal>         getVectorMpreal() const;
    QVector<QVector<interval_arithmetic::Interval<mpfr::mpreal>>> getMatrixInterval() const;
    QVector<interval_arithmetic::Interval<mpfr::mpreal>>         getVectorInterval() const;

    // Pomocnicze
    QString normalizeIntervalText(const QString &text) const;
    bool parseInterval(const QString &text, interval_arithmetic::Interval<mpfr::mpreal> &out) const;
    void highlightInvalidField(QLineEdit *f, bool ok, const QString &msg = {}) const;

    // GUI
    QSpinBox    *matrixSizeSpinBox;
    QComboBox   *dataTypeComboBox;
    QRadioButton *symRadio, *triRadio;
    QButtonGroup *matrixTypeGroup;
    QPushButton *solveButton;
    QTextEdit   *solutionTextEdit;

    QGridLayout *matrixLayout;
    QVBoxLayout *vectorLayout;

    // Wartości wpisane przez użytkownika
    QVector<QVector<QLineEdit*>> matrixInputs;          // double / mp
QVector<QVector<QLineEdit*>> matrixInputsInterval;  // jedna kratka na element

QVector<QLineEdit*>          vectorInputs;          // double / mp
QVector<QLineEdit*>          vectorInputsInterval;  // jedna kratka na element
};

#endif // MAINWINDOW_H
