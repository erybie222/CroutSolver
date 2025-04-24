#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QLineEdit>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Ustawienia interfejsu
    ui->matrixSizeSpinBox->setMinimum(2);
    ui->matrixSizeSpinBox->setMaximum(10);

    // Podpięcie sygnałów
    connect(ui->generateButton, &QPushButton::clicked, this, &MainWindow::generateMatrixInputs);
    connect(ui->solveButton, &QPushButton::clicked, this, &MainWindow::solveSystem);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::generateMatrixInputs()
{
    int size = ui->matrixSizeSpinBox->value();

    // Wyczyść stare layouty
    clearLayout(ui->matrixLayout);
    clearLayout(ui->vectorLayout);

    // Utwórz nowe pola dla macierzy i wektora
    matrixEdits.resize(size);
    for (int i = 0; i < size; ++i)
    matrixEdits[i].resize(size);

    vectorEdits.resize(size);

    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            QLineEdit* edit = new QLineEdit(this);
            edit->setFixedWidth(50);
            ui->matrixLayout->addWidget(edit, i, j);
            matrixEdits[i][j] = edit;
        }

        QLineEdit* vecEdit = new QLineEdit(this);
        vecEdit->setFixedWidth(50);
        ui->vectorLayout->addWidget(vecEdit, i, 0);
        vectorEdits[i] = vecEdit;
    }
}

void MainWindow::solveSystem()
{
    // TODO: dodajemy logikę rozwiązującą układ metodą Crouta
    QMessageBox::information(this, "Rozwiązanie", "Tu pojawi się wynik po zaimplementowaniu algorytmu Crouta.");
}

void MainWindow::clearLayout(QLayout* layout)
{
    while (QLayoutItem* item = layout->takeAt(0)) {
        if (QWidget* widget = item->widget()) {
            delete widget;
        }
        delete item;
    }
}
