#ifndef HANDWRITING_H
#define HANDWRITING_H
//handle core logic
#include <QWidget>
#include <QDebug>
#include <QMouseEvent>
#include<QImage>
#include<QPaintEvent>
#include<QPainter>
#include <QPushButton>
#include<QFile>
#include<QFileDialog>
#include<mainwindow.h>
#include <QStandardPaths>
#include<QMessageBox>
#include<QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QDateTime>
#include<QPixmap>
#include<QKeyEvent>
#include <torch/torch.h>
#include <torch/script.h>

const std::string PATH="C:/Users/JohnTsang/Desktop/mnist_model.pt";

class HandWriting: public QWidget
{
public:
    torch::Tensor imgToTensor(QImage img)
    {
        img = getProcessedImg(img);
        std::vector<float> data(28*28);
        for(int i=0;i<28;++i)
        {
            auto line=img.constScanLine(i);
            for(int j=0;j<28;++j)
            {
                float pixel = line[j] / 255.0f;
                float normalized = (pixel - 0.1307f) / 0.3081f;
                data[i * 28 + j] = normalized;
            }
        }
        torch::Tensor tensor=torch::from_blob(data.data(),{1,1,28,28},torch::kFloat32).clone();
        return tensor;
    }
    std::pair<int,float> predictDigit(const torch::Tensor& tensor) {
        if (!model_loaded) return {};
        torch::NoGradGuard no_grad;
        std::vector<torch::jit::IValue> inputs;
        inputs.push_back(tensor);
        auto output = module.forward(inputs).toTensor();
        int predicted = output.argmax(1).item<int>();

        float confidence = torch::softmax(output, 1).max().item<float>();

        auto pair=std::make_pair(predicted,confidence);
        return pair;
    }

    QString recognize()
    {
        if(!model_loaded)return "";
        if(canvas.isNull()){qDebug()<<"Empty Canvas";return "";}
        auto result = predictDigit(imgToTensor(canvas));
        auto predicted = result.first;
        auto confidence = result.second;
        output = "Predict result:" + QString::number(predicted) + ", "
                             + "Confidence: " + QString::number(confidence);
        qDebug()<<predicted <<confidence;
        return output;
    }

    QImage getProcessedImg()
    {
        QImage img(canvas);
        img = img.scaled(28,28,Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
        img = img.convertToFormat(QImage::Format_Grayscale8);
        return img;
    }

    QImage getProcessedImg(QImage _img)
    {
        QImage img(_img);
        img = img.scaled(28,28,Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
        img = img.convertToFormat(QImage::Format_Grayscale8);
        return img;
    }

    void clear()
    {
        canvas.fill(Qt::black);
        update();
        qDebug() << "Canvas cleared.";
    } //clear slot function

    void saveToImage()
    {
        if (canvas.isNull()) {
            qDebug() << "Empty Canvas";
            return;
        }

        QDir dir(QCoreApplication::applicationDirPath());
        QString basePath = dir.filePath("my_dataset");

        QString labelDir = QString::number(label_val);
        QString fullPath = basePath + "/" + labelDir;
        if (!dir.exists(fullPath)) {
            dir.mkpath(fullPath);
        }

        QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss_zzz");
        QString filePath = fullPath + "/" + timestamp + ".png";
        //path config

        QImage img= getProcessedImg();

        if (img.save(filePath, "PNG")) {
            qDebug() << "Saved to" << filePath;
            clear();
        } else {
            qDebug() << "Fail to save the canvas";
        }
    }
    QString getOutput(){return output;}
private:
    QImage canvas; //storaged image,note that its update must be viewd through paintEvent()
    QPoint lastPoint; //in order to determine start of a line
    bool is_drawing=false; //flag
    int label_val=0;
    bool model_loaded=false;
    torch::jit::script::Module module;
    QString output="";
    void drawPoint(const QPoint& pos)
    {
        QPainter painter(&canvas);
        painter.setPen(QPen(Qt::white,15,Qt::SolidLine,Qt::RoundCap));
        painter.drawPoint(pos);
        update(); //update window so that you can see new canvas
    }

    QList<QImage> splitDigits()
    {
        QImage img = getProcessedImg();
        auto w=img.width();auto h=img.height();
        QList<int> whiteCnt(w,0);
        for(int i=0;i<w;++i)
        {
            for(int j=0;j<h;++j)
            {
                auto pointRGB = img.pixel(i,j);
                if(qGray(pointRGB)>200)whiteCnt[i]++;
            }
        } //row i contains white pixal if whiteCnt[i]>0

        QList<QPair<int,int>> segments; //<start,end> for each segment
        int x=0;int start=0;int end=0;int threshold=5;
        while(x<w) //x:current row
        {
            while(whiteCnt[x]==0){++x;continue;} //skip all blank column on the left
            //if out of loop, we find first white pixal on the left,aka start
            start=x;
            while(whiteCnt[x]>0){++x;}
            //if out of loop, we find first blank column on the right,indexed end = x-1
            end=x-1;
            if(end-start>=threshold){segments.append({start,end});}
        }
        QList<QImage> imgList;
        for(auto seg:segments)
        {
            QRect rect(seg.first,0,seg.second,h);
            QImage segImage=img.copy(rect);
            segImage=getProcessedImg(segImage);
            imgList.append(segImage);
        }
        return imgList;
    }

    void paintEvent(QPaintEvent* ev)override{
        Q_UNUSED(ev);
        QPainter painter(this);painter.drawImage(0,0,canvas);}
    //update window with storaged canvas

    void mousePressEvent(QMouseEvent *event) override{
        if(event->button()==Qt::MouseButton::LeftButton)
        {
            is_drawing = true;
            lastPoint=event->pos();
            drawPoint(event->pos());
        }
    } //set is_drawing to true when left button is pressed and draw a point on current position

    void mouseMoveEvent(QMouseEvent *ev) override{
        if(is_drawing && ev->buttons()&Qt::MouseButton::LeftButton)
        {
            QPoint current = ev->pos();
            QPainter painter(&canvas);
            painter.setPen(QPen(Qt::white,15,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
            painter.drawLine(lastPoint,current); //start-end syntax
            lastPoint=current; //update lastPoint to currentPoint
            update(); //update window
        }
    }
    //if is_drawing, move to draw a line
    void mouseReleaseEvent(QMouseEvent *ev) override
    {
        if(ev->button()==Qt::MouseButton::LeftButton)
        {
            is_drawing=false;
        }
    } //set is_drawing to false when release left button
    void resizeEvent(QResizeEvent *event) override{
        QImage newCanvas(event->size(), QImage::Format_RGB32);
        newCanvas.fill(Qt::black);
        QPainter painter(&newCanvas);
        painter.drawImage(0, 0, canvas);
        canvas = newCanvas;
        QWidget::resizeEvent(event);
    }

    void keyPressEvent(QKeyEvent* ev)override
    {
        if(ev->modifiers()==Qt::ControlModifier&&ev->key()==Qt::Key_C)
        {
            clear();
            ev->accept();
        }
        else if(ev->modifiers()==Qt::ControlModifier&&ev->key()==Qt::Key_S)
        {
            saveToImage();
            ev->accept();
        }
        else if(ev->modifiers()==Qt::ControlModifier&&ev->key()==Qt::Key_R)
        {
            recognize();
            ev->accept();
        }

        else
        {
            QWidget::keyPressEvent(ev);
        }
    }


public:
    HandWriting(QWidget* parent = nullptr);
    void setLabel(int a){label_val=a;}

     //model layer
    void loadModel()
    {
        try {
            module = torch::jit::load((PATH));
            module.eval();
            model_loaded=true;
            qDebug() << "Model loaded.";

        } catch (const c10::Error& e) {
            qDebug() << "Error loading model." << e.what();
            model_loaded=false;
        }
    }
protected:

    // void mouseReleaseEvent(QMouseEvent* ev)override{qDebug() << "Mouse released"<< ev->pos();};
    // void mouseMoveEvent(QMouseEvent* ev) override{qDebug() << "Mouse moved" << ev->pos();};
};
#endif // HANDWRITING_H
