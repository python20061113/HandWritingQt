QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
QT += multimedia multimediawidgets
CONFIG += c++17
CONFIG += warn_off
# 不要强制写 CONFIG += debug，由 IDE 自己管理

DEFINES += QT_NO_KEYWORDS
DEFINES += C10_USE_MSVC_ASSERTIONS
DEFINES += _CRT_SECURE_NO_WARNINGS _SCL_SECURE_NO_WARNINGS
DEFINES += _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
DEFINES += NOMINMAX
DEFINES += _USE_MATH_DEFINES

# ==================== LibTorch 配置（基于你的实际文件列表） ====================
LIBTORCH_BASE = D:\libtorch\libtorch

# 头文件路径（两个都需要）
INCLUDEPATH += $$LIBTORCH_BASE/include
INCLUDEPATH += $$LIBTORCH_BASE/include/torch/csrc/api/include

# 库路径
LIBS += -L$$LIBTORCH_BASE/lib

# 链接顺序重要：torch_cpu 和 c10 必须同时出现，且 torch 保留
LIBS += -ltorch -ltorch_cpu -lc10

# 如果需要 GPU（你的 cu128 版本支持 CUDA），取消下面两行的注释
LIBS += -ltorch_cuda -lc10_cuda

# Windows 系统依赖库
LIBS += -lAdvapi32 -lOle32 -lShell32 -lUser32 -lWs2_32

# 如果你需要 GPU 支持，取消下面两行的注释
# LIBS += -lc10_cuda -ltorch_cuda

# ==================== 项目源文件 ====================
SOURCES += \
    handwriting.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    handwriting.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

TRANSLATIONS += translations/lang_zh_CN.ts \
                translations/lang_zh_HK.ts \
                translations/lang_zh_TW.ts \
                translations/lang_en.ts \
                translations/lang_sv.ts
# ==================== 部署规则 ====================
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc
