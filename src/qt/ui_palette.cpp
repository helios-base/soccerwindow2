// -*-c++-*-

/*!
  \file ui_palette.cpp
  \brief palettes of the widgets Source File.
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "ui_palette.h"

#include <QColor>

/*-------------------------------------------------------------------*/
/*!

*/
QPalette
UIPalette::light()
{
    // The same colors as the Fusion style palette in the light mode.
    const QColor window_text = Qt::black;
    const QColor background( 239, 239, 239 );
    const QColor light = background.lighter( 150 );
    const QColor mid = background.darker( 130 );
    const QColor midlight = mid.lighter( 110 );
    const QColor base = Qt::white;
    const QColor dark = background.darker( 150 );
    const QColor shadow = dark.darker( 135 );
    const QColor highlight( 48, 140, 198 );
    const QColor highlighted_text = Qt::white;
    const QColor disabled_text( 190, 190, 190 );
    const QColor disabled_dark = QColor( 209, 209, 209 ).darker( 110 );
    const QColor disabled_shadow = shadow.lighter( 150 );
    const QColor disabled_highlight( 145, 145, 145 );
    QColor placeholder = window_text;
    placeholder.setAlpha( 128 );

    QPalette pal( window_text, background, light, dark, mid, window_text, base );
    pal.setBrush( QPalette::Midlight, midlight );
    pal.setBrush( QPalette::Button, background );
    pal.setBrush( QPalette::Shadow, shadow );
    pal.setBrush( QPalette::HighlightedText, highlighted_text );

    pal.setBrush( QPalette::Disabled, QPalette::Text, disabled_text );
    pal.setBrush( QPalette::Disabled, QPalette::WindowText, disabled_text );
    pal.setBrush( QPalette::Disabled, QPalette::ButtonText, disabled_text );
    pal.setBrush( QPalette::Disabled, QPalette::Base, background );
    pal.setBrush( QPalette::Disabled, QPalette::Dark, disabled_dark );
    pal.setBrush( QPalette::Disabled, QPalette::Shadow, disabled_shadow );

    pal.setBrush( QPalette::Active, QPalette::Highlight, highlight );
    pal.setBrush( QPalette::Inactive, QPalette::Highlight, highlight );
    pal.setBrush( QPalette::Disabled, QPalette::Highlight, disabled_highlight );

    pal.setBrush( QPalette::PlaceholderText, placeholder );

    return pal;
}
