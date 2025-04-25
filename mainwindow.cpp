#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->generateButton, &QPushButton::clicked, this, &MainWindow::generateMatrixInputs);
    connect(ui->solveButton, &QPushButton::clicked, this, &MainWindow::solveSystem);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::clearLayout(QLayout *layout)
{
    if (!layout)
        return;
    QLayoutItem *item;
    while ((item = layout->takeAt(0)) != nullptr)
    {
        if (QWidget *widget = item->widget())
        {
            widget->deleteLater();
        }
        delete item;
    }
}

void MainWindow::generateMatrixInputs()
{
    clearLayout(ui->matrixLayout);
    clearLayout(ui->vectorLayout);
    matrixEdits.clear();
    vectorEdits.clear();

    int size = ui->sizeSpinBox->value();

    for (int i = 0; i < size; ++i)
    {
        QVector<QLineEdit *> row;
        for (int j = 0; j < size; ++j)
        {
            QLineEdit *edit = new QLineEdit;
            edit->setFixedWidth(50);
            ui->matrixLayout->addWidget(edit, i, j);
            row.append(edit);
        }
        matrixEdits.append(row);

        QLineEdit *vectorEdit = new QLineEdit;
        vectorEdit->setFixedWidth(50);
        ui->vectorLayout->addWidget(vectorEdit);
        vectorEdits.append(vectorEdit);
    }
}

void MainWindow::readMatrixAndVector()
{
    int size = ui->sizeSpinBox->value();
    A.resize(size);
    b.resize(size);

    for (int i = 0; i < size; ++i)
    {
        A[i].resize(size);
        for (int j = 0; j < size; ++j)
        {
            A[i][j] = matrixEdits[i][j]->text().toDouble();
        }
        b[i] = vectorEdits[i]->text().toDouble();
    }
}

void MainWindow::solveSystem()
{
    try
    {
        qDebug() << "Rozpoczynamy solveSystem()";

        readMatrixAndVector();
        qDebug() << "Macierz A:" << A;
        qDebug() << "Wektor b:" << b;

        QVector<double> x = solveCrout(A, b);
        qDebug() << "Wektor x:" << x;

        QString result;
        for (int i = 0; i < x.size(); ++i)
        {
            result += QString("x%1 = %2\n").arg(i + 1).arg(x[i]);
        }
        ui->resultLabel->setText(result);
    }
    catch (const std::exception &e)
    {
        qDebug() << "Błąd (std::exception):" << e.what();
        ui->resultLabel->setText("Wystąpił wyjątek: " + QString(e.what()));
    }
    catch (...)
    {
        qDebug() << "Błąd: nieznany wyjątek";
        ui->resultLabel->setText("Wystąpił nieznany błąd.");
    }
}

QVector<double> MainWindow::solveCrout(const QVector<QVector<double>> &A, const QVector<double> &b)
{
    int n = A.size();
    QVector<QVector<double>> L(n, QVector<double>(n, 0.0));
    QVector<QVector<double>> U(n, QVector<double>(n, 0.0));
    QVector<double> y(n, 0.0);
    QVector<double> x(n, 0.0);

    for (int i = 0; i < n; ++i)
    {
        U[i][i] = 1.0;

        for (int j = 0; j <= i; ++j)
        {
            double sum = 0.0;
            for (int k = 0; k < j; ++k)
                sum += L[i][k] * U[k][j];
            L[i][j] = A[i][j] - sum;
        }

        for (int j = i + 1; j < n; ++j)
        {
            double sum = 0.0;
            for (int k = 0; k < i; ++k)
                sum += L[i][k] * U[k][j];
            U[i][j] = (A[i][j] - sum) / L[i][i];
        }
    }

    for (int i = 0; i < n; ++i)
    {
        double sum = 0.0;
        for (int k = 0; k < i; ++k)
            sum += L[i][k] * y[k];
        y[i] = (b[i] - sum) / L[i][i];
    }

    for (int i = n - 1; i >= 0; --i)
    {
        double sum = 0.0;
        for (int k = i + 1; k < n; ++k)
            sum += U[i][k] * x[k];
        x[i] = y[i] - sum;
    }

    return x;
}
