#include "mainwindow.h"
#include <QApplication>
#include <QIcon>
#include <QStyleFactory>
#include <QPalette>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    w.setWindowTitle("音乐播放器");
    // w.setWindowIcon(QIcon("musicicon.png"));
    a.setStyle(QStyleFactory::create("Fusion")); // 使用Fusion样式（跨平台一致，不受系统主题影响）
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(30, 30, 30));          // 窗口背景色（深灰/黑）
    darkPalette.setColor(QPalette::WindowText, Qt::white);               // 窗口文字色
    darkPalette.setColor(QPalette::Base, QColor(45, 45, 45));             // 控件背景色
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);                     // 控件文字色
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));          // 按钮背景色
    darkPalette.setColor(QPalette::ButtonText, Qt::white);               // 按钮文字色
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);

    a.setPalette(darkPalette);
    w.resize(847, 558);
    w.show();

    return a.exec();
}