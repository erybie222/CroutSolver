// mainwindow.cpp

#include "mainwindow.h"
#include <cmath>  // dla std::isnan i std::isinf

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

    // --- Top controls: size on the left, type and matrix-type on the right ---
    auto *topLayout = new QHBoxLayout;
    {
        // Left: matrix size
        auto *sizeLabel       = new QLabel("Rozmiar macierzy:");
        matrixSizeSpinBox     = new QSpinBox;
        matrixSizeSpinBox->setRange(2,20);
        matrixSizeSpinBox->setValue(3);
        topLayout->addWidget(sizeLabel);
        topLayout->addWidget(matrixSizeSpinBox);

        topLayout->addStretch(1);  

       
        auto *typeLabel       = new QLabel("Typ danych:");
        dataTypeComboBox      = new QComboBox;
        dataTypeComboBox->addItems({
            "Zmiennoprzecinkowe",
            "Wysokoprecyzyjne",
            "Przedziałowe"
        });
        topLayout->addWidget(typeLabel);
        topLayout->addWidget(dataTypeComboBox);

        // Right: matrix type
        topLayout->addSpacing(20);
        auto *matrixTypeBox = new QGroupBox("Rodzaj macierzy:");
        auto *mtLayout      = new QHBoxLayout(matrixTypeBox);
        symRadio            = new QRadioButton("Symetryczna");
        triRadio            = new QRadioButton("Trójdiagonalna");
        symRadio->setChecked(true);
        mtLayout->addWidget(symRadio);
        mtLayout->addWidget(triRadio);
        matrixTypeGroup = new QButtonGroup(this);
        matrixTypeGroup->addButton(symRadio, 0);
        matrixTypeGroup->addButton(triRadio, 1);
        topLayout->addWidget(matrixTypeBox);
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

    // w MainWindow::MainWindow(...)
    setWindowTitle(
        "Rozwiązywanie układu równań liniowych z macierzą symetryczną oraz "
        "macierzą trójdiagonalną metodą Crouta"
    );

    
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


// --------------------  MainWindow::solveSystem()  --------------------
void MainWindow::solveSystem()
{
    const int n     = matrixSizeSpinBox->value();
    const int dtype = dataTypeComboBox->currentIndex();   // 0=double, 1=mpreal, 2=interval
    const int mtype = matrixTypeGroup->checkedId();       // 0=symetryczna, 1=trójdiagonalna

    solutionTextEdit->clear();
    auto pad3 = [](const QString &s) {
        int e = s.indexOf('E');
        if (e < 0 || e+2 >= s.size()) return s;
        QString d = s.mid(e+2);
        if (d.size() == 2) d.prepend('0');
        return s.left(e+2) + d;
    };

    QStringList out;
    int status = 0;  // 0=OK, 1=zerowa szerokość (interval), 2=NaN/Inf (double), 3=singularność

    /* ------------------------------------------------------------------ */
    /*                             double                                 */
    /* ------------------------------------------------------------------ */
    if (dtype == 0) {
        auto A = getMatrixDouble();
        auto b = getVectorDouble();

        QVector<QVector<double>> U;
        QVector<double>          x, y;

        if (mtype == 0) {
            // symetryczny
            std::tie(std::ignore, U, y, x) = solveCroutSymmetric(A, b);
        } else {
            // tri-diagonalny
            QVector<double> a(n-1), d(n), c(n-1);
            for (int i = 0; i < n; ++i) {
                d[i] = A[i][i];
                if (i < n-1) c[i] = A[i][i+1];
                if (i > 0)   a[i-1] = A[i][i-1];
            }
            QList<double> lq, dq, uq, yq, xq;
            std::tie(lq, dq, uq, yq, xq) = solveCroutTridiagonal(a, d, c, b);

            U = QVector<QVector<double>>(n, QVector<double>(n,0.0));
            x.resize(n);
            for (int i = 0; i < n; ++i) {
                U[i][i] = dq[i];
                if (i < n-1) U[i][i+1] = uq[i];
                x[i] = xq[i];
            }
        }

        // 1) NaN/Inf w x
        for (double v : x) {
            if (std::isnan(v) || std::isinf(v)) {
                status = 2;
                break;
            }
        }
        // 2) pivot == 0 ⇒ singularność
        if (status == 0) {
            for (int i = 0; i < n; ++i) {
                if (U[i][i] == 0.0) {
                    status = 3;
                    break;
                }
            }
        }
        // 3) wypisz tylko gdy OK
        if (status == 0) {
            for (int i = 0; i < n; ++i) {
                QString xs = pad3(QString::asprintf("%.14E", x[i]).toUpper());
                out << QString("x[%1]=%2").arg(i+1).arg(xs);
            }
        }
    }

    /* ------------------------------------------------------------------ */
    /*                            mpreal                                 */
    /* ------------------------------------------------------------------ */
    else if (dtype == 1) {
        using mp = mpfr::mpreal;
        auto A = getMatrixMpreal();
        auto b = getVectorMpreal();

        QVector<QVector<mp>> U;
        QVector<mp>          x, y;

        if (mtype == 0) {
            std::tie(std::ignore, U, y, x) = solveCroutSymmetric(A, b);
        } else {
            QVector<mp> a(n-1), d(n), c(n-1);
            for (int i = 0; i < n; ++i) {
                d[i] = A[i][i];
                if (i < n-1) c[i] = A[i][i+1];
                if (i > 0)   a[i-1] = A[i][i-1];
            }
            QList<mp> lq, dq, uq, yq, xq;
            std::tie(lq, dq, uq, yq, xq) = solveCroutTridiagonal(a, d, c, b);

            U = QVector<QVector<mp>>(n, QVector<mp>(n,0));
            x.resize(n);
            for (int i = 0; i < n; ++i) {
                U[i][i] = dq[i];
                if (i < n-1) U[i][i+1] = uq[i];
                x[i] = xq[i];
            }
        }

        // pivot == 0 ⇒ singularność
        for (int i = 0; i < n && status == 0; ++i) {
            if (U[i][i] == mp(0)) {
                status = 3;
                break;
            }
        }
        // wypisz tylko gdy OK
        if (status == 0) {
            for (int i = 0; i < n; ++i) {
                QString xs = pad3(QString::asprintf("%.14E", x[i].toDouble()).toUpper());
                out << QString("x[%1]=%2").arg(i+1).arg(xs);
            }
        }
    }

    /* ------------------------------------------------------------------ */
    /*                   Interval<mpreal>                                  */
    /* ------------------------------------------------------------------ */
    else {
        using I = Interval<mpfr::mpreal>;
        auto A = getMatrixInterval();
        auto b = getVectorInterval();

        QVector<QVector<I>> U;
        QVector<I>          x, y;

        if (mtype == 0) {
            std::tie(std::ignore, U, y, x) = solveCroutSymmetric(A, b);
        } else {
            QVector<I> a(n-1), d(n), c(n-1);
            for (int i = 0; i < n; ++i) {
                d[i] = A[i][i];
                if (i < n-1) c[i] = A[i][i+1];
                if (i > 0)   a[i-1] = A[i][i-1];
            }
            QList<I> lq, dq, uq, yq, xq;
            std::tie(lq, dq, uq, yq, xq) = solveCroutTridiagonal(a, d, c, b);

            U = QVector<QVector<I>>(n, QVector<I>(n));
            x.resize(n);
            for (int i = 0; i < n; ++i) {
                U[i][i] = dq[i];
                if (i < n-1) U[i][i+1] = uq[i];
                x[i] = xq[i];
            }
        }

        // 1) szerokość > 0 ⇒ status=1
        for (int i = 0; i < n; ++i) {
            double w = (x[i].b - x[i].a).toDouble();
            if (w > 0.0) {
                status = 1;
                break;
            }
        }
        // 2) pivot zawiera zero ⇒ status=3
        if (status == 0) {
            for (int i = 0; i < n; ++i) {
                I p = U[i][i];
                if (p.a <= 0.0 && 0.0 <= p.b) {
                    status = 3;
                    break;
                }
            }
        }
        // 3) wypisz tylko gdy OK
        if (status == 0) {
            for (int i = 0; i < n; ++i) {
                double w = (x[i].b - x[i].a).toDouble();
                QString xs = QStringUtils::toQString(x[i])
                                 .toUpper()
                                 .replace(",", ";");
                QString ws = pad3(QString::asprintf("%.3E", w).toUpper());
                out << QString("x[%1]=[%2], szer.=%3")
                           .arg(i+1).arg(xs).arg(ws);
            }
        }
    }

    // — wyświetlamy —
    if (status == 0) {
        solutionTextEdit->append(out.join(", "));
    }
    solutionTextEdit->append(QString("st = %1").arg(status));
}