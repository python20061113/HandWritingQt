#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QToolBar>
#include<QWidget>
#include<QScrollBar>
#include<QSpinBox>
#include<QLabel>
#include<QSettings>
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
    handwriting->loadBackground(handwriting->getBgPath());
    label = new QLabel(this);
    label->setText(tr("Set Label"));
    recLabel = new QLabel(tr("Pending..."),this);
    recLabel->show();
    toolBar = addToolBar(tr("Tool"));
    // recLabel->setBaseSize(100,100);

    spinBox = new QSpinBox(this);
    spinBox->setRange(0,9);
    spinBox->setSingleStep(1);
    spinBox->setValue(0);
    //spinBox for label setting
    lang = new QComboBox(this);
    lang->setToolTip(tr("Language"));
    lang->addItem("English","en");
    lang->addItem("简体中文（中国）","zh_CN");
    lang->addItem("繁體中文（台灣）","zh_TW");
    lang->addItem("繁體中文（香港）","zh_HK");
    lang->addItem("Svenska","sv");
    connect(lang,&QComboBox::currentIndexChanged,this,&MainWindow::switchLanguage);

    saveSound = new QSoundEffect(this);
    saveSound->setSource(QUrl("qrc:/media/yoshino_save-new.wav"));
    saveSound->setVolume(1.0);

    exitSound = new QSoundEffect(this);
    exitSound->setSource(QUrl("qrc:/media/yoshino_goodbye-new.wav"));

    showSound = new QSoundEffect(this);
    showSound->setSource(QUrl("qrc:/media/yoshino_yuzu-new.wav"));

    clearSound = new QSoundEffect(this);
    clearSound->setSource(QUrl("qrc:/media/yoshino_reset-new.wav"));

    recAction = new QAction(tr("Recognize(Ctrl+R)"),this);
    recMultAction =new QAction(tr("Multi-Recognize"),this);
    connect(handwriting, &HandWriting::recFinished,
            recLabel, &QLabel::setText);
    connect(recAction,&QAction::triggered,this,
            [this,handwriting]()
            {handwriting->recognize();
            recLabel->setText(handwriting->getOutput());
            recLabel->show();});
    connect(recMultAction,&QAction::triggered,this,
            [this,handwriting](){
        handwriting->multiRecognize();
        recLabel->setText(handwriting->getOutput());
        recLabel->show();
    });
    //update recLabel w/ output so that you can see

    toolBar->addWidget(lang);
    clearAction = new QAction(tr("Clear(Ctrl+C)"),this);
    connect(clearAction,&QAction::triggered,this,[this](){clearSound->play();});
    connect(clearAction,&QAction::triggered,handwriting,&HandWriting::clear);

    saveAction = new QAction(tr("Save(Ctrl+S)"),this);
    connect(saveAction, &QAction::triggered, this,
            [this]() {
        saveSound->play();
    });
    connect(saveAction,&QAction::triggered,handwriting,&HandWriting::saveToImage);

    toolBar->addAction(clearAction);
    toolBar->addAction(saveAction);
    toolBar->addAction(recAction);
    toolBar->addAction(recMultAction);

    toolBar->addWidget(label);
    toolBar->addWidget(spinBox);
    toolBar->addWidget(recLabel);
    connect(spinBox,QOverload<int>::of(&QSpinBox::valueChanged),handwriting,&HandWriting::setLabel);
    // setCentralWidget(handwriting);



    handwriting->loadModel();
    QString lastLang = QSettings().value("language", "en").toString();
    int idx = lang->findData(lastLang);
    if (idx != -1) lang->setCurrentIndex(idx);
    else lang->setCurrentIndex(0);
    qDebug() << handwriting->size();
    toolBar->show();
    qDebug() << "Toolbar visible:" << toolBar->isVisible();
    qDebug() << "Toolbar actions count:" << toolBar->actions().size();
}
void MainWindow::switchLanguage(int index) {
    QString langCode = "lang_"+lang->itemData(index).toString();
    // 加载对应的 .qm 文件（从文件系统）
    QTranslator *newTranslator = new QTranslator;

    if (newTranslator->load(":/translations/" + langCode + ".qm")) {

        // 移除旧的翻译器
        if (current_tr) {
            qApp->removeTranslator(current_tr);
            delete current_tr;   // 可选，不删除也可以
        }
        current_tr = newTranslator;
        qApp->installTranslator(current_tr);
        // 刷新界面
        ui->retranslateUi(this);
        retranslateUi();
        QSettings().setValue("language", langCode);
    } else {
        delete newTranslator;
        qDebug() << "Failed to load translation:" << langCode;
    }
}
void MainWindow::retranslateUi()
{
    ui->retranslateUi(this);
    label->setText(tr("Set Label"));
    recLabel->setText(tr("Pending..."));
    toolBar->setToolTip(tr("Tool"));
    lang->setToolTip(tr("Language"));
    recAction->setText(tr("Recognize(Ctrl+R)"));
    recMultAction->setText(tr("Multi-Recognize"));
    clearAction->setText(tr("Clear(Ctrl+C)"));
    saveAction->setText(tr("Save(Ctrl+S)"));
    lang->blockSignals(true);
    int idx = lang->currentIndex();
    lang->setItemText(0, "English");
    lang->setItemText(1, "简体中文（中国）");
    lang->setItemText(2, "繁體中文（台灣）");
    lang->setItemText(3, "繁體中文（香港）");
    lang->setItemText(4,"Svenska");
    lang->setCurrentIndex(idx);
    lang->setToolTip(tr("Language"));
    lang->blockSignals(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}
