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

    if (type == "double")
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

        auto [L, U, y, x] = solveCroutFull(A, b);
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

        auto [L, U, y, x] = solveCroutFull(A, b);
        displaySolutionDetails(L, U, y, x);
    }
}



// Brakujące definicje metod solveCrout dla double i mpreal
QVector<double> MainWindow::solveCrout(const QVector<QVector<double>> &A, const QVector<double> &b)
{
    int n = A.size();
    QVector<QVector<double>> L(n, QVector<double>(n));
    QVector<QVector<double>> U(n, QVector<double>(n));
    QVector<double> y(n), x(n);

    for (int i = 0; i < n; ++i)
    {
        L[i][i] = 1.0;
        for (int j = i; j < n; ++j)
        {
            double sum = 0;
            for (int k = 0; k < i; ++k)
                sum += L[i][k] * U[k][j];
            U[i][j] = A[i][j] - sum;
        }
        for (int j = i + 1; j < n; ++j)
        {
            double sum = 0;
            for (int k = 0; k < i; ++k)
                sum += L[j][k] * U[k][i];
            L[j][i] = (A[j][i] - sum) / U[i][i];
        }
    }

    for (int i = 0; i < n; ++i)
    {
        double sum = 0;
        for (int k = 0; k < i; ++k)
            sum += L[i][k] * y[k];
        y[i] = (b[i] - sum) / L[i][i];
    }

    for (int i = n - 1; i >= 0; --i)
    {
        double sum = 0;
        for (int k = i + 1; k < n; ++k)
            sum += U[i][k] * x[k];
        x[i] = (y[i] - sum) / U[i][i];
    }

    return x;
}

QVector<mpreal> MainWindow::solveCrout(const QVector<QVector<mpreal>> &A, const QVector<mpreal> &b)
{
    int n = A.size();
    QVector<QVector<mpreal>> L(n, QVector<mpreal>(n));
    QVector<QVector<mpreal>> U(n, QVector<mpreal>(n));
    QVector<mpreal> y(n), x(n);

    for (int i = 0; i < n; ++i)
    {
        L[i][i] = 1;
        for (int j = i; j < n; ++j)
        {
            mpreal sum = 0;
            for (int k = 0; k < i; ++k)
                sum += L[i][k] * U[k][j];
            U[i][j] = A[i][j] - sum;
        }
        for (int j = i + 1; j < n; ++j)
        {
            mpreal sum = 0;
            for (int k = 0; k < i; ++k)
                sum += L[j][k] * U[k][i];
            L[j][i] = (A[j][i] - sum) / U[i][i];
        }
    }

    for (int i = 0; i < n; ++i)
    {
        mpreal sum = 0;
        for (int k = 0; k < i; ++k)
            sum += L[i][k] * y[k];
        y[i] = (b[i] - sum) / L[i][i];
    }

    for (int i = n - 1; i >= 0; --i)
    {
        mpreal sum = 0;
        for (int k = i + 1; k < n; ++k)
            sum += U[i][k] * x[k];
        x[i] = (y[i] - sum) / U[i][i];
    }

    return x;
}

QVector<Interval<mpreal>> MainWindow::solveCrout(
    const QVector<QVector<Interval<mpreal>>> &A,
    const QVector<Interval<mpreal>> &b)
{
    int n = A.size();
    QVector<QVector<Interval<mpreal>>> L(n, QVector<Interval<mpreal>>(n));
    QVector<QVector<Interval<mpreal>>> U(n, QVector<Interval<mpreal>>(n));
    QVector<Interval<mpreal>> y(n);
    QVector<Interval<mpreal>> x(n);

    for (int i = 0; i < n; ++i)
    {
        L[i][i] = Interval<mpreal>(1, 1);
        for (int j = i; j < n; ++j)
        {
            Interval<mpreal> sum(0, 0);
            for (int k = 0; k < i; ++k)
                sum = sum + L[i][k] * U[k][j];
            U[i][j] = A[i][j] - sum;
        }
        for (int j = i + 1; j < n; ++j)
        {
            Interval<mpreal> sum(0, 0);
            for (int k = 0; k < i; ++k)
                sum = sum + L[j][k] * U[k][i];
            L[j][i] = (A[j][i] - sum) / U[i][i];
        }
    }

    for (int i = 0; i < n; ++i)
    {
        Interval<mpreal> sum(0, 0);
        for (int k = 0; k < i; ++k)
            sum = sum + L[i][k] * y[k];
        y[i] = (b[i] - sum) / L[i][i];
    }

    for (int i = n - 1; i >= 0; --i)
    {
        Interval<mpreal> sum(0, 0);
        for (int k = i + 1; k < n; ++k)
            sum = sum + U[i][k] * x[k];
        x[i] = (y[i] - sum) / U[i][i];
    }

    return x;
}

std::tuple<QVector<QVector<double>>, QVector<QVector<double>>, QVector<double>, QVector<double>>
MainWindow::solveCroutFull(const QVector<QVector<double>> &A, const QVector<double> &b)
{
    int n = A.size();
    QVector<QVector<double>> L(n, QVector<double>(n, 0));
    QVector<QVector<double>> U(n, QVector<double>(n, 0));
    QVector<double> y(n), x(n);

    for (int i = 0; i < n; ++i)
    {
        L[i][i] = 1.0;
        for (int j = i; j < n; ++j)
        {
            double sum = 0;
            for (int k = 0; k < i; ++k)
                sum += L[i][k] * U[k][j];
            U[i][j] = A[i][j] - sum;
        }
        for (int j = i + 1; j < n; ++j)
        {
            double sum = 0;
            for (int k = 0; k < i; ++k)
                sum += L[j][k] * U[k][i];
            L[j][i] = (A[j][i] - sum) / U[i][i];
        }
    }

    for (int i = 0; i < n; ++i)
    {
        double sum = 0;
        for (int k = 0; k < i; ++k)
            sum += L[i][k] * y[k];
        y[i] = (b[i] - sum) / L[i][i];
    }

    for (int i = n - 1; i >= 0; --i)
    {
        double sum = 0;
        for (int k = i + 1; k < n; ++k)
            sum += U[i][k] * x[k];
        x[i] = (y[i] - sum) / U[i][i];
    }

    return {L, U, y, x};
}

std::tuple<QVector<QVector<mpreal>>, QVector<QVector<mpreal>>, QVector<mpreal>, QVector<mpreal>>
MainWindow::solveCroutFull(const QVector<QVector<mpreal>> &A, const QVector<mpreal> &b)
{
    int n = A.size();
    QVector<QVector<mpreal>> L(n, QVector<mpreal>(n));
    QVector<QVector<mpreal>> U(n, QVector<mpreal>(n));
    QVector<mpreal> y(n), x(n);

    for (int i = 0; i < n; ++i)
    {
        L[i][i] = 1;
        for (int j = i; j < n; ++j)
        {
            mpreal sum = 0;
            for (int k = 0; k < i; ++k)
                sum += L[i][k] * U[k][j];
            U[i][j] = A[i][j] - sum;
        }
        for (int j = i + 1; j < n; ++j)
        {
            mpreal sum = 0;
            for (int k = 0; k < i; ++k)
                sum += L[j][k] * U[k][i];
            L[j][i] = (A[j][i] - sum) / U[i][i];
        }
    }

    for (int i = 0; i < n; ++i)
    {
        mpreal sum = 0;
        for (int k = 0; k < i; ++k)
            sum += L[i][k] * y[k];
        y[i] = (b[i] - sum) / L[i][i];
    }

    for (int i = n - 1; i >= 0; --i)
    {
        mpreal sum = 0;
        for (int k = i + 1; k < n; ++k)
            sum += U[i][k] * x[k];
        x[i] = (y[i] - sum) / U[i][i];
    }

    return {L, U, y, x};
}

std::tuple<QVector<QVector<Interval<mpreal>>>, QVector<QVector<Interval<mpreal>>>, QVector<Interval<mpreal>>, QVector<Interval<mpreal>>>
MainWindow::solveCroutFull(const QVector<QVector<Interval<mpreal>>> &A, const QVector<Interval<mpreal>> &b)
{
    int n = A.size();
    QVector<QVector<Interval<mpreal>>> L(n, QVector<Interval<mpreal>>(n));
    QVector<QVector<Interval<mpreal>>> U(n, QVector<Interval<mpreal>>(n));
    QVector<Interval<mpreal>> y(n), x(n);

    for (int i = 0; i < n; ++i)
    {
        L[i][i] = Interval<mpreal>(1, 1);
        for (int j = i; j < n; ++j)
        {
            Interval<mpreal> sum(0, 0);
            for (int k = 0; k < i; ++k)
                sum = sum + L[i][k] * U[k][j];
            U[i][j] = A[i][j] - sum;
        }
        for (int j = i + 1; j < n; ++j)
        {
            Interval<mpreal> sum(0, 0);
            for (int k = 0; k < i; ++k)
                sum = sum + L[j][k] * U[k][i];
            L[j][i] = (A[j][i] - sum) / U[i][i];
        }
    }

    for (int i = 0; i < n; ++i)
    {
        Interval<mpreal> sum(0, 0);
        for (int k = 0; k < i; ++k)
            sum = sum + L[i][k] * y[k];
        y[i] = (b[i] - sum) / L[i][i];
    }

    for (int i = n - 1; i >= 0; --i)
    {
        Interval<mpreal> sum(0, 0);
        for (int k = i + 1; k < n; ++k)
            sum = sum + U[i][k] * x[k];
        x[i] = (y[i] - sum) / U[i][i];
    }

    return {L, U, y, x};
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

