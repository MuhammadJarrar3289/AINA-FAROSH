#include "Backend.h"
#include <QQuickWindow>
#include <QQuickItem>
#include <QPdfWriter>
#include <QPainter>
#include <QGuiApplication>
#include <QScreen>
#include <QFile>
#include <QImage>
#include <QDebug>

Backend::Backend(QObject *parent) : QObject(parent) {}

bool Backend::exportCurrentViewToPdf(const QString &qmlObjectName, const QString &outputPath, int dpi) {
    // Find the root QQuickWindow (assumes single window)
    QObject *root = qGuiApp->allWindows().isEmpty() ? nullptr : qGuiApp->allWindows().first();
    if (!root) {
        qWarning() << "No QQuickWindow found";
        return false;
    }

    QQuickWindow *win = qobject_cast<QQuickWindow *>(root);
    if (!win) {
        qWarning() << "Root is not QQuickWindow";
        return false;
    }

    // Find the QML item by objectName
    QQuickItem *item = win->contentItem()->findChild<QQuickItem*>(qmlObjectName);
    if (!item) {
        qWarning() << "QML item" << qmlObjectName << "not found";
        return false;
    }

    // Grab a high-res image using grabToImage (async) but we will use synchronous approach for POC
    QImage image(item->width(), item->height(), QImage::Format_ARGB32);
    image.fill(Qt::white);
    QPainter painter(&image);
    win->render(&painter);
    painter.end();

    // Scale to requested DPI: assume 96 DPI base
    const int baseDpi = 96;
    double scale = double(dpi) / baseDpi;
    QImage scaled = image.scaled(image.width()*scale, image.height()*scale, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // Write to PDF
    QPdfWriter writer(outputPath);
    writer.setResolution(dpi);
    QSizeF pageSizeMM((scaled.width()/dpi)*25.4, (scaled.height()/dpi)*25.4); // convert inches->mm via dpi
    writer.setPageSizeMM(pageSizeMM);
    QPainter pdfPainter(&writer);
    pdfPainter.drawImage(QPoint(0,0), scaled);
    pdfPainter.end();

    return true;
}
