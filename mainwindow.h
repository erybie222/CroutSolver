#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QLineEdit> // <-- to dodaj koniecznie

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void generateMatrixInputs();
    void solveSystem();

private:
    void clearLayout(QLayout* layout);

    Ui::MainWindow *ui;
    QVector<QVector<QLineEdit*>> matrixEdits;
    QVector<QLineEdit*> vectorEdits;
};

#endif // MAINWINDOW_H
