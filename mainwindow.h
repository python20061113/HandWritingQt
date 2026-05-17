#ifndef MAINWINDOW_H
#define MAINWINDOW_H
//handle UI
#include <QMainWindow>
#include<QWidget>
#include<QLabel>
#include<QSpinBox>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QToolBar *toolBar;
    QLabel *label;
    QSpinBox *spinBox;
    QString recResult="";
    QLabel* recLabel;

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
