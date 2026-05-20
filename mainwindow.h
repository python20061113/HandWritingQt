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
#include<QTimer>
#include<QSoundEffect>
#include<QCloseEvent>
#include<QShowEvent>
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
    QSoundEffect* saveSound;
    QSoundEffect* exitSound;
    QSoundEffect* showSound;
    QSoundEffect* clearSound;
    bool pendingExit = false;
private:
    Ui::MainWindow *ui;
    void switchLanguage(int idx);
    void retranslateUi();
    void closeEvent(QCloseEvent* ev)
    {
        int interval=300;
        if(!pendingExit && exitSound->isPlaying()==false) //first time exit: !pendingExit && !playing
        {
            pendingExit=true; //set exit request flag to true
            connect(exitSound,&QSoundEffect::playingChanged,this,
        [this,interval]()
        {
            if(!exitSound->isPlaying())
            {
                QTimer::singleShot(interval,this,&QMainWindow::close);
            }
        }); //when finished, close Mainwindow
            exitSound->play();
            ev->ignore();
        }
        else if(pendingExit&&exitSound->isPlaying()==true)
        {
            ev->ignore(); //when the audio is playing ignore exit request
        }
        else if(pendingExit&&exitSound->isPlaying()==false)
        {
            ev->accept();
        }
        else
        {
            ev->accept();
        }
    }
    void showEvent(QShowEvent* ev)
    {
        QMainWindow::showEvent(ev);
        if(!showSound->isPlaying())
        {
            showSound->play();
        }
    }
};
#endif // MAINWINDOW_H
