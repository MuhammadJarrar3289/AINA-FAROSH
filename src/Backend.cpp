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
#include <QTextDocument>
#include <QTextOption>
#include <QFontDatabase>

Backend::Backend(QObject *parent) : QObject(parent) {}

// Export a poetry canvas to a vector PDF by extracting text content from the QML editor
bool Backend::exportCurrentViewToPdf(const QString &qmlObjectName, const QString &outputPath, int dpi) {
    // Find the root QQuickWindow (assumes single window)
    QQuickWindow *win = nullptr;
    const auto windows = qGuiApp->allWindows();
    if (!windows.isEmpty()) {
        win = qobject_cast<QQuickWindow *>(windows.first());
    }
    if (!win) {
        qWarning() << "No QQuickWindow found";
        return false;
    }

    // Find the QML item by objectName (poetryCanvas)
    QQuickItem *canvas = win->contentItem()->findChild<QQuickItem*>(qmlObjectName);
    if (!canvas) {
        qWarning() << "QML item" << qmlObjectName << "not found";
        return false;
    }

    // Try to find the embedded PoetryEditor by objectName "poetryEditor"
    QObject *editor = canvas->findChild<QObject*>("poetryEditor");
    QString content;
    QString fontFamily = QFont().family();
    int fontSize = 20;
    if (editor) {
        // PoetryEditor exposes 'content', 'fontFamily', 'fontSize'
        QVariant c = editor->property("content");
        if (c.isValid()) content = c.toString();
        QVariant ff = editor->property("fontFamily");
        if (ff.isValid()) fontFamily = ff.toString();
        QVariant fs = editor->property("fontSize");
        if (fs.isValid()) fontSize = fs.toInt();
        qDebug() << "Export: got editor content length" << content.length() << "font" << fontFamily << "size" << fontSize;
    } else {
        qWarning() << "PoetryEditor (poetryEditor) not found. Falling back to raster export.";
    }

    // If we have textual content, produce a vector PDF using QTextDocument
    if (!content.isEmpty()) {
        // Determine canvas logical size
        qreal w = canvas->width();
        qreal h = canvas->height();
        if (w <= 0 || h <= 0) {
            qWarning() << "Invalid canvas size" << w << h;
            return false;
        }

        const int baseDpi = 96;
        double scale = double(dpi) / baseDpi;

        // Create PDF writer with page size matching canvas at requested DPI
        QPdfWriter writer(outputPath);
        writer.setResolution(dpi);
        QSizeF pageSizeMM((w / baseDpi) * 25.4, (h / baseDpi) * 25.4);
        writer.setPageSizeMM(pageSizeMM);

        QPainter painter(&writer);
        painter.setRenderHint(QPainter::Antialiasing);

        // Scale painter so we can draw using logical pixels (matching QML sizes)
        painter.scale(scale, scale);

        // Setup a simple layout: draw the title if available, then the poem content
        QString titleText;
        QObject *titleObj = canvas->findChild<QObject*>("titleText");
        if (titleObj) {
            QVariant t = titleObj->property("text");
            if (t.isValid()) titleText = t.toString();
        }

        // Draw title
        qreal x = 0;
        qreal y = 0;
        qreal margin = 0; // poetryCanvas already accounts for margins

        if (!titleText.isEmpty()) {
            QFont titleFont(fontFamily);
            titleFont.setPixelSize(34);
            painter.setFont(titleFont);
            painter.setPen(Qt::black);
            QRectF titleRect(x, y, w, 60);
            // Right-aligned title
            QTextOption titleOpt;
            titleOpt.setAlignment(Qt::AlignHCenter);
            painter.drawText(titleRect, Qt::AlignHCenter | Qt::AlignVCenter, titleText);
            y += 60 + 8; // move below title
        }

        // Prepare QTextDocument for poem content
        QTextDocument doc;
        QFont bodyFont(fontFamily);
        bodyFont.setPixelSize(fontSize);
        doc.setDefaultFont(bodyFont);
        doc.setDefaultTextOption(QTextOption(Qt::AlignRight));
        // Preserve line breaks: setPlainText will keep them
        doc.setPlainText(content);

        // Set document width to canvas width so lines wrap only if they exceed width
        doc.setTextWidth(w);

        // Render the document at position (x,y)
        painter.translate(x, y);
        doc.drawContents(&painter);
        painter.end();

        qDebug() << "Vector PDF exported to" << outputPath;
        return true;
    }

    // Fallback: raster capture (previous behavior)
    qWarning() << "No textual content available for vector export — falling back to raster capture.";

    QImage image(canvas->width(), canvas->height(), QImage::Format_ARGB32);
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
    QSizeF pageSizeMM((scaled.width()/dpi)*25.4, (scaled.height()/dpi)*25.4);
    writer.setPageSizeMM(pageSizeMM);
    QPainter pdfPainter(&writer);
    pdfPainter.drawImage(QPoint(0,0), scaled);
    pdfPainter.end();

    return true;
}
