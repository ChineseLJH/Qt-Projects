#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#ifdef Q_OS_ANDROID
#include <QJniObject>
#include <QJniEnvironment>
#include "BluetoothManager.h"
#endif

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

#ifdef Q_OS_ANDROID
    auto activity = QNativeInterface::QAndroidApplication::context();
    QStringList perms = {
        "android.permission.ACCESS_FINE_LOCATION",
        "android.permission.BLUETOOTH_SCAN",
        "android.permission.BLUETOOTH_CONNECT",
        "android.permission.BLUETOOTH_ADVERTISE"
    };

    bool needRequest = false;
    for (const QString &p : perms) {
        jint result = QJniObject::callStaticMethod<jint>(
            "androidx/core/app/ActivityCompat",
            "checkSelfPermission",
            "(Landroid/content/Context;Ljava/lang/String;)I",
            activity.object(), QJniObject::fromString(p).object<jstring>()
            );
        if (result != 0) {
            needRequest = true;
            break;
        }
    }

    if (needRequest) {
        QJniEnvironment env;
        jclass stringClass = env->FindClass("java/lang/String");
        jobjectArray jStrArray = env->NewObjectArray(perms.size(), stringClass, nullptr);
        for (int i = 0; i < perms.size(); ++i) {
            env->SetObjectArrayElement(jStrArray, i, QJniObject::fromString(perms[i]).object<jstring>());
        }

        QJniObject::callStaticMethod<void>(
            "androidx/core/app/ActivityCompat",
            "requestPermissions",
            "(Landroid/app/Activity;[Ljava/lang/String;I)V",
            activity.object(), jStrArray, 0
            );
    }
#endif

    QQmlApplicationEngine engine;
    qmlRegisterType<BluetoothManager>("MyApp", 1, 0, "BluetoothManager");

    const QUrl url(u"qrc:/main.qml"_qs);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
                         if (!obj && url == objUrl)
                             QCoreApplication::exit(-1);
                     }, Qt::QueuedConnection);

    engine.load(url);
    return app.exec();
}


