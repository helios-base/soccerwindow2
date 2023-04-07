// -*-c++-*-

/*!
  \file formation_editor_painter.cpp
  \brief formation edit data painter class Source File.
*/

/*
 *Copyright:

 Copyright (C) Hidehisa AKIYAMA

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

#include "formation_editor_painter.h"

#include "draw_config.h"
#include "options.h"
#include "main_data.h"
#include "formation_edit_data.h"

using namespace rcsc;

/*-------------------------------------------------------------------*/
FormationEditorPainter::FormationEditorPainter( const MainData & main_data )
    : M_main_data( main_data ),
      M_triangle_pen( QColor( 255, 0, 0 ), 0, Qt::SolidLine )
{


}

/*-------------------------------------------------------------------*/
void
FormationEditorPainter::setData( std::weak_ptr< FormationEditData > data )
{
    M_edit_data = data;
}

/*-------------------------------------------------------------------*/
void
FormationEditorPainter::draw( QPainter & painter )
{
    const double scale = Options::instance().fieldScale();

    QTransform old_transform = painter.worldTransform();
    {
        M_transform.reset();
        M_transform.translate( painter.viewport().width() * 0.5 - Options::instance().focusPoint().x * scale,
                               painter.viewport().height() * 0.5 - Options::instance().focusPoint().y * scale );
        M_transform.scale( scale, scale );
        painter.setWorldTransform( M_transform );
    }

    drawBackgroundContainedTriangle( painter );
    drawContainedTriangle( painter );

    drawGoalieMovableArea( painter );
    drawShootLines( painter );
    drawFreeKickCircle( painter );

    drawBackgroundData( painter );

    drawTriangulation( painter );
    drawPlayers( painter );
    drawBall( painter );

    painter.setWorldTransform( old_transform );
}

/*-------------------------------------------------------------------*/
void
FormationEditorPainter::setAntialiasFlag( QPainter & painter,
                                          bool on )
{
#ifdef USE_HIGH_QUALITY_ANTIALIASING
    painter.setRenderHints( QPainter::HighQualityAntialiasing |
                            QPainter::Antialiasing,
                            on );
#else
    painter.setRenderHint( QPainter::Antialiasing, on );
#endif
}

/*-------------------------------------------------------------------*/
void
FormationEditorPainter::drawTriangulation( QPainter & painter )
{
    std::shared_ptr< FormationEditData > ptr = M_edit_data.lock();
    if ( ! ptr )
    {
        std::cerr << "(FormationEditorPainter::drawTriangulation) no edit data" << std::endl;
        return;
    }

    if ( Options::instance().antiAliasing() )
    {
        setAntialiasFlag( painter, false );
    }


    if ( Options::instance().feditShowTriangulation() )
    {

        const Triangulation::PointCont & points = ptr->triangulation().points();

        // triangulation

        painter.setPen( M_triangle_pen );
        painter.setBrush( Qt::NoBrush );

        for ( const Triangulation::Segment & e : ptr->triangulation().edges() )
        {
            painter.drawLine( QLineF( points[e.first].x,
                                      points[e.first].y,
                                      points[e.second].x,
                                      points[e.second].y ) );
        }
    }
    else
    {
        // only vertices

        const double r = M_transform.inverted().map( QLineF( 0.0, 0.0, 1.0, 0.0 ) ).length();
        const double d = r * 2.0;

        painter.setPen( M_triangle_pen );
        painter.setBrush( Qt::NoBrush );

        for ( const FormationData::Data & data : ptr->formationData()->dataCont() )
        {
            painter.drawRect( QRectF( data.ball_.x - r, data.ball_.y - r, d, d ) );
        }
    }


    if ( Options::instance().antiAliasing() )
    {
        setAntialiasFlag( painter, true );
    }
}

/*-------------------------------------------------------------------*/
void
FormationEditorPainter::drawContainedTriangle( QPainter & painter )
{

}

/*-------------------------------------------------------------------*/
void
FormationEditorPainter::drawBall( QPainter & painter )
{

}

/*-------------------------------------------------------------------*/
void
FormationEditorPainter::drawPlayers( QPainter & painter )
{

}


/*-------------------------------------------------------------------*/
void
FormationEditorPainter::drawBackgroundData( QPainter & painter )
{

}

/*-------------------------------------------------------------------*/
void
FormationEditorPainter::drawBackgroundContainedTriangle( QPainter & painter )
{

}

/*-------------------------------------------------------------------*/
void
FormationEditorPainter::drawGoalieMovableArea( QPainter & painter )
{

}

/*-------------------------------------------------------------------*/
void
FormationEditorPainter::drawShootLines( QPainter & painter )
{

}

/*-------------------------------------------------------------------*/
void
FormationEditorPainter::drawFreeKickCircle( QPainter & painter )
{

}
