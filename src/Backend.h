#pragma once
#include <QObject>
#include <QString>

class Backend : public QObject {
    Q_OBJECT
public:
    explicit Backend(QObject *parent = nullptr);

    Q_INVOKABLE bool exportCurrentViewToPdf(const QString &qmlObjectName, const QString &outputPath, int dpi = 300);
};
