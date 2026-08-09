#include "Backend.h"
#include "ProjectManager.h"
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
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

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

    // Prepare export report
    QJsonObject report;
    QJsonArray fontsReportArr;

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

        // Determine embedding decision for the font used
        bool embedAllowed = true; // default
        QString fontPathUsed;
        if (m_projectManager) {
            QVariantList fonts = m_projectManager->getFonts();
            for (const QVariant &v : fonts) {
                QVariantMap m = v.toMap();
                QString name = m.value("name").toString();
                QString path = m.value("path").toString();
                bool auto_detect = m.value("auto_detected_embed").toBool();
                QVariant ov = m.value("embed_override");
                bool overridePresent = ov.isValid() && !ov.isNull();
                bool overrideVal = overridePresent ? ov.toBool() : false;
                if (name == fontFamily || QFileInfo(path).baseName() == fontFamily) {
                    fontPathUsed = path;
                    if (overridePresent) embedAllowed = overrideVal;
                    else embedAllowed = auto_detect;

                    QJsonObject fr;
                    fr["name"] = name;
                    fr["path"] = path;
                    fr["auto_detected_embed"] = auto_detect;
                    if (overridePresent) fr["embed_override"] = overrideVal;
                    else fr["embed_override"] = QJsonValue();
                    fr["decided_embed"] = embedAllowed;
                    fontsReportArr.append(fr);
                    break;
                }
            }
        } else {
            qWarning() << "No ProjectManager available; defaulting to embedAllowed=true";
        }

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

        if (!titleText.isEmpty()) {
            QFont titleFont(fontFamily);
            titleFont.setPixelSize(34);
            painter.setFont(titleFont);
            painter.setPen(Qt::black);
            QRectF titleRect(x, y, w, 60);
            painter.drawText(titleRect, Qt::AlignHCenter | Qt::AlignVCenter, titleText);
            y += 60 + 8; // move below title
        }

        // Prepare QTextDocument for poem content
        QTextDocument doc;
        QFont bodyFont(fontFamily);
        bodyFont.setPixelSize(fontSize);
        doc.setDefaultFont(bodyFont);
        QTextOption opt;
        opt.setTextDirection(Qt::RightToLeft);
        opt.setAlignment(Qt::AlignRight);
        doc.setDefaultTextOption(opt);
        // Preserve line breaks: setPlainText will keep them
        doc.setPlainText(content);

        // Set document width to canvas width so lines wrap only if they exceed width
        doc.setTextWidth(w);

        if (embedAllowed) {
            // Draw document vectorially — fonts that allow embedding will be embedded as subsets by Qt
            painter.save();
            painter.translate(x, y);
            doc.drawContents(&painter);
            painter.restore();
            report["message"] = "Exported with embedded fonts where allowed.";
        } else {
            // Rasterize only the poem text (high-res) and draw image into PDF
            qDebug() << "Embedding not allowed for font" << fontFamily << "— rasterizing poem text runs.";
            // Render doc to high-res image
            QSize imgSize(int(w * scale), int(doc.size().height() * scale));
            if (imgSize.width() <=0 || imgSize.height() <=0) {
                qWarning() << "Invalid image size for rasterization" << imgSize;
            } else {
                QImage img(imgSize, QImage::Format_ARGB32);
                img.fill(Qt::transparent);
                QPainter imgPainter(&img);
                imgPainter.setRenderHint(QPainter::Antialiasing);
                imgPainter.scale(scale, scale);
                painter.setPen(Qt::NoPen);
                imgPainter.translate(0, 0);
                doc.drawContents(&imgPainter);
                imgPainter.end();

                // Draw the raster image into the PDF at position (x,y) — painter currently scaled, so draw with device-independent size
                painter.drawImage(QPointF(x, y), img);
                report["message"] = "Exported with rasterized text for fonts disallowing embedding.";

                QJsonObject fr;
                fr["fontFamily"] = fontFamily;
                fr["embed_decision"] = false;
                fr["note"] = "Rasterized poem text due to embedding restrictions.";
                fontsReportArr.append(fr);
            }
        }

        painter.end();

        report["fonts"] = fontsReportArr;
        // Write report next to PDF
        QString reportPath = QFileInfo(outputPath).absolutePath() + "/" + QFileInfo(outputPath).completeBaseName() + "_export_report.json";
        QFile rep(reportPath);
        if (rep.open(QIODevice::WriteOnly)) {
            rep.write(QJsonDocument(report).toJson(QJsonDocument::Indented));
            rep.close();
            qDebug() << "Wrote export report to" << reportPath;
        }

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
