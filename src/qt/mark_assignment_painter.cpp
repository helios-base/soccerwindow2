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

void
draw_text_with_background( QPainter & painter,
                            const QPointF & baseline_pos,
                            const QString & text,
                            const QColor & text_color )
{
    const QFontMetrics fm( painter.font() );
    const QRect text_rect = fm.boundingRect( text );
    const int pad = 2;
    // Conversion of boundingRect from relative to absolute coordinates,
    // since boundingRect is relative to baseline-left
    const QRectF bg_rect( baseline_pos.x() + text_rect.x() - pad,
                          baseline_pos.y() + text_rect.y() - pad,
                          text_rect.width()  + pad * 2,
                          text_rect.height() + pad * 2 );

    // Background plate (semi-transparent black with thin white border)
    painter.setPen( QPen( QColor( 255, 255, 255, 120 ), 1 ) );
    painter.setBrush( QColor( 0, 0, 0, 160 ) );
    painter.drawRoundedRect( bg_rect, 2, 2 );

    // Text
    painter.setPen( QPen( text_color ) );
    painter.setBrush( Qt::NoBrush );
    painter.drawText( baseline_pos, text );
}
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

    const double player_r = ( opt.enlargeMode()
                              ? std::max( 2.0,  opt.scale( 1.0 ) )
                              : std::max( 2.0,  opt.scale( 0.3 ) ) );

    const DrawConfig & dconf = DrawConfig::instance();

    const MarkAssignmentGroup & group = M_main_data.markAssignmentData().log().getAssignmentGroupAt( view_data->time() );

    // draw ball if the ball position is known in the log, to make it easier to understand the marking assignments in relation to the ball position
    if ( group.ball_pos_.isValid() )
    {
        const double ball_x = opt.screenX( group.ball_pos_.x );
        const double ball_y = opt.screenY( group.ball_pos_.y );
        const QPointF ball_pos( ball_x, ball_y );
        painter.setPen( QPen( QColor( 255, 255, 0, 128 ), 3 ) );
        painter.setBrush( QColor( 255, 255, 255, 128 ) );
        painter.drawEllipse( ball_pos, player_r + 4, player_r + 4 );
        // painter.setFont( dconf.debugCommentFont() );
        // draw_text_with_background( painter, QPointF( ball_x - player_r, ball_y - player_r ), QString( "Ball" ), Qt::yellow );
    }

    const std::set< std::pair< int, char > > & hilighted_set = M_main_data.markAssignmentData().highlightedAssignments();

    //
    // draw targets first to make them appear below markers and assignment lines
    //
    // create target player set to draw all targets even if some of them are not assigned to any marker
    std::set< Marker, Marker::Less > marker_set;
    std::set< MarkTargetKey, MarkTargetKey::Less > target_player_set;
    for ( const MarkAssignment & assignment : group.assignments_ )
    {
        marker_set.insert( assignment.marker_ );
        target_player_set.insert( assignment.target_ );
    }
    // draw all targets
    for ( const MarkTargetKey & target : target_player_set )
    {
        const double target_x = opt.screenX( target.pos_.x );
        const double target_y = opt.screenY( target.pos_.y );
        const QPointF target_pos( target_x, target_y );
        // QColor fill_color( "#FF7777" );
        // fill_color.setAlpha( 128 );
        draw_halo_ellipse( painter, target_pos, player_r, 
                           QColor( 255, 11, 119, 128 ), Qt::black );
        painter.setPen( QPen( Qt::red, 2 ) );
        painter.setFont( dconf.debugCommentFont() );
        const QString label_text = ( target.unum_ > 0 )
            ? QString( "T%1" ).arg( target.unum_ )
            : QString( "T(%1,%2)" ).arg( target.pos_.x, 0, 'f', 1 ).arg( target.pos_.y, 0, 'f', 1 );
        draw_text_with_background( painter, QPointF( target_x - player_r, target_y - player_r ), label_text, Qt::red );
    }

    //
    // draw all markers (including unassigned ones) to make them appear above targets
    //
    for ( const Marker & marker : marker_set )
    {
        const double marker_x = opt.screenX( marker.pos_.x );
        const double marker_y = opt.screenY( marker.pos_.y );
        const QPointF marker_pos( marker_x, marker_y );
        // QColor fill_color( "#DDDD00" );
        // fill_color.setAlpha( 128 );
        draw_halo_ellipse( painter, marker_pos, player_r,
                           QColor( 221, 221, 0, 128 ), Qt::black );
        painter.setPen( QPen( Qt::yellow, 2 ) );
        painter.setFont( dconf.debugCommentFont() );
        draw_text_with_background( painter, QPointF( marker_x - player_r, marker_y - player_r ),
                                   QString( "M%1" ).arg( marker.unum_ ), Qt::yellow );
    }

    //
    // draw  assignment lines
    //
    for ( const MarkAssignment & assignment : group.assignments_ )
    {
        const bool is_highlighted = hilighted_set.count( { assignment.marker_.unum_, assignment.target_.id_ } ) > 0;

        const double move_point_x = opt.screenX( assignment.move_point_.x );
        const double move_point_y = opt.screenY( assignment.move_point_.y );
        const double marker_x = opt.screenX( assignment.marker_.pos_.x );
        const double marker_y = opt.screenY( assignment.marker_.pos_.y );
        const double target_x = opt.screenX( assignment.target_.pos_.x );
        const double target_y = opt.screenY( assignment.target_.pos_.y );

        const QPointF marker_pos( marker_x, marker_y );
        const QPointF target_pos( target_x, target_y );
        const QPointF move_point( move_point_x, move_point_y );

        // draw highlighted move_point for any selected cell (regardless of assigned_)
        if ( is_highlighted
             && assignment.move_point_.isValid() )
        {
            painter.setPen( QPen( QColor( 255, 165, 0 ), 3 ) );
            painter.setBrush( QColor( 255, 165, 0, 100 ) );
            painter.drawEllipse( move_point, player_r * 1.5, player_r * 1.5 );

            if ( ! assignment.assigned_ )
            {
                // draw line from marker to move_point to indicate the association between them,
                // since the move_point is drawn with halo and may be visually separated from the marker
               painter.setPen( QPen( QColor( 255, 165, 0 ), 2 ) );
               painter.drawLine( marker_pos, move_point );
               painter.drawLine( move_point, target_pos );
            }
        }

        if ( ! assignment.assigned_ )
        {
            continue;
        }


        // draw move_point as a small semi-transparent rectangle to indicate the point
        // that the marker is supposed to move to when marking the target
        painter.setPen( QPen( Qt::black, 2 ) );
        painter.setBrush( QColor( 255, 11, 119, 128 ) );
        painter.drawRect( QRectF( move_point.x() - player_r*0.5, move_point.y() - player_r*0.5,
                                  player_r, player_r ) );
        // draw line from move_point to target_pos to associate move_point with target_pos
        painter.setPen( QPen( QColor( 255, 11, 119 ) ) );
        painter.drawLine( move_point, target_pos );

        //draw_straight_assignment( painter, marker_pos, target_pos, QPen( Qt::blue, 4 ) );
        if ( is_highlighted )
        {
            draw_straight_assignment( painter, marker_pos, move_point, QPen( QColor( 255, 165, 0 ), 6 ) );
        }
        else
        {
            draw_straight_assignment( painter, marker_pos, move_point, QPen( Qt::blue, 4 ) );
        }
        //draw_curved_assignment( painter, marker_pos, target_pos, 20.0, QPen( Qt::blue, 2 ) );

#if 0
        QPointF unit = move_point - marker_pos;
        const qreal len = std::hypot( unit.x(), unit.y() );
        if ( len > 0.001 )
        {
            unit /= len;
        }
        QPointF normal( -unit.y(), unit.x() );
        QPointF label_pos = marker_pos + unit * ( player_r + std::min( 1.0, len * 0.1 ) ) + normal * 10.0;
#else
        QPointF label_pos( move_point.x() + player_r, move_point.y() - player_r );
#endif

        painter.setPen( QPen( Qt::cyan, 2 ) );
        painter.setFont( dconf.debugCommentFont() );
        const QString label_text = ( assignment.target_.unum_ > 0 ) 
            ? QString( "M%1->T%2" ).arg( assignment.marker_.unum_ ).arg( assignment.target_.unum_ ) 
            : QString( "M%1->T(%2,%3)" ).arg( assignment.marker_.unum_ )
              .arg( assignment.target_.pos_.x, 0, 'f', 1 )
              .arg( assignment.target_.pos_.y, 0, 'f', 1 );
        draw_text_with_background( painter, label_pos, label_text, Qt::cyan );
    }
}
