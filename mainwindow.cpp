#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QWidget *central = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);

    QHBoxLayout *topLayout = new QHBoxLayout;
    QLabel *sizeLabel = new QLabel("Rozmiar macierzy:", this);
    matrixSizeSpinBox = new QSpinBox(this);
    matrixSizeSpinBox->setRange(2, 10);
    matrixSizeSpinBox->setValue(3);
    dataTypeComboBox = new QComboBox(this);
    dataTypeComboBox->addItems({"double", "mpreal", "interval"});
    dataTypeComboBox->setCurrentText("interval");

    topLayout->addWidget(sizeLabel);
    topLayout->addWidget(matrixSizeSpinBox);
    topLayout->addWidget(dataTypeComboBox);
    topLayout->addStretch();

    matrixLayout = new QGridLayout;
    vectorLayout = new QVBoxLayout;
    QFrame *separator = new QFrame(this);
    separator->setFrameShape(QFrame::VLine);
    separator->setLineWidth(1);

    QHBoxLayout *matrixGroupLayout = new QHBoxLayout;
    matrixGroupLayout->addLayout(matrixLayout);
    matrixGroupLayout->addWidget(separator);
    matrixGroupLayout->addLayout(vectorLayout);

    solveButton = new QPushButton("Rozwiąż", this);
    solutionTextEdit = new QTextEdit(this);
    solutionTextEdit->setReadOnly(true);

    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(matrixGroupLayout);
    mainLayout->addWidget(solveButton);
    mainLayout->addWidget(solutionTextEdit);
    setCentralWidget(central);

    connect(matrixSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::createMatrixInputs);

    connect(dataTypeComboBox, &QComboBox::currentTextChanged, this, [=](const QString &text)
            {
        selectedType = text;
        createMatrixInputs(matrixSizeSpinBox->value()); });

    connect(solveButton, &QPushButton::clicked, this, &MainWindow::solveSystem);

    createMatrixInputs(matrixSizeSpinBox->value());
}

MainWindow::~MainWindow() {}

void MainWindow::createMatrixInputs(int size)
{
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

    QString type = selectedType.toLower();

    for (int i = 0; i < size; ++i)
    {
        QVector<QLineEdit *> row;
        for (int j = 0; j < size; ++j)
        {
            if (type == "interval")
            {
                QHBoxLayout *pairLayout = new QHBoxLayout;
                QLineEdit *a = new QLineEdit(this);
                QLineEdit *b = new QLineEdit(this);
                a->setFixedWidth(40);
                b->setFixedWidth(40);
                a->setAlignment(Qt::AlignCenter);
                b->setAlignment(Qt::AlignCenter);
                pairLayout->addWidget(a);
                pairLayout->addWidget(b);

                QWidget *container = new QWidget(this);
                container->setLayout(pairLayout);
                matrixLayout->addWidget(container, i, j);
                row.append(a);
                row.append(b);
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
            QHBoxLayout *vPair = new QHBoxLayout;
            QLineEdit *a = new QLineEdit(this);
            QLineEdit *b = new QLineEdit(this);
            a->setFixedWidth(40);
            b->setFixedWidth(40);
            a->setAlignment(Qt::AlignCenter);
            b->setAlignment(Qt::AlignCenter);
            vPair->addWidget(a);
            vPair->addWidget(b);

            QWidget *vContainer = new QWidget(this);
            vContainer->setLayout(vPair);
            vectorLayout->addWidget(vContainer);
            vectorInputs.append(a);
            vectorInputs.append(b);
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
    QString type = selectedType.toLower();
    int size = matrixSizeSpinBox->value();

    if (type == "interval")
    {
        QVector<QVector<Interval<mpreal>>> A(size, QVector<Interval<mpreal>>(size));
        QVector<Interval<mpreal>> b(size);

        for (int i = 0, k = 0; i < size; ++i)
        {
            for (int j = 0; j < size; ++j, k += 2)
            {
                mpreal a = QString(matrixInputs[i][2 * j]->text()).toDouble();
                mpreal b_val = QString(matrixInputs[i][2 * j + 1]->text()).toDouble();
                A[i][j] = Interval<mpreal>(a, b_val);
            }
        }

        for (int i = 0; i < size; ++i)
        {
            mpreal a = QString(vectorInputs[2 * i]->text()).toDouble();
            mpreal b_val = QString(vectorInputs[2 * i + 1]->text()).toDouble();
            b[i] = Interval<mpreal>(a, b_val);
        }

        QVector<Interval<mpreal>> x = solveCrout(A, b);
        QString result;
        for (int i = 0; i < size; ++i)
            result += QString("x[%1] = [%2, %3]\n")
                          .arg(i)
                          .arg(QString::fromStdString(x[i].a.toString()))
                          .arg(QString::fromStdString(x[i].b.toString()));

        solutionTextEdit->setText(result);
    }

    // inne typy (double, mpreal) możesz dorobić później – teraz skupiłem się na interval
}

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
        L[i][i] = 1.0;
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

QVector<Interval<mpreal>> MainWindow::solveCrout(const QVector<QVector<Interval<mpreal>>> &A, const QVector<Interval<mpreal>> &b)
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

    return x;
}
