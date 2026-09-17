#pragma once

#include <QQuickImageProvider>
#include "QFontAwesome.h"

/// Image provider for QML: engine.addImageProvider("fa", new QFontAwesomeQuickImageProvider());
/// URLs look like image://fa/solid/user or image://fa/regular/thumbs-up?color=ff0000
class QFontAwesomeQuickImageProvider : public QQuickImageProvider
{
public:
    QFontAwesomeQuickImageProvider()
      : QQuickImageProvider(QQuickImageProvider::ImageType::Pixmap)
    {}

    /// Kept for source compatibility; the instance argument is ignored (QFontAwesome is a singleton).
    explicit QFontAwesomeQuickImageProvider(fa::QFontAwesome*)
      : QFontAwesomeQuickImageProvider()
    {}

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override {
      return fa::QFontAwesome::requestPixmap(id, size, requestedSize);
    }
};
