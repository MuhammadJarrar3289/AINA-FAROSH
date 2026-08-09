#pragma once
#include <QObject>
#include <QString>

class ProjectManager;

class Backend : public QObject {
    Q_OBJECT
public:
    explicit Backend(QObject *parent = nullptr);

    Q_INVOKABLE bool exportCurrentViewToPdf(const QString &qmlObjectName, const QString &outputPath, int dpi = 300);
    void setProjectManager(ProjectManager *pm) { m_projectManager = pm; }

private:
    ProjectManager *m_projectManager = nullptr;
};
