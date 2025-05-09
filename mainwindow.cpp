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

    // === Layout główny ===
    mainLayout = new QVBoxLayout();

    // === Kontrolki ===
    matrixSizeSpinBox = new QSpinBox();
    matrixSizeSpinBox->setRange(2, 20);
    matrixSizeSpinBox->setValue(3);

    dataTypeComboBox = new QComboBox();
    dataTypeComboBox->addItems({"double", "mpreal", "interval"});

    matrixTypeComboBox = new QComboBox();
    matrixTypeComboBox->addItems({"general", "symmetric", "tridiagonal"});

    solveButton = new QPushButton("Solve");

    controlsLayout = new QHBoxLayout();
    controlsLayout->addWidget(new QLabel("Size:"));
    controlsLayout->addWidget(matrixSizeSpinBox);
    controlsLayout->addSpacing(10);
    controlsLayout->addWidget(new QLabel("Data Type:"));
    controlsLayout->addWidget(dataTypeComboBox);
    controlsLayout->addSpacing(10);
    controlsLayout->addWidget(new QLabel("Matrix Type:"));
    controlsLayout->addWidget(matrixTypeComboBox);
    controlsLayout->addStretch();
    controlsLayout->addWidget(solveButton);

    // === Macierz A ===
    matrixLayout = new QGridLayout();
    QGroupBox *matrixGroup = new QGroupBox("Matrix A");
    matrixGroup->setLayout(matrixLayout);

    // === Wektor b ===
    vectorLayout = new QVBoxLayout();
    QGroupBox *vectorGroup = new QGroupBox("Vector b");
    vectorGroup->setLayout(vectorLayout);

    QHBoxLayout *inputLayout = new QHBoxLayout();
    inputLayout->addWidget(matrixGroup);
    inputLayout->addWidget(vectorGroup);

    // === Wyniki ===
    solutionTextEdit = new QTextEdit();
    solutionTextEdit->setReadOnly(true);
    solutionTextEdit->setStyleSheet("background-color: #f7f7f7; font-family: monospace;");

    QGroupBox *outputGroup = new QGroupBox("Solution Output");
    QVBoxLayout *outputLayout = new QVBoxLayout();
    outputLayout->addWidget(solutionTextEdit);
    outputGroup->setLayout(outputLayout);

    // === Złożenie interfejsu ===
    mainLayout->addLayout(controlsLayout);
    mainLayout->addSpacing(15);
    mainLayout->addLayout(inputLayout);
    mainLayout->addSpacing(15);
    mainLayout->addWidget(outputGroup);

    centralWidget->setLayout(mainLayout);

    // === Zachowanie ===
    connect(matrixSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int newSize) {
        solutionTextEdit->clear();
        createMatrixInputs(newSize);
    });

    connect(solveButton, &QPushButton::clicked, this, [=]() {
        solutionTextEdit->clear();
        solveSystem();
    });

    // === Inicjalizacja ===
    createMatrixInputs(matrixSizeSpinBox->value());

    // === Okno ===
    setWindowTitle("CroutSolver");
    resize(900, 700);
}



void MainWindow::createMatrixInputs(int size) {
    // Czyszczenie poprzednich pól
    QLayoutItem *child;
    while ((child = matrixLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }
    while ((child = vectorLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    matrixInputs.clear();
    vectorInputs.clear();

    // Tworzenie nowych pól wejściowych
    for (int i = 0; i < size; ++i) {
        QVector<QLineEdit*> row;
        for (int j = 0; j < size; ++j) {
            QLineEdit *lineEdit = new QLineEdit();
            lineEdit->setFixedWidth(60);
            lineEdit->setAlignment(Qt::AlignCenter);
            lineEdit->setPlaceholderText("0");
            matrixLayout->addWidget(lineEdit, i, j);
            row.push_back(lineEdit);
        }
        matrixInputs.push_back(row);

        QLineEdit *bEdit = new QLineEdit();
        bEdit->setFixedWidth(60);
        bEdit->setAlignment(Qt::AlignCenter);
        bEdit->setPlaceholderText("0");
        vectorLayout->addWidget(bEdit);
        vectorInputs.push_back(bEdit);
    }
}

bool MainWindow::parseInterval(const QString &text, interval_arithmetic::Interval<mpfr::mpreal> &result) const {
    QStringList bounds = text.split(';');
    if (bounds.size() != 2) return false;

    bool ok1 = true, ok2 = true;
    mpfr::mpreal a(bounds[0].toStdString());
    mpfr::mpreal b(bounds[1].toStdString());

    result = interval_arithmetic::Interval<mpfr::mpreal>(a, b);
    return ok1 && ok2;
}

void MainWindow::highlightInvalidField(QLineEdit *field, bool isValid) const {
    if (isValid) {
        field->setStyleSheet("");
    } else {
        field->setStyleSheet("background-color: #ffcccc;");
    }
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
            interval_arithmetic::Interval<mpfr::mpreal> interval;
            bool ok = parseInterval(matrixInputs[i][j]->text(), interval);
            highlightInvalidField(matrixInputs[i][j], ok);
            matrix[i][j] = ok ? interval : interval_arithmetic::Interval<mpfr::mpreal>(0, 0);
        }
    }
    return matrix;
}

QVector<interval_arithmetic::Interval<mpfr::mpreal>> MainWindow::getVectorInterval() const {
    QVector<interval_arithmetic::Interval<mpfr::mpreal>> vec(vectorInputs.size());
    for (int i = 0; i < vectorInputs.size(); ++i) {
        interval_arithmetic::Interval<mpfr::mpreal> interval;
        bool ok = parseInterval(vectorInputs[i]->text(), interval);
        highlightInvalidField(vectorInputs[i], ok);
        vec[i] = ok ? interval : interval_arithmetic::Interval<mpfr::mpreal>(0, 0);
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
