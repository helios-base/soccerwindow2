// -*-c++-*-

/*!
  \file ui_palette.h
  \brief palettes of the widgets Header File.
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

#ifndef SOCCERWINDOW2_QT_UI_PALETTE_H
#define SOCCERWINDOW2_QT_UI_PALETTE_H

#include <QColor>
#include <QPalette>

class QWidget;

/*!
  \class UIPalette
  \brief palettes of the widgets
*/
class UIPalette {
private:
    // not used
    UIPalette() = delete;

public:

    /*!
      \brief create the light palette.

      The palette has the same colors as the standard palette of the Fusion
      style in the light mode. Qt6 makes the standard palette dark when the
      desktop uses a dark color scheme, so the palette is built explicitly.
      \return the light palette
    */
    static
    QPalette light();

    /*!
      \brief create the dark palette.

      The palette has the same colors as the standard palette of the Fusion
      style in the dark mode (Qt 6.5 or later). It is used regardless of the
      color scheme of the desktop and the version of Qt.
      \return the dark palette
    */
    static
    QPalette dark();

    /*!
      \brief check whether a palette is dark: its window is dark
      \param palette the palette
      \return true if the palette is dark
    */
    static
    bool isDark( const QPalette & palette );

    /*!
      \brief the color of the text of an error on the window of a palette:
      a red legible on the light window and on the dark one (ported from
      rcssmonitor)
      \param palette the palette
      \return the color
    */
    static
    QColor errorText( const QPalette & palette );

    /*!
      \brief mark a text (e.g. a label) as wrong or not: its color is
      errorText() while it is wrong. Only that color is set, so that the
      text follows the application's palette (the color scheme) in the
      others, and in all again when it is not wrong. The color is of the
      palette when it is marked: mark it again when the palette changes.
      \param text the widget of the text
      \param wrong true while the text tells what is wrong
    */
    static
    void markWrongText( QWidget * text,
                        const bool wrong );

private:

    /*!
      \brief create the palette of the Fusion style.
      \param dark true for the dark mode, false for the light mode
      \return the palette
    */
    static
    QPalette fusion( const bool dark );
};

#endif
