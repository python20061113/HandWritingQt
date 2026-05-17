
#include<handwriting.h>
#include <torch/torch.h>
#include <QApplication>
#include <QDebug>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
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
