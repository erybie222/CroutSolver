/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QVBoxLayout *topLayout;
    QHBoxLayout *hboxLayout;
    QLabel *sizeLabel;
    QSpinBox *sizeSpinBox;
    QPushButton *generateButton;
    QHBoxLayout *hboxLayout1;
    QGridLayout *matrixLayout;
    QVBoxLayout *vectorLayout;
    QPushButton *solveButton;
    QLabel *resultLabel;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(640, 480);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        topLayout = new QVBoxLayout(centralwidget);
        topLayout->setObjectName("topLayout");
        hboxLayout = new QHBoxLayout();
        hboxLayout->setObjectName("hboxLayout");
        sizeLabel = new QLabel(centralwidget);
        sizeLabel->setObjectName("sizeLabel");

        hboxLayout->addWidget(sizeLabel);

        sizeSpinBox = new QSpinBox(centralwidget);
        sizeSpinBox->setObjectName("sizeSpinBox");
        sizeSpinBox->setMinimum(2);
        sizeSpinBox->setMaximum(10);
        sizeSpinBox->setValue(3);

        hboxLayout->addWidget(sizeSpinBox);


        topLayout->addLayout(hboxLayout);

        generateButton = new QPushButton(centralwidget);
        generateButton->setObjectName("generateButton");

        topLayout->addWidget(generateButton);

        hboxLayout1 = new QHBoxLayout();
        hboxLayout1->setObjectName("hboxLayout1");
        matrixLayout = new QGridLayout();
        matrixLayout->setObjectName("matrixLayout");

        hboxLayout1->addLayout(matrixLayout);

        vectorLayout = new QVBoxLayout();
        vectorLayout->setObjectName("vectorLayout");

        hboxLayout1->addLayout(vectorLayout);


        topLayout->addLayout(hboxLayout1);

        solveButton = new QPushButton(centralwidget);
        solveButton->setObjectName("solveButton");

        topLayout->addWidget(solveButton);

        resultLabel = new QLabel(centralwidget);
        resultLabel->setObjectName("resultLabel");

        topLayout->addWidget(resultLabel);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Crout Solver", nullptr));
        sizeLabel->setText(QCoreApplication::translate("MainWindow", "Rozmiar macierzy:", nullptr));
        generateButton->setText(QCoreApplication::translate("MainWindow", "Generuj macierz", nullptr));
        solveButton->setText(QCoreApplication::translate("MainWindow", "Rozwi\304\205\305\274", nullptr));
        resultLabel->setText(QCoreApplication::translate("MainWindow", "Wynik:", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
