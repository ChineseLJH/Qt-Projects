#include "mainwindow.h"
#include <QApplication>
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 设置应用信息，这对于QStandardPaths找到正确路径很重要
    // 修正了这里的拼写错误
    QCoreApplication::setOrganizationName("MyCompany");
    QCoreApplication::setApplicationName("KeShe");

    MainWindow w;
    w.show();
    return a.exec();
}

