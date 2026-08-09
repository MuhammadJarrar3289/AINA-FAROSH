#include "ProjectManager.h"
#include "FontInspector.h"
#include <QDir>
#include <QFileInfo>
#include <QFontDatabase>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QCoreApplication>

ProjectManager::ProjectManager(QObject *parent) : QObject(parent) {
    // Default manifest location: project root (one level up from the executable directory).
    // This assumes a typical out-of-source build where the build folder is inside the project.
    QDir appDir(QCoreApplication::applicationDirPath());
    QDir projectDir = appDir;
    if (projectDir.cdUp()) {
        manifestPath = projectDir.filePath("project.manifest.json");
    } else {
        // Fallback to exe directory if we cannot go up
        manifestPath = appDir.filePath("project.manifest.json");
    }
    qDebug() << "ProjectManager manifestPath set to" << manifestPath;

    // Try to auto-load manifest if present
    manifestLoaded = loadManifest("");
}

QVariantList ProjectManager::scanFonts() {
    m_fonts.clear();
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
        QString absPath = fi.absoluteFilePath();
        entry["path"] = absPath;
        entry["fileName"] = fi.fileName();

        // Try to add to QFontDatabase to get family name(s)
        int id = QFontDatabase::addApplicationFont(absPath);
        QString family = fi.baseName();
        if (id != -1) {
            QStringList families = QFontDatabase::applicationFontFamilies(id);
            if (!families.isEmpty()) family = families.at(0);
        }
        entry["name"] = family;

        QString reason;
        bool canEmbed = FontInspector::canEmbedFont(absPath, reason);
        entry["auto_detected_embed"] = canEmbed;
        entry["reason"] = reason;
        entry["license"] = QString("");

        // If manifest had an entry for this font, merge overrides and license
        QVariantMap manifestEntry;
        if (manifestLoaded) {
            QString keyPath = absPath;
            QString keyFile = fi.fileName();
            if (manifestIndex.contains(keyPath)) manifestEntry = manifestIndex.value(keyPath);
            else if (manifestIndex.contains(keyFile)) manifestEntry = manifestIndex.value(keyFile);
        }
        if (!manifestEntry.isEmpty()) {
            // preserve manifest values where present
            if (manifestEntry.contains("embed_override")) entry["embed_override"] = manifestEntry.value("embed_override");
            else entry["embed_override"] = QVariant();
            if (manifestEntry.contains("license")) entry["license"] = manifestEntry.value("license");
            if (manifestEntry.contains("auto_detected_embed")) entry["auto_detected_embed"] = manifestEntry.value("auto_detected_embed");
        } else {
            entry["embed_override"] = QVariant(); // no override initially
        }

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
            // Auto-save manifest so overrides persist without extra user action
            saveManifest("");
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
        // Save relative path where possible (relative to project root)
        QString path = m.value("path").toString();
        QString savePath = path;
        QFileInfo pf(manifestPath);
        QDir projectDir = pf.dir();
        QDir fileDir = QFileInfo(path).dir();
        QString rel = projectDir.relativeFilePath(path);
        if (!rel.startsWith("..")) savePath = rel;
        fo["path"] = QJsonValue::fromVariant(savePath);
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
    manifestIndex.clear();
    for (const QJsonValue &val : arr) {
        QJsonObject fo = val.toObject();
        QVariantMap entry;
        entry["name"] = fo.value("name").toString();
        entry["path"] = fo.value("path").toString();
        entry["auto_detected_embed"] = fo.value("auto_detected_embed").toBool();
        if (fo.contains("embed_override") && !fo.value("embed_override").isNull()) entry["embed_override"] = fo.value("embed_override").toBool();
        else entry["embed_override"] = QVariant();
        entry["license"] = fo.value("license").toString();

        // Index by the saved path (which may be relative) and by filename
        QString savedPath = entry["path"].toString();
        QFileInfo mf(savedPath);
        QString fileName = mf.fileName();
        // Resolve relative to projectDir if necessary
        QFileInfo p(manifestPath);
        QDir projectDir = p.dir();
        QString absPath = savedPath;
        if (QDir::isRelativePath(savedPath)) absPath = projectDir.filePath(savedPath);
        manifestIndex.insert(absPath, entry);
        manifestIndex.insert(fileName, entry);
    }
    manifestLoaded = true;
    qDebug() << "Loaded manifest from" << path << "with" << manifestIndex.size() << "entries";
    return true;
}
