# HandWritingQt
```markdown
# HandWriting 手写数字识别组件//曾哥——古希腊掌管诺手的神

## 1. 概述

`HandWriting` 是一个基于 Qt 框架和 LibTorch（PyTorch C++ 前端）的手写数字识别控件。它提供了一个画板界面，用户可以使用鼠标绘制数字，并实时加载预训练的 MNIST 模型进行识别。此外，还支持保存手写图像、更换背景画布、多数字分割与连续识别等扩展功能。

**主要依赖**
- Qt 5/6：Core, GUI, Widgets
- LibTorch（PyTorch C++ API）

**文件结构建议**
```text
HandWriting.h        // 本类定义
HandWriting.cpp      // 构造函数及外部实现（示例中未完整给出）
main.cpp             // 应用程序入口
mnist_model.pt       // 预训练模型文件
resources/           // 背景图片等资源
```

---

## 2. 类成员总览

### 2.1 公开方法

| 方法 | 返回值 | 说明 |
|------|--------|------|
| `HandWriting(QWidget* parent = nullptr)` | - | 构造函数（需在 `.cpp` 中实现初始化） |
| `void loadModel()` | `void` | 加载 `mnist_model.pt` 模型文件，设置 `model_loaded` 标志 |
| `QString recognize()` | `QString` | 识别当前画板中的单个数字，返回格式 `"数字 (conf: XX.XX%)"` |
| `QString multiRecognize()` | `QString` | 分割并识别画板中多个分离的数字，返回连续数字串及平均置信度 |
| `void clear()` | `void` | 清空画板（黑色背景，透明画笔轨迹） |
| `void saveToImage()` | `void` | 将当前画板预处理后（28×28 灰度）保存到 `my_dataset/<label>/` 目录 |
| `void loadBackground(const QString& path)` | `void` | 加载背景图片（支持任意格式），自动缩放适配窗口 |
| `void setLabel(int a)` | `void` | 设置保存图像的标签值（子目录名称） |
| `QString getOutput()` | `QString` | 获取最近一次识别的结果字符串 |
| `QString getBgPath()` | `QString` | 获取当前背景图片路径 |
| `void setBgPath(QString p)` | `void` | 设置背景图片路径 |

### 2.2 信号与槽（可扩展）

本类未显式定义信号，但 `clear()`、`saveToImage()` 等函数可直接作为槽函数使用。

### 2.3 私有成员

| 成员 | 类型 | 说明 |
|------|------|------|
| `canvas` | `QImage` | 真实画板（RGB32，黑色背景，存储所有笔迹） |
| `background` | `QImage` | 原始背景图片 |
| `scaledBackground` | `QImage` | 缩放后的背景缓存（避免每帧重复缩放） |
| `lastSize` | `QSize` | 上次窗口大小，用于判断是否需要重建背景缓存 |
| `canvas_display` | `QImage` | 显示图层（ARGB32，透明背景，只存储画笔轨迹） |
| `bgPath` | `QString` | 背景图片路径，默认为 `":/bgImage/SD006CB.png"` |
| `lastPoint` | `QPoint` | 鼠标上一位置（用于绘制连续线条） |
| `is_drawing` | `bool` | 是否正在绘制 |
| `label_val` | `int` | 当前数字标签（0-9，用于保存图像的分类目录） |
| `model_loaded` | `bool` | 模型是否成功加载 |
| `module` | `torch::jit::script::Module` | LibTorch 模型对象 |
| `output` | `QString` | 最近一次识别的结果字符串 |

### 2.4 私有方法

| 方法 | 返回值 | 说明 |
|------|--------|------|
| `torch::Tensor imgToTensor(QImage img)` | `torch::Tensor` | 将 QImage 转换为模型输入张量 `[1,1,28,28]`，自动缩放、灰度、归一化 |
| `std::pair<int,float> predictDigit(const torch::Tensor& tensor)` | `std::pair<int,float>` | 执行模型推理，返回 `{数字, 置信度}` |
| `QImage getProcessedImg()` | `QImage` | 返回当前画板缩放至 28×28 的灰度图像 |
| `QImage getProcessedImg(QImage _img)` | `QImage` | 将任意 QImage 缩放至 28×28 灰度图像 |
| `void drawPoint(const QPoint& pos)` | `void` | 在画板和显示图层上绘制一个点（笔触 15px 白色圆帽） |
| `QList<QImage> splitDigits()` | `QList<QImage>` | 垂直投影分割多个数字，返回预处理后的 28×28 灰度图像列表 |
| `void updateBackgroundCache()` | `void` | 根据窗口大小更新背景缓存（按需调用） |

---

## 3. 核心功能实现说明

### 3.1 模型加载与推理

模型文件必须放置在可执行文件同目录下，文件名为 `mnist_model.pt`。模型为 TorchScript 格式（可通过 PyTorch 的 `torch.jit.trace` 或 `script` 导出）。

```cpp
void HandWriting::loadModel()
{
    try {
        QString modelPath = QCoreApplication::applicationDirPath() + "/mnist_model.pt";
        module = torch::jit::load(modelPath.toStdString());
        module.eval();
        model_loaded = true;
        qDebug() << "Model loaded.";
    } catch (const c10::Error& e) {
        qDebug() << "Error loading model:" << e.what();
        model_loaded = false;
    }
}
```

推理步骤如下：
1. 调用 `imgToTensor` 将画板内容转换为张量。
2. 执行前向传播，得到输出 logits。
3. 通过 `argmax` 获取预测数字，通过 `softmax` 计算置信度。

### 3.2 图像预处理细节

- **缩放**：使用 `Qt::SmoothTransformation` 平滑缩放至 28×28，忽略宽高比。
- **灰度化**：`QImage::Format_Grayscale8`。
- **归一化**：像素值除以 255.0 得到 `[0,1]` 范围，再减去均值 0.1307，除以标准差 0.3081（MNIST 标准归一化参数）。

```cpp
float normalized = (pixel / 255.0f - 0.1307f) / 0.3081f;
```

### 3.3 多数字分割算法

采用**垂直投影法**：
1. 将原始画板转为灰度图像。
2. 统计每一列中白色像素（亮度 > 200）的数量。
3. 找出连续的非空列区间，宽度小于 `minWidth`（默认 8 像素）的忽略。
4. 对每个区间裁剪并缩放到 28×28，返回图像列表。

此方法适用于手写数字之间有明显空白间隔的场景。

### 3.4 画板绘制机制

- **双图层设计**：  
  - `canvas`：存储所有笔迹，黑色背景，RGB32 格式，用于保存和图像处理。  
  - `canvas_display`：存储笔迹，透明背景，ARGB32 格式，仅用于绘制显示。  
  这样可以实现背景图片与笔迹的独立合成，避免背景覆盖笔迹。
- **笔触参数**：`QPen(Qt::white, 15, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin)`。
- **事件处理**：  
  - `mousePressEvent`：开始绘制，记录起始点并画点。  
  - `mouseMoveEvent`：绘制两点之间的连线，更新 `lastPoint`。  
  - `mouseReleaseEvent`：结束绘制。

### 3.5 背景加载与缓存

背景图片通过 `loadBackground` 加载后，会在 `paintEvent` 中自动缩放至窗口大小。为了性能，使用 `updateBackgroundCache` 仅在窗口尺寸变化时重新缩放背景，否则直接使用缓存的 `scaledBackground`。

### 3.6 保存图像

- 调用 `saveToImage()` 时，将当前画板预处理为 28×28 灰度图。
- 保存路径为 `my_dataset/<label_val>/时间戳.png`，例如 `my_dataset/5/20250315_143022_123.png`。
- 如果目录不存在，自动创建。

---

## 4. 事件处理汇总

| 事件 | 处理逻辑 |
|------|----------|
| `paintEvent` | 绘制背景 → 绘制 `canvas_display`（笔迹） |
| `resizeEvent` | 调整 `canvas` 和 `canvas_display` 尺寸，保留原有内容，重置背景缓存 |
| `mousePressEvent` | 左键按下时设置 `is_drawing = true`，调用 `drawPoint` |
| `mouseMoveEvent` | 左键按住时绘制线段 |
| `mouseReleaseEvent` | 左键释放时设置 `is_drawing = false` |
| `keyPressEvent` | 响应快捷键：<br>• `Ctrl+C` → `clear()`<br>• `Ctrl+S` → `saveToImage()`<br>• `Ctrl+R` → `recognize()` |

---

## 5. 使用示例

### 5.1 最小集成示例

```cpp
#include <QApplication>
#include "HandWriting.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    HandWriting widget;
    widget.loadModel();          // 加载模型
    widget.setLabel(5);          // 设置保存标签（可选）
    widget.loadBackground(":/bgImage/SD006CB.png");
    widget.resize(600, 400);
    widget.show();
    return a.exec();
}
```

### 5.2 按钮控制（UI 扩展）

你可以将 `HandWriting` 控件与 `QPushButton` 等结合：

```cpp
QPushButton *btnRecognize = new QPushButton("识别");
QObject::connect(btnRecognize, &QPushButton::clicked, &widget, [&](){
    QString result = widget.recognize();
    QMessageBox::information(nullptr, "识别结果", result);
});

QPushButton *btnSave = new QPushButton("保存");
QObject::connect(btnSave, &QPushButton::clicked, &widget, &HandWriting::saveToImage);

QPushButton *btnClear = new QPushButton("清空");
QObject::connect(btnClear, &QPushButton::clicked, &widget, &HandWriting::clear);
```

### 5.3 多数字识别示例

```cpp
QString result = widget.multiRecognize();
// 输出类似 "42 (conf: 96.73%)"
```

---

## 6. 注意事项与常见问题

| 问题 | 解决方案 |
|------|----------|
| 模型加载失败 | 检查 `mnist_model.pt` 是否位于可执行文件同一目录，且是 TorchScript 格式。 |
| 识别结果总是某个数字 | 检查画板是否背景为黑色、笔迹为白色；预处理归一化参数是否与训练时一致。 |
| 多数字分割不准确 | 调整 `splitDigits()` 中的 `minWidth` 阈值（原始图像像素），或优化二值化阈值（200）。 |
| 背景图片显示模糊 | 使用高分辨率图片，或设置缩放模式为 `Qt::KeepAspectRatio`。 |
| 编译错误：`torch/torch.h` 找不到 | 正确配置 LibTorch 的包含路径和库路径，并在 CMake 或 .pro 文件中链接 `torch` 和 `c10`。 |
| 快捷键不生效 | 确保窗口具有焦点；检查是否有其他控件抢占了键盘事件。 |

---

## 7. 扩展建议

- **增加撤销/重做功能**：维护一个 `QStack<QImage>` 存储历史画板状态。
- **支持触摸屏/数位板**：重写 `tabletEvent` 获取压感信息，动态改变笔触大小。
- **导出识别结果**：将 `output` 写入文本文件或复制到剪贴板。
- **实时连续识别**：在 `mouseReleaseEvent` 中自动调用 `recognize()` 并显示结果。
- **模型热切换**：允许用户通过文件对话框选择不同的 `.pt` 模型文件。
- **多语言支持**：使用 `QTranslator` 实现界面中英文切换。

---

## 8. 完整类代码（HandWriting.h）

```cpp
#ifndef HANDWRITING_H
#define HANDWRITING_H

#include <QImage>
#include <QPaintEvent>
#include <QPainter>
#include <QPushButton>
#include <QFile>
#include <QFileDialog>
#include <mainwindow.h>
#include <QStandardPaths>
#include <QMessageBox>
#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QDateTime>
#include <QPixmap>
#include <QKeyEvent>
#include <torch/torch.h>
#include <torch/script.h>

class HandWriting : public QWidget
{
    Q_OBJECT
public:
    // 构造函数（需在.cpp中实现）
    HandWriting(QWidget* parent = nullptr);

    // 模型加载
    void loadModel();

    // 识别单个数字
    QString recognize();

    // 识别多个数字（垂直分割）
    QString multiRecognize();

    // 清空画板
    void clear();

    // 保存当前画板为图像
    void saveToImage();

    // 设置保存标签（0-9）
    void setLabel(int a) { label_val = a; }

    // 获取最近一次识别结果
    QString getOutput() { return output; }

    // 背景图片路径
    QString getBgPath() { return bgPath; }
    void setBgPath(QString p) { bgPath = p; }

    // 加载背景图片
    void loadBackground(const QString& path);

    // 图像转张量（公开以便测试）
    torch::Tensor imgToTensor(QImage img);

    // 预测数字及置信度
    std::pair<int, float> predictDigit(const torch::Tensor& tensor);

protected:
    void paintEvent(QPaintEvent* ev) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* ev) override;
    void mouseReleaseEvent(QMouseEvent* ev) override;
    void resizeEvent(QResizeEvent* event) override;
    void keyPressEvent(QKeyEvent* ev) override;

private:
    // 图像预处理
    QImage getProcessedImg();
    QImage getProcessedImg(QImage _img);
    void drawPoint(const QPoint& pos);
    QList<QImage> splitDigits();
    void updateBackgroundCache();

    // 画板数据
    QImage canvas;           // 真实画板（黑底）
    QImage background;       // 原始背景
    QImage scaledBackground; // 缩放后的背景缓存
    QSize lastSize;          // 上次窗口大小
    QImage canvas_display;   // 显示图层（透明底，仅笔迹）
    QString bgPath = ":/bgImage/SD006CB.png";
    QPoint lastPoint;
    bool is_drawing = false;
    int label_val = 0;

    // 模型相关
    bool model_loaded = false;
    torch::jit::script::Module module;
    QString output = "";
};

#endif // HANDWRITING_H
```

> **注**：构造函数 `HandWriting::HandWriting(QWidget* parent)` 的实现需要初始化画板尺寸、设置背景模式、启用鼠标追踪等，此处未给出，请根据实际需求补充。

---

## 9. 许可证

本文档及所附代码示例可按需使用，无特殊限制。LibTorch 和 Qt 分别遵循其各自的许可证（BSD-3-Clause 和 LGPL）。使用前请确认兼容性。
```
