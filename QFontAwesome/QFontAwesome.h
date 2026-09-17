/**
 * QFontAwesome - use font-awesome (or other font icons) in your c++ / Qt Application
 */

#ifndef QFONTAWESOME_H
#define QFONTAWESOME_H

#include <QIcon>
#include <QIconEngine>
#include <QPainter>
#include <QRect>
#include <QVariantMap>
#include <QColor>
#include <QPixmap>
#include <QSharedPointer>

#include "QFontAwesomeAnim.h"

/// The fa::fa_* codepoint constants. The application generates this header from the Font Awesome metadata of
/// the fonts it ships (build_headers.py); it also defines QFONTAWESOME_FA_VERSION and, for Pro metadata,
/// FONT_AWESOME_PRO.
#include "QFontAwesomeEnumGenerated.h"


namespace fa {

/// A list of all style-names.
///
/// Every Font Awesome 7 style is declared, whatever the build tier: a style is usable once its font
/// file has been loaded with QFontAwesome::loadFont() / loadFonts(). Which glyph constants and names exist
/// follows the generated headers (Free or Pro metadata).
enum fa_styles {
    fa_solid = 0,
    fa_regular = 1,
    fa_brands = 2,

    fa_light = 3,
    fa_thin = 4,

    fa_duotone = 5, // DEPRECATED, use fa_duotone_solid
    fa_duotone_solid = 5,
    fa_duotone_regular = 6,
    fa_duotone_light = 7,
    fa_duotone_thin = 8,

    fa_sharp_solid = 9,
    fa_sharp_regular = 10,
    fa_sharp_light = 11,
    fa_sharp_thin = 12,

    fa_sharp_duotone_solid = 13,
    fa_sharp_duotone_regular = 14,
    fa_sharp_duotone_light = 15,
    fa_sharp_duotone_thin = 16,

    fa_chisel_regular = 17,

    fa_etch_solid =  18,

    fa_jelly_regular = 19,
    fa_jelly_duotone_regular = 20,
    fa_jelly_fill_regular = 21,

    fa_notdog_duotone_solid =  22,
    fa_notdog_solid = 23,

    fa_slab_press_regular = 24,
    fa_slab_regular = 25,

    fa_thumbprint_light = 26,

    fa_whiteboard_semibold = 27
};


struct QFontAwesomeNamedIcon {
    const char *name;
    ushort icon;
};

class QFontAwesomeIconPainter;

/// One entry of the Font Awesome font registry: a font file QFontAwesome knows how to use.
///
/// The module does not ship any font files. The application puts the files it is licensed to use in its
/// own resources (or on disk) and loads them with QFontAwesome::loadFonts() / loadFont(). A file is
/// matched to its style by the family name and weight stored inside the font, so the file name does not
/// matter; fileName is the name the file has in the Font Awesome download, for reference.
struct QFontAwesomeFontInfo
{
    enum Tier { Free, Pro, ProPlus };

    int style;          ///< the fa_styles value this font provides
    QString styleName;  ///< the style name as used in icon names, e.g. "fa-solid"
    QString family;     ///< the typographic family stored in the font, e.g. "Font Awesome 7 Free"
    int weight;         ///< the OS/2 weight class stored in the font (100..900)
    QString fileName;   ///< the file name in the Font Awesome download
    Tier tier;          ///< the Font Awesome plan that includes this font
    bool loaded;        ///< true when a font for this style is currently loaded
};

//---------------------------------------------------------------------------------------

/// The main class for managing icons.
///
/// QFontAwesome is a process-wide singleton. The first call to any static accessor
/// (icon(), pixmap(), font(), ...) creates the instance, so a single line is enough anywhere
/// in the application:
///
///     button->setIcon(fa::QFontAwesome::icon(fa::fa_solid, fa::fa_gear));
///
/// The font files are NOT part of this module. Load them once at startup, after QApplication
/// is constructed and on the GUI thread, from the application's own resources or from disk:
///
///     fa::QFontAwesome::loadFonts(":/fonts/fontawesome");
class QFontAwesome : public QObject
{
Q_OBJECT

public:
    /// Offset of the secondary layer glyph in the duotone fonts.
    static const int DUOTONE_HEX_ICON_VALUE = 0x100000;

public:
    // ---------------------------------------------------------------------------------
    // Global instance
    // ---------------------------------------------------------------------------------

    /// Creates the global instance and builds the icon name tables. Idempotent and optional:
    /// every static call initializes lazily. Must be called on the GUI thread after QApplication
    /// exists. The instance is owned by the application object and lives until it is destroyed.
    static void init();

    /// True once init() (explicit or implicit) has run.
    static bool isInitialized();

    /// Returns the global instance, creating and initializing it when needed.
    static QFontAwesome* instance();

    /// Destroys the global instance and unregisters the loaded fonts. Icons created earlier must
    /// not be painted afterwards. Mainly useful for tests.
    static void shutdown();

    // ---------------------------------------------------------------------------------
    // Fonts
    // ---------------------------------------------------------------------------------

    /// Loads a Font Awesome font file from a resource (":/...") or disk path. The style is detected
    /// from the family name and weight stored in the font (falling back to the file name), so files
    /// may be renamed freely. Returns true when the style is available afterwards; loading a style
    /// that is already loaded is a no-op that returns true.
    static bool loadFont(const QString& path);

    /// Loads a font file for an explicit style, skipping detection. Use this for fonts whose
    /// metadata is not in the registry (e.g. subsetted or custom-built Font Awesome fonts).
    static bool loadFont(int style, const QString& path);

    /// Same as loadFont(), for font data already in memory. origin is only used in warnings.
    static bool loadFontData(const QByteArray& data, const QString& origin = QString());
    static bool loadFontData(int style, const QByteArray& data, const QString& origin = QString());

    /// Loads every *.otf / *.ttf file in a resource or disk directory (not recursive). Files that
    /// are not a known Font Awesome font are skipped with a warning. Returns the number of files
    /// that provide a loaded style.
    static int loadFonts(const QString& directory);

    /// True when a font for the style has been loaded.
    static bool isFontLoaded(int style);

    /// The styles that currently have a loaded font.
    static QList<int> loadedStyles();

    /// The registry of every Font Awesome font file QFontAwesome can use, with its load state.
    static QList<QFontAwesomeFontInfo> knownFonts();

    // ---------------------------------------------------------------------------------
    // Icons
    // ---------------------------------------------------------------------------------

    /// Icon for a style + codepoint, e.g. icon(fa::fa_solid, fa::fa_gear). Fastest path.
    static QIcon icon(int style, int character, const QVariantMap& options = QVariantMap());

    /// Icon for a style + codepoint with an explicit colour (applies to every mode/state).
    static QIcon icon(int style, int character, const QColor& color);

    /// Icon by name, e.g. "fa-solid fa-gear", "solid gear" or just "gear" (style defaults to solid).
    static QIcon icon(const QString& name, const QVariantMap& options = QVariantMap());

    /// Icon by name with an explicit colour.
    static QIcon icon(const QString& name, const QColor& color);

    /// Icon drawn by a custom painter. Ownership of the painter is NOT transferred.
    static QIcon icon(QFontAwesomeIconPainter* painter, const QVariantMap& optionMap = QVariantMap());

    /// True when the name resolves to a known glyph (in the given style) or a registered painter.
    /// Does not check whether the style's font is loaded; use isFontLoaded() for that.
    static bool hasIcon(const QString& name);

    // ---------------------------------------------------------------------------------
    // Pixmaps (for QLabel::setPixmap and other raw-pixmap consumers)
    // ---------------------------------------------------------------------------------

    /// Renders an icon into a pixmap of the given logical size. The pixmap is rendered at
    /// devicePixelRatio (defaults to the application's ratio) and tagged with it, so it
    /// stays crisp on HiDPI screens.
    static QPixmap pixmap(int style, int character, const QSize& size,
                          const QVariantMap& options = QVariantMap(), qreal devicePixelRatio = 0.0);
    static QPixmap pixmap(int style, int character, int size, const QColor& color = QColor(),
                          qreal devicePixelRatio = 0.0);
    static QPixmap pixmap(const QString& name, const QSize& size,
                          const QVariantMap& options = QVariantMap(), qreal devicePixelRatio = 0.0);
    static QPixmap pixmap(const QString& name, int size, const QColor& color = QColor(),
                          qreal devicePixelRatio = 0.0);

    /// Renders any QIcon into a HiDPI-tagged pixmap (helper used by the overloads above).
    static QPixmap pixmap(const QIcon& icon, const QSize& size, qreal devicePixelRatio = 0.0,
                          QIcon::Mode mode = QIcon::Normal, QIcon::State state = QIcon::Off);

    // ---------------------------------------------------------------------------------
    // Fonts, options, painters
    // ---------------------------------------------------------------------------------

    /// The icon font for a style at a pixel size. Useful for QLabel::setText(QChar(fa::fa_gear)).
    /// Returns a default QFont when no font is loaded for the style.
    static QFont font(int style, int size);

    /// The font family name registered for a style, or an empty string when it is not loaded.
    static QString fontName(int style);

    /// Registers a custom painter under a name. Ownership is transferred. Replacing or removing
    /// (nullptr) a painter is safe: icons created with the old painter keep it alive.
    static void give(const QString& name, QFontAwesomeIconPainter* painter);

    /// Default options merged into every icon. Colour options left unset are resolved from
    /// QApplication::palette() at paint time, so icons follow light/dark theme changes.
    static void setDefaultOption(const QString& name, const QVariant& value);
    static QVariant defaultOption(const QString& name);

    /// All icon names known for a style, mapped to their codepoints.
    static const QHash<QString, int> namedCodePoints(int style);

    /// Pixmap request in the "style/name?option=value" form used by QFontAwesomeQuickImageProvider.
    static QPixmap requestPixmap(const QString& id, QSize* size, const QSize& requestedSize);

    /// Converts string colour options ("#ff0000" / "ff0000") in a map to QColor values.
    static void transformStringVariantOptions(QVariantMap& options);

    /// Converts between style enum values and their "fa-<style>" names.
    static int stringToStyleEnum(const QString& style);
    static QString styleEnumToString(int style);

    /// Colour for a mode/state from the current application palette. Used by the built-in
    /// painter when no colour option is set; exposed for custom painters.
    static QColor paletteColor(QIcon::Mode mode, QIcon::State state);

    virtual ~QFontAwesome();

Q_SIGNALS:
    /// Emitted after resetDefaultOptions().
    void defaultOptionsReset();

public Q_SLOTS:
    /// Clears the default options back to their initial values.
    void resetDefaultOptions();

private:
    explicit QFontAwesome(QObject* parent);
    Q_DISABLE_COPY(QFontAwesome)

    /// A font registered in QFontDatabase for a style.
    struct LoadedFont {
        int fontId = -1;
        QString family;
        int weight = 400;
    };

    void buildNamedCodePoints();
    void addToNamedCodePoints(int style, const fa::QFontAwesomeNamedIcon* faCommonIconArray, int size);
    bool registerFont(int style, const QByteArray& data, const QString& origin);

    QIcon createIcon(int style, int character, const QVariantMap& options);
    QIcon createIcon(const QString& name, const QVariantMap& options);
    QIcon createIcon(QFontAwesomeIconPainter* painter, const QVariantMap& optionMap,
                     const QSharedPointer<QFontAwesomeIconPainter>& owner = QSharedPointer<QFontAwesomeIconPainter>());
    bool resolveName(const QString& name, int* style, QString* iconName) const;

    static QFontAwesome* s_instance;

    QHash<int, LoadedFont>           _loadedFonts;            ///< The registered font for each loaded style
    QHash<int, QHash<QString, int>*> _namedCodepointsByStyle; ///< A map with names mapped to code-points for each style
    QList<QHash<QString, int>*>      _namedCodepointsList;    ///< The list of all created named-codepoints

    QHash<QString, QSharedPointer<QFontAwesomeIconPainter>> _painterMap;  ///< A map of custom painters, shared with their icons
    QVariantMap _defaultOptions;                           ///< The default icon options
};

//---------------------------------------------------------------------------------------

/// The QFontAwesomeIconPainter is a specialized painter for painting icons.
/// Implement it to create custom font-icon code. The awesome parameter is the global
/// instance; the static QFontAwesome API can be used just as well.
class QFontAwesomeIconPainter
{
public:
    virtual ~QFontAwesomeIconPainter();
    virtual void paint(QFontAwesome* awesome, QPainter* painter, const QRect& rect,  QIcon::Mode mode, QIcon::State state,
                       const QVariantMap& options) = 0;
};

} // fa

Q_DECLARE_METATYPE(fa::QFontAwesomeAnimation*)

#endif // QFONTAWESOME_H
