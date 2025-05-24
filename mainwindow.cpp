// mainwindow.cpp

#include "mainwindow.h"

#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QFrame>
#include <QGroupBox>
#include <QSpinBox>
#include <QComboBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QRegularExpression>

#include "qstring_utils.hpp"
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


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Central widget & main layout
    auto *central    = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(central);
    setCentralWidget(central);

    // --- Top controls: size on the left, type on the right ---
    auto *topLayout = new QHBoxLayout;
    {
        // Left: matrix size
        auto *sizeLabel       = new QLabel("Rozmiar macierzy:");
        matrixSizeSpinBox     = new QSpinBox;
        matrixSizeSpinBox->setRange(2,20);
        matrixSizeSpinBox->setValue(3);

        topLayout->addWidget(sizeLabel);
        topLayout->addWidget(matrixSizeSpinBox);

        // stretch pushes the next widgets to the right edge
        topLayout->addStretch(1);

        // Right: data type
        auto *typeLabel       = new QLabel("Typ danych:");
        dataTypeComboBox      = new QComboBox;
        dataTypeComboBox->addItems({
            "Zmiennoprzecinkowe (double)",
            "Wysokoprecyzyjne (mpreal)",
            "Przedziałowe"
        });

        topLayout->addWidget(typeLabel);
        topLayout->addWidget(dataTypeComboBox);
    }
    mainLayout->addLayout(topLayout);
    mainLayout->addSpacing(15);

    // --- Middle: matrix A and vector b side by side ---
    auto *inputLayout = new QHBoxLayout;
    {
        // Matrix A
        matrixLayout   = new QGridLayout;
        auto *matrixGroup = new QGroupBox("Macierz A");
        matrixGroup->setLayout(matrixLayout);
        inputLayout->addWidget(matrixGroup);

        // Vertical separator
        auto *sep = new QFrame;
        sep->setFrameShape(QFrame::VLine);
        sep->setFrameShadow(QFrame::Sunken);
        inputLayout->addWidget(sep);

        // Vector b
        vectorLayout   = new QVBoxLayout;
        auto *vectorGroup = new QGroupBox("Wektor b");
        vectorGroup->setLayout(vectorLayout);
        inputLayout->addWidget(vectorGroup);
    }
    mainLayout->addLayout(inputLayout);
    mainLayout->addSpacing(15);

    // --- Solve button, centered under inputs ---
    {
        solveButton = new QPushButton("Rozwiąż");
        auto *btnLayout = new QHBoxLayout;
        btnLayout->addStretch(1);
        btnLayout->addWidget(solveButton);
        btnLayout->addStretch(1);
        mainLayout->addLayout(btnLayout);
    }
    mainLayout->addSpacing(15);

    // --- Bottom: results ---
    {
        solutionTextEdit = new QTextEdit;
        solutionTextEdit->setReadOnly(true);
        auto *outputGroup  = new QGroupBox("Wyniki");
        auto *outputLayout = new QVBoxLayout(outputGroup);
        outputLayout->addWidget(solutionTextEdit);
        mainLayout->addWidget(outputGroup);
    }

    // --- Signals ---
    connect(matrixSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::createMatrixInputs);
    connect(dataTypeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this]{ createMatrixInputs(matrixSizeSpinBox->value()); });
    connect(solveButton, &QPushButton::clicked,
            this, &MainWindow::solveSystem);

    // Initial population
    createMatrixInputs(matrixSizeSpinBox->value());

    setWindowTitle("CroutSolver");
    resize(900,700);
}


MainWindow::~MainWindow() = default;

// --- Helpers ---

QString MainWindow::normalizeIntervalText(const QString &text) const {
    QString s = text.trimmed();
    s.replace(",", ";");
    s.replace(QRegularExpression("\\s+"), "");
    return s;
}

bool MainWindow::parseInterval(const QString &text, Interval<mpreal> &out) const {
    auto parts = normalizeIntervalText(text).split(';');
    if (parts.size() != 2) return false;
    mpreal a(parts[0].toStdString()), b(parts[1].toStdString());
    if (a > b) return false;
    out = Interval<mpreal>(a,b);
    return true;
}

void MainWindow::highlightInvalidField(QLineEdit *f, bool ok, const QString &msg) const {
    if (ok) {
        f->setStyleSheet("");
        f->setToolTip({});
    } else {
        f->setStyleSheet("background-color: #ffcccc;");
        f->setToolTip(msg.isEmpty() ? "Niepoprawny format" : msg);
    }
}

// --- Create inputs ---

void MainWindow::createMatrixInputs(int size) {
    // Clear old
    QLayoutItem *it;
    while ((it = matrixLayout->takeAt(0))) { delete it->widget(); delete it; }
    while ((it = vectorLayout->takeAt(0))) { delete it->widget(); delete it; }

    matrixInputs.clear();
    matrixInputsInterval.clear();
    vectorInputs.clear();
    vectorInputsInterval.clear();

    bool isInterval = (dataTypeComboBox->currentIndex() == 2);

    // Matrix A
    for (int i = 0; i < size; ++i) {
        QVector<QLineEdit*> row;
        QVector<QPair<QLineEdit*,QLineEdit*>> irow;
        for (int j = 0; j < size; ++j) {
            if (isInterval) {
                auto *cell = new QFrame;
                auto *lay  = new QHBoxLayout(cell);
                lay->setContentsMargins(0,0,0,0);
                lay->setSpacing(2);
                QLabel *lb = new QLabel("["), *sep = new QLabel(";"), *rb = new QLabel("]");
                QLineEdit *low = new QLineEdit("0"), *high = new QLineEdit("0");
                for (auto *l : {lb, sep, rb}) l->setFixedWidth(10), l->setAlignment(Qt::AlignCenter);
                for (auto *e : {low, high})   e->setFixedWidth(40), e->setAlignment(Qt::AlignCenter);
                lay->addWidget(lb); lay->addWidget(low);
                lay->addWidget(sep); lay->addWidget(high);
                lay->addWidget(rb);
                matrixLayout->addWidget(cell, i, j);
                irow.emplace_back(low, high);
            } else {
                auto *e = new QLineEdit("0");
                e->setFixedWidth(60);
                e->setAlignment(Qt::AlignCenter);
                matrixLayout->addWidget(e, i, j);
                row.append(e);
            }
        }
        if (isInterval) matrixInputsInterval.append(irow);
        else            matrixInputs.append(row);
    }

    // Vector b
    if (isInterval) {
        for (int i = 0; i < size; ++i) {
            auto *cell = new QFrame;
            auto *lay  = new QHBoxLayout(cell);
            lay->setContentsMargins(0,0,0,0);
            lay->setSpacing(2);
            QLabel *lb = new QLabel("["), *sep = new QLabel(";"), *rb = new QLabel("]");
            QLineEdit *low = new QLineEdit("0"), *high = new QLineEdit("0");
            for (auto *l : {lb, sep, rb}) l->setFixedWidth(10), l->setAlignment(Qt::AlignCenter);
            for (auto *e : {low, high})   e->setFixedWidth(40), e->setAlignment(Qt::AlignCenter);
            lay->addWidget(lb); lay->addWidget(low);
            lay->addWidget(sep); lay->addWidget(high);
            lay->addWidget(rb);
            vectorLayout->addWidget(cell);
            vectorInputsInterval.emplace_back(low, high);
        }
    } else {
        for (int i = 0; i < size; ++i) {
            auto *e = new QLineEdit("0");
            e->setFixedWidth(60);
            e->setAlignment(Qt::AlignCenter);
            vectorLayout->addWidget(e);
            vectorInputs.append(e);
        }
    }
}

// --- Getters ---

QVector<QVector<double>> MainWindow::getMatrixDouble() const {
    QVector<QVector<double>> M(matrixInputs.size());
    for (int i = 0; i < M.size(); ++i) {
        M[i].resize(matrixInputs[i].size());
        for (int j = 0; j < M[i].size(); ++j)
            M[i][j] = matrixInputs[i][j]->text().toDouble();
    }
    return M;
}

QVector<double> MainWindow::getVectorDouble() const {
    QVector<double> v(vectorInputs.size());
    for (int i = 0; i < v.size(); ++i)
        v[i] = vectorInputs[i]->text().toDouble();
    return v;
}

QVector<QVector<mpreal>> MainWindow::getMatrixMpreal() const {
    QVector<QVector<mpreal>> M(matrixInputs.size());
    for (int i = 0; i < M.size(); ++i) {
        M[i].resize(matrixInputs[i].size());
        for (int j = 0; j < M[i].size(); ++j)
            M[i][j] = mpreal(matrixInputs[i][j]->text().toStdString());
    }
    return M;
}

QVector<mpreal> MainWindow::getVectorMpreal() const {
    QVector<mpreal> v(vectorInputs.size());
    for (int i = 0; i < v.size(); ++i)
        v[i] = mpreal(vectorInputs[i]->text().toStdString());
    return v;
}

QVector<QVector<Interval<mpreal>>> MainWindow::getMatrixInterval() const {
    QVector<QVector<Interval<mpreal>>> M(matrixInputsInterval.size());
    for (int i = 0; i < M.size(); ++i) {
        M[i].resize(matrixInputsInterval[i].size());
        for (int j = 0; j < M[i].size(); ++j) {
            auto [low, high] = matrixInputsInterval[i][j];
            mpreal a(low->text().toStdString()), b(high->text().toStdString());
            M[i][j] = (a <= b ? Interval<mpreal>(a,b) : Interval<mpreal>(0,0));
        }
    }
    return M;
}

QVector<Interval<mpreal>> MainWindow::getVectorInterval() const {
    QVector<Interval<mpreal>> v(vectorInputsInterval.size());
    for (int i = 0; i < v.size(); ++i) {
        auto [low, high] = vectorInputsInterval[i];
        mpreal a(low->text().toStdString()), b(high->text().toStdString());
        v[i] = (a <= b ? Interval<mpreal>(a,b) : Interval<mpreal>(0,0));
    }
    return v;
}

// --- solveSystem() ---

void MainWindow::solveSystem() {
    const int n     = matrixSizeSpinBox->value();
    const int dtype = dataTypeComboBox->currentIndex();
    const int mtype = matrixTypeGroup->checkedId();

    solutionTextEdit->clear();

    auto pad3exp = [](const QString &s){
        int e = s.indexOf('E');
        if (e < 0 || e+2 >= s.size()) return s;
        QString d = s.mid(e+2);
        if (d.size() == 2) d.prepend('0');
        return s.left(e+2) + d;
    };

    QStringList out;

    if (dtype == 0) {
        // double
        auto A = getMatrixDouble();
        auto b = getVectorDouble();
        QVector<QVector<double>> L, U;
        QVector<double> y, x;

        if (mtype == 0) {
            std::tie(L,U,y,x) = solveCroutSymmetric(A,b);
        } else {
            QVector<double> a(n-1), d(n), c(n-1);
            for (int i = 0; i < n; ++i) {
                d[i] = A[i][i];
                if (i < n-1) c[i] = A[i][i+1];
                if (i > 0)   a[i-1] = A[i][i-1];
            }
            QList<double> lq, diagq, upq, yLq, xLq;
            std::tie(lq,diagq,upq,yLq,xLq) = solveCroutTridiagonal(a,d,c,b);
            QVector<double> l(lq.begin(),lq.end()),
                            diag(diagq.begin(),diagq.end()),
                            up(upq.begin(),upq.end()),
                            yL(yLq.begin(),yLq.end()),
                            xL(xLq.begin(),xLq.end());

            L = QVector<QVector<double>>(n, QVector<double>(n,0.0));
            U = QVector<QVector<double>>(n, QVector<double>(n,0.0));
            y.resize(n); x.resize(n);
            for (int i = 0; i < n; ++i) {
                L[i][i] = 1.0;
                if (i > 0)   L[i][i-1] = l[i-1];
                U[i][i]   = diag[i];
                if (i < n-1) U[i][i+1] = up[i];
                y[i] = yL[i];
                x[i] = xL[i];
            }
        }

        for (int i = 0; i < n; ++i) {
            QString xi = pad3exp(QString::asprintf("%.14E", x[i]).toUpper());
            QString w  = pad3exp(QString::asprintf("%.3E", 0.0).toUpper());
            out << QString("d[%1]=%2, szer.=%3").arg(i+1).arg(xi).arg(w);
        }
    }
    else if (dtype == 1) {
        // mpreal
        using M = mpfr::mpreal;
        auto A = getMatrixMpreal();
        auto b = getVectorMpreal();
        QVector<QVector<M>> L, U;
        QVector<M> y, x;

        if (mtype == 0) {
            std::tie(L,U,y,x) = solveCroutSymmetric(A,b);
        } else {
            QVector<M> a(n-1), d(n), c(n-1);
            for (int i = 0; i < n; ++i) {
                d[i] = A[i][i];
                if (i < n-1) c[i] = A[i][i+1];
                if (i > 0)   a[i-1] = A[i][i-1];
            }
            QList<M> lq, diagq, upq, yLq, xLq;
            std::tie(lq,diagq,upq,yLq,xLq) = solveCroutTridiagonal(a,d,c,b);
            QVector<M> l(lq.begin(),lq.end()),
                        diag(diagq.begin(),diagq.end()),
                        up(upq.begin(),upq.end()),
                        yL(yLq.begin(),yLq.end()),
                        xL(xLq.begin(),xLq.end());

            L = QVector<QVector<M>>(n, QVector<M>(n,0));
            U = QVector<QVector<M>>(n, QVector<M>(n,0));
            y.resize(n); x.resize(n);
            for (int i = 0; i < n; ++i) {
                L[i][i] = 1;
                if (i > 0)   L[i][i-1] = l[i-1];
                U[i][i]   = diag[i];
                if (i < n-1) U[i][i+1] = up[i];
                y[i] = yL[i];
                x[i] = xL[i];
            }
        }

        for (int i = 0; i < n; ++i) {
            double dv = x[i].toDouble();
            QString xi = pad3exp(QString::asprintf("%.14E", dv).toUpper());
            QString w  = pad3exp(QString::asprintf("%.3E", 0.0).toUpper());
            out << QString("d[%1]=%2, szer.=%3").arg(i+1).arg(xi).arg(w);
        }
    }
    else {
        // interval<mpreal>
        using I = Interval<mpfr::mpreal>;
        auto A = getMatrixInterval();
        auto b = getVectorInterval();
        QVector<QVector<I>> L, U;
        QVector<I> y, x;

        if (mtype == 0) {
            std::tie(L,U,y,x) = solveCroutSymmetric(A,b);
        } else {
            QVector<I> a(n-1), d(n), c(n-1);
            for (int i = 0; i < n; ++i) {
                d[i] = A[i][i];
                if (i < n-1) c[i] = A[i][i+1];
                if (i > 0)   a[i-1] = A[i][i-1];
            }
            QList<I> lq, diagq, upq, yLq, xLq;
            std::tie(lq,diagq,upq,yLq,xLq) = solveCroutTridiagonal(a,d,c,b);
            QVector<I> l(lq.begin(),lq.end()),
                        diag(diagq.begin(),diagq.end()),
                        up(upq.begin(),upq.end()),
                        yL(yLq.begin(),yLq.end()),
                        xL(xLq.begin(),xLq.end());

            L = QVector<QVector<I>>(n, QVector<I>(n));
            U = QVector<QVector<I>>(n, QVector<I>(n));
            y.resize(n); x.resize(n);
            for (int i = 0; i < n; ++i) {
                L[i][i] = I(1,1);
                if (i > 0)   L[i][i-1] = l[i-1];
                U[i][i]   = diag[i];
                if (i < n-1) U[i][i+1] = up[i];
                y[i] = yL[i];
                x[i] = xL[i];
            }
        }

        for (int i = 0; i < n; ++i) {
            QString xi = QStringUtils::toQString(x[i])
                               .toUpper()
                               .replace(",", ";")
                               .replace(" ", "");
            mpfr::mpreal lo = x[i].a, hi = x[i].b;
            double width = (hi - lo).toDouble();
            QString w = pad3exp(QString::asprintf("%.3E", width).toUpper());
            out << QString("d[%1]=%2, szer.=%3").arg(i+1).arg(xi).arg(w);
        }
    }

    solutionTextEdit->append(out.join(", "));
    solutionTextEdit->append("st = 0");
}
