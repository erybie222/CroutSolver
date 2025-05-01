#include "mainwindow.h"
#include "interval.hpp"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QComboBox>

using namespace mpfr;
using namespace interval_arithmetic;

MainWindow::~MainWindow() = default;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QWidget *central = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);

    // Kontrolki górne
    QHBoxLayout *controlsLayout = new QHBoxLayout;
    controlsLayout->addWidget(new QLabel("Rozmiar macierzy:"));
    matrixSizeSpinBox = new QSpinBox(this);
    matrixSizeSpinBox->setRange(2, 10);
    matrixSizeSpinBox->setValue(3);
    controlsLayout->addWidget(matrixSizeSpinBox);

    controlsLayout->addWidget(new QLabel("Typ danych:"));
    dataTypeComboBox = new QComboBox(this);
    dataTypeComboBox->addItem("double");
    dataTypeComboBox->addItem("mpreal");
    dataTypeComboBox->addItem("interval");
    controlsLayout->addWidget(dataTypeComboBox);
    controlsLayout->addStretch();

    mainLayout->addLayout(controlsLayout);

    // Layouty dla macierzy i wektora
    matrixLayout = new QGridLayout;
    vectorLayout = new QVBoxLayout;
    matrixLayout->setSpacing(2);
    vectorLayout->setSpacing(2);

    QFrame *separator = new QFrame(this);
    separator->setFrameShape(QFrame::VLine);
    separator->setLineWidth(1);

    QHBoxLayout *matrixGroup = new QHBoxLayout;
    matrixGroup->addLayout(matrixLayout);
    matrixGroup->addWidget(separator);
    matrixGroup->addLayout(vectorLayout);

    mainLayout->addLayout(matrixGroup);

    // Przycisk i pole tekstowe
    solveButton = new QPushButton("Rozwiąż", this);
    mainLayout->addWidget(solveButton);

    solutionTextEdit = new QTextEdit(this);
    solutionTextEdit->setReadOnly(true);
    mainLayout->addWidget(solutionTextEdit);

    setCentralWidget(central);

    // Połączenia
    connect(matrixSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::createMatrixInputs);
    connect(dataTypeComboBox, &QComboBox::currentTextChanged, this, [this](const QString &text)
            {
        selectedType = text;
        createMatrixInputs(matrixSizeSpinBox->value()); });
    connect(solveButton, &QPushButton::clicked, this, &MainWindow::solveSystem);

    selectedType = dataTypeComboBox->currentText();
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
                QLineEdit *left = new QLineEdit(this);
                QLineEdit *right = new QLineEdit(this);
                left->setFixedWidth(40);
                right->setFixedWidth(40);
                left->setAlignment(Qt::AlignCenter);
                right->setAlignment(Qt::AlignCenter);
                cellLayout->addWidget(left);
                cellLayout->addWidget(right);

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
    QString type = selectedType.toLower();

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
        QVector<double> x = solveCrout(A, b);
        QString res;
        for (int i = 0; i < size; ++i)
            res += QString("x[%1] = %2\n").arg(i).arg(x[i]);
        solutionTextEdit->setText(res);
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
        QVector<mpreal> x = solveCrout(A, b);
        QString res;
        for (int i = 0; i < size; ++i)
            res += QString("x[%1] = %2\n").arg(i).arg(QString::fromStdString(x[i].toString()));
        solutionTextEdit->setText(res);
    }
    else if (type == "interval")
    {
        QVector<QVector<Interval<mpreal>>> A(size, QVector<Interval<mpreal>>(size));
        QVector<Interval<mpreal>> b(size);
        for (int i = 0; i < size; ++i)
        {
            for (int j = 0; j < size; ++j)
            {
                mpreal a = mpreal(matrixInputs[i][2 * j]->text().toStdString());
                mpreal b_ = mpreal(matrixInputs[i][2 * j + 1]->text().toStdString());
                A[i][j] = Interval<mpreal>(a, b_);
            }
            mpreal ba = mpreal(vectorInputs[2 * i]->text().toStdString());
            mpreal bb = mpreal(vectorInputs[2 * i + 1]->text().toStdString());
            b[i] = Interval<mpreal>(ba, bb);
        }

        QVector<Interval<mpreal>> x = solveCrout(A, b);
        QString res;
        for (int i = 0; i < size; ++i)
            res += QString("x[%1] = [%2, %3]\n").arg(i).arg(QString::fromStdString(x[i].a.toString())).arg(QString::fromStdString(x[i].b.toString()));
        solutionTextEdit->setText(res);
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
