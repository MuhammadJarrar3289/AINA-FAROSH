#pragma once
#include <QObject>
#include <QVariantList>

class ProjectManager : public QObject {
    Q_OBJECT
public:
    explicit ProjectManager(QObject *parent = nullptr);

    Q_INVOKABLE QVariantList scanFonts();
    Q_INVOKABLE QVariantList getFonts() const;
    Q_INVOKABLE bool setEmbedOverride(const QString &fontPath, const QVariant &overrideVal);
    Q_INVOKABLE bool saveManifest(const QString &outPath);
    Q_INVOKABLE bool loadManifest(const QString &inPath);

signals:
    void fontsChanged();

private:
    QVariantList m_fonts; // each entry is a QVariantMap with keys: name,path,auto_detected_embed,embed_override,license
    QString manifestPath;
};
