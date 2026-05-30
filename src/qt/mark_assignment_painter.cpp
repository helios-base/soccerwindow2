// -*-c++-*-

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

#include "mark_assignment_painter.h"

#include "options.h"
#include "main_data.h"
#include "mark_cost_features_log.h"
#include "draw_config.h"

#include <set>

using namespace rcsc;

/*-------------------------------------------------------------------*/
MarkAssignmentPainter::MarkAssignmentPainter( const MainData & main_data )
    : M_main_data( main_data )
{

}

/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/

namespace {

/*-------------------------------------------------------------------*/

void
draw_arrow_head( QPainter & painter,
                 const QPointF & tip,
                 const QPointF & prev,
                 qreal size )
{
    const qreal angle = std::atan2( tip.y() - prev.y(), tip.x() - prev.x() );
    const qreal a1 = angle + M_PI - 0.5;
    const qreal a2 = angle + M_PI + 0.5;

    QPolygonF arrow;
    arrow << tip
          << QPointF( tip.x() + std::cos( a1 ) * size,
                      tip.y() + std::sin( a1 ) * size )
          << QPointF( tip.x() + std::cos( a2 ) * size,
                      tip.y() + std::sin( a2 ) * size );

    painter.drawPolygon( arrow );
}

void
draw_halo_ellipse( QPainter & painter,
                   const QPointF & center,
                   qreal radius,
                   const QColor & fill,
                   const QColor & outline )
{
    painter.setPen( QPen( outline, 2 ) );
    painter.setBrush( QBrush( fill ) );
    painter.drawEllipse( QRectF( center.x() - radius,
                                 center.y() - radius,
                                 radius * 2,
                                 radius * 2 ) );

    painter.setPen( QPen( outline.lighter( 150 ), 1 ) );
    painter.setBrush( Qt::NoBrush );
    painter.drawEllipse( QRectF( center.x() - radius - 2,
                                 center.y() - radius - 2,
                                 ( radius + 2 ) * 2,
                                 ( radius + 2 ) * 2 ) );
}

#if 1

void
draw_straight_assignment( QPainter & painter,
                        const QPointF & from,
                        const QPointF & to,
                        const QPen & pen )
{
    painter.setPen( pen );
    painter.setBrush( Qt::NoBrush );
    painter.drawLine( QLineF( from, to ) );

    const QPointF prev( ( from.x() + to.x() ) * 0.5,
                        ( from.y() + to.y() ) * 0.5 );
    draw_arrow_head( painter, to, prev, 20.0 );
}

#else

QPointF
perpendicular_offset( const QPointF & from,
                      const QPointF & to,
                      qreal bend )
{
    const qreal dx = to.x() - from.x();
    const qreal dy = to.y() - from.y();
    const qreal len = std::hypot( dx, dy );
    if ( len < 1.0 )
    {
        return QPointF( 0.0, 0.0 );
    }

    const qreal nx = -dy / len;
    const qreal ny =  dx / len;
    return QPointF( nx * bend, ny * bend );
}

void
draw_curved_assignment( QPainter & painter,
                        const QPointF & from,
                        const QPointF & to,
                        qreal bend,
                        const QPen & pen )
{
    const QPointF offset = perpendicular_offset( from, to, bend );
    const QPointF control( ( from.x() + to.x() ) * 0.5 + offset.x(),
                           ( from.y() + to.y() ) * 0.5 + offset.y() );

    QPainterPath path;
    path.moveTo( from );
    path.quadTo( control, to );

    painter.setPen( pen );
    painter.setBrush( Qt::NoBrush );
    painter.drawPath( path );

    const QPointF prev( ( control.x() + to.x() ) * 0.5,
                        ( control.y() + to.y() ) * 0.5 );
    draw_arrow_head( painter, to, prev, 20.0 );
}
#endif

}

/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/

/*-------------------------------------------------------------------*/
void
MarkAssignmentPainter::draw( QPainter & painter )
{
    const Options & opt = Options::instance();

    if ( ! opt.showMarkAssignmentView() )
    {
        return;
    }

    const MonitorViewData::ConstPtr view_data = M_main_data.getCurrentViewData();
    if ( ! view_data )    
    {
        return;
    }

    const DrawConfig & dconf = DrawConfig::instance();

    const std::vector< MarkAssignment > & assignments
        = M_main_data.markCostFeaturesLog().getAssignmentsAt( view_data->time() );

    // create target player set to draw all targets even if some of them are not assigned to any marker
    std::set< MarkTargetKey, MarkTargetKey::Less > target_player_set;
    for ( const MarkAssignment & assignment : assignments )
    {
        target_player_set.insert( assignment.target_ );
    }
    // draw all targets
    for ( const MarkTargetKey & target : target_player_set )
    {
        const double target_x = opt.screenX( target.pos_.x );
        const double target_y = opt.screenY( target.pos_.y );
        const double player_r = std::max( 2.0,  opt.scale( 1.0 ) );
        const QPointF target_pos( target_x, target_y );
        draw_halo_ellipse( painter, target_pos, player_r, QColor( "#FF7777" ), Qt::black );
        painter.setPen( QPen( Qt::red, 2 ) );
        painter.setFont( dconf.debugCommentFont() );
        painter.drawText( QPointF( target_x - player_r, target_y - player_r ),
                          QString( "T: %1" ).arg( target.unum_ ) );
    }


    for ( const MarkAssignment & assignment : assignments )
    {
    //     std::cerr << "(MarkAssignmentPainter::draw) assignment: assigned = " << assignment.assigned_
    //               << ", marker = (unum = " << assignment.marker_.unum_ << ", pos = " << assignment.marker_.pos_ << ")"
    //               << ", target = (id = " << assignment.target_.id_ << ", unum = " << assignment.target_.unum_ << ", pos = " << assignment.target_.pos_ << ")"
    //               << std::endl;
        if ( ! assignment.assigned_ ) 
        {
            continue;
        }

        const double marker_x = opt.screenX( assignment.marker_.pos_.x );
        const double marker_y = opt.screenY( assignment.marker_.pos_.y );
        const double target_x = opt.screenX( assignment.target_.pos_.x );
        const double target_y = opt.screenY( assignment.target_.pos_.y );
        const double player_r = std::max( 2.0,  opt.scale( 1.0 ) );
#if 1
        const QPointF marker_pos( marker_x, marker_y );
        const QPointF target_pos( target_x, target_y );
        draw_halo_ellipse( painter, marker_pos, player_r, QColor( "#DDDD00" ), Qt::black );
        //draw_halo_ellipse( painter, target_pos, player_r, QColor( "#FF7777" ), Qt::black );
        draw_straight_assignment( painter, marker_pos, target_pos, QPen( Qt::red, 4 ) );
        //draw_curved_assignment( painter, marker_pos, target_pos, 20.0, QPen( Qt::red, 2 ) );
#else
        painter.setPen( QPen( Qt::black, 2 ) );
        painter.setBrush( QBrush( "#DDDD00", Qt::SolidPattern ) );
        painter.drawEllipse( QRectF( marker_x - player_r, marker_y - player_r,
                                     player_r * 2, player_r * 2 ) );
        painter.setBrush( QBrush( "#FF7777", Qt::SolidPattern ) );
        painter.drawRect( QRectF( target_x - player_r, target_y - player_r,
                                  player_r * 2, player_r * 2 ) );
        painter.setPen( QPen( Qt::cyan, 2 ) );
        painter.drawLine( QLineF( marker_x, marker_y, target_x, target_y ) );
#endif
        painter.setPen( QPen( Qt::red, 2 ) );
        painter.setFont( dconf.debugCommentFont() );
        painter.drawText( QPointF( marker_x - player_r, marker_y - player_r ),
                          QString( "%1->%2" ).arg( assignment.marker_.unum_ ).arg( assignment.target_.unum_ ) );
        // painter.drawText( QPointF( target_x - player_r, target_y - player_r ),
        //                   QString( "T: %1" ).arg( assignment.target_.unum_ ) );
    }

}
