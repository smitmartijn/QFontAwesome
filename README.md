# QFontAwesome - Font Awesome for Qt Applications

A typical layout in the application repository:

```
myapp/
  3rdparty/QFontAwesome/              # this repository, e.g. as a git submodule
  3rdparty/fontawesome/otfs/*.otf     # the fonts, listed in the application's .qrc
  3rdparty/fontawesome/generated/     # headers generated from the matching metadata/icons.json
```

Point the `.pri` at the generated headers and include it in your qmake project:

```sh
QFONTAWESOME_GENERATED_DIR = $$PWD/3rdparty/fontawesome/generated
include(3rdparty/QFontAwesome/QFontAwesome/QFontAwesome.pri)
```

## Generating the icon headers

The `fa::fa_*` constants and the icon name tables are generated from the `metadata/icons.json` of the same
download as the fonts. Run this once, and again whenever you upgrade Font Awesome, then commit the output in the
application repository. Python 3.7+ is only needed for this step, not for building.

```sh
python3 3rdparty/QFontAwesome/build_headers.py \
    path/to/fontawesome-free-7.0.0-desktop/metadata/icons.json \
    3rdparty/fontawesome/generated
```

- The tier follows the metadata: Free metadata gives the free icons, Pro metadata adds the Pro icons and defines
  `FONT_AWESOME_PRO`. There is no build configuration for it.
- The Font Awesome version is read from the fonts in the download's `otfs/` folder (pass `--fa-version 7.0.0` when
  that folder is not next to `metadata/`). It is stored as `QFONTAWESOME_FA_VERSION`, and loading a font of
  another version logs a warning, since icons may be missing or moved.

## Loading the fonts

QFontAwesome does **not** contain any font files. Download the fonts you are licensed to use from
[fontawesome.com](https://fontawesome.com/download) (the desktop `.otf` files), add them to your
application's own resources and load them once at startup, after `QApplication` is constructed:

```xml
<!-- myapp.qrc -->
<RCC>
    <qresource prefix="/fonts/fontawesome">
        <file>3rdparty/fontawesome/Font Awesome 7 Free-Solid-900.otf</file>
        <file>3rdparty/fontawesome/Font Awesome 7 Free-Regular-400.otf</file>
        <file>3rdparty/fontawesome/Font Awesome 7 Brands-Regular-400.otf</file>
    </qresource>
</RCC>
```

```c++
QApplication app(argc, argv);
if (fa::QFontAwesome::loadFonts(":/fonts/fontawesome") == 0) {
    qWarning() << "Font Awesome fonts failed to load";
}
```

Each file is matched to its style by the family name and weight stored inside the font, so the files may be
renamed or aliased freely. Paths on disk work too. Other ways to load:

```c++
fa::QFontAwesome::loadFont(":/icons/solid.otf");                  // one file, style detected
fa::QFontAwesome::loadFont(fa::fa_solid, ":/icons/custom.otf");   // force a style (subsetted/custom fonts)
fa::QFontAwesome::loadFontData(bytes);                            // from memory
```

Loading a style that is already loaded is a no-op. `isFontLoaded(style)` and `loadedStyles()` report what is
available; `knownFonts()` returns the registry of every Font Awesome 7 font file QFontAwesome can use (style,
style name, family, weight, download file name, Free/Pro/Pro+ tier and whether it is loaded). An icon whose
style has no loaded font paints nothing and logs a warning once.

The Pro fonts also provide the free styles: loading `Font Awesome 7 Pro-Solid-900.otf` makes `fa::fa_solid`
available.

## Basic Usage

QFontAwesome is a process-wide singleton. There is nothing to construct: the first static call builds the
name tables, every later call reuses that instance. Load the fonts first, see [Loading the fonts](#loading-the-fonts).

```c++
#include "QFontAwesome.h"

QPushButton* btn = new QPushButton(fa::QFontAwesome::icon(fa::fa_solid, fa::fa_wine_glass), "Cheers!");
```

For QML usage see the [QML section](#qml-usage) below.

## Examples

Icons are created with the static `icon` overloads. Use an icon name from the
[Font Awesome Library](https://fontawesome.com/icons).

```c++
using namespace fa;

// Fastest: style enum + glyph enum
QPushButton* btn = new QPushButton(QFontAwesome::icon(fa_solid, fa_wine_glass), "Cheers!");

// Same, with an explicit colour for every mode/state
QPushButton* btn = new QPushButton(QFontAwesome::icon(fa_solid, fa_wine_glass, QColor("#394c60")), "Cheers!");

// String names, with or without the 'fa-' prefix; the style defaults to solid
QPushButton* btn = new QPushButton(QFontAwesome::icon("fa-solid fa-coffee"), "Black please!");
QPushButton* btn = new QPushButton(QFontAwesome::icon("solid coffee"), "Black please!");
QPushButton* btn = new QPushButton(QFontAwesome::icon("coffee"), "Black please!");

// Pixmaps for QLabel::setPixmap and friends. Rendered at the application's device pixel ratio
// (or the one you pass) and tagged with it, so they are crisp on HiDPI screens.
label->setPixmap(QFontAwesome::pixmap(fa_solid, fa_gear, 16, QColor("#394c60")));
label->setPixmap(QFontAwesome::pixmap("regular clock", QSize(24, 24)));

// Does a name exist?
if (QFontAwesome::hasIcon("brands apple")) { ... }
```

Options give per mode/state control. The available options are listed in [Default options](#default-options).

```c++
QVariantMap options;
options.insert("color", QColor(255, 0, 0));
options.insert("color-disabled", QColor(128, 128, 128));
QPushButton* musicButton = new QPushButton(QFontAwesome::icon(fa::fa_solid, fa::fa_music, options), "Music");
```

Default options apply to every icon created afterwards:

```c++
QFontAwesome::setDefaultOption("color-disabled", QColor(0, 255, 0));
```

Colours that are not set (neither per icon nor as default) are taken from `QApplication::palette()`
**at paint time**, so such icons follow a light/dark switch without being re-created.

It is also possible to render a label directly with the font:

```c++
QLabel* label = new QLabel(QChar(fa::fa_github));
label->setFont(QFontAwesome::font(fa::fa_brands, 16));
```

### Caching

Pixmaps produced by the built-in glyph painter are stored in `QPixmapCache`, keyed on style, glyph, size,
mode, state, colour and scale factor. Widgets that call `QIcon::pixmap()` on every repaint therefore do not
re-render the glyph. Animated icons (`anim` option) bypass the cache.

## Example Custom Painter

This example registers a custom painter for supporting an custom icon named 'duplicate'
It simply draws 2 "plus marks".

```c++
class DuplicateIconPainter : public fa::QFontAwesomeIconPainter
{
public:
    virtual void paint(fa::QFontAwesome* awesome, QPainter* painter, const QRect& rectIn, QIcon::Mode mode, QIcon::State state, const QVariantMap& options)
    {
        int drawSize = qRound(rectIn.height() * 0.5);
        int offset = rectIn.height() / 4;
        QChar chr = QChar(static_cast<int>(fa::plus));
        int st = fa::fa_solid;

        painter->setFont(fa::QFontAwesome::font(st, drawSize));

        painter->setPen(QColor(100,100,100));
        painter->drawText(QRect(QPoint(offset * 2, offset * 2),
                          QSize(drawSize, drawSize)), chr ,
                          QTextOption(Qt::AlignCenter|Qt::AlignVCenter));

        painter->setPen(QColor(50,50,50));
        painter->drawText(QRect(QPoint(rectIn.width() - drawSize-offset, rectIn.height() - drawSize - offset),
                                QSize(drawSize, drawSize) ), chr ,
                                QTextOption(Qt::AlignCenter | Qt::AlignVCenter));
    }
};

fa::QFontAwesome::give("duplicate", new DuplicateIconPainter());
```

After this, this icon can be used with the given string name:

```c++
fa::QFontAwesome::icon("duplicate")
```

`give()` takes ownership of the painter. Registering another painter under the same name, or `give(name, nullptr)`
to remove it, is safe: icons created earlier keep using (and owning a reference to) the painter they were created
with. `hasIcon("duplicate")` returns true while a painter is registered.

## Default options

The following options are the defaults in the QFontAwesome class.

```c++
setDefaultOption("scale-factor", 1.0);
setDefaultOption("text", QString()); // internal option
setDefaultOption("text-disabled", QString());
setDefaultOption("text-active", QString());
setDefaultOption("text-selected", QString());
```

Colour options (`color`, `color-disabled`, `color-active`, `color-selected` and the pro `duotone-color*`
variants) have no default value. When missing, `color*` resolves at paint time to
`QApplication::palette().color(<group for the mode>, QPalette::Text)` (see `QFontAwesome::paletteColor()`), and
`duotone-color*` to the primary colour at 40% opacity, like Font Awesome's default secondary layer. The secondary
layer is drawn for every duotone style (duotone, sharp duotone, jelly duo, notdog duo).

When creating an icon, it first populates the options map with the default options from the QFontAwesome object.
After that the options are expanded/overwritten by the options supplied to the icon.

It is possible to use another glyph per icon-state. For example to make an icon-unlock symbol switch to locked when selected,
you could supply the following option:

```c++
options.insert("text-selected", QString(fa::fa_lock));
```

Color and text options have the following structure:
`keyname-iconmode-iconstate`

When iconmode normal is empty\
And iconstate on is blank

So the list of items used is:

- color
- color-disabled
- color-active
- color-selected
- color-off
- color-disabled-off
- color-active-off
- color-selected-off
- duotone-color (pro)
- duotone-color-disabled (pro)
- duotone-color-active (pro)
- duotone-color-selected (pro)
- duotone-color-off (pro)
- duotone-color-disabled-off (pro)
- duotone-color-active-off (pro)
- duotone-color-selected-off (pro)
- text
- text-disabled
- text-active
- text-selected
- text-off
- text-disabled-off
- text-active-off
- text-selected-off
- style
- style-disabled
- style-active
- style-selected
- style-off
- style-disabled-off
- style-active-off
- style-selected-off
