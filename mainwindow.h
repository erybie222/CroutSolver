#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QLineEdit>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void generateMatrixInputs();
    void solveSystem();

private:
    QVector<QVector<QLineEdit *>> matrixEdits;
    QVector<QLineEdit *> vectorEdits;
    Ui::MainWindow *ui;
    QVector<QVector<double>> A;
    QVector<double> b;
    void clearLayout(QLayout *layout);
    void readMatrixAndVector();

    // 👇 TO JEST DEKLARACJA FUNKCJI CZŁONKOWEJ KLASY!
    QVector<double> solveCrout(const QVector<QVector<double>> &A, const QVector<double> &b);
};

#endif // MAINWINDOW_H
