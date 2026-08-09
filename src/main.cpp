#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QFontDatabase>
#include <QDir>
#include <QDebug>
#include "Backend.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setOrganizationName("UrduPoetryStudio");
    app.setApplicationName("Urdu Poetry Studio - POC");

    // Load application fonts from ./fonts/ if present (preferred) so users can add fonts without rebuilding resources
    QDir appDir(QCoreApplication::applicationDirPath());
    QString fontsPath = appDir.filePath("fonts");
    int loadedFonts = 0;
    if (QDir(fontsPath).exists()) {
        QStringList filters;
        filters << "*.ttf" << "*.otf";
        QDir fontsDir(fontsPath);
        QFileInfoList list = fontsDir.entryInfoList(filters, QDir::Files | QDir::NoSymLinks);
        for (const QFileInfo &fi : list) {
            int id = QFontDatabase::addApplicationFont(fi.absoluteFilePath());
            if (id != -1) {
                QString family = QFontDatabase::applicationFontFamilies(id).at(0);
                qDebug() << "Loaded bundled font from fonts/:" << fi.fileName() << "->" << family;
                ++loadedFonts;
            } else {
                qWarning() << "Failed to load font:" << fi.absoluteFilePath();
            }
        }
    }

    if (loadedFonts == 0) {
        qWarning() << "No fonts found in ./fonts/. The app will use system-installed fonts or you can add fonts to the fonts/ folder.";
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
