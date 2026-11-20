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

  The icons of soccerwindow2 are drawn with black and gray lines for a light
  background. The icons created by this class replace the dark gray pixels
  with the text color when the application uses a dark palette, so that the
  icons can be seen on a dark background.
*/
class UIIcon {
private:
    // not used
    UIIcon() = delete;

public:

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
