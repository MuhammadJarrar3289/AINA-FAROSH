#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QFontDatabase>
#include "Backend.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setOrganizationName("UrduPoetryStudio");
    app.setApplicationName("Urdu Poetry Studio - POC");

    // Try to load bundled font
    int id = QFontDatabase::addApplicationFont(":/fonts/NotoNastaliqUrdu-Regular.ttf");
    if (id != -1) {
        QString family = QFontDatabase::applicationFontFamilies(id).at(0);
        qWarning() << "Loaded font:" << family;
    } else {
        qWarning() << "Could not load bundled font. Ensure fonts/NotoNastaliqUrdu-Regular.ttf exists or install a Nastaliq font system-wide.";
    }

    QQmlApplicationEngine engine;
    Backend backend;
    engine.rootContext()->setContextProperty("Backend", &backend);
    const QUrl url(QStringLiteral("qrc:/qml/Main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
