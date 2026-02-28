#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "udpdataclient.h"
#include "myudp.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    qmlRegisterType<MyUdp>("MyApp", 1, 0, "MyUdp");

    qmlRegisterType<UdpDataClient>("MyApp", 1, 0, "UdpDataClient");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}
