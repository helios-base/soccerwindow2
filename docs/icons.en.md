# Icons

[日本語](icons.md) | English

This document describes how the icons of soccerwindow2 are made and embedded,
and how to add or modify an icon.

- [Overview](#overview)
- [Files](#files)
- [Adding or modifying an icon](#adding-or-modifying-an-icon)
- [Drawing rules](#drawing-rules)
- [palette.txt](#palettetxt)
- [generate.sh](#generatesh)
- [The Qt resource system (.qrc)](#the-qt-resource-system-qrc)
- [Tests](#tests)

## Overview

- Each icon is drawn as an SVG file (`icons/svg/NAME.svg`) with the colors for the light color scheme.
- `icons/generate.sh` converts the SVG file into four PNG files with Inkscape:
  16x16 and 32x32 pixels for the light and dark color schemes.
  The colors for the dark color scheme are taken from `icons/palette.txt`.
- The PNG files are committed to the repository, so Inkscape is needed only to create them.
  The build and the CI do not need Inkscape.
- The PNG files are embedded into the program with the Qt resource system (`icons/icons.qrc`).
- The program creates an icon by `UIIcon::fromName( "NAME" )` (`src/qt/ui_icon.h`).
  Whenever the icon is painted, the images for the current palette (light or dark) are used,
  so the icons follow the change of "View > UI Color Scheme".
  The 32x32 images are used on high-DPI displays (e.g. device pixel ratio 2) and for 32x32 icons.

Some icons are still XPM images in `xpm/` (`UIIcon::fromXpm()`).
They are being replaced with the new icons (see `docs/plan/2026-11-21_icon-redesign.md`).

## Files

```
icons/
  svg/NAME.svg          the source of the icon (edit this file)
  light/16/NAME.png     the icon for the light color scheme (16x16)    } created by
  light/32/NAME.png     the icon for the light color scheme (32x32)    } generate.sh
  dark/16/NAME.png      the icon for the dark color scheme (16x16)     }
  dark/32/NAME.png      the icon for the dark color scheme (32x32)     }
  palette.txt           the colors (light -> dark)
  palette.awk           checks and replaces the colors (used by generate.sh and the test)
  generate.sh           creates the PNG files and the lists below
  icons.qrc             the Qt resource file (the list of the PNG files)  } created by
  icons.am              the lists of the files for Automake              } generate.sh
  generated.sha256      the checksums of the SVG files and palette.txt   }
test/check_icons.sh     checks the files above
```

The files marked "created by generate.sh" must not be edited by hand.

## Adding or modifying an icon

1. Draw or edit `icons/svg/NAME.svg` with Inkscape (or a text editor), following the [drawing rules](#drawing-rules).
   NAME may contain only `a-z`, `0-9`, and `_`.
2. Create the PNG files and update the lists:
   ```
   icons/generate.sh NAME
   ```
3. Check the files and look at the PNG files:
   ```
   test/check_icons.sh
   ```
4. Use the icon in the source code (for a new icon):
   ```cpp
   #include "ui_icon.h"

   QAction * act = new QAction( UIIcon::fromName( "NAME" ), tr( "..." ), this );
   ```
5. Build and run the tests (`./cmake-helper.sh test`).
   CMake embeds the new files automatically.
   For Autotools, `make` runs automake again because `icons/icons.am` has changed.
6. Commit `icons/svg/NAME.svg`, the four PNG files, `icons/icons.qrc`, `icons/icons.am`, and `icons/generated.sha256`.

To remove an icon, remove its SVG file and its four PNG files (`git rm`), and run `icons/generate.sh --lists`.

## Drawing rules

These rules keep the line widths and the colors of the icons consistent.

- Size: `width="16" height="16" viewBox="0 0 16 16"`.
  The 32x32 images are created from the same SVG file at twice the size.
- Pixel grid: draw the 1-pixel lines on the centers of the pixels (e.g. `x="1.5"`),
  and the edges of the filled shapes on the pixel boundaries (e.g. `x="2"`),
  so that the 16x16 images are sharp. The 32x32 images then become sharp as well.
- Margin: use the area from 1 to 15 (14x14 pixels) for most icons.
  The outline of a shape may touch the edge when the shape needs the space.
- Lines: the outlines are 1 pixel wide in the `fg` color.
  Draw the symbols (e.g. arrows and crosses) with 1 or 2 pixel lines.
  Keep the lines horizontal, vertical, or at 45 degrees where possible.
- Small circles (7 pixels or less in the outer diameter): draw them as rounded rectangles on the pixel grid,
  because `circle` becomes blurred in the 16x16 images.
  Make `rx` 0.5 smaller than half of the width.
  e.g. a circle of 5 pixels is `<rect x="1.5" y="1.5" width="4" height="4" rx="1.5"/>`, and a circle of 7 pixels is `width="6" height="6" rx="2.5"`.
  Large circles (e.g. the circle of "record") can be drawn by `circle`.
- Log player symbols (play, stop, step, etc.): make the triangles 10 pixels high (y 3 to 13) and 5 pixels wide (45 degree edges),
  and the bars 2 pixels wide. They are used side by side, so their sizes must be the same.
- Filled shapes: objects with parts (e.g. folders and documents) have the `fg` outline.
  Simple colored marks (e.g. the red circle of "record") have no outline.
  Solid symbols (e.g. the triangle of "play") are filled with `fg` without an outline.
- Colors: use only the LIGHT colors of `palette.txt`.
  Do not use transparency to make a color lighter; add a color to `palette.txt` instead.
- The icons that show players and teams use the colors that have the same meaning in the field drawing of soccerwindow2.

When you edit the file with Inkscape, check that Inkscape has not added colors that are not in `palette.txt`
(e.g. a gradient or a color picked by the color picker). `generate.sh` and `test/check_icons.sh` report them.

## palette.txt

Each line has three fields: the role, the LIGHT color, and the DARK color.

```
# role      light    dark
fg          #303030  #e0e0e0
paper       #ffffff  #505050
```

- The SVG files use only the LIGHT colors.
  `generate.sh` replaces each LIGHT color with its DARK color to create the icons for the dark color scheme.
- The colors are written as `#rrggbb` in lower case. The roles and the LIGHT colors must be unique.
- Choose a DARK color that has a similar contrast against a dark background (e.g. `#323232`)
  as the LIGHT color has against a light background (e.g. `#efefef`).
- After changing `palette.txt`, run `icons/generate.sh` (without names) to create all the icons again.
  `generate.sh` does it automatically even if names are given.

## generate.sh

```
icons/generate.sh [NAME...]     create the PNG files of the icons (default: all the icons)
icons/generate.sh --lists       update only icons.qrc and icons.am (Inkscape is not used)
icons/generate.sh --help        show the usage
```

- Inkscape 1.0 or later is required. Specify the command by the environment variable `INKSCAPE` if needed,
  e.g. `INKSCAPE=/snap/bin/inkscape` or `INKSCAPE="flatpak run org.inkscape.Inkscape"`.
- The commands are printed before they are run, e.g.:
  ```
  + inkscape --pipe --export-type=png --export-filename=- --export-background-opacity=0 -w 16 -h 16 < svg/open.svg > light/16/open.png
  + awk -v mode=dark -f palette.awk palette.txt svg/open.svg | inkscape --pipe ... -w 16 -h 16 > dark/16/open.png
  ```
  The SVG data is given from the standard input and the PNG data is written to the standard output,
  so that Inkscape does not access the files by itself
  (Inkscape installed by snap cannot read the files in hidden directories, and it does not use the current directory of the shell).
- Before running Inkscape, the colors of the SVG files are checked with `palette.txt`.
- After creating the PNG files, `icons.qrc`, `icons.am`, and `generated.sha256` are updated.
  `generated.sha256` records the checksums of `palette.txt` and the SVG files from which the PNG files were created.
  `test/check_icons.sh` uses it to detect an SVG file that was edited without running `generate.sh`.

## The Qt resource system (.qrc)

The Qt resource system embeds files into the program.
The files are listed in a resource collection file (`.qrc`), and the resource compiler `rcc`
converts them into a C++ source file that is compiled and linked into the program.
The program reads the files with the paths that start with `:/`, in the same way as the files on the disk
(e.g. `QPixmap( ":/icons/light/16/open.png" )`).

### Format

`icons/icons.qrc` is an XML file:

```xml
<!DOCTYPE RCC>
<RCC version="1.0">
<qresource prefix="/icons">
    <file>light/16/open.png</file>
    <file>light/32/open.png</file>
    <file>dark/16/open.png</file>
    <file>dark/32/open.png</file>
</qresource>
</RCC>
```

- `<file>` gives the path of a file relative to the directory of the `.qrc` file.
- `prefix` of `<qresource>` is prepended to the paths in the program.
  The file `icons/light/16/open.png` is read as `:/icons/light/16/open.png`.
- A file can be given another name by the `alias` attribute: `<file alias="open.png">light/16/open.png</file>`
  is read as `:/icons/open.png`.
- `rcc` compresses the files if it reduces the size. The PNG files are usually stored as they are.

`icons/icons.qrc` is created by `generate.sh` from the SVG files, so you do not need to edit it.
To embed other files (e.g. other images), create another `.qrc` file in the same format
and add it to the build in the same way as `icons/icons.qrc` (see below).

### Build

CMake (`src/qt/CMakeLists.txt`): the `.qrc` file is added to the sources of the target, and `AUTORCC` runs `rcc`.
CMake knows the files listed in the `.qrc` file, so it runs `rcc` again when one of them is changed.

```cmake
add_library(soccerwindow2_gui STATIC
  ...
  ${PROJECT_SOURCE_DIR}/icons/icons.qrc
  )
set_target_properties(soccerwindow2_gui PROPERTIES AUTOMOC ON AUTORCC ON)
```

Autotools (`src/qt/Makefile.am`): `configure` looks for `rcc` of the selected Qt
(it can be specified by `QT_RCC=/path/to/rcc`), and `make` creates `qrc_icons.cpp`:

```make
ICONS_DIR = $(top_srcdir)/icons
include $(top_srcdir)/icons/icons.am

qrc_icons.cpp: $(ICONS_DIR)/icons.qrc $(ICONS_PNG_FILES)
	$(QT_RCC) --name icons --output $@ $(ICONS_DIR)/icons.qrc
```

`qrc_icons.cpp` is listed in `nodist_soccerwindow2_SOURCES`.
The top-level `Makefile.am` includes `icons/icons.am` as well to add the SVG and PNG files to the source tarball (`EXTRA_DIST`).

### Initialization

The resources in a static library must be registered by `Q_INIT_RESOURCE( NAME )` before they are used,
where NAME is the base name of the `.qrc` file (or the name given by `rcc --name`).
Otherwise the linker may drop them. In the CMake build, the icons are linked into the static library `soccerwindow2_gui`.

`UIIcon::fromName()` calls `UIIcon::initResources()`, which calls `Q_INIT_RESOURCE( icons )`, automatically.
If you read the files directly by the `:/icons/...` paths, call `UIIcon::initResources()` first.
`Q_INIT_RESOURCE` must be called from a function in the global namespace.

## Tests

- `test/check_icons.sh` (CTest `check_icons` and `make check`) checks, without Inkscape, that
  - `palette.txt` is valid and the SVG files use only its LIGHT colors,
  - each SVG file has the four PNG files of the correct sizes, and no PNG file exists without its SVG file,
  - `icons.qrc` and `icons.am` list exactly these files, and
  - the PNG files were created from the current SVG files and `palette.txt` (`generated.sha256`).
- The GUI test (`test/test_gui_smoke.cpp`) checks that the icons are embedded,
  that `UIIcon::fromName()` uses the images for the current palette and the device pixel ratio,
  and that the tool buttons are painted with the images for the current palette.
