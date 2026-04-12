#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    // 1. 创建Qt应用程序实例，管理整个程序的生命周期
    QApplication a(argc, argv);
    // 2. 创建我们的游戏主窗口对象
    MainWindow w;
    // 3. 把窗口显示出来
    w.show();
    // 4. 启动Qt的事件循环，让程序一直运行，能响应用户操作
    return a.exec();
}