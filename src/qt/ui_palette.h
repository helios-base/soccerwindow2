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

#include <QPalette>

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
};

#endif
