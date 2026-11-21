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
    return fusion( false );
}

/*-------------------------------------------------------------------*/
/*!

*/
QPalette
UIPalette::dark()
{
    return fusion( true );
}

/*-------------------------------------------------------------------*/
/*!

*/
QPalette
UIPalette::fusion( const bool dark )
{
    // The same colors as the standard palette of the Fusion style
    // (qt_fusionPalette() of Qt 6.5 or later).
    const QColor window_text = dark ? QColor( 240, 240, 240 ) : QColor( Qt::black );
    const QColor background = dark ? QColor( 50, 50, 50 ) : QColor( 239, 239, 239 );
    const QColor light = background.lighter( 150 );
    const QColor mid = background.darker( 130 );
    const QColor midlight = mid.lighter( 110 );
    const QColor base = dark ? background.darker( 140 ) : QColor( Qt::white );
    const QColor dark_color = background.darker( 150 );
    const QColor shadow = dark_color.darker( 135 );
    const QColor text = dark ? window_text : QColor( Qt::black );
    const QColor highlight( 48, 140, 198 );
    const QColor highlighted_text = dark ? window_text : QColor( Qt::white );
    const QColor disabled_text = dark ? QColor( 130, 130, 130 ) : QColor( 190, 190, 190 );
    const QColor disabled_dark = QColor( 209, 209, 209 ).darker( 110 );
    const QColor disabled_shadow = shadow.lighter( 150 );
    const QColor disabled_highlight( 145, 145, 145 );
    QColor placeholder = text;
    placeholder.setAlpha( 128 );

    QPalette pal( window_text, background, light, dark_color, mid, text, base );
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

    if ( dark )
    {
        // a light blue that is more legible on a dark background than Qt::blue
        pal.setBrush( QPalette::Link, highlight );
    }

    return pal;
}
