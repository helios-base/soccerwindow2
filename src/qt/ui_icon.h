// -*-c++-*-

/*!
  \file ui_icon.h
  \brief icons adapted to the palette Header File.
*/

/*
 *Copyright:

 Copyright (C) Hidehisa AKIYAMA

 This code is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2, or (at your option)
 any later version.

 This code is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this code; see the file COPYING.  If not, write to
 the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

 *EndCopyright:
 */

/////////////////////////////////////////////////////////////////////

#ifndef SOCCERWINDOW2_QT_UI_ICON_H
#define SOCCERWINDOW2_QT_UI_ICON_H

#include <QIcon>
#include <QImage>

class QPalette;

/*!
  \class UIIcon
  \brief icons adapted to the palette of the application

  There are two kinds of icons.

  - fromName() creates an icon from the PNG files embedded with the Qt
    resource system (icons/icons.qrc). Each icon has the images for the light
    and dark color schemes (16x16 and 32x32 pixels each), and the images are
    selected according to the palette whenever the icon is painted.
    See docs/icons.md.
  - fromXpm() creates an icon from XPM data. The XPM icons are drawn with
    black and gray lines for a light background. When the application uses
    a dark palette, the dark gray pixels are replaced with the text color, so
    that the icons can be seen on a dark background.
*/
class UIIcon {
private:
    // not used
    UIIcon() = delete;

public:

    /*!
      \brief register the icons embedded with the Qt resource system.
      fromName() calls this function automatically. Call it before using the
      resource paths (":/icons/...") directly.
      The function can be called more than once.
    */
    static
    void initResources();

    /*!
      \brief create an icon from the embedded PNG files.
      The following files are used (NAME is the name of the icon):
        :/icons/light/16/NAME.png  :/icons/light/32/NAME.png
        :/icons/dark/16/NAME.png   :/icons/dark/32/NAME.png
      The images are selected according to the palette of the application
      whenever the icon is painted, so the icon follows the change of the
      palette.
      \param name the name of the icon (e.g. "open")
      \return the icon. If the files are not found, a null icon is returned
      and a warning is printed.
    */
    static
    QIcon fromName( const QString & name );

    /*!
      \brief create an icon from XPM data.
      The colors are adapted to the palette of the application whenever the
      icon is painted, so the icon follows the change of the palette.
      \param xpm XPM data
      \return the icon
    */
    static
    QIcon fromXpm( const char * const * xpm );

    /*!
      \brief check if the palette is dark.
      \param palette the palette
      \return true if the window color of the palette is dark
    */
    static
    bool isDark( const QPalette & palette );

    /*!
      \brief adapt the colors of the image to the palette.
      If the palette is dark, the dark gray pixels (low saturation and low
      lightness) are replaced with the text color of the palette. The alpha
      values are kept. Otherwise, the image is returned as it is.
      \param image the source image
      \param palette the palette
      \return the adapted image
    */
    static
    QImage adaptToPalette( const QImage & image,
                           const QPalette & palette );
};

#endif
