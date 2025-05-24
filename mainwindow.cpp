#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QFrame>
#include "qstring_utils.hpp"
#include <QHBoxLayout>    // <-- dodane
#include <QGroupBox>
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
    auto *central = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(central);
    setCentralWidget(central);

    // --- Controls ---
    auto *sizeLabel = new QLabel("Rozmiar macierzy:");
    matrixSizeSpinBox = new QSpinBox;
    matrixSizeSpinBox->setRange(2,20);
    matrixSizeSpinBox->setValue(3);

    auto *dataTypeLabel = new QLabel("Typ danych:");
    dataTypeComboBox = new QComboBox;
    dataTypeComboBox->addItems({
        "Zmiennoprzecinkowe (double)",
        "Wysokoprecyzyjne (mpreal)",
        "Przedziałowe"
    });

    auto *matrixTypeBox = new QGroupBox("Rodzaj macierzy:");
    auto *mtLayout = new QHBoxLayout(matrixTypeBox);
    symRadio = new QRadioButton("Symetryczna");
    triRadio = new QRadioButton("Trójdiagonalna");
    symRadio->setChecked(true);
    mtLayout->addWidget(symRadio);
    mtLayout->addWidget(triRadio);
    matrixTypeGroup = new QButtonGroup(this);
    matrixTypeGroup->addButton(symRadio,0);
    matrixTypeGroup->addButton(triRadio,1);

    solveButton = new QPushButton("Rozwiąż");

    auto *controls = new QHBoxLayout;
    controls->addWidget(sizeLabel);
    controls->addWidget(matrixSizeSpinBox);
    controls->addSpacing(20);
    controls->addWidget(dataTypeLabel);
    controls->addWidget(dataTypeComboBox);
    controls->addSpacing(20);
    controls->addWidget(matrixTypeBox);
    controls->addStretch();
    controls->addWidget(solveButton);

    // --- Matrix A & vector b inputs ---
    matrixLayout = new QGridLayout;
    auto *matrixGroup = new QGroupBox("Macierz A");
    matrixGroup->setLayout(matrixLayout);

    vectorLayout = new QVBoxLayout;
    auto *vectorGroup = new QGroupBox("Wektor b");
    vectorGroup->setLayout(vectorLayout);

    auto *inputLayout = new QHBoxLayout;
    inputLayout->addWidget(matrixGroup);
    auto *sep = new QFrame;
    sep->setFrameShape(QFrame::VLine);
    sep->setFrameShadow(QFrame::Sunken);
    inputLayout->addWidget(sep);
    inputLayout->addWidget(vectorGroup);

    // --- Output ---
    solutionTextEdit = new QTextEdit;
    solutionTextEdit->setReadOnly(true);
    auto *outputGroup = new QGroupBox("Wyniki");
    auto *outputLayout = new QVBoxLayout(outputGroup);
    outputLayout->addWidget(solutionTextEdit);

    // --- Assemble ---
    mainLayout->addLayout(controls);
    mainLayout->addSpacing(15);
    mainLayout->addLayout(inputLayout);
    mainLayout->addSpacing(15);
    mainLayout->addWidget(outputGroup);

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
    auto s = normalizeIntervalText(text).split(';');
    if (s.size() != 2) return false;
    mpreal a(s[0].toStdString()), b(s[1].toStdString());
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
    // clear old
    QLayoutItem *it;
    while ((it = matrixLayout->takeAt(0))) { delete it->widget(); delete it; }
    while ((it = vectorLayout->takeAt(0))) { delete it->widget(); delete it; }
    matrixInputs.clear(); matrixInputsInterval.clear();
    vectorInputs.clear();  vectorInputsInterval.clear();

    bool isInterval = (dataTypeComboBox->currentIndex() == 2);

    for (int i = 0; i < size; ++i) {
        QVector<QLineEdit*> row;
        QVector<QPair<QLineEdit*,QLineEdit*>> irow;
        for (int j = 0; j < size; ++j) {
            if (isInterval) {
                QFrame *cell = new QFrame;
                QHBoxLayout *lay = new QHBoxLayout(cell);
                lay->setContentsMargins(0,0,0,0);
                lay->setSpacing(2);

                QLabel *Lb = new QLabel("["), *Sb = new QLabel(";"), *Rb = new QLabel("]");
                QLineEdit *low = new QLineEdit("0"), *high = new QLineEdit("0");
                for (auto *lab:{Lb,Sb,Rb}) { lab->setFixedWidth(10); lab->setAlignment(Qt::AlignCenter); }
                for (auto *ed:{low,high}) { ed->setFixedWidth(40); ed->setAlignment(Qt::AlignCenter); }

                lay->addWidget(Lb);
                lay->addWidget(low);
                lay->addWidget(Sb);
                lay->addWidget(high);
                lay->addWidget(Rb);
                matrixLayout->addWidget(cell, i, j);
                irow.emplace_back(low, high);
            } else {
                QLineEdit *e = new QLineEdit("0");
                e->setFixedWidth(60);
                e->setAlignment(Qt::AlignCenter);
                matrixLayout->addWidget(e, i, j);
                row.append(e);
            }
        }
        if (isInterval) matrixInputsInterval.append(irow);
        else            matrixInputs.append(row);
    }

    // vector b
    if (isInterval) {
        for (int i = 0; i < size; ++i) {
            QFrame *cell = new QFrame;
            QHBoxLayout *lay = new QHBoxLayout(cell);
            lay->setContentsMargins(0,0,0,0);
            lay->setSpacing(2);

            QLabel *Lb=new QLabel("["), *Sb=new QLabel(";"), *Rb=new QLabel("]");
            QLineEdit *low=new QLineEdit("0"), *high=new QLineEdit("0");
            for (auto *lab:{Lb,Sb,Rb}) { lab->setFixedWidth(10); lab->setAlignment(Qt::AlignCenter); }
            for (auto *ed:{low,high}) { ed->setFixedWidth(40); ed->setAlignment(Qt::AlignCenter); }

            lay->addWidget(Lb);
            lay->addWidget(low);
            lay->addWidget(Sb);
            lay->addWidget(high);
            lay->addWidget(Rb);
            vectorLayout->addWidget(cell);
            vectorInputsInterval.emplace_back(low, high);
        }
    } else {
        for (int i = 0; i < size; ++i) {
            QLineEdit *e = new QLineEdit("0");
            e->setFixedWidth(60);
            e->setAlignment(Qt::AlignCenter);
            vectorLayout->addWidget(e);
            vectorInputs.append(e);
        }
    }
}

// --- Gettery ---

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
            auto [low,high] = matrixInputsInterval[i][j];
            mpreal a(low->text().toStdString()), b(high->text().toStdString());
            M[i][j] = (a<=b ? Interval<mpreal>(a,b) : Interval<mpreal>(0,0));
        }
    }
    return M;
}

QVector<Interval<mpreal>> MainWindow::getVectorInterval() const {
    QVector<Interval<mpreal>> v(vectorInputsInterval.size());
    for (int i = 0; i < v.size(); ++i) {
        auto [low,high] = vectorInputsInterval[i];
        mpreal a(low->text().toStdString()), b(high->text().toStdString());
        v[i] = (a<=b ? Interval<mpreal>(a,b) : Interval<mpreal>(0,0));
    }
    return v;
}

// --- solveSystem() ---

void MainWindow::solveSystem() {
    const int n     = matrixSizeSpinBox->value();
    const int dtype = dataTypeComboBox->currentIndex();   // 0=double,1=mpreal,2=interval
    const int mtype = matrixTypeGroup->checkedId();       // 0=symetryczna,1=trójdiagonalna

    solutionTextEdit->clear();

    // pomocnik do 3-cyfrowego wykładnika
    auto pad3exp = [](const QString &s){
        int e = s.indexOf('E');
        if (e<0||e+2>=s.size()) return s;
        QString d = s.mid(e+2);
        if (d.size()==2) d.prepend('0');
        return s.left(e+2)+d;
    };

    if (dtype == 0) {
        // --- double ---
        auto A = getMatrixDouble();
        auto b = getVectorDouble();
        QVector<QVector<double>> L, U;
        QVector<double> y, x;

        if (mtype == 0) {
            std::tie(L,U,y,x) = solveCroutSymmetric(A,b);
        } else {
            QVector<double> a(n-1), d(n), c(n-1);
            for (int i=0;i<n;++i) {
                d[i]=A[i][i];
                if (i<n-1) c[i]=A[i][i+1];
                if (i>0)   a[i-1]=A[i][i-1];
            }
            QList<double> l,diag,up,yL,xL;
            std::tie(l,diag,up,yL,xL)=solveCroutTridiagonal(a,d,c,b);
            L.resize(n, QVector<double>(n,0.0));
            U.resize(n, QVector<double>(n,0.0));
            y.resize(n); x.resize(n);
            for (int i=0;i<n;++i) {
                L[i][i]=1.0;
                if (i>0) L[i][i-1]=l[i-1];
                U[i][i]=diag[i];
                if (i<n-1) U[i][i+1]=up[i];
                y[i]=yL[i]; x[i]=xL[i];
            }
        }

        QStringList out;
        for (int i=0;i<n;++i) {
            QString xi = pad3exp(QString::asprintf("%.14E", x[i]).toUpper());
            out << QString("d[%1] = %2").arg(i+1).arg(xi);
        }
        solutionTextEdit->append(out.join(", "));
        solutionTextEdit->append("st = 0");
    }
    else if (dtype == 1) {
        // --- mpreal ---
        using M = mpreal;
        auto A = getMatrixMpreal();
        auto b = getVectorMpreal();
        QVector<QVector<M>> L, U;
        QVector<M> y, x;

        if (mtype == 0) {
            std::tie(L,U,y,x)=solveCroutSymmetric(A,b);
        } else {
            QVector<M> a(n-1), d(n), c(n-1);
            for (int i=0;i<n;++i) {
                d[i]=A[i][i];
                if (i<n-1) c[i]=A[i][i+1];
                if (i>0)   a[i-1]=A[i][i-1];
            }
            QList<M> l,diag,up,yL,xL;
            std::tie(l,diag,up,yL,xL)=solveCroutTridiagonal(a,d,c,b);
            L.resize(n, QVector<M>(n,0));
            U.resize(n, QVector<M>(n,0));
            y.resize(n); x.resize(n);
            for (int i=0;i<n;++i) {
                L[i][i]=1;
                if(i>0) L[i][i-1]=l[i-1];
                U[i][i]=diag[i];
                if(i<n-1) U[i][i+1]=up[i];
                y[i]=yL[i]; x[i]=xL[i];
            }
        }

        QStringList out;
        for (int i=0;i<n;++i) {
            double dv = x[i].toDouble();
            QString xi = pad3exp(QString::asprintf("%.14E", dv).toUpper());
            out << QString("d[%1] = %2").arg(i+1).arg(xi);
        }
        solutionTextEdit->append(out.join(", "));
        solutionTextEdit->append("st = 0");
    }
    else {
        // --- Interval<mpreal> ---
        using I = Interval<mpreal>;
        auto A = getMatrixInterval();
        auto b = getVectorInterval();
        QVector<QVector<I>> L, U;
        QVector<I> y, x;

        if (mtype == 0) {
            std::tie(L,U,y,x)=solveCroutSymmetric(A,b);
        } else {
            QVector<I> a(n-1), d(n), c(n-1);
            for (int i=0;i<n;++i) {
                d[i]=A[i][i];
                if (i<n-1) c[i]=A[i][i+1];
                if (i>0)   a[i-1]=A[i][i-1];
            }
            QList<I> l,diag,up,yL,xL;
            std::tie(l,diag,up,yL,xL)=solveCroutTridiagonal(a,d,c,b);
            L.resize(n, QVector<I>(n));
            U.resize(n, QVector<I>(n));
            y.resize(n); x.resize(n);
            for (int i=0;i<n;++i) {
                L[i][i]=I(1,1);
                if(i>0) L[i][i-1]=l[i-1];
                U[i][i]=diag[i];
                if(i<n-1) U[i][i+1]=up[i];
                y[i]=yL[i]; x[i]=xL[i];
            }
        }

        QStringList out;
        for (int i=0;i<n;++i) {
            QString xi = QStringUtils::toQString(x[i]).toUpper();
            xi.replace(",", ";").replace(" ", "");
            out << QString("d[%1] = %2").arg(i+1).arg(xi);
        }
        solutionTextEdit->append(out.join(", "));
        solutionTextEdit->append("st = 0");
    }
}
