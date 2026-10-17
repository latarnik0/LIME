#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>
#include <csignal>
#include "include/structs.hpp"
#include "include/lime_core.hpp"
#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    const QUrl url(u"qrc:/LimeModule/Main.qml"_qs);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.load(url);
    
    return app.exec();
}