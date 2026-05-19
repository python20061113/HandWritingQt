#include "handwriting.h"


HandWriting::HandWriting(QWidget *parent) : QWidget(parent)
{
    // setFixedSize(800,600);
    canvas=QImage(size(),QImage::Format_RGB32);
    canvas.fill(Qt::black);
    canvas_display=QImage(size(),QImage::Format_ARGB32_Premultiplied);
    canvas_display.fill(Qt::transparent);
    setAttribute(Qt::WA_TranslucentBackground);
    setAutoFillBackground(false);

    background.setDevicePixelRatio(devicePixelRatio());
    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

}
