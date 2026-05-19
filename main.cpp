
#include<handwriting.h>
#include <torch/torch.h>
#include <QApplication>
#include <QDebug>
#include<QTranslator>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    // QTranslator* tr_zhTW = new QTranslator();
    // if(tr_zhTW->load(":/translation/lang_zh_TW.qm"))
    // {
    //     a.installTranslator(tr_zhTW);
    // }
    // else
    // {
    //     qDebug()<<"translation NOT LOADED";
    // }
    // QTranslator* tr_zhCN=new QTranslator();
    // if(tr_zhCN->load("lang_zh_CN.qm"))
    // {
    //     a.installTranslator(tr_zhCN);
    // }
    // QTranslator* tr_zhHK=new QTranslator();
    // if(tr_zhHK->load("lang_zh_HK.qm"))
    // {
    //     a.installTranslator(tr_zhHK);
    // }
    MainWindow w;
    static auto trans = w.current_tr;
    w.show();

    //test
    torch::Tensor tensor = torch::rand({3, 3});
    std::cout << "Hello LibTorch! Tensor:\n" << tensor << std::endl;


    if (torch::cuda::is_available()) {
        std::cout << "CUDA is available! Number of GPUs: " << torch::cuda::device_count() << std::endl;
        torch::Tensor gpu_tensor = torch::rand({2, 2}, torch::device(torch::kCUDA));
        std::cout << "GPU tensor:\n" << gpu_tensor << std::endl;
    } else {
        std::cout << "CUDA is NOT available. Running on CPU." << std::endl;
    }


    qDebug() << "Main started";

    return a.exec();
}
