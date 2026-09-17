#ifndef QFONTAWESOMEANIMATION_H
#define QFONTAWESOMEANIMATION_H

#include <QObject>

class QPainter;
class QRect;
class QTimer;
class QWidget;

namespace fa {
    ///
    /// Basic Animation Support for QFontAwesome (Inspired by https://github.com/spyder-ide/qfontawesome)
    ///
    class QFontAwesomeAnimation : public QObject
    {
    Q_OBJECT

    public:
        QFontAwesomeAnimation(QWidget* parentWidget, int interval = 10, int step = 1);
        void setup(QPainter& painter, const QRect& rect);

    public slots:
        void update();

    private:
        QWidget* parentWidgetRef_;
        QTimer* timer_;
        int interval_;
        int step_;
        float angle_;
    };
} // namespace fa

#endif // QFONTAWESOMEANIMATION_H
