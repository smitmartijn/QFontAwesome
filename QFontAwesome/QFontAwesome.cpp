/**
 * QFontAwesome - use font-awesome (or other font icons) in your c++ / Qt Application
 */

#include "QFontAwesome.h"
#include "QFontAwesomeAnim.h"

#include <QApplication>
#include <QPalette>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFontDatabase>
#include <QFontMetrics>
#include <QPainterPath>
#include <QPixmapCache>
#include <QRegularExpression>
#include <QSet>
#include <QString>
#include <QThread>
#include <QTransform>
#include <QUrlQuery>
#include <QtEndian>

#include <algorithm>
#include <utility>


#if (QT_VERSION >= QT_VERSION_CHECK(6, 5, 0))
#define USE_COLOR_SCHEME
#include <QStyleHints>
#endif

namespace fa {

#include "QFontAwesomeStringGenerated.h"

QFontAwesome* QFontAwesome::s_instance = nullptr;

//---------------------------------------------------------------------------------------
// Font registry
//---------------------------------------------------------------------------------------

namespace {

struct FontRegistryEntry {
    int style;
    const char* styleName;
    const char* family;
    int weight;
    const char* fileName;
    QFontAwesomeFontInfo::Tier tier;
};

/// Every Font Awesome 7 font file QFontAwesome can use. A style can be provided by more than one
/// font (the Pro Solid font also covers fa-solid); the first entry of a style holds its canonical name.
const FontRegistryEntry fontRegistry[] = {
    { fa_solid,   "fa-solid",   "Font Awesome 7 Free",   900, "Font Awesome 7 Free-Solid-900.otf",     QFontAwesomeFontInfo::Free },
    { fa_regular, "fa-regular", "Font Awesome 7 Free",   400, "Font Awesome 7 Free-Regular-400.otf",   QFontAwesomeFontInfo::Free },
    { fa_brands,  "fa-brands",  "Font Awesome 7 Brands", 400, "Font Awesome 7 Brands-Regular-400.otf", QFontAwesomeFontInfo::Free },

    { fa_solid,   "fa-solid",   "Font Awesome 7 Pro", 900, "Font Awesome 7 Pro-Solid-900.otf",   QFontAwesomeFontInfo::Pro },
    { fa_regular, "fa-regular", "Font Awesome 7 Pro", 400, "Font Awesome 7 Pro-Regular-400.otf", QFontAwesomeFontInfo::Pro },
    { fa_light,   "fa-light",   "Font Awesome 7 Pro", 300, "Font Awesome 7 Pro-Light-300.otf",   QFontAwesomeFontInfo::Pro },
    { fa_thin,    "fa-thin",    "Font Awesome 7 Pro", 100, "Font Awesome 7 Pro-Thin-100.otf",    QFontAwesomeFontInfo::Pro },

    { fa_duotone_solid,   "fa-duotone-solid",   "Font Awesome 7 Duotone", 900, "Font Awesome 7 Duotone-Solid-900.otf",   QFontAwesomeFontInfo::Pro },
    { fa_duotone_regular, "fa-duotone-regular", "Font Awesome 7 Duotone", 400, "Font Awesome 7 Duotone-Regular-400.otf", QFontAwesomeFontInfo::Pro },
    { fa_duotone_light,   "fa-duotone-light",   "Font Awesome 7 Duotone", 300, "Font Awesome 7 Duotone-Light-300.otf",   QFontAwesomeFontInfo::Pro },
    { fa_duotone_thin,    "fa-duotone-thin",    "Font Awesome 7 Duotone", 100, "Font Awesome 7 Duotone-Thin-100.otf",    QFontAwesomeFontInfo::Pro },

    { fa_sharp_solid,   "fa-sharp-solid",   "Font Awesome 7 Sharp", 900, "Font Awesome 7 Sharp-Solid-900.otf",   QFontAwesomeFontInfo::Pro },
    { fa_sharp_regular, "fa-sharp-regular", "Font Awesome 7 Sharp", 400, "Font Awesome 7 Sharp-Regular-400.otf", QFontAwesomeFontInfo::Pro },
    { fa_sharp_light,   "fa-sharp-light",   "Font Awesome 7 Sharp", 300, "Font Awesome 7 Sharp-Light-300.otf",   QFontAwesomeFontInfo::Pro },
    { fa_sharp_thin,    "fa-sharp-thin",    "Font Awesome 7 Sharp", 100, "Font Awesome 7 Sharp-Thin-100.otf",    QFontAwesomeFontInfo::Pro },

    { fa_sharp_duotone_solid,   "fa-sharp-duotone-solid",   "Font Awesome 7 Sharp Duotone", 900, "Font Awesome 7 Sharp Duotone-Solid-900.otf",   QFontAwesomeFontInfo::Pro },
    { fa_sharp_duotone_regular, "fa-sharp-duotone-regular", "Font Awesome 7 Sharp Duotone", 400, "Font Awesome 7 Sharp Duotone-Regular-400.otf", QFontAwesomeFontInfo::Pro },
    { fa_sharp_duotone_light,   "fa-sharp-duotone-light",   "Font Awesome 7 Sharp Duotone", 300, "Font Awesome 7 Sharp Duotone-Light-300.otf",   QFontAwesomeFontInfo::Pro },
    { fa_sharp_duotone_thin,    "fa-sharp-duotone-thin",    "Font Awesome 7 Sharp Duotone", 100, "Font Awesome 7 Sharp Duotone-Thin-100.otf",    QFontAwesomeFontInfo::Pro },

    { fa_chisel_regular,        "fa-chisel",      "Font Awesome 7 Chisel",      400, "Font Awesome 7 Chisel-Regular-400.otf",      QFontAwesomeFontInfo::ProPlus },
    { fa_etch_solid,            "fa-etch",        "Font Awesome 7 Etch",        900, "Font Awesome 7 Etch-Solid-900.otf",          QFontAwesomeFontInfo::ProPlus },
    { fa_jelly_regular,         "fa-jelly",       "Font Awesome 7 Jelly",       400, "Font Awesome 7 Jelly-Regular-400.otf",       QFontAwesomeFontInfo::ProPlus },
    { fa_jelly_duotone_regular, "fa-jelly-duo",   "Font Awesome 7 Jelly Duo",   400, "Font Awesome 7 Jelly Duo-Regular-400.otf",   QFontAwesomeFontInfo::ProPlus },
    { fa_jelly_fill_regular,    "fa-jelly-fill",  "Font Awesome 7 Jelly Fill",  400, "Font Awesome 7 Jelly Fill-Regular-400.otf",  QFontAwesomeFontInfo::ProPlus },
    { fa_notdog_duotone_solid,  "fa-notdog-duo",  "Font Awesome 7 Notdog Duo",  900, "Font Awesome 7 Notdog Duo-Solid-900.otf",    QFontAwesomeFontInfo::ProPlus },
    { fa_notdog_solid,          "fa-notdog",      "Font Awesome 7 Notdog",      900, "Font Awesome 7 Notdog-Solid-900.otf",        QFontAwesomeFontInfo::ProPlus },
    { fa_slab_press_regular,    "fa-slab-press",  "Font Awesome 7 Slab Press",  400, "Font Awesome 7 Slab Press-Regular-400.otf",  QFontAwesomeFontInfo::ProPlus },
    { fa_slab_regular,          "fa-slab",        "Font Awesome 7 Slab",        400, "Font Awesome 7 Slab-Regular-400.otf",        QFontAwesomeFontInfo::ProPlus },
    { fa_thumbprint_light,      "fa-thumbprint",  "Font Awesome 7 Thumbprint",  300, "Font Awesome 7 Thumbprint-Light-300.otf",    QFontAwesomeFontInfo::ProPlus },
    { fa_whiteboard_semibold,   "fa-whiteboard",  "Font Awesome 7 Whiteboard",  600, "Font Awesome 7 Whiteboard-Semibold-600.otf", QFontAwesomeFontInfo::ProPlus },
};

const FontRegistryEntry* firstRegistryEntry(int style)
{
    for (const FontRegistryEntry& entry : fontRegistry) {
        if (entry.style == style) {
            return &entry;
        }
    }
    return nullptr;
}

/// The family name and weight class read from an OpenType / TrueType file.
struct FontMetadata {
    QString family;
    int weight = 0;
    QString faVersion; ///< "7.0.0" from the "Font Awesome version: 7.0.0" version string, when present
};

/// Reads the typographic family (name ID 16, falling back to 1) and the OS/2 weight class straight from
/// the font data. Qt cannot be used for this: the family returned by QFontDatabase depends on the
/// platform font database ("Font Awesome 7 Free" vs "Font Awesome 7 Free Solid") and it has no weight
/// lookup per application font.
bool readFontMetadata(const QByteArray& data, FontMetadata* meta)
{
    const quint64 size = static_cast<quint64>(data.size());
    const uchar* p = reinterpret_cast<const uchar*>(data.constData());
    auto u16 = [p](quint64 offset) { return qFromBigEndian<quint16>(p + offset); };
    auto u32 = [p](quint64 offset) { return qFromBigEndian<quint32>(p + offset); };

    if (size < 12) {
        return false;
    }
    const quint32 version = u32(0);
    if (version != 0x00010000 && version != 0x4F54544F /* OTTO */ && version != 0x74727565 /* true */) {
        return false;
    }

    const quint64 numTables = u16(4);
    if (12 + numTables * 16 > size) {
        return false;
    }

    quint64 nameOffset = 0, nameLength = 0;
    for (quint64 i = 0; i < numTables; ++i) {
        const quint64 record = 12 + i * 16;
        const quint32 tag = u32(record);
        const quint64 offset = u32(record + 8);
        const quint64 length = u32(record + 12);
        if (offset + length > size) {
            return false;
        }
        if (tag == 0x4F532F32 /* OS/2 */ && length >= 6) {
            meta->weight = u16(offset + 4);
        } else if (tag == 0x6E616D65 /* name */) {
            nameOffset = offset;
            nameLength = length;
        }
    }

    if (nameLength < 6) {
        return false;
    }
    const quint64 count = u16(nameOffset + 2);
    const quint64 stringsOffset = nameOffset + u16(nameOffset + 4);
    if (6 + count * 12 > nameLength) {
        return false;
    }

    int bestScore = 0;
    int bestVersionScore = 0;
    for (quint64 i = 0; i < count; ++i) {
        const quint64 record = nameOffset + 6 + i * 12;
        const quint16 platformId = u16(record);
        const quint16 encodingId = u16(record + 2);
        const quint16 languageId = u16(record + 4);
        const quint16 nameId = u16(record + 6);
        const quint64 length = u16(record + 8);
        const quint64 offset = stringsOffset + u16(record + 10);
        if ((nameId != 16 && nameId != 1 && nameId != 5) || offset + length > nameOffset + nameLength) {
            continue;
        }

        QString value;
        int score = nameId == 16 ? 10 : 0;
        if (nameId == 5) score = 0;
        if (platformId == 3 || platformId == 0) { // Windows / Unicode: UTF-16BE
            value.reserve(static_cast<int>(length / 2));
            for (quint64 c = 0; c + 1 < length; c += 2) {
                value.append(QChar(u16(offset + c)));
            }
            score += (platformId == 3 && languageId == 0x0409) ? 3 : 2;
        } else if (platformId == 1 && encodingId == 0) { // Macintosh Roman, ASCII for these names
            value = QString::fromLatin1(reinterpret_cast<const char*>(p + offset), static_cast<int>(length));
            score += 1;
        } else {
            continue;
        }
        if (nameId == 5) {
            // e.g. "Version 896.00390625 (Font Awesome version: 7.0.0)"
            static const QRegularExpression versionPattern(QStringLiteral("Font Awesome version: ([0-9][0-9A-Za-z.\\-]*)"));
            const QRegularExpressionMatch match = versionPattern.match(value);
            if (match.hasMatch() && score > bestVersionScore) {
                bestVersionScore = score;
                meta->faVersion = match.captured(1);
            }
        } else if (score > bestScore && !value.isEmpty()) {
            bestScore = score;
            meta->family = value;
        }
    }

    return !meta->family.isEmpty() && meta->weight > 0;
}

/// Finds the style for font metadata. An exact family match wins; otherwise a legacy family name that
/// includes the style ("Font Awesome 7 Free Solid") matches the longest registry family it starts with.
int styleForMetadata(const FontMetadata& meta)
{
    const FontRegistryEntry* best = nullptr;
    for (const FontRegistryEntry& entry : fontRegistry) {
        if (entry.weight != meta.weight) {
            continue;
        }
        const QString family = QLatin1String(entry.family);
        if (meta.family.compare(family, Qt::CaseInsensitive) == 0) {
            return entry.style;
        }
        if (meta.family.startsWith(family + QLatin1Char(' '), Qt::CaseInsensitive)
                && (!best || qstrlen(entry.family) > qstrlen(best->family))) {
            best = &entry;
        }
    }
    return best ? best->style : -1;
}

int styleForFileName(const QString& origin)
{
    const QString fileName = QFileInfo(origin).fileName();
    for (const FontRegistryEntry& entry : fontRegistry) {
        if (fileName.compare(QLatin1String(entry.fileName), Qt::CaseInsensitive) == 0) {
            return entry.style;
        }
    }
    return -1;
}

QFont::Weight toQtWeight(int weight)
{
    return static_cast<QFont::Weight>(qBound(100, weight, 900));
}

/// Styles whose fonts carry a secondary layer at codepoint | DUOTONE_HEX_ICON_VALUE.
bool isDuotoneStyle(int style)
{
    switch (style) {
        case fa_duotone_solid:
        case fa_duotone_regular:
        case fa_duotone_light:
        case fa_duotone_thin:
        case fa_sharp_duotone_solid:
        case fa_sharp_duotone_regular:
        case fa_sharp_duotone_light:
        case fa_sharp_duotone_thin:
        case fa_jelly_duotone_regular:
        case fa_notdog_duotone_solid:
            return true;
        default:
            return false;
    }
}

/// Bumped whenever the set of loaded fonts changes; part of every pixmap cache key.
int s_fontGeneration = 0;

/// Styles already reported as painted without a loaded font.
QSet<int>& warnedUnloadedStyles()
{
    static QSet<int> styles;
    return styles;
}

} // namespace

QFontAwesomeIconPainter::~QFontAwesomeIconPainter()
{
}

// internal helper: the option keys to test for a mode/state, most specific first
static QStringList optionKeysForModeAndState(const QString& key, QIcon::Mode mode, QIcon::State state)
{
    QString modePostfix;
    switch (mode) {
        case QIcon::Disabled:
            modePostfix = QStringLiteral("-disabled");
            break;
        case QIcon::Active:
            modePostfix = QStringLiteral("-active");
            break;
        case QIcon::Selected:
            modePostfix = QStringLiteral("-selected");
            break;
        default:
            break;
    }

    QString statePostfix;
    if (state == QIcon::Off) {
        statePostfix = QStringLiteral("-off");
    }

    // the keys that need to bet tested:   key-mode-state | key-mode | key-state | key
    QStringList result;
    if (!modePostfix.isEmpty()) {
        if (!statePostfix.isEmpty()) {
            result.push_back(key + modePostfix + statePostfix);
        }
        result.push_back(key + modePostfix);
    }
    if (!statePostfix.isEmpty()) {
        result.push_back(key + statePostfix);
    }
    return result;
}

static QVariant optionValueForModeAndState(const QString& baseKey, QIcon::Mode mode, QIcon::State state,
                                           const QVariantMap& options)
{
    const QStringList keys = optionKeysForModeAndState(baseKey, mode, state);
    for (const QString& key : keys) {
        if (options.contains(key) && !(options.value(key).toString().isEmpty())) {
            return options.value(key);
        }
    }
    return options.value(baseKey);
}

// internal helper: the colour for a mode/state, falling back to the application palette
static QColor resolveColor(const QString& baseKey, QIcon::Mode mode, QIcon::State state, const QVariantMap& options)
{
    QColor color = optionValueForModeAndState(baseKey, mode, state, options).value<QColor>();
    if (!color.isValid()) {
        color = QFontAwesome::paletteColor(mode, state);
    }
    return color;
}

// internal helper: the secondary duotone colour, defaulting to the primary colour at 40% opacity like Font Awesome
static QColor resolveDuotoneColor(const QColor& primary, QIcon::Mode mode, QIcon::State state, const QVariantMap& options)
{
    QColor color = optionValueForModeAndState(QStringLiteral("duotone-color"), mode, state, options).value<QColor>();
    if (!color.isValid()) {
        color = primary;
        color.setAlphaF(primary.alphaF() * 0.4f);
    }
    return color;
}

/// Breathing room left on every side of the glyph's ink, in pixels of the rectangle being painted.
/// A glyph that reaches the edge loses its antialiased outline, and the font engine can snap the
/// glyph origin vertically by a fraction of a pixel while rasterising, so a whole pixel is reserved
/// rather than a percentage: a percentage is sub-pixel at icon sizes and disappears exactly where
/// it is needed most.
static constexpr qreal kInkMargin = 1.0;

/// The font-awesome icon painter
class QFontAwesomeCharIconPainter: public QFontAwesomeIconPainter
{
public:
    virtual ~QFontAwesomeCharIconPainter()
    {
    }

    virtual void paint(QFontAwesome* awesome, QPainter* painter, const QRect& rect, QIcon::Mode mode, QIcon::State state,
                        const QVariantMap& options)
    {
        Q_UNUSED(awesome);

        const QColor color = resolveColor(QStringLiteral("color"), mode, state, options);
        QString text = optionValueForModeAndState(QStringLiteral("text"), mode, state, options).toString();
        const int st = optionValueForModeAndState(QStringLiteral("style"), mode, state, options).toInt();

        if (text.isEmpty()) {
            qWarning() << "QFontAwesome: icon without a glyph (unknown name or codepoint 0)";
            return;
        }
        if (!QFontAwesome::isFontLoaded(st)) {
            if (!warnedUnloadedStyles().contains(st)) {
                warnedUnloadedStyles().insert(st);
                qWarning() << "QFontAwesome: no font loaded for style" << QFontAwesome::styleEnumToString(st)
                           << "- load it with QFontAwesome::loadFonts() / loadFont()";
            }
            return;
        }

        painter->save();

        painter->setRenderHint(QPainter::Antialiasing);

        QVariant var = options.value(QStringLiteral("anim"));
        QFontAwesomeAnimation* anim = var.value<QFontAwesomeAnimation*>();
        if (anim) {
            anim->setup(*painter, rect);
        }

        const QRectF textRect(rect);

        // The secondary duotone layer is a supplementary-plane character, so it needs a surrogate
        // pair. It is a glyph of its own: it can be larger than the primary layer, and the two are
        // designed to overlay, so they are measured together and share a single transform.
        QString secondaryText;
        if (isDuotoneStyle(st)) {
            const char32_t secondaryCp = text.at(0).unicode() | QFontAwesome::DUOTONE_HEX_ICON_VALUE;
            secondaryText = QString::fromUcs4(&secondaryCp, 1);
        }

        // Draw the glyph as an outline instead of as text. Text drawing sizes the glyph from an
        // integer pixel size and puts it on a baseline the font engine rounds to whole pixels,
        // and the metrics that describe it - tightBoundingRect() included - do not match the
        // rasterised outline closely enough to compensate for: at icon sizes that error is about a
        // whole pixel, which is exactly the clipped edge. A path carries the real outline, so its
        // bounding rectangle is the geometry that gets filled, and both the fit and the centring
        // are exact at any fractional scale. Glyph outlines rely on non-zero winding, so the fill
        // rule is set explicitly rather than left at QPainterPath's odd-even default.
        const int drawSize = qRound(textRect.height() * options.value(QStringLiteral("scale-factor"), 1.0).toDouble());
        const QFont ft = QFontAwesome::font(st, qMax(1, drawSize));

        QPainterPath primaryPath;
        primaryPath.setFillRule(Qt::WindingFill);
        primaryPath.addText(QPointF(0.0, 0.0), ft, text);

        QPainterPath secondaryPath;
        if (!secondaryText.isEmpty()) {
            secondaryPath.setFillRule(Qt::WindingFill);
            secondaryPath.addText(QPointF(0.0, 0.0), ft, secondaryText);
        }

        QRectF bounds = primaryPath.boundingRect();
        if (!secondaryPath.isEmpty()) {
            bounds |= secondaryPath.boundingRect();
        }

        if (!bounds.isEmpty()) {
            // Scale the outline into the rectangle less kInkMargin on every side, then centre it on
            // its own bounds. This only ever shrinks: a glyph with room to spare keeps its size.
            const QRectF fitRect = textRect.adjusted(kInkMargin, kInkMargin, -kInkMargin, -kInkMargin);
            const qreal scale = qMin(qMin(fitRect.width() / bounds.width(),
                                          fitRect.height() / bounds.height()), 1.0);

            QTransform transform = QTransform::fromScale(scale, scale);
            const QRectF scaledBounds = transform.mapRect(bounds);
            transform *= QTransform::fromTranslate(textRect.center().x() - scaledBounds.center().x(),
                                                   textRect.center().y() - scaledBounds.center().y());

            painter->fillPath(transform.map(primaryPath), color);

            if (!secondaryPath.isEmpty()) {
                painter->fillPath(transform.map(secondaryPath),
                                  resolveDuotoneColor(color, mode, state, options));
            }
        }

        painter->restore();
    }

    /// A cache key that fully describes the rendering for the given mode/state, or an empty
    /// string when the icon must not be cached (animated icons).
    QString cacheKey(const QSize& size, QIcon::Mode mode, QIcon::State state, const QVariantMap& options)
    {
        if (options.contains(QStringLiteral("anim"))) {
            return QString();
        }
        const QColor color = resolveColor(QStringLiteral("color"), mode, state, options);
        const QString text = optionValueForModeAndState(QStringLiteral("text"), mode, state, options).toString();
        const int st = optionValueForModeAndState(QStringLiteral("style"), mode, state, options).toInt();
        QString key = QStringLiteral("qfa:%1:%2:%3:%4x%5:%6:%7")
                .arg(s_fontGeneration).arg(st).arg(text).arg(size.width()).arg(size.height())
                .arg(color.rgba(), 8, 16, QLatin1Char('0'))
                .arg(options.value(QStringLiteral("scale-factor"), 1.0).toDouble());
        if (isDuotoneStyle(st)) {
            key += QStringLiteral(":%1").arg(resolveDuotoneColor(color, mode, state, options).rgba(), 8, 16, QLatin1Char('0'));
        }
        return key;
    }
};

//---------------------------------------------------------------------------------------

/// The painter icon engine.
class QFontAwesomeIconPainterIconEngine : public QIconEngine
{

public:

    /// owner keeps a painter registered with give() alive for as long as this icon exists; it is null for
    /// the built-in glyph painter and for painters the caller owns.
    QFontAwesomeIconPainterIconEngine(QFontAwesomeIconPainter* painter, const QSharedPointer<QFontAwesomeIconPainter>& owner,
                                      const QVariantMap& options, QFontAwesomeCharIconPainter* cachingPainter)
        : iconPainterRef_(painter)
        , owner_(owner)
        , cachingPainterRef_(cachingPainter)
        , options_(options)
    {
    }

    virtual ~QFontAwesomeIconPainterIconEngine(){}

    QFontAwesomeIconPainterIconEngine* clone() const override
    {
        return new QFontAwesomeIconPainterIconEngine(iconPainterRef_, owner_, options_, cachingPainterRef_);
    }

    void paint(QPainter* painter, const QRect& rect, QIcon::Mode mode, QIcon::State state) override
    {
        iconPainterRef_->paint(QFontAwesome::instance(), painter, rect, mode, state, options_);
    }

    QPixmap pixmap(const QSize& size, QIcon::Mode mode, QIcon::State state) override
    {
        // The built-in glyph painter is cached in the global QPixmapCache: widgets call
        // QIcon::pixmap() on every repaint and re-rendering the glyph each time is wasteful.
        QString key;
        if (cachingPainterRef_) {
            key = cachingPainterRef_->cacheKey(size, mode, state, options_);
            QPixmap cached;
            if (!key.isEmpty() && QPixmapCache::find(key, &cached)) {
                return cached;
            }
        }

        QPixmap pm(size);
        pm.fill(Qt::transparent); // we need transparency
        {
            QPainter p(&pm);
            paint(&p, QRect(QPoint(0, 0), size), mode, state);
        }

        if (!key.isEmpty()) {
            QPixmapCache::insert(key, pm);
        }
        return pm;
    }

    /// HiDPI entry point used by QIcon::pixmap(size, devicePixelRatio). Since Qt 6.8 the size
    /// argument is in device-independent pixels; earlier versions already multiplied it by scale.
    QPixmap scaledPixmap(const QSize& size, QIcon::Mode mode, QIcon::State state, qreal scale) override
    {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 8, 0))
        const QSize deviceSize = size * scale;
#else
        const QSize deviceSize = size;
#endif
        QPixmap pm = pixmap(deviceSize, mode, state);
        pm.setDevicePixelRatio(scale);
        return pm;
    }

    QString key() const override
    {
        return QStringLiteral("QFontAwesome");
    }

    QString iconName() override
    {
        return options_.value(QStringLiteral("name")).toString();
    }

    bool isNull() override
    {
        return false;
    }

private:

    QFontAwesomeIconPainter* iconPainterRef_;        ///< a reference to the icon painter
    QSharedPointer<QFontAwesomeIconPainter> owner_;  ///< keeps a registered (give()) painter alive
    QFontAwesomeCharIconPainter* cachingPainterRef_; ///< non-null when the painter is the cacheable glyph painter
    QVariantMap options_;                            ///< the options for this icon painter
};

/// The built-in glyph painter. It is stateless and lives for the whole process, so icons never
/// reference a destroyed painter, not even after QFontAwesome::shutdown().
static QFontAwesomeCharIconPainter* glyphPainter()
{
    static QFontAwesomeCharIconPainter painter;
    return &painter;
}

//---------------------------------------------------------------------------------------
// Global instance
//---------------------------------------------------------------------------------------

void QFontAwesome::init()
{
    if (s_instance) {
        return;
    }

    Q_ASSERT_X(qApp, "QFontAwesome::init", "QApplication must be constructed before QFontAwesome is used");
    Q_ASSERT_X(!qApp || QThread::currentThread() == qApp->thread(), "QFontAwesome::init",
               "QFontAwesome must be initialized on the GUI thread");

    s_instance = new QFontAwesome(qApp);
    QObject::connect(s_instance, &QObject::destroyed, [](){ s_instance = nullptr; });
}

bool QFontAwesome::isInitialized()
{
    return s_instance != nullptr;
}

QFontAwesome* QFontAwesome::instance()
{
    if (!s_instance) {
        init();
    }
    return s_instance;
}

void QFontAwesome::shutdown()
{
    delete s_instance; // the destroyed() connection resets s_instance
    s_instance = nullptr;
}

//---------------------------------------------------------------------------------------
// Construction
//---------------------------------------------------------------------------------------

QFontAwesome::QFontAwesome(QObject* parent)
    : QObject(parent)
    , _namedCodepointsByStyle()
    , _namedCodepointsList()
{
    resetDefaultOptions();

    buildNamedCodePoints();

#ifdef USE_COLOR_SCHEME
    // Colours are resolved from the palette at paint time, so an existing icon follows a
    // dark/light switch by itself. Cached pixmaps carry the colour in their key and are
    // therefore automatically invalidated too. Signal listeners so they can repaint.
    QObject::connect(QApplication::styleHints(), &QStyleHints::colorSchemeChanged, this, [this](Qt::ColorScheme colorScheme){
        Q_UNUSED(colorScheme);
        Q_EMIT defaultOptionsReset();
    });
#endif
}

QFontAwesome::~QFontAwesome()
{
    for (const LoadedFont& font : std::as_const(_loadedFonts)) {
        QFontDatabase::removeApplicationFont(font.fontId);
    }
    ++s_fontGeneration;

    qDeleteAll(_namedCodepointsList);
}

/// Resets the default options. Colour options are intentionally left unset: the painter
/// resolves them from QApplication::palette() at paint time (see paletteColor()).
void QFontAwesome::resetDefaultOptions()
{
    _defaultOptions.clear();

    _defaultOptions.insert(QStringLiteral("scale-factor"), 1.0);
    _defaultOptions.insert(QStringLiteral("text"), QVariant());
    _defaultOptions.insert(QStringLiteral("text-disabled"), QVariant());
    _defaultOptions.insert(QStringLiteral("text-active"), QVariant());
    _defaultOptions.insert(QStringLiteral("text-selected"), QVariant());

    Q_EMIT defaultOptionsReset();
}

QColor QFontAwesome::paletteColor(QIcon::Mode mode, QIcon::State state)
{
    Q_UNUSED(state);
    const QPalette palette = QApplication::palette();
    switch (mode) {
        case QIcon::Disabled: return palette.color(QPalette::Disabled, QPalette::Text);
        case QIcon::Active:   return palette.color(QPalette::Active, QPalette::Text);
        case QIcon::Selected: return palette.color(QPalette::Active, QPalette::Text);
        default:              return palette.color(QPalette::Normal, QPalette::Text);
    }
}

/// Builds the name -> codepoint maps. Every non-brand style shares the common table (plus the pro
/// table when compiled in); in a free build fa-regular only lists the icons of the free regular font.
void QFontAwesome::buildNamedCodePoints()
{
    addToNamedCodePoints(fa::fa_brands, faBrandsIconArray, sizeof(faBrandsIconArray)/sizeof(QFontAwesomeNamedIcon));
    addToNamedCodePoints(fa::fa_solid, faCommonIconArray, sizeof(faCommonIconArray)/sizeof(QFontAwesomeNamedIcon));

#ifdef FONT_AWESOME_PRO
    addToNamedCodePoints(fa::fa_solid, faProIconArray, sizeof(faProIconArray)/sizeof(QFontAwesomeNamedIcon));
#else
    addToNamedCodePoints(fa::fa_regular, faRegularFreeIconArray, sizeof(faRegularFreeIconArray)/sizeof(QFontAwesomeNamedIcon));
#endif

    QHash<QString, int>* common = _namedCodepointsByStyle.value(fa::fa_solid);
    for (const FontRegistryEntry& entry : fontRegistry) {
        if (!_namedCodepointsByStyle.contains(entry.style)) {
            _namedCodepointsByStyle.insert(entry.style, common);
        }
    }
}

/// Add the given array as named codepoints
void QFontAwesome::addToNamedCodePoints(int style, const QFontAwesomeNamedIcon *QFontAwesomeNamedIcons, int size)
{
    QHash<QString, int> *namedCodepoints = _namedCodepointsByStyle.value(style, nullptr);
    if (namedCodepoints == nullptr) {
        namedCodepoints = new QHash<QString, int>();
        _namedCodepointsList.append(namedCodepoints);
        _namedCodepointsByStyle.insert(style, namedCodepoints);
    }

    for (int i = 0; i < size; ++i) {
        namedCodepoints->insert(QFontAwesomeNamedIcons[i].name, QFontAwesomeNamedIcons[i].icon);
    }
}

//---------------------------------------------------------------------------------------
// Font loading
//---------------------------------------------------------------------------------------

/// Registers font data for a style in QFontDatabase. The caller has resolved the style.
bool QFontAwesome::registerFont(int style, const QByteArray& data, const QString& origin)
{
    if (_loadedFonts.contains(style)) {
        return true;
    }

    const int fontId = QFontDatabase::addApplicationFontFromData(data);
    if (fontId < 0) {
        qWarning() << "QFontAwesome: font" << origin << "could not be registered";
        return false;
    }
    const QStringList families = QFontDatabase::applicationFontFamilies(fontId);
    if (families.isEmpty()) {
        qWarning() << "QFontAwesome: font" << origin << "registered no font families";
        QFontDatabase::removeApplicationFont(fontId);
        return false;
    }

    // An explicitly styled font may carry any weight; use the one stored in the font when readable.
    FontMetadata meta;
    const FontRegistryEntry* entry = firstRegistryEntry(style);
    const int weight = readFontMetadata(data, &meta) ? meta.weight : entry->weight;

    // Codepoint constants and names come from generated headers; a font of another version may lack or move icons.
    if (!meta.faVersion.isEmpty() && meta.faVersion != QLatin1String(QFONTAWESOME_FA_VERSION)) {
        qWarning().nospace().noquote() << "QFontAwesome: font \"" << origin << "\" is Font Awesome " << meta.faVersion
                                       << " but the generated headers are for " << QFONTAWESOME_FA_VERSION
                                       << "; re-run build_headers.py";
    }

    LoadedFont font;
    font.fontId = fontId;
    font.family = families.at(0);
    font.weight = weight;
    _loadedFonts.insert(style, font);
    ++s_fontGeneration;
    return true;
}

bool QFontAwesome::loadFont(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "QFontAwesome: font" << path << "could not be opened";
        return false;
    }
    return loadFontData(file.readAll(), path);
}

bool QFontAwesome::loadFont(int style, const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "QFontAwesome: font" << path << "could not be opened";
        return false;
    }
    return loadFontData(style, file.readAll(), path);
}

bool QFontAwesome::loadFontData(const QByteArray& data, const QString& origin)
{
    FontMetadata meta;
    int style = -1;
    if (readFontMetadata(data, &meta)) {
        style = styleForMetadata(meta);
    }
    if (style < 0) {
        style = styleForFileName(origin);
    }
    if (style < 0) {
        qWarning().nospace().noquote() << "QFontAwesome: font \"" << origin << "\" (family \"" << meta.family
                                       << "\", weight " << meta.weight
                                       << ") is not a known Font Awesome font; use loadFont(style, path) to force a style";
        return false;
    }
    return instance()->registerFont(style, data, origin);
}

bool QFontAwesome::loadFontData(int style, const QByteArray& data, const QString& origin)
{
    if (!firstRegistryEntry(style)) {
        qWarning() << "QFontAwesome: unknown style" << style << "for font" << origin;
        return false;
    }
    return instance()->registerFont(style, data, origin);
}

int QFontAwesome::loadFonts(const QString& directory)
{
    QDir dir(directory);
    const QStringList files = dir.entryList({ QStringLiteral("*.otf"), QStringLiteral("*.ttf") },
                                            QDir::Files | QDir::Readable, QDir::Name);
    if (files.isEmpty()) {
        qWarning() << "QFontAwesome: no font files found in" << directory;
        return 0;
    }

    int loaded = 0;
    for (const QString& file : files) {
        if (loadFont(dir.filePath(file))) {
            ++loaded;
        }
    }
    return loaded;
}

bool QFontAwesome::isFontLoaded(int style)
{
    return instance()->_loadedFonts.contains(style);
}

QList<int> QFontAwesome::loadedStyles()
{
    QList<int> styles = instance()->_loadedFonts.keys();
    std::sort(styles.begin(), styles.end());
    return styles;
}

QList<QFontAwesomeFontInfo> QFontAwesome::knownFonts()
{
    const QFontAwesome* self = instance();
    QList<QFontAwesomeFontInfo> result;
    for (const FontRegistryEntry& entry : fontRegistry) {
        QFontAwesomeFontInfo info;
        info.style = entry.style;
        info.styleName = QLatin1String(entry.styleName);
        info.family = QLatin1String(entry.family);
        info.weight = entry.weight;
        info.fileName = QLatin1String(entry.fileName);
        info.tier = entry.tier;
        info.loaded = self->_loadedFonts.contains(entry.style);
        result.append(info);
    }
    return result;
}

//---------------------------------------------------------------------------------------
// Static facade
//---------------------------------------------------------------------------------------

const QHash<QString, int> QFontAwesome::namedCodePoints(int style)
{
    QFontAwesome* self = instance();
    if (!self->_namedCodepointsByStyle.contains(style)) return QHash<QString, int>();
    return *self->_namedCodepointsByStyle[style];
}

void QFontAwesome::setDefaultOption(const QString& name, const QVariant& value)
{
    instance()->_defaultOptions.insert(name, value);
}

QVariant QFontAwesome::defaultOption(const QString& name)
{
    return instance()->_defaultOptions.value(name);
}

QIcon QFontAwesome::icon(int style, int character, const QVariantMap& options)
{
    return instance()->createIcon(style, character, options);
}

QIcon QFontAwesome::icon(int style, int character, const QColor& color)
{
    QVariantMap options;
    options.insert(QStringLiteral("color"), color);
    return icon(style, character, options);
}

QIcon QFontAwesome::icon(const QString& name, const QVariantMap& options)
{
    return instance()->createIcon(name, options);
}

QIcon QFontAwesome::icon(const QString& name, const QColor& color)
{
    QVariantMap options;
    options.insert(QStringLiteral("color"), color);
    return icon(name, options);
}

QIcon QFontAwesome::icon(QFontAwesomeIconPainter* painter, const QVariantMap& optionMap)
{
    return instance()->createIcon(painter, optionMap);
}

bool QFontAwesome::hasIcon(const QString& name)
{
    const QFontAwesome* self = instance();
    int style = fa::fa_solid;
    QString iconName;
    return self->resolveName(name, &style, &iconName) || self->_painterMap.contains(name);
}

QPixmap QFontAwesome::pixmap(const QIcon& icon, const QSize& size, qreal devicePixelRatio, QIcon::Mode mode, QIcon::State state)
{
    if (icon.isNull() || size.isEmpty()) return QPixmap();
    const qreal dpr = devicePixelRatio > 0.0 ? devicePixelRatio : (qApp ? qApp->devicePixelRatio() : 1.0);
    return icon.pixmap(size, dpr, mode, state);
}

QPixmap QFontAwesome::pixmap(int style, int character, const QSize& size, const QVariantMap& options, qreal devicePixelRatio)
{
    return pixmap(icon(style, character, options), size, devicePixelRatio);
}

QPixmap QFontAwesome::pixmap(int style, int character, int size, const QColor& color, qreal devicePixelRatio)
{
    QVariantMap options;
    if (color.isValid()) options.insert(QStringLiteral("color"), color);
    return pixmap(style, character, QSize(size, size), options, devicePixelRatio);
}

QPixmap QFontAwesome::pixmap(const QString& name, const QSize& size, const QVariantMap& options, qreal devicePixelRatio)
{
    return pixmap(icon(name, options), size, devicePixelRatio);
}

QPixmap QFontAwesome::pixmap(const QString& name, int size, const QColor& color, qreal devicePixelRatio)
{
    QVariantMap options;
    if (color.isValid()) options.insert(QStringLiteral("color"), color);
    return pixmap(name, QSize(size, size), options, devicePixelRatio);
}

void QFontAwesome::give(const QString& name, QFontAwesomeIconPainter* painter)
{
    QFontAwesome* self = instance();
    // icons still using a replaced painter hold their own reference to it
    if (painter) {
        self->_painterMap.insert(name, QSharedPointer<QFontAwesomeIconPainter>(painter));
    } else {
        self->_painterMap.remove(name);
    }
}

/// Creates/Gets the icon font with a given size in pixels. This can be useful to use a label for displaying icons
///
///    QLabel* label = new QLabel(QChar(fa::fa_gear));
///    label->setFont(QFontAwesome::font(fa::fa_solid, 16));
QFont QFontAwesome::font(int style, int size)
{
    const QFontAwesome* self = instance();
    const auto it = self->_loadedFonts.constFind(style);
    if (it == self->_loadedFonts.constEnd()) return QFont();

    QFont font(it->family);
    font.setPixelSize(size);
    font.setWeight(toQtWeight(it->weight));

    return font;
}

QString QFontAwesome::fontName(int style)
{
    return instance()->_loadedFonts.value(style).family;
}

//---------------------------------------------------------------------------------------
// Icon creation (instance side)
//---------------------------------------------------------------------------------------

// internal helper method to merge to option maps
static QVariantMap mergeOptions(const QVariantMap& defaults, const QVariantMap& override)
{
    QVariantMap result = defaults;
    if (!override.isEmpty()) {
        QMapIterator<QString, QVariant> itr(override);
        while (itr.hasNext()) {
            itr.next();
            result.insert(itr.key(), itr.value());
        }
    }
    return result;
}

/// Creates an icon with the given code-point for given style
QIcon QFontAwesome::createIcon(int style, int character, const QVariantMap &options)
{
    // create a merged QVariantMap to have default options and icon-specific options
    QVariantMap optionMap = mergeOptions(_defaultOptions, options);
    optionMap.insert(QStringLiteral("text"), QString(QChar(character)));
    optionMap.insert(QStringLiteral("style"), style);

    return createIcon(glyphPainter(), optionMap);
}

/// Splits "fa-solid fa-gear" / "solid gear" / "gear" into a style and an icon name (without the fa- prefix).
/// Returns true when the name is a known glyph in that style.
bool QFontAwesome::resolveName(const QString& name, int* style, QString* iconName) const
{
    const int spaceIndex = name.indexOf(QLatin1Char(' '));
    *style = fa::fa_solid;

    if (spaceIndex > 0) {
        const QString styleName = name.left(spaceIndex);
        *style = stringToStyleEnum(styleName.startsWith(QLatin1String("fa-")) ? styleName : QLatin1String("fa-") + styleName);
        *iconName = name.mid(spaceIndex + 1);
    } else {
        *iconName = name;
    }

    if (iconName->startsWith(QLatin1String("fa-"))) {
        *iconName = iconName->mid(3);
    }

    const QHash<QString, int>* named = _namedCodepointsByStyle.value(*style, nullptr);
    return named && named->contains(*iconName);
}

/// Creates an icon with the given name
///
/// You can use the icon names as defined on https://fontawesome.com/icons adding the style prefix,
/// e.g. "fa-solid fa-address-book" (The fa- prefix for the icon name is optional)
QIcon QFontAwesome::createIcon(const QString& name, const QVariantMap& options)
{
    int style = fa::fa_solid;
    QString iconName;

    // when it's a named codepoint
    if (resolveName(name, &style, &iconName)) {
        QVariantMap named = options;
        named.insert(QStringLiteral("name"), name);
        return createIcon(style, _namedCodepointsByStyle.value(style)->value(iconName), named);
    }

    // otherwise try the painter map
    const QSharedPointer<QFontAwesomeIconPainter> painter = _painterMap.value(name);
    if (!painter) {
        qWarning() << "QFontAwesome: unknown icon" << name;
        return QIcon();
    }

    // create a merged QVariantMap to have default options and icon-specific options
    QVariantMap optionMap = mergeOptions(_defaultOptions, options);
    optionMap.insert(QStringLiteral("style"), style);
    optionMap.insert(QStringLiteral("name"), name);

    return createIcon(painter.data(), optionMap, painter);
}

/// Create a dynamic icon by simply supplying a painter object
/// The ownership of the painter is NOT transferred.
QIcon QFontAwesome::createIcon(QFontAwesomeIconPainter* painter, const QVariantMap& optionMap,
                               const QSharedPointer<QFontAwesomeIconPainter>& owner)
{
    QFontAwesomeCharIconPainter* cachingPainter = (painter == glyphPainter()) ? glyphPainter() : nullptr;
    QFontAwesomeIconPainterIconEngine* engine = new QFontAwesomeIconPainterIconEngine(painter, owner, optionMap, cachingPainter);
    return QIcon(engine);
}

//---------------------------------------------------------------------------------------
// QML support
//---------------------------------------------------------------------------------------

/// Requests a pixmap which can directly be used by QQuickImageProvider
///
/// \param An identifier in the format "regular/name?option1=x&option2=y
QPixmap QFontAwesome::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    QString baseId = id;
    QVariantMap options;

    int queryStart = id.indexOf('?');
    if (queryStart != -1) {
        baseId = id.left(queryStart);
        QString queryString = id.mid(queryStart + 1);

        QUrlQuery query(queryString);
        for (const auto &item : query.queryItems()) {
            options.insert(item.first, item.second);
        }
        transformStringVariantOptions(options);
    }

    int nameStart = baseId.indexOf("/");
    if (nameStart != -1) {
      baseId = "fa-" + baseId.left(nameStart) + " fa-" + baseId.mid(nameStart + 1);
    } else {
      baseId = "fa-solid fa-" + baseId;
    }

    QIcon icn = icon(baseId, options);
    QSize actualSize = requestedSize.isValid() ? requestedSize : QSize(128, 128);
    if (size) {
      *size = actualSize;
    }

    return icn.pixmap(actualSize);
}

/// Transforms a String based hash map to the correct object types.
/// All color types which aren't colors are converted to QColor objects and interpreted
/// as HEX codes (# is optional)
void QFontAwesome::transformStringVariantOptions(QVariantMap& options)
{
    // iterate over a copy of the keys: assigning through operator[] may detach and invalidate iterators
    const QStringList keys = options.keys();
    for (const QString& key : keys) {
        const QVariant value = options.value(key);
        if (key.contains(QLatin1String("color")) && value.userType() == QMetaType::QString) {
            const QString text = value.toString().trimmed();
            // an empty or invalid value yields an invalid QColor, which falls back to the palette colour
            options[key] = QColor(text.startsWith(QLatin1Char('#')) ? text : QLatin1Char('#') + text);
        }
    }
}


int QFontAwesome::stringToStyleEnum(const QString& style)
{
    if (style == QLatin1String("fa-duotone")) return fa::fa_duotone_solid; // DEPRECATED
    for (const FontRegistryEntry& entry : fontRegistry) {
        if (style == QLatin1String(entry.styleName)) return entry.style;
    }
    return fa::fa_solid;
}

QString QFontAwesome::styleEnumToString(int style)
{
    const FontRegistryEntry* entry = firstRegistryEntry(style);
    return QLatin1String(entry ? entry->styleName : "fa-solid");
}

} // namespace fa
