#include "lime_qml_controller.hpp"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

int main(int argc, char *argv[])
{
	QGuiApplication app(argc, argv);

	LimeQmlController controller;

	QQmlApplicationEngine engine;
	engine.rootContext()->setContextProperty("Lime", &controller);

	const QUrl url(QStringLiteral("qrc:/LimeModule/Main.qml"));
	QObject::connect(
	    &engine,
	    &QQmlApplicationEngine::objectCreationFailed,
	    &app,
	    []() { QCoreApplication::exit(-1); },
	    Qt::QueuedConnection);

	engine.load(url);
	return app.exec();
}
