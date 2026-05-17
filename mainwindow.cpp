#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QToolBar>
#include<QWidget>
#include<QScrollBar>
#include<QSpinBox>
#include<QLabel>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // qDebug() << "Button geometry:" << ui->clearButton->geometry();
    // qDebug() << "Button visible:" << ui->clearButton->isVisible();
    // qDebug() << "Button size:" << ui->clearButton->size();
    auto handwriting = ui->widget;
    handwriting->setFocusPolicy(Qt::StrongFocus);
    toolBar = addToolBar("Tool");

    label = new QLabel(handwriting);
    label->setText("Set Label");
    recLabel = new QLabel(handwriting);

    spinBox = new QSpinBox(handwriting);
    spinBox->setRange(0,9);
    spinBox->setSingleStep(1);
    spinBox->setValue(0);
    //spinBox for label setting

    QAction* recAction = new QAction("Recognize(Ctrl+R)",this);
    connect(recAction,&QAction::triggered,handwriting,&HandWriting::recognize); //update storged output
    connect(recAction,&QAction::triggered,recLabel,
            [this,handwriting]()
            {recLabel->setText(handwriting->getOutput());recLabel->show();}); //update recLabel w/ output so that you can see

    toolBar->addAction("Clear(Ctrl+C)", handwriting, &HandWriting::clear);
    toolBar->addAction("Save(Ctrl+S)", handwriting, &HandWriting::saveToImage);
    toolBar->addAction(recAction);

    toolBar->addWidget(label);
    toolBar->addWidget(spinBox);

    connect(spinBox,QOverload<int>::of(&QSpinBox::valueChanged),handwriting,&HandWriting::setLabel);
    // setCentralWidget(handwriting);

    handwriting->loadModel();
}

MainWindow::~MainWindow()
{
    delete ui;
}
