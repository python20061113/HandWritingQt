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
#include<QTimer>


    class HandWriting: public QWidget
{
    Q_OBJECT
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
        output.clear();
        if(!model_loaded)return "";
        if(canvas.isNull()){qDebug()<<"Empty Canvas";return "";}
        auto result = predictDigit(imgToTensor(canvas));
        auto predicted = result.first;
        auto confidence = result.second;
        output+= QString::number(predicted);
        output += tr(" (conf: %1%)").arg(confidence * 100, 0, 'f', 2);
        qDebug()<<predicted <<confidence;
        return output;
    }

    // QString multiRecognize()
    // {
    //     output="Predict result: ";
    //     if(!model_loaded)return"";
    //     const auto vec(splitDigits());float confidence=0;
    //     if(vec.empty())return{};
    //     std::vector<std::pair<int,float>> resultVec;
    //     for(const auto& div:vec)
    //     {
    //         auto now =predictDigit(imgToTensor(div));
    //         resultVec.push_back(now);
    //         confidence+=now.second;
    //     }
    //     confidence/=resultVec.size();
    //     for(const auto& pair:resultVec)
    //     {
    //         output+=(QString::number(pair.first));
    //     }
    //     output+=("Confidence:");
    //     output+=(QString::number(confidence));
    //     return output;
    // }

    QString multiRecognize() {
        if (!model_loaded) return {};
        auto digitImages = splitDigits();
        if (digitImages.isEmpty()) return {};
        float tconf=0.00;
        QString result;
        for (const QImage& img : std::as_const(digitImages)) {
            auto [digit, conf] = predictDigit(imgToTensor(img));
            tconf+=conf;
            result += QString::number(digit);
        }
        auto avgConf = tconf/digitImages.size();
        result += tr(" (conf: %1%)").arg(avgConf * 100, 0, 'f', 2);
        qDebug()<<result;
        output=result;
        return result;
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
        canvas_display.fill(Qt::transparent);
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
            // clear();
        } else {
            qDebug() << "Fail to save the canvas";
        }
    }
    QString getOutput(){return output;}
    QString getBgPath(){return bgPath;}
    void setBgPath(QString p){bgPath=p;}
    void loadBackground(const QString& path)
    {
        bool sucessed = background.load(path);
        if(!sucessed){qDebug()<<"fail" ;return;}
        else
        {
            qDebug()<<"bg loaded";
            background=background.scaled(size(),Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation);
            background=background.convertToFormat(QImage::Format_ARGB32_Premultiplied);
        }
        update();
    }


private:
    QImage canvas; //storaged image,note that its update must be viewd through paintEvent()
    QImage background;
    QImage scaledBackground;
    QSize lastSize;
    QImage canvas_display;
    QString bgPath=":/bgImage/SD006CB.png";
    QPoint lastPoint; //in order to determine start of a line
    bool is_drawing=false; //flag
    int label_val=0;
    bool model_loaded=false;
    torch::jit::script::Module module;
    QString output="";
    QTimer* timer;
    void drawPoint(const QPoint& pos)
    {
        QPainter painter(&canvas);
        QPainter p2(&canvas_display);
        painter.setPen(QPen(Qt::white,15,Qt::SolidLine,Qt::RoundCap));
        painter.drawPoint(pos);
        p2.setPen(QPen(Qt::white,15,Qt::SolidLine,Qt::RoundCap));
        p2.drawPoint(pos);
        update(); //update window so that you can see new canvas
    }

    // QList<QImage> splitDigits()
    // {
    //     QImage img = getProcessedImg();
    //     auto w=img.width();auto h=img.height();
    //     QList<int> whiteCnt(w,0);
    //     for(int i=0;i<w;++i)
    //     {
    //         for(int j=0;j<h;++j)
    //         {
    //             auto pointRGB = img.pixel(i,j);
    //             if(qGray(pointRGB)>200)whiteCnt[i]++;
    //         }
    //     } //row i contains white pixal if whiteCnt[i]>0

    //     QList<QPair<int,int>> segments; //<start,end> for each segment
    //     int x=0;int start=0;int end=0;int threshold=5;
    //     while(x<w) //x:current row
    //     {
    //         while(whiteCnt[x]==0){++x;continue;} //skip all blank column on the left
    //         //if out of loop, we find first white pixal on the left,aka start
    //         start=x;
    //         while(whiteCnt[x]>0){++x;}
    //         //if out of loop, we find first blank column on the right,indexed end = x-1
    //         end=x-1;
    //         if(end-start>=threshold){segments.append({start,end});}
    //     }
    //     QList<QImage> imgList;
    //     for(auto seg:segments)
    //     {
    //         QRect rect(seg.first,0,seg.second,h);
    //         QImage segImage=img.copy(rect);
    //         segImage=getProcessedImg(segImage);
    //         imgList.append(segImage);
    //     }
    //     return imgList;
    // }

    QList<QImage> splitDigits() {
        QList<QImage> result;
        // 使用原始画板图像（未缩放）
        QImage original = canvas;
        if (original.isNull()) return result;

        // 转为灰度，便于判断
        QImage gray = original.convertToFormat(QImage::Format_Grayscale8);
        int w = gray.width();
        int h = gray.height();

        // 垂直投影：统计每一列白色像素数量（假设黑底白字，白色为笔画）
        QVector<int> whiteCnt(w, 0);
        for (int x = 0; x < w; ++x) {
            for (int y = 0; y < h; ++y) {
                if (qGray(gray.pixel(x, y)) > 200)   // 白色阈值
                    whiteCnt[x]++;
            }
        }

        // 查找连续的非空白列区间
        QList<QPair<int,int>> segments;
        int x = 0;
        const int minWidth = 8;   // 最小数字宽度（原始图像上的像素）
        while (x < w) {
            // 跳过空白列（注意先判断边界）
            while (x < w && whiteCnt[x] == 0) ++x;
            if (x >= w) break;
            int start = x;
            while (x < w && whiteCnt[x] > 0) ++x;
            int end = x - 1;
            if (end - start + 1 >= minWidth) {
                segments.append({start, end});
            }
        }

        // 对每个分割区域，裁剪并预处理为 28x28 灰度图
        for (const auto& seg : segments) {
            QRect rect(seg.first, 0, seg.second - seg.first + 1, h);
            QImage digit = gray.copy(rect);
            // 缩放到 28x28，转为灰度（已经是灰度），可选反色
            digit = digit.scaled(28, 28, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            digit = digit.convertToFormat(QImage::Format_Grayscale8);
            // 如果模型训练时使用白底黑字，则需反色，这里假设黑底白字（与画板一致）
            // digit.invertPixels();   // 根据你的模型决定
            result.append(digit);
        }
        return result;
    }

    void paintEvent(QPaintEvent* ev)override{
        Q_UNUSED(ev);
        QPainter painter(this);

        if(!background.isNull()){
            // painter.setRenderHint(QPainter::Antialiasing);
            // painter.setRenderHint(QPainter::SmoothPixmapTransform);
            updateBackgroundCache();
            painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
            painter.drawImage(0,0,scaledBackground);
        }
        else
        {
            painter.fillRect(rect(),Qt::transparent);
        }
        // painter.setRenderHint(QPainter::Antialiasing,false);
        // painter.setRenderHint(QPainter::SmoothPixmapTransform,false);
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter.drawImage(0,0,canvas_display);
    }
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
            QPainter p2(&canvas_display);
            p2.setPen(QPen(Qt::white,15,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
            p2.drawLine(lastPoint,current);
            lastPoint=current; //update lastPoint to currentPoint
            update(); //update window
        }
    }
    //if is_drawing, move to draw a line
    void mouseReleaseEvent(QMouseEvent *ev) override
    {
        if(ev->button()==Qt::MouseButton::LeftButton)
        {
            const int waiting_time=300;
            is_drawing=false;
            timer->start(waiting_time);
        }
    } //set is_drawing to false when release left button


    void resizeEvent(QResizeEvent *event) override{
        QImage newCanvas(event->size(), QImage::Format_RGB32);
        QImage newCanvas_display(event->size(),QImage::Format_ARGB32_Premultiplied);
        newCanvas.fill(Qt::black);
        newCanvas_display.fill(Qt::transparent);
        QPainter painter(&newCanvas);
        QPainter p2(&newCanvas_display);
        painter.drawImage(0, 0, canvas);
        p2.drawImage(0,0,canvas_display);
        canvas = newCanvas;
        canvas_display=newCanvas_display;
        // if(!background.isNull()){
        //     background=background.scaled(event->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation);
        // }
        QWidget::resizeEvent(event);
        lastSize=QSize();
        update();
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

    void updateBackgroundCache() {
        if (background.isNull()) return;
        QSize widgetSize = size();
        if (lastSize == widgetSize && ! scaledBackground.isNull())
            return;
        scaledBackground = background.scaled(widgetSize,
                                             Qt::KeepAspectRatioByExpanding,
                                             Qt::SmoothTransformation);
        lastSize = widgetSize;
    }
public:
    HandWriting(QWidget* parent = nullptr);
    void setLabel(int a){label_val=a;}

    //model layer
    void loadModel()
    {
        try {
            QString modelPath=QCoreApplication::applicationDirPath() + "/mnist_model.pt";
            module = torch::jit::load(modelPath.toStdString());
            module.eval();
            model_loaded=true;
            qDebug() << "Model loaded. PATH:" << modelPath;

        } catch (const c10::Error& e) {
            qDebug() << "Error loading model." << e.what();
            model_loaded=false;
        }
    }
protected:

    // void mouseReleaseEvent(QMouseEvent* ev)override{qDebug() << "Mouse released"<< ev->pos();};
    // void mouseMoveEvent(QMouseEvent* ev) override{qDebug() << "Mouse moved" << ev->pos();};
Q_SIGNALS:
    void recFinished(const QString& str);
};
#endif // HANDWRITING_H
