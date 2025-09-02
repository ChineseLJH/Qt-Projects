#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "tcpclient.h"
#include "myudp.h"



int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    qmlRegisterType<MyUdp>("MyApp", 1, 0, "MyUdp");
    qmlRegisterType<TcpClient>("MyApp", 1, 0, "TcpClient");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}
