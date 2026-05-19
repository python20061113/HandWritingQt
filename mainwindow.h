#ifndef MAINWINDOW_H
#define MAINWINDOW_H
//handle UI
#include <QMainWindow>
#include<QWidget>
#include<QLabel>
#include<QSpinBox>
#include<QComboBox>
#include<QTranslator>
#include<QApplication>
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
    QString recResult;
    QLabel* recLabel;
    QAction* recAction;
    QAction* recMultAction;
    QAction* clearAction;
    QAction* saveAction;
    QComboBox* lang;
    QTranslator* current_tr= nullptr;
private:
    Ui::MainWindow *ui;
    void switchLanguage(int idx);
    void retranslateUi();
};
#endif // MAINWINDOW_H
