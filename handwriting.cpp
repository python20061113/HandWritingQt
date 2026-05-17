#include "handwriting.h"


HandWriting::HandWriting(QWidget *parent) : QWidget(parent)
{
    // setFixedSize(800,600);
    canvas=QImage(size(),QImage::Format_RGB32);
    canvas.fill(Qt::black);
    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
}
