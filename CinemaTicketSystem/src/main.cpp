#include "widget.h"
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QDebug> // 引入标准输出日志

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 1. 底层路径解析：获取当前二进制程序运行的绝对磁盘路径
    // 例如：/home/luojihao/Qt-Projects/CinemaTicketSystem/build
    QString exePath = QCoreApplication::applicationDirPath();
    
    // 2. 拼接目标文件的绝对寻址路径
    QString stylePath = exePath + "/assets/style.qss";
    
    QFile styleFile(stylePath);
    
    // 3. 验证内存访问与文件读取权限
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&styleFile);
        // 将解析出的字符数组注入到 Qt 应用程序的全局样式树中
        a.setStyleSheet(stream.readAll());
        styleFile.close();
    } else {
        // 如果寻址失败，强制将底层寻址错误抛出到控制台
        qDebug() << "[System I/O Error] 样式表加载失败，无效的物理地址：" << stylePath;
    }

    Widget w;
    w.show();
    return a.exec();
}