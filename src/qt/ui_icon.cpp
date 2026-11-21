// -*-c++-*-

/*!
  \file ui_icon.cpp
  \brief icons adapted to the palette Source File.
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

#include "ui_icon.h"

#include <QApplication>
#include <QIconEngine>
#include <QPainter>
#include <QPalette>
#include <QPixmap>
#include <QStyle>
#include <QStyleOption>

namespace {

//! the maximum HSL saturation of the pixels to be replaced
const int MAX_SATURATION = 60;
//! the maximum HSL lightness of the pixels to be replaced
const int MAX_LIGHTNESS = 110;

/*-------------------------------------------------------------------*/
/*!
  \class AdaptiveXpmIconEngine
  \brief icon engine that adapts the colors of an XPM image to the palette
*/
class AdaptiveXpmIconEngine
    : public QIconEngine {
private:
    QPixmap M_pixmap; //!< the original image

public:

    explicit
    AdaptiveXpmIconEngine( const char * const * xpm )
        : QIconEngine(),
          M_pixmap( xpm )
      { }

    AdaptiveXpmIconEngine( const AdaptiveXpmIconEngine & other )
        : QIconEngine( other ),
          M_pixmap( other.M_pixmap )
      { }

    QSize actualSize( const QSize & size,
                      QIcon::Mode,
                      QIcon::State ) override
      {
          // never larger than the original image (the same as QIcon(QPixmap))
          QSize actual = M_pixmap.size();
          if ( actual.width() > size.width()
               || actual.height() > size.height() )
          {
              actual.scale( size, Qt::KeepAspectRatio );
          }
          return actual;
      }

    QPixmap pixmap( const QSize & size,
                    QIcon::Mode mode,
                    QIcon::State state ) override
      {
          const QPalette palette = QGuiApplication::palette();

          QPixmap pix = QPixmap::fromImage( UIIcon::adaptToPalette( M_pixmap.toImage(), palette ) );

          const QSize actual = actualSize( size, mode, state );
          if ( pix.size() != actual )
          {
              pix = pix.scaled( actual, Qt::KeepAspectRatio, Qt::SmoothTransformation );
          }

          if ( mode == QIcon::Disabled
               || mode == QIcon::Selected )
          {
              if ( QStyle * style = QApplication::style() )
              {
                  QStyleOption opt( 0 );
                  opt.palette = palette;
                  pix = style->generatedIconPixmap( mode, pix, &opt );
              }
          }

          return pix;
      }

    void paint( QPainter * painter,
                const QRect & rect,
                QIcon::Mode mode,
                QIcon::State state ) override
      {
          painter->drawPixmap( rect, pixmap( rect.size(), mode, state ) );
      }

    QIconEngine * clone() const override
      {
          return new AdaptiveXpmIconEngine( *this );
      }

    QString key() const override
      {
          return QStringLiteral( "AdaptiveXpmIconEngine" );
      }
};

}

/*-------------------------------------------------------------------*/
/*!

*/
QIcon
UIIcon::fromXpm( const char * const * xpm )
{
    return QIcon( new AdaptiveXpmIconEngine( xpm ) );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
UIIcon::isDark( const QPalette & palette )
{
    return palette.color( QPalette::Window ).lightness() < 128;
}

/*-------------------------------------------------------------------*/
/*!

*/
QImage
UIIcon::adaptToPalette( const QImage & image,
                        const QPalette & palette )
{
    if ( ! isDark( palette ) )
    {
        return image;
    }

    const QColor text_color = palette.color( QPalette::WindowText );

    QImage result = image.convertToFormat( QImage::Format_ARGB32 );
    for ( int y = 0; y < result.height(); ++y )
    {
        for ( int x = 0; x < result.width(); ++x )
        {
            const QColor c = result.pixelColor( x, y );
            if ( c.alpha() > 0
                 && c.hslSaturation() < MAX_SATURATION
                 && c.lightness() < MAX_LIGHTNESS )
            {
                QColor n = text_color;
                n.setAlpha( c.alpha() );
                result.setPixelColor( x, y, n );
            }
        }
    }

    return result;
}
