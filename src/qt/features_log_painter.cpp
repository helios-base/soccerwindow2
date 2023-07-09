// -*-c++-*-

/*!
  \file features_log_painter.cpp
  \brief features log painter class Source File.
*/

/*
 *Copyright:

 Copyright (C) Hidehisa Akiyama

 This code is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3, or (at your option)
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

#include <QtGlobal>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtWidgets>
#else
#include <QtGui>
#endif

#include "features_log_painter.h"

#include "options.h"
#include "main_data.h"
#include "features_log.h"

#include <iostream>

namespace {

const double selected_width = 4.0;

/*-------------------------------------------------------------------*/
void
draw_texts( QPainter & painter,
            const DrawTextCont & cont,
            const bool selected )
{
    const Options & opt = Options::instance();

    for ( const auto & t : cont )
    {
        QColor col( t.color_.c_str() );
        QPen pen( ( col.isValid() ? col : Qt::white ) );
        if ( selected )
        {
            painter.save();
            QFont font = painter.font();
            font.setWeight( QFont::Bold );
            painter.setFont( font );
        }

        painter.setPen( pen );
        painter.drawText( QPointF( opt.screenX( t.x_ ),
                                   opt.screenY( t.y_ ) ),
                          QString::fromStdString( t.msg_ ) );

        if ( selected )
        {
            painter.restore();
        }
    }
}

/*-------------------------------------------------------------------*/
void
draw_points( QPainter & painter,
             const DrawPointCont & cont,
             const bool selected )
{
    const Options & opt = Options::instance();

    for ( const auto & p : cont )
    {
        QColor col( p.color_.c_str() );
        QPen pen( col.isValid() ? col : Qt::white );
        if ( selected ) pen.setWidth( selected_width );

        painter.setPen( pen );
        painter.drawPoint( QPointF( opt.screenX( p.x_ ),
                                    opt.screenY( p.y_ ) ) );
    }
}

/*-------------------------------------------------------------------*/
void
draw_lines( QPainter & painter,
            const DrawLineCont & cont,
            const bool selected )
{
    const Options & opt = Options::instance();

    for ( const auto & l : cont )
    {
        QColor col( l.color_.c_str() );
        QPen pen( col.isValid() ? col : Qt::white );
        if ( selected ) pen.setWidth( selected_width );

        painter.setPen( pen );
        painter.drawLine( QLineF( opt.screenX( l.x1_ ),
                                  opt.screenY( l.y1_ ),
                                  opt.screenX( l.x2_ ),
                                  opt.screenY( l.y2_ ) ) );
    }
}

/*-------------------------------------------------------------------*/
void
draw_rects( QPainter & painter,
            const DrawRectCont & cont,
            const bool selected )
{
    const Options & opt = Options::instance();

    for ( const auto & r : cont )
    {
        QColor line_col( r.line_color_.c_str() );
        QColor fill_col( r.fill_color_.c_str() );

        QPen pen( ( line_col.isValid() ? line_col : Qt::white ) );
        if ( selected ) pen.setWidth( selected_width );

        painter.setPen( pen );

        if ( fill_col.isValid() )
        {
            painter.setBrush( fill_col );
        }
        else
        {
            painter.setBrush( Qt::NoBrush );
        }

        painter.drawRect( QRectF( opt.screenX( r.left_ ),
                                  opt.screenY( r.top_ ),
                                  opt.scale( r.width_ ),
                                  opt.scale( r.height_ ) ) );
    }
}

/*-------------------------------------------------------------------*/
void
draw_circles( QPainter & painter,
              const DrawCircleCont & cont,
              const bool selected )
{
    const Options & opt = Options::instance();

    for ( const auto & c : cont )
    {
        QColor line_col( c.line_color_.c_str() );
        QColor fill_col( c.fill_color_.c_str() );

        QPen pen( ( line_col.isValid() ? line_col : Qt::white ) );
        if ( selected ) pen.setWidth( selected_width );

        painter.setPen( pen );

        if ( fill_col.isValid() )
        {
            painter.setBrush( fill_col );
        }
        else
        {
            painter.setBrush( Qt::NoBrush );
        }

        double r = opt.scale( c.r_ );
        painter.drawEllipse( QPointF( opt.screenX( c.x_ ),
                                      opt.screenY( c.y_ ) ),
                             r, r );
    }
}

}


/*-------------------------------------------------------------------*/
/*!

*/
FeaturesLogPainter::FeaturesLogPainter( const MainData & main_data )
    : M_main_data( main_data )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
void
FeaturesLogPainter::draw( QPainter & painter )
{
    const Options & opt = Options::instance();

    if ( ! opt.showDrawData() )
    {
        return;
    }

    MonitorViewData::ConstPtr monitor_view = M_main_data.getCurrentViewData();
    if ( ! monitor_view )
    {
        return;
    }

    FeaturesLog::ConstPtr features_log = M_main_data.featuresLog();
    if ( ! features_log )
    {
        return;
    }

    FeaturesLog::Map::const_iterator it = features_log->timedMap().find( monitor_view->time() );
    if ( it == features_log->timedMap().end() )
    {
        return;
    }

    for ( const Features::Ptr & f : it->second->featuresList() )
    {
        if ( f && f->drawData() )
        {
            const bool selected = ( M_main_data.selectedFeaturesIndex() == f->index() );

            draw_rects( painter, f->drawData()->rects_, selected );
            draw_circles( painter, f->drawData()->circles_, selected );
            draw_lines( painter, f->drawData()->lines_, selected );
            draw_texts( painter, f->drawData()->texts_, selected );
            draw_points( painter, f->drawData()->points_, selected );
        }
    }

}
