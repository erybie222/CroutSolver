#include "mainwindow.h"
#include "interval.hpp"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QComboBox>
#include <QGroupBox>
#include <QFrame>
#include <QApplication>
#include "qstring_utils.hpp"

#include <sstream>

#include "solver/general/crout_general_double.h"
#include "solver/general/crout_general_mpreal.h"
#include "solver/general/crout_general_interval.h"

#include "solver/symmetric/crout_symmetric_double.h"
#include "solver/symmetric/crout_symmetric_mpreal.h"
#include "solver/symmetric/crout_symmetric_interval.h"

#include "solver/tridiagonal/crout_tridiagonal_double.h"
#include "solver/tridiagonal/crout_tridiagonal_mpreal.h"
#include "solver/tridiagonal/crout_tridiagonal_interval.h"

using namespace solver::general;
using namespace solver::symmetric;
using namespace solver::tridiagonal;

using namespace mpfr;
using namespace interval_arithmetic;

MainWindow::~MainWindow() = default;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    mainLayout = new QVBoxLayout(centralWidget);
    controlsLayout = new QHBoxLayout();
    matrixLayout = new QGridLayout();
    vectorLayout = new QVBoxLayout();

    matrixSizeSpinBox = new QSpinBox(this);
    matrixSizeSpinBox->setMinimum(2);
    matrixSizeSpinBox->setMaximum(10);
    matrixSizeSpinBox->setValue(3);

    dataTypeComboBox = new QComboBox(this);
    dataTypeComboBox->addItem("double");
    dataTypeComboBox->addItem("mpreal");
    dataTypeComboBox->addItem("interval");

    matrixTypeComboBox = new QComboBox(this);
    matrixTypeComboBox->addItem("general");
    matrixTypeComboBox->addItem("symmetric");
    matrixTypeComboBox->addItem("tridiagonal");

    solveButton = new QPushButton("Solve", this);
    solutionTextEdit = new QTextEdit(this);
    solutionTextEdit->setReadOnly(true);

    controlsLayout->addWidget(new QLabel("Matrix size:", this));
    controlsLayout->addWidget(matrixSizeSpinBox);
    controlsLayout->addWidget(new QLabel("Data type:", this));
    controlsLayout->addWidget(dataTypeComboBox);
    controlsLayout->addWidget(new QLabel("Matrix type:", this));
    controlsLayout->addWidget(matrixTypeComboBox);
    controlsLayout->addWidget(solveButton);

    QHBoxLayout *inputLayout = new QHBoxLayout();
    QVBoxLayout *matrixBoxLayout = new QVBoxLayout();
    QVBoxLayout *vectorBoxLayout = new QVBoxLayout();

    QLabel *matrixLabel = new QLabel("Matrix A:");
    QLabel *vectorLabel = new QLabel("Vector b:");

    QFrame *matrixFrame = new QFrame(this);
    matrixFrame->setFrameShape(QFrame::StyledPanel);
    matrixFrame->setLayout(matrixLayout);

    QFrame *vectorFrame = new QFrame(this);
    vectorFrame->setFrameShape(QFrame::StyledPanel);
    vectorFrame->setLayout(vectorLayout);

    matrixBoxLayout->addWidget(matrixLabel);
    matrixBoxLayout->addWidget(matrixFrame);
    vectorBoxLayout->addWidget(vectorLabel);
    vectorBoxLayout->addWidget(vectorFrame);

    inputLayout->addLayout(matrixBoxLayout);
    inputLayout->addLayout(vectorBoxLayout);

    mainLayout->addLayout(controlsLayout);
    mainLayout->addLayout(inputLayout);
    mainLayout->addWidget(solutionTextEdit);

    createMatrixInputs(matrixSizeSpinBox->value());

    connect(matrixSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [=](int size) { createMatrixInputs(size); });

    connect(solveButton, &QPushButton::clicked,
            this, &MainWindow::solveSystem);
}

void MainWindow::createMatrixInputs(int size) {
    // Usuń stare widżety z layoutów
    QLayoutItem *item;
    while ((item = matrixLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
    while ((item = vectorLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    // Wyczyść wektory przechowujące QLineEdity
    matrixInputs.clear();
    vectorInputs.clear();

    // Utwórz nowe pola dla macierzy A i wektora b
    for (int i = 0; i < size; ++i) {
        QVector<QLineEdit *> row;
        for (int j = 0; j < size; ++j) {
            QLineEdit *lineEdit = new QLineEdit(this);
            lineEdit->setFixedWidth(60);
            matrixLayout->addWidget(lineEdit, i, j);
            row.append(lineEdit);
        }
        matrixInputs.append(row);

        QLineEdit *vecEdit = new QLineEdit(this);
        vecEdit->setFixedWidth(60);
        vectorLayout->addWidget(vecEdit);
        vectorInputs.append(vecEdit);
    }

    // Wymuś odświeżenie widoku
    matrixLayout->update();
    vectorLayout->update();
}


void MainWindow::solveSystem() {
    const int size = matrixInputs.size();
    const QString matrixType = matrixTypeComboBox->currentText();
    const QString dataType = dataTypeComboBox->currentText();

    if (dataType == "double") {
        QVector<QVector<double>> A = getMatrixDouble();
        QVector<double> b = getVectorDouble();
        QVector<QVector<double>> L, U;
        QVector<double> y, x;

        if (matrixType == "general")
            std::tie(L, U, y, x) = solveCroutGeneral(A, b);
        else if (matrixType == "symmetric")
            std::tie(L, U, y, x) = solveCroutSymmetric(A, b);
        else if (matrixType == "tridiagonal") {
            QVector<double> a(size), d(size), c(size);
            for (int i = 0; i < size; ++i) {
                d[i] = A[i][i];
                if (i > 0) a[i] = A[i][i - 1];
                if (i < size - 1) c[i] = A[i][i + 1];
            }
            QList<double> l, diag, up, yList, xList;
            std::tie(l, diag, up, yList, xList) = solveCroutTridiagonal(a, d, c, b);

            L = QVector<QVector<double>>(size, QVector<double>(size, 0.0));
            U = QVector<QVector<double>>(size, QVector<double>(size, 0.0));
            y.resize(size); x.resize(size);
            for (int i = 0; i < size; ++i) {
                L[i][i] = 1.0;
                if (i > 0) L[i][i - 1] = l[i];
                U[i][i] = diag[i];
                if (i < size - 1) U[i][i + 1] = up[i];
                y[i] = yList[i];
                x[i] = xList[i];
            }
        }

        displaySolutionDetails(L, U, y, x);
    }

    else if (dataType == "mpreal") {
        QVector<QVector<mpfr::mpreal>> A = getMatrixMpreal();
        QVector<mpfr::mpreal> b = getVectorMpreal();
        QVector<QVector<mpfr::mpreal>> L, U;
        QVector<mpfr::mpreal> y, x;

        if (matrixType == "general")
            std::tie(L, U, y, x) = solveCroutGeneral(A, b);
        else if (matrixType == "symmetric")
            std::tie(L, U, y, x) = solveCroutSymmetric(A, b);
        else if (matrixType == "tridiagonal") {
            QVector<mpfr::mpreal> a(size), d(size), c(size);
            for (int i = 0; i < size; ++i) {
                d[i] = A[i][i];
                if (i > 0) a[i] = A[i][i - 1];
                if (i < size - 1) c[i] = A[i][i + 1];
            }
            QList<mpfr::mpreal> l, diag, up, yList, xList;
            std::tie(l, diag, up, yList, xList) = solveCroutTridiagonal(a, d, c, b);

            L = QVector<QVector<mpfr::mpreal>>(size, QVector<mpfr::mpreal>(size, 0.0));
            U = QVector<QVector<mpfr::mpreal>>(size, QVector<mpfr::mpreal>(size, 0.0));
            y.resize(size); x.resize(size);
            for (int i = 0; i < size; ++i) {
                L[i][i] = 1.0;
                if (i > 0) L[i][i - 1] = l[i];
                U[i][i] = diag[i];
                if (i < size - 1) U[i][i + 1] = up[i];
                y[i] = yList[i];
                x[i] = xList[i];
            }
        }

        displaySolutionDetails(L, U, y, x);
    }

    else if (dataType == "interval") {
        using Interval = interval_arithmetic::Interval<mpfr::mpreal>;
        QVector<QVector<Interval>> A = getMatrixInterval();
        QVector<Interval> b = getVectorInterval();
        QVector<QVector<Interval>> L, U;
        QVector<Interval> y, x;

        if (matrixType == "general")
            std::tie(L, U, y, x) = solveCroutGeneral(A, b);
        else if (matrixType == "symmetric")
            std::tie(L, U, y, x) = solveCroutSymmetric(A, b);
        else if (matrixType == "tridiagonal") {
            QVector<Interval> a(size), d(size), c(size);
            for (int i = 0; i < size; ++i) {
                d[i] = A[i][i];
                if (i > 0) a[i] = A[i][i - 1];
                if (i < size - 1) c[i] = A[i][i + 1];
            }
            QList<Interval> l, diag, up, yList, xList;
            std::tie(l, diag, up, yList, xList) = solveCroutTridiagonal(a, d, c, b);

            L = QVector<QVector<Interval>>(size, QVector<Interval>(size));
            U = QVector<QVector<Interval>>(size, QVector<Interval>(size));
            y.resize(size); x.resize(size);
            for (int i = 0; i < size; ++i) {
                L[i][i] = Interval(1.0 , 1.0);
                if (i > 0) L[i][i - 1] = l[i];
                U[i][i] = diag[i];
                if (i < size - 1) U[i][i + 1] = up[i];
                y[i] = yList[i];
                x[i] = xList[i];
            }
        }

        displaySolutionDetails(L, U, y, x);
    }
}

// --- GETTERY ---

QVector<QVector<double>> MainWindow::getMatrixDouble() const {
    QVector<QVector<double>> matrix(matrixInputs.size());
    for (int i = 0; i < matrixInputs.size(); ++i) {
        matrix[i].resize(matrixInputs[i].size());
        for (int j = 0; j < matrixInputs[i].size(); ++j)
            matrix[i][j] = matrixInputs[i][j]->text().toDouble();
    }
    return matrix;
}

QVector<double> MainWindow::getVectorDouble() const {
    QVector<double> vec(vectorInputs.size());
    for (int i = 0; i < vectorInputs.size(); ++i)
        vec[i] = vectorInputs[i]->text().toDouble();
    return vec;
}

QVector<QVector<mpfr::mpreal>> MainWindow::getMatrixMpreal() const {
    QVector<QVector<mpfr::mpreal>> matrix(matrixInputs.size());
    for (int i = 0; i < matrixInputs.size(); ++i) {
        matrix[i].resize(matrixInputs[i].size());
        for (int j = 0; j < matrixInputs[i].size(); ++j)
            matrix[i][j] = mpfr::mpreal(matrixInputs[i][j]->text().toStdString());
    }
    return matrix;
}

QVector<mpfr::mpreal> MainWindow::getVectorMpreal() const {
    QVector<mpfr::mpreal> vec(vectorInputs.size());
    for (int i = 0; i < vectorInputs.size(); ++i)
        vec[i] = mpfr::mpreal(vectorInputs[i]->text().toStdString());
    return vec;
}

QVector<QVector<interval_arithmetic::Interval<mpfr::mpreal>>> MainWindow::getMatrixInterval() const {
    QVector<QVector<interval_arithmetic::Interval<mpfr::mpreal>>> matrix(matrixInputs.size());
    for (int i = 0; i < matrixInputs.size(); ++i) {
        matrix[i].resize(matrixInputs[i].size());
        for (int j = 0; j < matrixInputs[i].size(); ++j) {
            QStringList bounds = matrixInputs[i][j]->text().split(';');
            if (bounds.size() == 2) {
                mpfr::mpreal a(bounds[0].toStdString());
                mpfr::mpreal b(bounds[1].toStdString());
                matrix[i][j] = interval_arithmetic::Interval<mpfr::mpreal>(a, b);
            }
        }
    }
    return matrix;
}


QVector<interval_arithmetic::Interval<mpfr::mpreal>> MainWindow::getVectorInterval() const {
    QVector<interval_arithmetic::Interval<mpfr::mpreal>> vec(vectorInputs.size());
    for (int i = 0; i < vectorInputs.size(); ++i) {
        QStringList bounds = vectorInputs[i]->text().split(';');
        if (bounds.size() == 2) {
            mpfr::mpreal a(bounds[0].toStdString());
            mpfr::mpreal b(bounds[1].toStdString());
            vec[i] = interval_arithmetic::Interval<mpfr::mpreal>(a, b);
        }
    }
    return vec;
}


// --- DISPLAY ---

void MainWindow::displaySolutionDetails(
    const QVector<QVector<double>> &L,
    const QVector<QVector<double>> &U,
    const QVector<double> &y,
    const QVector<double> &x)
{
    solutionTextEdit->append("L matrix:");
    for (const auto &row : L)
        solutionTextEdit->append(QStringUtils::toQStringVector(row));
    solutionTextEdit->append("U matrix:");
    for (const auto &row : U)
        solutionTextEdit->append(QStringUtils::toQStringVector(row));
    solutionTextEdit->append("y vector:");
    solutionTextEdit->append(QStringUtils::toQStringVector(y));
    solutionTextEdit->append("x vector:");
    solutionTextEdit->append(QStringUtils::toQStringVector(x));
}

void MainWindow::displaySolutionDetails(
    const QVector<QVector<mpfr::mpreal>> &L,
    const QVector<QVector<mpfr::mpreal>> &U,
    const QVector<mpfr::mpreal> &y,
    const QVector<mpfr::mpreal> &x)
{
    solutionTextEdit->append("L matrix (mpreal):");
    for (const auto &row : L)
        solutionTextEdit->append(QStringUtils::toQStringVector(row));
    solutionTextEdit->append("U matrix (mpreal):");
    for (const auto &row : U)
        solutionTextEdit->append(QStringUtils::toQStringVector(row));
    solutionTextEdit->append("y vector:");
    solutionTextEdit->append(QStringUtils::toQStringVector(y));
    solutionTextEdit->append("x vector:");
    solutionTextEdit->append(QStringUtils::toQStringVector(x));
}

void MainWindow::displaySolutionDetails(
    const QVector<QVector<interval_arithmetic::Interval<mpfr::mpreal>>> &L,
    const QVector<QVector<interval_arithmetic::Interval<mpfr::mpreal>>> &U,
    const QVector<interval_arithmetic::Interval<mpfr::mpreal>> &y,
    const QVector<interval_arithmetic::Interval<mpfr::mpreal>> &x)
{
    solutionTextEdit->append("L matrix (interval):");
    for (const auto &row : L)
        solutionTextEdit->append(QStringUtils::toQStringVector(row));
    solutionTextEdit->append("U matrix (interval):");
    for (const auto &row : U)
        solutionTextEdit->append(QStringUtils::toQStringVector(row));
    solutionTextEdit->append("y vector:");
    solutionTextEdit->append(QStringUtils::toQStringVector(y));
    solutionTextEdit->append("x vector:");
    solutionTextEdit->append(QStringUtils::toQStringVector(x));
}
