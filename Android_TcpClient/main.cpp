#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "TcpClient.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    qmlRegisterType<TcpClient>("App.Network", 1, 0, "TcpClient");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}

