// mainwindow.cpp

#include "mainwindow.h"
#include <cmath>  // dla std::isnan i std::isinf
#include "interval.hpp"

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
#include "interval_rounding_fix.hpp"

namespace IA = interval_arithmetic;                 
using I = IA::Interval<mpfr::mpreal>;        

using namespace solver::general;
using namespace solver::symmetric;
using namespace solver::tridiagonal;
using namespace mpfr;
using namespace interval_arithmetic;


/*--------------------------------------------------------------*/
/*  Odczytuje QLineEdit jako IA::Interval                       */
/*--------------------------------------------------------------*/
/*--------------------------------------------------------------*/
/*  Odczytuje QLineEdit jako IA::Interval                       */
/*--------------------------------------------------------------*/

/*--------------------------------------------------------------*/
/*  Odczytuje QLineEdit jako IA::Interval                       */
/*  Separator dziesiętny: '.'                                    */
/*  Separator przedziału: ';'                                    */
/*--------------------------------------------------------------*/


static I readIntervalCell(const QLineEdit* e)
{
    // 1) Usuń białe znaki:
    QString txt = e->text().trimmed().replace(QRegularExpression("\\s+"), "");
    // 2) Rozdziel tylko po średniku ';'
    const auto parts = txt.split(';', Qt::SkipEmptyParts);

    // --- Obliczamy maszynowe epsilon dla mpreal (~2^(-precision)) ---
    static const mpreal eps = std::numeric_limits<mpfr::mpreal>::epsilon();
    ;  // jeśli precyzja=40 bitów => eps ≈ 2^(-40)

    // --- Jeśli użytkownik nie podał średnika (tylko jedna liczba) ---
    if (parts.size() == 1) {
        mpreal v(parts[0].toStdString());        // np. "2.5" → 2.5
        // Zwracamy [v - eps ; v + eps]
        return I(v - eps, v + eps);
    }

    // --- Jeśli są dokładnie dwa fragmenty: 'a;b' ---
    if (parts.size() == 2) {
        mpreal a(parts[0].trimmed().toStdString());
        mpreal b(parts[1].trimmed().toStdString());
        if (a > b) std::swap(a, b);
        // Zwracamy [a - eps ; b + eps]
        return I(a - eps, b + eps);
    }

    // --- W każdym innym (błędnym) wypadku dajemy [0;0] powiększone o ±eps: ---
    mpreal ZERO = mpreal(0);
    return I(ZERO - eps, ZERO + eps);
}



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    IA::Interval<mpfr::mpreal>::Initialize();          
    IA::Interval<mpfr::mpreal>::SetMode( IA::DINT_MODE ); 

    // Central widget & main layout
    auto *central    = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(central);
    setCentralWidget(central);

    // --- Top controls ---
    auto *topLayout = new QHBoxLayout;
    {
        // Rozmiar macierzy
        auto *sizeLabel       = new QLabel("Rozmiar macierzy:");
        matrixSizeSpinBox     = new QSpinBox;
        matrixSizeSpinBox->setRange(2,20);
        matrixSizeSpinBox->setValue(3);
        topLayout->addWidget(sizeLabel);
        topLayout->addWidget(matrixSizeSpinBox);

        topLayout->addStretch(1);  

        // Typ danych
        auto *typeLabel       = new QLabel("Typ danych:");
        dataTypeComboBox      = new QComboBox;
        dataTypeComboBox->addItems({
            "Zmiennoprzecinkowe",
            "Wysokoprecyzyjne",
            "Przedziałowe"
        });
        topLayout->addWidget(typeLabel);
        topLayout->addWidget(dataTypeComboBox);

        // Rodzaj macierzy (symetryczna / trójdiagonalna)
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

    // --- Middle: macierz A i wektor b ---
    auto *inputLayout = new QHBoxLayout;
    {
        // Macierz A
        matrixLayout   = new QGridLayout;
        auto *matrixGroup = new QGroupBox("Macierz A");
        matrixGroup->setLayout(matrixLayout);
        inputLayout->addWidget(matrixGroup);

        // Separator
        auto *sep = new QFrame;
        sep->setFrameShape(QFrame::VLine);
        sep->setFrameShadow(QFrame::Sunken);
        inputLayout->addWidget(sep);

        // Wektor b
        vectorLayout   = new QVBoxLayout;
        auto *vectorGroup = new QGroupBox("Wektor b");
        vectorGroup->setLayout(vectorLayout);
        inputLayout->addWidget(vectorGroup);
    }
    mainLayout->addLayout(inputLayout);
    mainLayout->addSpacing(15);

    // --- Przycisk „Rozwiąż” ---
    {
        solveButton = new QPushButton("Rozwiąż");
        auto *btnLayout = new QHBoxLayout;
        btnLayout->addStretch(1);
        btnLayout->addWidget(solveButton);
        btnLayout->addStretch(1);
        mainLayout->addLayout(btnLayout);
    }
    mainLayout->addSpacing(15);

    // --- Wyniki ---
    {
        solutionTextEdit = new QTextEdit;
        solutionTextEdit->setReadOnly(true);
        auto *outputGroup  = new QGroupBox("Wyniki");
        auto *outputLayout = new QVBoxLayout(outputGroup);
        outputLayout->addWidget(solutionTextEdit);
        mainLayout->addWidget(outputGroup);
    }

    // --- Sygnały ---
    connect(matrixSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::createMatrixInputs);
    connect(dataTypeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this]{ createMatrixInputs(matrixSizeSpinBox->value()); });
    connect(solveButton, &QPushButton::clicked,
            this, &MainWindow::solveSystem);

    // Pierwsze wypełnienie pól
    createMatrixInputs(matrixSizeSpinBox->value());

    setWindowTitle(
        "Rozwiązywanie układu równań liniowych z macierzą symetryczną oraz "
        "macierzą trójdiagonalną metodą Crouta"
    );
    resize(900,700);
}

MainWindow::~MainWindow() = default;

// --- Helpers (normalizeIntervalText i parseInterval – nie ruszamy) ---

// QString MainWindow::normalizeIntervalText(const QString &text) const {
//     QString s = text.trimmed();
//     s.replace(",", ";");
//     s.replace(QRegularExpression("\\s+"), "");
//     return s;
// }

bool MainWindow::parseInterval(const QString &text, Interval<mpreal> &out) const {
    QString t = text.trimmed();
    auto parts = t.split(';');
    if (parts.size() != 2) return false;

    try {
        mpreal a(parts[0].toStdString());
        mpreal b(parts[1].toStdString());
        if (a > b) return false; // lub std::swap(a,b)
        out = Interval<mpreal>(a, b);
        return true;
    }
    catch (...) {
        return false;
    }
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

// --- Tworzenie pól macierzy i wektora b ---
void MainWindow::createMatrixInputs(int size) {
    // Usuwamy dotychczasowe widgety
    QLayoutItem *it;
    while ((it = matrixLayout->takeAt(0))) { delete it->widget(); delete it; }
    while ((it = vectorLayout->takeAt(0))) { delete it->widget(); delete it; }

    matrixInputs.clear();
    matrixInputsInterval.clear();
    vectorInputs.clear();
    vectorInputsInterval.clear();

    bool isInterval = (dataTypeComboBox->currentIndex() == 2);

    // Macierz A
    for (int i = 0; i < size; ++i) {
        QVector<QLineEdit*> row;
        for (int j = 0; j < size; ++j) {
            if (isInterval) {
                // Dla przedziałów zakładamy domyślnie "[0;0]", wpisane jako "0,0"
                QLineEdit *cell = new QLineEdit("0.0;0.0");
                cell->setFixedWidth(80);
                cell->setAlignment(Qt::AlignCenter);
                matrixLayout->addWidget(cell, i, j);
                row.append(cell);
            } else {
                auto *e = new QLineEdit("0");
                e->setFixedWidth(60);
                e->setAlignment(Qt::AlignCenter);
                matrixLayout->addWidget(e, i, j);
                row.append(e);
            }
        }
        if (isInterval)
            matrixInputsInterval.append(row);
        else
            matrixInputs.append(row);
    }

    // Wektor b
    if (isInterval) {
        for (int i = 0; i < size; ++i) {
            auto *cell = new QLineEdit("0;0");
            cell->setFixedWidth(80);
            cell->setAlignment(Qt::AlignCenter);
            vectorLayout->addWidget(cell);
            vectorInputsInterval.append(cell);
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

// --- Gettery: zwracają macierz / wektor dla double, mpreal i interval ---

QVector<QVector<double>> MainWindow::getMatrixDouble() const {
    QVector<QVector<double>> M(matrixInputs.size());
    for (int i = 0; i < M.size(); ++i) {
        M[i].resize(matrixInputs[i].size());
        for (int j = 0; j < M[i].size(); ++j) {
            M[i][j] = matrixInputs[i][j]->text().toDouble();
        }
    }
    return M;
}

QVector<double> MainWindow::getVectorDouble() const {
    QVector<double> v(vectorInputs.size());
    for (int i = 0; i < v.size(); ++i) {
        v[i] = vectorInputs[i]->text().toDouble();
    }
    return v;
}

QVector<QVector<mpreal>> MainWindow::getMatrixMpreal() const {
    QVector<QVector<mpreal>> M(matrixInputs.size());
    for (int i = 0; i < M.size(); ++i) {
        M[i].resize(matrixInputs[i].size());
        for (int j = 0; j < M[i].size(); ++j) {
            M[i][j] = mpreal(matrixInputs[i][j]->text().toStdString());
        }
    }
    return M;
}

QVector<mpreal> MainWindow::getVectorMpreal() const {
    QVector<mpreal> v(vectorInputs.size());
    for (int i = 0; i < v.size(); ++i) {
        v[i] = mpreal(vectorInputs[i]->text().toStdString());
    }
    return v;
}

QVector<QVector<I>> MainWindow::getMatrixInterval() const {
    QVector<QVector<I>> M(matrixInputsInterval.size());
    for (int i = 0; i < M.size(); ++i) {
        M[i].resize(matrixInputsInterval[i].size());
        for (int j = 0; j < M[i].size(); ++j) {
            M[i][j] = readIntervalCell(matrixInputsInterval[i][j]);
        }
    }
    return M;
}

QVector<I> MainWindow::getVectorInterval() const {
    QVector<I> v(vectorInputsInterval.size());
    for (int i = 0; i < v.size(); ++i) {
        v[i] = readIntervalCell(vectorInputsInterval[i]);
    }
    return v;
}


// --------------------  MainWindow::solveSystem()  --------------------
void MainWindow::solveSystem()
{
    const int n     = matrixSizeSpinBox->value();
    const int dtype = dataTypeComboBox->currentIndex();   // 0=double, 1=mpreal, 2=interval
    const int mtype = matrixTypeGroup->checkedId();       // 0=symetryczna, 1=trójdiagonalna

    solutionTextEdit->clear();

    // Pomocnicza do wyrównania wykładnika „E”
    auto pad3 = [](const QString &s) {
        int e = s.indexOf('E');
        if (e < 0 || e+2 >= s.size()) return s;
        QString d = s.mid(e+2);
        if (d.size() == 2) d.prepend('0');
        return s.left(e+2) + d;
    };

    QStringList out;
    int status = 0;  // 0=OK, 1=szerokość>0 (interval), 2=NaN/Inf (double), 3=singularność

    /* ======================== double ======================== */
    if (dtype == 0) {
        auto A = getMatrixDouble();
        auto b = getVectorDouble();

        QVector<QVector<double>> U;
        QVector<double>          x, y;

        if (mtype == 0) {
            // Symetryczny
            std::tie(std::ignore, U, y, x) = solveCroutSymmetric(A, b);
        } else {
            // Trójdiagonalny
            QVector<double> a(n-1), d(n), c(n-1);
            for (int i = 0; i < n; ++i) {
                d[i] = A[i][i];
                if (i < n-1) c[i] = A[i][i+1];
                if (i > 0)   a[i-1] = A[i][i-1];
            }
            QList<double> lq, dq, uq, yq, xq;
            std::tie(lq, dq, uq, yq, xq)
                = solveCroutTridiagonal(a, d, c, b);

            U = QVector<QVector<double>>(n, QVector<double>(n,0.0));
            x.resize(n);
            for (int i = 0; i < n; ++i) {
                U[i][i] = dq[i];
                if (i < n-1) U[i][i+1] = uq[i];
                x[i] = xq[i];
            }
        }

        // 1) NaN/Inf?
        for (double v : x) {
            if (std::isnan(v) || std::isinf(v)) {
                status = 2;
                break;
            }
        }
        // 2) singularność (pivot==0)
        if (status == 0) {
            for (int i = 0; i < n; ++i) {
                if (U[i][i] == 0.0) {
                    status = 3;
                    break;
                }
            }
        }
        // 3) wypisz tylko, gdy OK
        if (status == 0) {
            for (int i = 0; i < n; ++i) {
                QString xs = pad3(QString::asprintf("%.14E", x[i]).toUpper());
                out << QString("x[%1]=%2").arg(i+1).arg(xs);
            }
        }
    }

    /* ======================= mpreal ======================== */
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
            std::tie(lq, dq, uq, yq, xq)
                = solveCroutTridiagonal(a, d, c, b);

            U = QVector<QVector<mp>>(n, QVector<mp>(n,0));
            x.resize(n);
            for (int i = 0; i < n; ++i) {
                U[i][i] = dq[i];
                if (i < n-1) U[i][i+1] = uq[i];
                x[i] = xq[i];
            }
        }

        // singularność (pivot==0)
        for (int i = 0; i < n && status == 0; ++i) {
            if (U[i][i] == mp(0)) {
                status = 3;
                break;
            }
        }
        // wypisz tylko, gdy OK
        if (status == 0) {
            for (int i = 0; i < n; ++i) {
                QString xs = pad3(QString::asprintf("%.14E", x[i].toDouble()).toUpper());
                out << QString("x[%1]=%2").arg(i+1).arg(xs);
            }
        }
    }

      /* ===================== Interval ======================= */
    // --- fragment solveSystem(), gałąź dtype==2 ---
    else if (dtype == 2) {                       // dtype == 2
        using I = IA::Interval<mpfr::mpreal>;

        // 0. Pobranie przedziałów z GUI --------------------------------
        const auto A = getMatrixInterval();
        const auto b = getVectorInterval();

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

        // 1. Sprawdzenie, czy któryś wynik zawiera zero → singularność ----
        if (status == 0) {
            for (int i = 0; i < n; ++i) {
                if (x[i].containsZero()) {
                    status = 3;
                    break;
                }
            }
        }

        // 2. Przygotowanie wyników do wyświetlenia lub st = kod błędu -------
        if (status == 0) {
            // Ustawiamy EPS = 2^-100, poniżej którego traktujemy szerokość jako dokładnie 0
            const mpfr::mpreal EPS = mpfr::pow(mpreal(2), -100);  // ~7.9e-31

            QStringList lines;
            for (int i = 0; i < n; ++i) {
                // Rozpakowujemy końce przedziału do łańcuchów:
                std::string ls, rs;
                x[i].IEndsToStrings(ls, rs);

                // Obliczamy szerokość przedziału:
                mpfr::mpreal w = x[i].GetWidth();
                // Jeśli szerokość jest ujemna (rzadko, bo GetWidth zwraca >=0), bierzemy wartość bezwzględną:
                if (w < 0) w = -w;

                // Formatujemy szerokość:
                QString wtxt;
                if (w < EPS) {
                    // Jeżeli „praktycznie 0” (poniżej EPS), wypisujemy dokładnie "0"
                    wtxt = "0";
                } else {
                    // W przeciwnym razie wypisujemy w notacji naukowej z 6 cyframi znaczącymi
                    wtxt = QString::fromStdString(
                        w.toString(6, std::ios_base::scientific)
                    ).toUpper();
                }

                // Składamy linię typu: x[i] = [lewo ; prawo]    szerokość = wtxt
                lines << QString("x[%1] = [%2 ; %3]    szerokość = %4")
                             .arg(i + 1)                             // numer zmiennej
                             .arg(QString::fromStdString(ls).toUpper())  // lewy koniec przedziału
                             .arg(QString::fromStdString(rs).toUpper())  // prawy koniec przedziału
                             .arg(wtxt);                                // szerokość
            }

            // Wstawiamy wszystkie linie do QTextEdit (każda linia w nowym wierszu)
            solutionTextEdit->setPlainText(lines.join('\n'));
        } else {
            // Jeśli status != 0, wypisujemy tylko kod błędu
            solutionTextEdit->setPlainText(QString("st = %1").arg(status));
        }
    } // ← koniec gałęzi dtype==2
    // … (następne nawiasy zamykające funkcji i klasy)
}
