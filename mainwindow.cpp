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
#include <QtCore/QStringBuilder> 

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
    matrixGroup->setStyleSheet("QGroupBox { border: 1px solid gray; margin-top: 10px; } QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 3px 0 3px; }");

    // === Wektor b ===
    vectorLayout = new QVBoxLayout();
    QGroupBox *vectorGroup = new QGroupBox("Vector b");
    vectorGroup->setLayout(vectorLayout);
    vectorGroup->setStyleSheet("QGroupBox { border: 1px solid gray; margin-top: 10px; } QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 3px 0 3px; }");


    QHBoxLayout *inputLayout = new QHBoxLayout();
    inputLayout->setSpacing(20); // opcjonalnie, lepszy odstęp

    inputLayout->addWidget(matrixGroup);

    // --- SEPARATOR ---
    QFrame *separator = new QFrame();
    separator->setFrameShape(QFrame::VLine);
    separator->setFrameShadow(QFrame::Sunken);
    separator->setLineWidth(1);
    separator->setMidLineWidth(0);
    inputLayout->addWidget(separator);
    // ------------------

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

        // w MainWindow::MainWindow(...), tuż po connect(matrixSizeSpinBox, …)
    connect(dataTypeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](){
        solutionTextEdit->clear();
        createMatrixInputs(matrixSizeSpinBox->value());
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


bool MainWindow::parseInterval(const QString &text, interval_arithmetic::Interval<mpfr::mpreal> &result) const {
    QString cleanedText = normalizeIntervalText(text);
    QStringList bounds = cleanedText.split(';');

    if (bounds.size() != 2)
        return false;

    bool ok = true;
    mpfr::mpreal a(bounds[0].toStdString(), 10);
    mpfr::mpreal b(bounds[1].toStdString(), 10);

    if (a > b) {
        ok = false;
    }

    result = interval_arithmetic::Interval<mpfr::mpreal>(a, b);
    return ok;
}

void MainWindow::highlightInvalidField(QLineEdit *field, bool isValid) const {
    if (isValid) {
        field->setStyleSheet("");
        field->setToolTip("");
    } else {
        field->setStyleSheet("background-color: #ffcccc;");
        field->setToolTip("Invalid input");
    }
}

QVector<QVector<interval_arithmetic::Interval<mpfr::mpreal>>> MainWindow::getMatrixInterval() const {
    QVector<QVector<interval_arithmetic::Interval<mpfr::mpreal>>> matrix(matrixInputsInterval.size());
    for (int i = 0; i < matrixInputsInterval.size(); ++i) {
        matrix[i].resize(matrixInputsInterval[i].size());
        for (int j = 0; j < matrixInputsInterval[i].size(); ++j) {
            QLineEdit *lowField = matrixInputsInterval[i][j].first;
            QLineEdit *highField = matrixInputsInterval[i][j].second;

            bool ok1 = true, ok2 = true;
            mpfr::mpreal a(lowField->text().toStdString());
            mpfr::mpreal b(highField->text().toStdString());

            bool valid = (a <= b);
            matrix[i][j] = valid ? interval_arithmetic::Interval<mpfr::mpreal>(a, b)
                                 : interval_arithmetic::Interval<mpfr::mpreal>(0, 0);

            QString style = valid ? "" : "background-color: #ffcccc;";
            lowField->setStyleSheet(style);
            highField->setStyleSheet(style);
        }
    }
    return matrix;
}

QVector<interval_arithmetic::Interval<mpfr::mpreal>> MainWindow::getVectorInterval() const {
    QVector<interval_arithmetic::Interval<mpfr::mpreal>> vector(vectorInputsInterval.size());
    for (int i = 0; i < vectorInputsInterval.size(); ++i) {
        QLineEdit *lowField = vectorInputsInterval[i].first;
        QLineEdit *highField = vectorInputsInterval[i].second;

        bool ok1 = true, ok2 = true;
        mpfr::mpreal a(lowField->text().toStdString());
        mpfr::mpreal b(highField->text().toStdString());

        bool valid = (a <= b);
        vector[i] = valid ? interval_arithmetic::Interval<mpfr::mpreal>(a, b)
                          : interval_arithmetic::Interval<mpfr::mpreal>(0, 0);

        QString style = valid ? "" : "background-color: #ffcccc;";
        lowField->setStyleSheet(style);
        highField->setStyleSheet(style);
    }
    return vector;
}


void MainWindow::createMatrixInputs(int size) {
    // Usuń stare widgety
    QLayoutItem *child;
    while ((child = matrixLayout->takeAt(0))) {
        delete child->widget();
        delete child;
    }
    while ((child = vectorLayout->takeAt(0))) {
        delete child->widget();
        delete child;
    }
    matrixInputs.clear();
    matrixInputsInterval.clear();
    vectorInputs.clear();
    vectorInputsInterval.clear();

    bool isInterval = (dataTypeComboBox->currentText() == "interval");

    // === MACIERZ A ===
    for (int i = 0; i < size; ++i) {
        QVector<QLineEdit*>        row;
        QVector<QPair<QLineEdit*,QLineEdit*>> intervalRow;
        for (int j = 0; j < size; ++j) {
            if (isInterval) {
                // przedział [low;high]
                QFrame *cell = new QFrame();
                QHBoxLayout *lay = new QHBoxLayout(cell);
                lay->setContentsMargins(0,0,0,0);
                lay->setSpacing(2);

                QLabel    *Lb = new QLabel("[");
                QLineEdit *low = new QLineEdit("0");
                QLabel    *Sb = new QLabel(";");
                QLineEdit *high= new QLineEdit("0");
                QLabel    *Rb = new QLabel("]");

                // najpierw ustalamy sizePolicy dla etykiet
                for (QLabel *lab : {Lb,Sb,Rb}) {
                    lab->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
                    lab->setFixedWidth(10);
                    lab->setAlignment(Qt::AlignCenter);
                }
                // potem dla pól edycji
                for (QLineEdit *edit : {low, high}) {
                    edit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
                    edit->setFixedWidth(40);
                    edit->setAlignment(Qt::AlignCenter);
                }

                lay->addWidget(Lb);
                lay->addWidget(low);
                lay->addWidget(Sb);
                lay->addWidget(high);
                lay->addWidget(Rb);
                matrixLayout->addWidget(cell, i, j);

                intervalRow.emplace_back(low, high);
            }
            else {
                QLineEdit *e = new QLineEdit("0");
                e->setFixedWidth(60);
                e->setAlignment(Qt::AlignCenter);
                matrixLayout->addWidget(e, i, j);
                row.append(e);
            }
        }
        if (isInterval)
            matrixInputsInterval.append(intervalRow);
        else
            matrixInputs.append(row);
    }

    // === WEKTOR b ===
    if (isInterval) {
        for (int i = 0; i < size; ++i) {
            QFrame *cell = new QFrame();
            QHBoxLayout *lay = new QHBoxLayout(cell);
            lay->setContentsMargins(0,0,0,0);
            lay->setSpacing(2);

            QLabel    *Lb = new QLabel("[");
            QLineEdit *low = new QLineEdit("0");
            QLabel    *Sb = new QLabel(";");
            QLineEdit *high= new QLineEdit("0");
            QLabel    *Rb = new QLabel("]");

            for (QLabel *lab : {Lb,Sb,Rb}) {
                lab->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
                lab->setFixedWidth(10);
                lab->setAlignment(Qt::AlignCenter);
            }
            for (QLineEdit *edit : {low, high}) {
                edit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
                edit->setFixedWidth(40);
                edit->setAlignment(Qt::AlignCenter);
            }

            lay->addWidget(Lb);
            lay->addWidget(low);
            lay->addWidget(Sb);
            lay->addWidget(high);
            lay->addWidget(Rb);
            vectorLayout->addWidget(cell);

            vectorInputsInterval.emplace_back(low, high);
        }
    }
    else {
        for (int i = 0; i < size; ++i) {
            QLineEdit *e = new QLineEdit("0");
            e->setFixedWidth(60);
            e->setAlignment(Qt::AlignCenter);
            vectorLayout->addWidget(e);
            vectorInputs.append(e);
        }
    }
}




void MainWindow::highlightInvalidField(QLineEdit *field, bool isValid, const QString &message) const {
    if (isValid) {
        field->setStyleSheet("");
        field->setToolTip("");  // czyść
    } else {
        QString color = (message.contains("a > b")) ? "#fff2cc" : "#ffcccc";
        field->setStyleSheet(QString("background-color: %1;").arg(color));
        field->setToolTip(message);
    }
}


QString MainWindow::normalizeIntervalText(const QString &text) const {
    QString cleaned = text.trimmed();
    cleaned.replace(",", ";");
    cleaned.replace(QRegularExpression("\\s+"), ""); // usuń wszystkie spacje
    return cleaned;
}



void MainWindow::solveSystem() {
    const int n = matrixSizeSpinBox->value();
    const QString matrixType = matrixTypeComboBox->currentText();
    const QString dataType   = dataTypeComboBox->currentText();

    solutionTextEdit->clear();

    if (dataType == "double") {
        // 1) Pobierz A i b
        auto A = getMatrixDouble();
        auto b = getVectorDouble();

        // 2) Rozwiąż
        QVector<QVector<double>> L, U;
        QVector<double> y, x;
        if (matrixType == "general") {
            std::tie(L, U, y, x) = solveCroutGeneral(A, b);
        }
        else if (matrixType == "symmetric") {
            std::tie(L, U, y, x) = solveCroutSymmetric(A, b);
        }
        else {  // tridiagonal
            QVector<double> a(n-1), d(n), c(n-1);
            for (int i = 0; i < n; ++i) {
                d[i] = A[i][i];
                if (i < n-1) c[i] = A[i][i+1];
                if (i > 0)   a[i-1] = A[i][i-1];
            }
            QList<double> l, diag, up, yL, xL;
            std::tie(l, diag, up, yL, xL) = solveCroutTridiagonal(a, d, c, b);

            L = QVector<QVector<double>>(n, QVector<double>(n, 0.0));
            U = QVector<QVector<double>>(n, QVector<double>(n, 0.0));
            y.resize(n); x.resize(n);
            for (int i = 0; i < n; ++i) {
                L[i][i] = 1.0;
                if (i > 0)       L[i][i-1] = l[i-1];
                U[i][i] = diag[i];
                if (i < n-1)     U[i][i+1] = up[i];
                y[i]    = yL[i];
                x[i]    = xL[i];
            }
        }

        displaySolutionDetails(L, U, y, x);
    }
    else if (dataType == "mpreal") {
        using M = mpfr::mpreal;
        auto A = getMatrixMpreal();
        auto b = getVectorMpreal();

        QVector<QVector<M>> L, U;
        QVector<M> y, x;
        if (matrixType == "general") {
            std::tie(L, U, y, x) = solveCroutGeneral(A, b);
        }
        else if (matrixType == "symmetric") {
            std::tie(L, U, y, x) = solveCroutSymmetric(A, b);
        }
        else {  // tridiagonal
            QVector<M> a(n-1), d(n), c(n-1);
            for (int i = 0; i < n; ++i) {
                d[i] = A[i][i];
                if (i < n-1) c[i] = A[i][i+1];
                if (i > 0)   a[i-1] = A[i][i-1];
            }
            QList<M> l, diag, up, yL, xL;
            std::tie(l, diag, up, yL, xL) = solveCroutTridiagonal(a, d, c, b);

            L = QVector<QVector<M>>(n, QVector<M>(n, 0));
            U = QVector<QVector<M>>(n, QVector<M>(n, 0));
            y.resize(n); x.resize(n);
            for (int i = 0; i < n; ++i) {
                L[i][i] = 1;
                if (i > 0)       L[i][i-1] = l[i-1] / diag[i-1];
                U[i][i] = diag[i];
                if (i < n-1)     U[i][i+1] = up[i];
                y[i]    = yL[i];
                x[i]    = xL[i];
            }
        }

        displaySolutionDetails(L, U, y, x);
    }
    else {  // interval<mpreal>
        using I = interval_arithmetic::Interval<mpfr::mpreal>;
        auto A = getMatrixInterval();
        auto b = getVectorInterval();

        QVector<QVector<I>> L, U;
        QVector<I> y, x;
        if (matrixType == "general") {
            std::tie(L, U, y, x) = solveCroutGeneral(A, b);
        }
        else if (matrixType == "symmetric") {
            std::tie(L, U, y, x) = solveCroutSymmetric(A, b);
        }
        else {  // tridiagonal
            QVector<I> a(n-1), d(n), c(n-1);
            for (int i = 0; i < n; ++i) {
                d[i] = A[i][i];
                if (i < n-1) c[i] = A[i][i+1];
                if (i > 0)   a[i-1] = A[i][i-1];
            }
            QList<I> l, diag, up, yL, xL;
            std::tie(l, diag, up, yL, xL) = solveCroutTridiagonal(a, d, c, b);

            L = QVector<QVector<I>>(n, QVector<I>(n));
            U = QVector<QVector<I>>(n, QVector<I>(n));
            y.resize(n); x.resize(n);
            for (int i = 0; i < n; ++i) {
                L[i][i] = I(1,1);
                if (i > 0)       L[i][i-1] = l[i-1];
                U[i][i] = diag[i];
                if (i < n-1)     U[i][i+1] = up[i];
                y[i]    = yL[i];
                x[i]    = xL[i];
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


// --- DISPLAY ---



// --- helper to force 3-digit exponent in a "%.14E" string ---
static QString pad3exp(const QString &s) {
    int epos = s.indexOf('E');
    if (epos < 0 || epos + 2 >= s.size()) return s;
    QChar sign = s[epos+1];       // '+' or '-'
    QString digs = s.mid(epos+2); // e.g. "01" or "10"
    if (digs.size() == 2) 
        digs.prepend('0');        // "01" -> "001"
    return s.left(epos+2) + digs; // "…E-0" + "01"
}

// --- DISPLAY for double --------------------------------------
void MainWindow::displaySolutionDetails(
    const QVector<QVector<double>> & /*L*/,
    const QVector<QVector<double>> & /*U*/,
    const QVector<double> &        /*y*/,
    const QVector<double> &         x)
{
    QStringList parts;
    for (int i = 0; i < x.size(); ++i) {
        // "%.14E" gives 14 digits + scientific; then pad to 3-digit exponent
        QString xi = pad3exp(QString::asprintf("%.14E", x[i]).toUpper());
        parts << QString("d[%1] = %2").arg(i+1).arg(xi);
    }
    solutionTextEdit->append(parts.join(", "));
    solutionTextEdit->append("st = 0");
}

// --- DISPLAY for mpreal ------------------------------------
void MainWindow::displaySolutionDetails(
    const QVector<QVector<mpfr::mpreal>> & /*L*/,
    const QVector<QVector<mpfr::mpreal>> & /*U*/,
    const QVector<mpfr::mpreal> &         /*y*/,
    const QVector<mpfr::mpreal> &         x)
{
    QStringList parts;
    for (int i = 0; i < x.size(); ++i) {
        double d = x[i].toDouble(); 
        QString xi = pad3exp(QString::asprintf("%.14E", d).toUpper());
        parts << QString("d[%1] = %2").arg(i+1).arg(xi);
    }
    solutionTextEdit->append(parts.join(", "));
    solutionTextEdit->append("st = 0");
}

// --- DISPLAY for interval<mpreal> --------------------------
void MainWindow::displaySolutionDetails(
    const QVector<QVector<interval_arithmetic::Interval<mpfr::mpreal>>> & /*L*/,
    const QVector<QVector<interval_arithmetic::Interval<mpfr::mpreal>>> & /*U*/,
    const QVector<interval_arithmetic::Interval<mpfr::mpreal>> &         /*y*/,
    const QVector<interval_arithmetic::Interval<mpfr::mpreal>> &         x)
{
    // for intervals we just show the full "[aE…,bE…]" via your QStringUtils helper:
    QStringList parts;
    for (int i = 0; i < x.size(); ++i) {
        QString xi = QStringUtils::toQString(x[i]).toUpper();
        parts << QString("d[%1] = %2").arg(i+1).arg(xi);
    }
    solutionTextEdit->append(parts.join(", "));
    solutionTextEdit->append("st = 0");
}
