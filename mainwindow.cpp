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
#include "CroutSolver.h"



using namespace mpfr;
using namespace interval_arithmetic;

MainWindow::~MainWindow() = default;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    qApp->setStyleSheet(R"(
        QWidget {
            background-color: #2b2b2b;
            color: #f0f0f0;
            font-family: Consolas, monospace;
            font-size: 14px;
        }
        QLineEdit {
            background-color: #3c3f41;
            border: 1px solid #555;
            border-radius: 4px;
            padding: 4px;
            color: #f0f0f0;
        }
        QLineEdit:focus {
            border: 1px solid #007acc;
        }
        QPushButton {
            background-color: #007acc;
            border: none;
            padding: 6px 12px;
            color: white;
            border-radius: 4px;
        }
        QPushButton:hover {
            background-color: #005999;
        }
        QGroupBox {
            border: 1px solid #555;
            margin-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 0 3px;
        }
    )");
    

    QWidget *central = new QWidget(this);
    controlsLayout = new QHBoxLayout;

    // Pole wyboru typu macierzy
    controlsLayout->addWidget(new QLabel("Typ macierzy:"));
    matrixTypeComboBox = new QComboBox(this);
    matrixTypeComboBox->addItem("Dowolna", "general");
    matrixTypeComboBox->addItem("Symetryczna", "symmetric");
    matrixTypeComboBox->addItem("Trójdiagonalna", "tridiagonal");
    controlsLayout->addWidget(matrixTypeComboBox);


    // Layout górny — kontrolki
    controlsLayout->addWidget(new QLabel("Rozmiar macierzy:"));
    matrixSizeSpinBox = new QSpinBox(this);
    matrixSizeSpinBox->setRange(2, 10);
    matrixSizeSpinBox->setValue(3);
    controlsLayout->addWidget(matrixSizeSpinBox);

    controlsLayout->addWidget(new QLabel("Typ danych:"));
    dataTypeComboBox = new QComboBox(this);
    dataTypeComboBox->addItem("Liczby zmiennoprzecinkowe (double)", "double");
    dataTypeComboBox->addItem("Liczby wysokiej precyzji (mpreal)", "mpreal");
    dataTypeComboBox->addItem("Przedziały liczbowe (interval)", "interval");
    controlsLayout->addWidget(dataTypeComboBox);
    controlsLayout->addStretch();


    mainLayout = new QVBoxLayout(central);  // <-- TO JEST KLUCZOWE!
    mainLayout->addLayout(controlsLayout);

    QLabel *labelA = new QLabel("Macierz A:", this);
    labelA->setAlignment(Qt::AlignLeft);
    

    // Layouty dla macierzy i wektora — tworzenie
    matrixLayout = new QGridLayout;
    vectorLayout = new QVBoxLayout;
    matrixLayout->setSpacing(2);
    vectorLayout->setSpacing(2);

    // Kolumna dla macierzy A z etykietą
    QVBoxLayout *matrixColumn = new QVBoxLayout;
    matrixColumn->addWidget(new QLabel("Macierz A:", this));
    matrixColumn->addLayout(matrixLayout);

    // Separator
    QFrame *separator = new QFrame(this);
    separator->setFrameShape(QFrame::VLine);
    separator->setLineWidth(1);

    // Kolumna dla wektora b z etykietą
    QVBoxLayout *vectorColumn = new QVBoxLayout;
    vectorColumn->addWidget(new QLabel("Wektor b:", this));
    vectorColumn->addLayout(vectorLayout);

    // Layout grupujący macierz i wektor
    QHBoxLayout *matrixGroup = new QHBoxLayout;
    matrixGroup->addLayout(matrixColumn);
    matrixGroup->addWidget(separator);
    matrixGroup->addLayout(vectorColumn);

    QGroupBox *inputGroup = new QGroupBox("Dane wejściowe", this);
    inputGroup->setLayout(matrixGroup);
    inputGroup->setContentsMargins(10, 10, 10, 10);
    inputGroup->setStyleSheet(R"(
        QGroupBox {
            background-color: transparent;
            border: 1px solid #555;
            margin-top: 10px;
            color: #f0f0f0;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 3px;
            color: #f0f0f0;
        }
    )");
    

    mainLayout->addWidget(inputGroup);

    // Przycisk i pole wynikowe
    solveButton = new QPushButton("Rozwiąż", this);
    mainLayout->addWidget(solveButton);

    solutionTextEdit = new QTextEdit(this);
    solutionTextEdit->setStyleSheet(R"(
        QTextEdit {
            font-family: Consolas, monospace;
            font-size: 13px;
            background-color: #1e1e1e;
            color: #d4d4d4;
            border: 1px solid #555;
            padding: 6px;
        }
    )");
    
    solutionTextEdit->setReadOnly(true);
    QFont font("Courier");
    font.setStyleHint(QFont::Monospace);
    solutionTextEdit->setFont(font);

    QGroupBox *resultGroup = new QGroupBox("Wynik rozwiązania", this);
    QVBoxLayout *resultLayout = new QVBoxLayout(resultGroup);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    resultLayout->addWidget(solutionTextEdit);
    resultGroup->setContentsMargins(10, 10, 10, 10);
    mainLayout->addWidget(resultGroup);

    setCentralWidget(central);

    // Połączenia sygnałów
    connect(matrixSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::createMatrixInputs);
    connect(dataTypeComboBox, &QComboBox::currentIndexChanged, this, [this](int) {
        selectedType = dataTypeComboBox->currentData().toString();
        createMatrixInputs(matrixSizeSpinBox->value());
    });
    connect(solveButton, &QPushButton::clicked, this, &MainWindow::solveSystem);
    connect(matrixTypeComboBox, &QComboBox::currentIndexChanged, this, [this](int) {
        createMatrixInputs(matrixSizeSpinBox->value());
    });
    
    // Inicjalizacja
    selectedType = dataTypeComboBox->currentData().toString();
    createMatrixInputs(matrixSizeSpinBox->value());
}


void MainWindow::createMatrixInputs(int size)
{
    QString type = selectedType.toLower();

    // Czyścimy stare pola
    QLayoutItem *child;
    while ((child = matrixLayout->takeAt(0)))
    {
        delete child->widget();
        delete child;
    }
    while ((child = vectorLayout->takeAt(0)))
    {
        delete child->widget();
        delete child;
    }

    matrixInputs.clear();
    vectorInputs.clear();

    for (int i = 0; i < size; ++i)
    {
        QVector<QLineEdit *> row;
        for (int j = 0; j < size; ++j)
        {
            if (type == "interval")
            {
                QHBoxLayout *cellLayout = new QHBoxLayout;
                mainLayout->setContentsMargins(10, 10, 10, 10);
                mainLayout->setSpacing(10);

                QLineEdit *left = new QLineEdit(this);
                QLineEdit *right = new QLineEdit(this);
                left->setFixedWidth(40);
                right->setFixedWidth(40);
                left->setAlignment(Qt::AlignCenter);
                right->setAlignment(Qt::AlignCenter);
                cellLayout->addWidget(left);
                cellLayout->addWidget(right);

                left->setStyleSheet("background-color: #404040; color: #ffaaaa;");
                right->setStyleSheet("background-color: #404040; color: #aaffaa;");


                QWidget *container = new QWidget(this);
                container->setLayout(cellLayout);
                matrixLayout->addWidget(container, i, j);

                row.append(left);
                row.append(right);
            }
            else
            {
                QLineEdit *edit = new QLineEdit(this);
                edit->setFixedWidth(60);
                edit->setAlignment(Qt::AlignCenter);
                vectorLayout->setContentsMargins(10, 0, 0, 0);
                edit->setStyleSheet(R"(
                    QLineEdit {
                        background-color: #fdfdfd;
                        border: 1px solid #ccc;
                        border-radius: 4px;
                        padding: 4px;
                    }
                    QLineEdit:focus {
                        border: 1px solid #66afe9;
                    }
                )");
                
                matrixLayout->addWidget(edit, i, j);
                row.append(edit);
            }
        }
        matrixInputs.append(row);

        if (type == "interval")
        {
            QHBoxLayout *vecLayout = new QHBoxLayout;
            QLineEdit *left = new QLineEdit(this);
            QLineEdit *right = new QLineEdit(this);
            left->setFixedWidth(40);
            right->setFixedWidth(40);
            left->setAlignment(Qt::AlignCenter);
            right->setAlignment(Qt::AlignCenter);
            vecLayout->addWidget(left);
            vecLayout->addWidget(right);
            QWidget *container = new QWidget(this);
            container->setLayout(vecLayout);
            vectorLayout->addWidget(container);
            vectorInputs.append(left);
            vectorInputs.append(right);
            left->setStyleSheet(R"(
                QLineEdit {
                    background-color: #fdfdfd;
                    border: 1px solid #ccc;
                    border-radius: 4px;
                    padding: 4px;
                }
                QLineEdit:focus {
                    border: 1px solid #66afe9;
                }
            )");
            
            right->setStyleSheet(R"(
                QLineEdit {
                    background-color: #fdfdfd;
                    border: 1px solid #ccc;
                    border-radius: 4px;
                    padding: 4px;
                }
                QLineEdit:focus {
                    border: 1px solid #66afe9;
                }
            )");
            
        }
        else
        {
            QLineEdit *edit = new QLineEdit(this);
            edit->setFixedWidth(60);
            edit->setAlignment(Qt::AlignCenter);
            vectorLayout->addWidget(edit);
            vectorInputs.append(edit);
        }
    }
}

void MainWindow::solveSystem()
{
    int size = matrixSizeSpinBox->value();
    QString type = dataTypeComboBox->currentData().toString();
    QString matrixKind = matrixTypeComboBox->currentData().toString();  // general / symmetric / tridiagonal

    if (type == "double")
{
    QString matrixKind = matrixTypeComboBox->currentData().toString();

    if (matrixKind == "tridiagonal")
    {
        QVector<double> a(size, 0), b_diag(size), c(size, 0), d(size);
        for (int i = 0; i < size; ++i)
        {
            if (i > 0)
                a[i] = matrixInputs[i][i - 1]->text().toDouble();
            b_diag[i] = matrixInputs[i][i]->text().toDouble();
            if (i < size - 1)
                c[i] = matrixInputs[i][i + 1]->text().toDouble();
            d[i] = vectorInputs[i]->text().toDouble();
        }

        auto [L, U, y, x] = solveCroutTridiagonal(a, b_diag, c, d);
        displaySolutionDetails(L, U, y, x);
    }
    else // symmetric lub general
    {
        QVector<QVector<double>> A(size, QVector<double>(size));
        QVector<double> b(size);

        for (int i = 0; i < size; ++i)
        {
            for (int j = 0; j < size; ++j)
                A[i][j] = matrixInputs[i][j]->text().toDouble();
            b[i] = vectorInputs[i]->text().toDouble();
        }

        auto [L, U, y, x] = solveCroutFull(A, b);
        displaySolutionDetails(L, U, y, x);
    }
}

    else if (type == "mpreal")
    {
        QVector<QVector<mpreal>> A(size, QVector<mpreal>(size));
        QVector<mpreal> b(size);

        for (int i = 0; i < size; ++i)
        {
            for (int j = 0; j < size; ++j)
                A[i][j] = mpreal(matrixInputs[i][j]->text().toStdString());
            b[i] = mpreal(vectorInputs[i]->text().toStdString());
        }

        auto [L, U, y, x] = CroutSolverMP::solve(A, b, matrixKind);
        displaySolutionDetails(L, U, y, x);
    }
    else if (type == "interval")
    {
        QVector<QVector<Interval<mpreal>>> A(size, QVector<Interval<mpreal>>(size));
        QVector<Interval<mpreal>> b(size);

        for (int i = 0; i < size; ++i)
        {
            for (int j = 0; j < size; ++j)
            {
                int idx = j * 2;
                mpreal left = mpreal(matrixInputs[i][idx]->text().toStdString());
                mpreal right = mpreal(matrixInputs[i][idx + 1]->text().toStdString());
                A[i][j] = Interval<mpreal>(left, right);
            }

            int vIdx = i * 2;
            mpreal left = mpreal(vectorInputs[vIdx]->text().toStdString());
            mpreal right = mpreal(vectorInputs[vIdx + 1]->text().toStdString());
            b[i] = Interval<mpreal>(left, right);
        }

        auto [L, U, y, x] = CroutSolverInterval::solve(A, b, matrixKind);
        displaySolutionDetails(L, U, y, x);
    }
}


template <typename T>
QString MainWindow::toQString(const T &val)
{
    if constexpr (std::is_same_v<T, mpreal>)
        return QString::fromStdString(val.toString());
    else if constexpr (std::is_same_v<T, Interval<mpreal>>)
        return QString("[%1, %2]").arg(QString::fromStdString(val.a.toString())).arg(QString::fromStdString(val.b.toString()));
    else
        return QString::number(val);
}

void MainWindow::displaySolutionDetails(const QVector<QVector<double>> &L, const QVector<QVector<double>> &U, const QVector<double> &y, const QVector<double> &x)
{
    QString result;
    int n = x.size();

    result += "Macierz L:\n";
    for (const auto &row : L)
    {
        for (const auto &val : row)
            result += toQString(val).leftJustified(15);
        result += '\n';
    }

    result += "\nMacierz U:\n";
    for (const auto &row : U)
    {
        for (const auto &val : row)
            result += toQString(val).leftJustified(15);
        result += '\n';
    }

    result += "\nWektor y (z równania Ly = b):\n";
    for (int i = 0; i < n; ++i)
        result += QString("y[%1] = %2\n").arg(i).arg(toQString(y[i]));

    result += "\nWektor x (rozwiązanie):\n";
    for (int i = 0; i < n; ++i)
        result += QString("x[%1] = %2\n").arg(i).arg(toQString(x[i]));

    solutionTextEdit->setText(result);
}



void MainWindow::displaySolutionDetails(const QVector<QVector<mpreal>> &L, const QVector<QVector<mpreal>> &U, const QVector<mpreal> &y, const QVector<mpreal> &x)
{
    QString result;
    int n = x.size();

    result += "Macierz L:\n";
    for (const auto &row : L)
    {
        for (const auto &val : row)
            result += toQString(val).leftJustified(15);
        result += '\n';
    }

    result += "\nMacierz U:\n";
    for (const auto &row : U)
    {
        for (const auto &val : row)
            result += toQString(val).leftJustified(15);
        result += '\n';
    }

    result += "\nWektor y (z równania Ly = b):\n";
    for (int i = 0; i < n; ++i)
        result += QString("y[%1] = %2\n").arg(i).arg(toQString(y[i]));

    result += "\nWektor x (rozwiązanie):\n";
    for (int i = 0; i < n; ++i)
        result += QString("x[%1] = %2\n").arg(i).arg(toQString(x[i]));

    solutionTextEdit->setText(result);
}


void MainWindow::displaySolutionDetails(const QVector<QVector<Interval<mpreal>>> &L, const QVector<QVector<Interval<mpreal>>> &U, const QVector<Interval<mpreal>> &y, const QVector<Interval<mpreal>> &x)
{
    QString result;
    int n = x.size();

    result += "Macierz L:\n";
    for (const auto &row : L)
    {
        for (const auto &val : row)
            result += toQString(val).leftJustified(15);
        result += '\n';
    }

    result += "\nMacierz U:\n";
    for (const auto &row : U)
    {
        for (const auto &val : row)
            result += toQString(val).leftJustified(15);
        result += '\n';
    }

    result += "\nWektor y (z równania Ly = b):\n";
    for (int i = 0; i < n; ++i)
        result += QString("y[%1] = %2\n").arg(i).arg(toQString(y[i]));

    result += "\nWektor x (rozwiązanie):\n";
    for (int i = 0; i < n; ++i)
        result += QString("x[%1] = %2\n").arg(i).arg(toQString(x[i]));

    solutionTextEdit->setText(result);
}

std::tuple<QVector<QVector<double>>, QVector<QVector<double>>, QVector<double>, QVector<double>>
MainWindow::solveCroutTridiagonal(const QVector<double>& a, const QVector<double>& b, const QVector<double>& c, const QVector<double>& d)
{
    int n = b.size();
    QVector<QVector<double>> L(n, QVector<double>(n, 0.0));
    QVector<QVector<double>> U(n, QVector<double>(n, 0.0));
    QVector<double> y(n), x(n);

    QVector<double> l(n), u(n), z(n);

    u[0] = b[0];
    for (int i = 1; i < n; ++i) {
        l[i] = a[i] / u[i - 1];
        u[i] = b[i] - l[i] * c[i - 1];
    }

    // Forward substitution
    z[0] = d[0];
    for (int i = 1; i < n; ++i) {
        z[i] = d[i] - l[i] * z[i - 1];
    }

    // Backward substitution
    x[n - 1] = z[n - 1] / u[n - 1];
    for (int i = n - 2; i >= 0; --i) {
        x[i] = (z[i] - c[i] * x[i + 1]) / u[i];
    }

    // Build L and U
    for (int i = 0; i < n; ++i) {
        L[i][i] = 1.0;
        U[i][i] = u[i];
        if (i > 0) {
            L[i][i - 1] = l[i];
            U[i - 1][i] = c[i - 1];
        }
    }

    // Compute y = L^{-1} * b
    y = z;

    return {L, U, y, x};
}
