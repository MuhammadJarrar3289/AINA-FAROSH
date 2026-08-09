#include "ProjectManager.h"
#include "FontInspector.h"
#include <QDir>
#include <QFileInfo>
#include <QFontDatabase>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

ProjectManager::ProjectManager(QObject *parent) : QObject(parent) {
    manifestPath = "project.manifest.json"; // default for POC
}

QVariantList ProjectManager::scanFonts() {
    m_fonts.clear();
    QDir fontsDir(QCoreApplication::applicationDirPath());
    fontsDir.cdUp(); // try parent? actually applicationDirPath already points to exe folder; fonts directory expected next to exe
    // We'll prefer a ./fonts folder next to the exe
    QDir appDir(QCoreApplication::applicationDirPath());
    QDir fd(appDir.filePath("fonts"));
    if (!fd.exists()) {
        qWarning() << "Fonts folder not found:" << fd.path();
        emit fontsChanged();
        return m_fonts;
    }

    QStringList filters;
    filters << "*.ttf" << "*.otf";
    QFileInfoList list = fd.entryInfoList(filters, QDir::Files | QDir::NoSymLinks);
    for (const QFileInfo &fi : list) {
        QVariantMap entry;
        entry["path"] = fi.filePath();
        entry["fileName"] = fi.fileName();

        // Try to add to QFontDatabase to get family name(s)
        int id = QFontDatabase::addApplicationFont(fi.filePath());
        QString family = fi.baseName();
        if (id != -1) {
            QStringList families = QFontDatabase::applicationFontFamilies(id);
            if (!families.isEmpty()) family = families.at(0);
        }
        entry["name"] = family;

        QString reason;
        bool canEmbed = FontInspector::canEmbedFont(fi.filePath(), reason);
        entry["auto_detected_embed"] = canEmbed;
        entry["embed_override"] = QJsonValue::Null; // no override initially
        entry["reason"] = reason;
        entry["license"] = QString("");

        m_fonts.append(entry);
    }

    emit fontsChanged();
    return m_fonts;
}

QVariantList ProjectManager::getFonts() const {
    return m_fonts;
}

bool ProjectManager::setEmbedOverride(const QString &fontPath, const QVariant &overrideVal) {
    for (int i = 0; i < m_fonts.size(); ++i) {
        QVariantMap entry = m_fonts.at(i).toMap();
        if (entry.value("path").toString() == fontPath) {
            if (overrideVal.isNull()) entry["embed_override"] = QVariant();
            else entry["embed_override"] = overrideVal;
            m_fonts[i] = entry;
            emit fontsChanged();
            return true;
        }
    }
    return false;
}

bool ProjectManager::saveManifest(const QString &outPath) {
    QJsonObject root;
    QJsonArray fontsArr;
    for (const QVariant &v : m_fonts) {
        QVariantMap m = v.toMap();
        QJsonObject fo;
        fo["name"] = QJsonValue::fromVariant(m.value("name"));
        fo["path"] = QJsonValue::fromVariant(m.value("path"));
        fo["auto_detected_embed"] = QJsonValue::fromVariant(m.value("auto_detected_embed"));
        // embed_override: null or bool
        QVariant ov = m.value("embed_override");
        if (!ov.isValid() || ov.isNull()) fo["embed_override"] = QJsonValue();
        else fo["embed_override"] = ov.toBool();
        fo["license"] = QJsonValue::fromVariant(m.value("license"));
        fontsArr.append(fo);
    }
    root["fonts"] = fontsArr;
    QJsonDocument doc(root);
    QFile f(outPath.isEmpty() ? manifestPath : outPath);
    if (!f.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open manifest for writing:" << f.fileName();
        return false;
    }
    f.write(doc.toJson(QJsonDocument::Indented));
    f.close();
    qDebug() << "Saved manifest to" << f.fileName();
    return true;
}

bool ProjectManager::loadManifest(const QString &inPath) {
    QString path = inPath.isEmpty() ? manifestPath : inPath;
    QFile f(path);
    if (!f.exists()) return false;
    if (!f.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open manifest for reading:" << path;
        return false;
    }
    QByteArray data = f.readAll();
    f.close();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) return false;
    QJsonObject root = doc.object();
    if (!root.contains("fonts")) return false;
    QJsonArray arr = root["fonts"].toArray();
    m_fonts.clear();
    for (const QJsonValue &val : arr) {
        QJsonObject fo = val.toObject();
        QVariantMap entry;
        entry["name"] = fo.value("name").toString();
        entry["path"] = fo.value("path").toString();
        entry["auto_detected_embed"] = fo.value("auto_detected_embed").toBool();
        if (fo.contains("embed_override") && !fo.value("embed_override").isNull()) entry["embed_override"] = fo.value("embed_override").toBool();
        else entry["embed_override"] = QVariant();
        entry["license"] = fo.value("license").toString();
        m_fonts.append(entry);
    }
    emit fontsChanged();
    qDebug() << "Loaded manifest from" << path;
    return true;
}
