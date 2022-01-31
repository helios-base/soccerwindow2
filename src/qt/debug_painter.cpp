// -*-c++-*-

/*!
  \file debug_painter.cpp
  \brief debug info painter class Source File.
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

#include "debug_painter.h"

#include "draw_config.h"
// model
#include "options.h"
#include "main_data.h"

#include <rcsc/common/server_param.h>
#include <rcsc/geom/angle_deg.h>

#include <iostream>
#include <cmath>

namespace {

/*-------------------------------------------------------------------*/
/*!

*/
void
draw_communication( QPainter & painter,
                    const rcsc::SideID self_side,
                    const MonitorViewData & monitor_view,
                    const DebugViewData::Cont & debug_view_cont,
                    const DebugViewData & debug_view )
{
    const Options & opt = Options::instance();
    const DrawConfig & dconf = DrawConfig::instance();

    const double reverse = ( self_side == rcsc::RIGHT
                             ? -1.0
                             : 1.0 );

    const double say_r = std::max( 3.0, opt.scale( 3.5 ) );
    const double hear_r = std::max( 2.0, opt.scale( 3.0 ) );

    //
    // draw say message sender marks
    //

    painter.setPen( DrawConfig::instance().debugCommentFontPen() );
    painter.setBrush( DrawConfig::instance().transparentBrush() );

    for ( size_t i = 0; i < debug_view_cont.size(); ++i )
    {
        const DebugViewData::ConstPtr v = debug_view_cont[i];
        if ( ! v ) continue;
        if ( v->sayMessage().empty() ) continue;

        const std::shared_ptr< DebugViewData::SelfT > self = v->self();
        if ( ! self ) continue;

        const double ix = opt.screenX( self->x() * reverse );
        const double iy = opt.screenY( self->y() * reverse );

        painter.drawEllipse( QRectF( ix - say_r, iy - say_r, say_r * 2, say_r * 2 ) );
    }

    //
    // draw heard message sender marks
    //
    if ( ! debug_view.hearMessages().empty() )
    {
        const std::shared_ptr< DebugViewData::SelfT > self = debug_view.self();
        if ( ! self )
        {
            return;
        }

        const QPointF self_pos( opt.screenX( self->x() * reverse ),
                                opt.screenY( self->y() * reverse ) );

        const std::vector< rcsc::rcg::PlayerT > & players = monitor_view.players();

        //painter.setPen( dconf.debugShapePen() );
        painter.setPen( Qt::darkMagenta );
        painter.setBrush( dconf.transparentBrush() );

        for ( std::map< int, std::string >::const_reference hear : debug_view.hearMessages() )
        {
            for ( const rcsc::rcg::PlayerT & p : players )
            {
                if ( p.side() == self_side
                     && p.unum() == hear.first )
                {
                    QPointF pos( opt.screenX( p.x() ),
                                 opt.screenY( p.y() ) );

                    painter.drawEllipse( QRectF( pos.x() - hear_r,
                                                 pos.y() - hear_r,
                                                 hear_r * 2,
                                                 hear_r * 2 ) );
                    painter.drawLine( QLineF( pos, self_pos ) );
                    break;
                }
            }
        }
    }
}

}

/*-------------------------------------------------------------------*/
/*!

*/
void
DebugPainter::draw( QPainter & painter )
{
    const Options & opt = Options::instance();

    if ( ! opt.showDebugView() )
    {
        return;
    }

    int number = opt.selectedNumber();
    if ( number == 0 )
    {
        return;
    }

    const rcsc::SideID self_side = ( number < 0 ? rcsc::RIGHT : rcsc::LEFT );

    const DebugViewData::Map & view_map
        = ( self_side == rcsc::RIGHT
            ? M_main_data.viewHolder().rightDebugView()
            : M_main_data.viewHolder().leftDebugView() );

    MonitorViewData::ConstPtr monitor_view = M_main_data.getCurrentViewData();
    if ( ! monitor_view )
    {
        return;
    }

    rcsc::GameTime time = monitor_view->time();
    if( monitor_view->playmode() == rcsc::PM_BeforeKickOff )
    {
        time.setStopped( 0 );
    }

    DebugViewData::Map::const_iterator it = view_map.find( time );

    if ( it == view_map.end() )
    {
        // std::cerr << __FILE__ << ": (draw) "
        //           << "not found. monitor view time=" << monitor_view->time()
        //           << std::endl;
        return;
    }

    number = std::abs( number );

    if ( static_cast< int >( it->second.size() ) < number )
    {
        std::cerr << __FILE__ << ": (draw) "
                  << " Error debug view container size is small."
                  << std::endl;
        return;
    }

    //
    // draw selected player's debug data
    //

    const DebugViewData::ConstPtr view = it->second[ number - 1 ];
    if ( ! view )
    {
        // no data!
        // std::cerr << __FILE__ << ": (draw) "
        //          << " Error. No debug view data. number=" << number
        //          << std::endl;
        return;
    }

    const DrawConfig & dconf = DrawConfig::instance();

    if ( number != 12 )
    {
        // draw self
        if ( opt.showDebugViewSelf() )
        {
            drawSelf( painter, self_side, *view );
        }
    }

    // draw players
    if ( opt.showDebugViewTeammates() )
    {
        drawPlayers( painter,
                     self_side,
                     view->teammates(),
                     dconf.debugTeammateBrush() );
        drawPlayers( painter,
                     self_side,
                     view->unknownTeammates(),
                     dconf.debugUnknownTeammateBrush() );
        if ( number != 12 )
        {
            draw_communication( painter,
                                self_side,
                                *monitor_view,
                                it->second,
                                *view );
        }
    }

    if ( opt.showDebugViewOpponents() )
    {
        drawPlayers( painter,
                     self_side,
                     view->opponents(),
                     dconf.debugOpponentBrush() );
        drawPlayers( painter,
                     self_side,
                     view->unknownOpponents(),
                     dconf.debugUnknownOpponentBrush() );
        drawPlayers( painter,
                     self_side,
                     view->unknownPlayers(),
                     dconf.debugUnknownPlayerBrush() );
    }

    // draw ball
    if ( opt.showDebugViewBall()
         && view->ball() )
    {
        drawBall( painter, self_side, *view );
    }

    // draw targets
    drawTarget( painter, self_side, *view );

    // draw shapes
    if ( opt.showDebugViewShape() )
    {
        if ( opt.antiAliasing() )
        {
#ifdef USE_HIGH_QUALITY_ANTIALIASING
            painter.setRenderHints( QPainter::HighQualityAntialiasing |
                                    QPainter::Antialiasing,
                                    false );
#else
            painter.setRenderHint( QPainter::Antialiasing, false );
#endif
        }

        drawLines( painter, self_side, *view );
        drawTriangles( painter, self_side, *view );
        drawRectangles( painter, self_side, *view );
        drawCircles( painter, self_side, *view );

        if ( opt.antiAliasing() )
        {
#ifdef USE_HIGH_QUALITY_ANTIALIASING
            painter.setRenderHints( QPainter::HighQualityAntialiasing |
                                    QPainter::Antialiasing,
                                    true );
#else
            painter.setRenderHint( QPainter::Antialiasing, true );
#endif
        }
    }

    if ( opt.showDebugViewMessage() )
    {
        drawMessage( painter, *view );
    }
}

/*-------------------------------------------------------------------*/
/*!
  pos, vel, body, neck, comment
*/
void
DebugPainter::drawSelf( QPainter & painter,
                        const rcsc::SideID self_side,
                        const DebugViewData & view ) const
{
    const std::shared_ptr< DebugViewData::SelfT > self = view.self();
    if ( ! self )
    {
        std::cerr << __FILE__ << ": (drawSelf) No self data!" << std::endl;
        return;
    }

    const Options & opt = Options::instance();
    const DrawConfig & dconf = DrawConfig::instance();
    const rcsc::PlayerType & ptype = M_main_data.viewHolder().playerType( self->ptype_ );


    const double reverse = ( self_side == rcsc::RIGHT
                             ? -1.0
                             : 1.0 );
    const double sx = opt.screenX( self->x() * reverse );
    const double sy = opt.screenY( self->y() * reverse );
    const double kr = std::max( 2.0, opt.scale( ptype.kickableArea() ) );
    const double r = std::max( 2.0,
                               ( opt.enlargeMode()
                                 ? kr
                                 : opt.scale( ptype.playerSize() ) ) );

    // // draw target
    // if ( opt.showDebugViewTarget()
    //      && view.targetPoint() )
    // {
    //     double tx = opt.screenX( view.targetPoint()->x * reverse );
    //     double ty = opt.screenY( view.targetPoint()->y * reverse );

    //     painter.setPen( dconf.debugTargetPen() );
    //     painter.setBrush( dconf.transparentBrush() );

    //     painter.drawLine( QLineF( tx - r, ty - r,
    //                               tx + r, ty + r ) );
    //     painter.drawLine( QLineF( tx - r, ty + r,
    //                               tx + r, ty - r ) );
    // }

    // draw body half circle
    double body_start_dir = - ( self->body() + 90.0 );
    if ( opt.reverseSide() ) body_start_dir += 180.0;
    if ( self_side == rcsc::RIGHT ) body_start_dir -= 180.0;

    painter.setPen( dconf.transparentPen() );
    painter.setBrush( dconf.debugSelfBrush() );

    painter.drawPie( QRectF( sx - r,
                             sy - r,
                             r * 2,
                             r * 2 ),
                     static_cast< int >( rint( body_start_dir * 16 ) ),
                     180 * 16 );

    // draw edge

    painter.setPen( dconf.debugPlayerPen() );
    painter.setBrush( dconf.transparentBrush() );

    painter.drawEllipse( QRectF( sx - r,
                                 sy - r,
                                 r * 2,
                                 r * 2 ) );

    if ( ! opt.enlargeMode() )
    {
        painter.drawEllipse( QRectF( sx - kr,
                                     sy - kr,
                                     kr * 2,
                                     kr * 2 ) );
    }

    // draw face angle
    {
        rcsc::AngleDeg face_angle = self->body() + self->neck();
        if ( opt.reverseSide() ) face_angle += 180.0;

        double face_r = 3.0;
        double end_x = sx + opt.scale( face_r * face_angle.cos() );
        double end_y = sy + opt.scale( face_r * face_angle.sin() );

        painter.setPen( dconf.viewConePen() );
        painter.setBrush( dconf.transparentBrush() );

        painter.drawLine( QLineF( sx, sy, end_x, end_y ) );
    }

    // draw comment
    if ( opt.showDebugViewComment()
         && ! self->comment_.empty() )
    {
        painter.setFont( dconf.debugCommentFont() );
        painter.setPen( dconf.debugCommentFontPen() );

        painter.drawText( QPointF( sx - r,
                                   sy + r + painter.fontMetrics().ascent() ),
                          QString::fromStdString( self->comment_ ) );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
DebugPainter::drawBall( QPainter & painter,
                        const rcsc::SideID self_side,
                        const DebugViewData & view ) const
{
    const std::shared_ptr< DebugViewData::BallT > & ball = view.ball();

    if ( ! ball )
    {
        return;
    }

    const Options & opt = Options::instance();
    const DrawConfig & dconf = DrawConfig::instance();

    const double reverse = ( self_side == rcsc::RIGHT
                             ? -1.0
                             : 1.0 );

    rcsc::Vector2D bpos( ball->x() * reverse,
                         ball->y() * reverse );
    rcsc::Vector2D bvel( ball->vx() * reverse,
                         ball->vy() * reverse );

    const double ball_decay = rcsc::ServerParam::i().ballDecay();
    const QPointF p( opt.screenX( bpos.x ),
                     opt.screenY( bpos.y ) );
    const double r = std::max( 2.0,
                               ( opt.enlargeMode()
                                 ? opt.scale( 0.25 )
                                 : opt.scale( 0.07 ) ) );

    painter.setPen( dconf.transparentPen() );
    painter.setBrush( dconf.debugBallBrush() );

    painter.drawEllipse( QRectF( p.x() - r, p.y() - r,
                                 r * 2, r * 2 ) );


    if ( opt.antiAliasing() )
    {
#ifdef USE_HIGH_QUALITY_ANTIALIASING
        painter.setRenderHints( QPainter::HighQualityAntialiasing |
                                QPainter::Antialiasing,
                                false );
#else
        painter.setRenderHint( QPainter::Antialiasing, false );
#endif
    }

#ifdef USE_GLWIDGET
    {
        painter.setPen( dconf.debugBallTracePen() );
        painter.setBrush( dconf.debugBallTraceBrush() );

        QPointF b = p;
        for ( int i = 0; i < 10; ++i )
        {
            bpos += bvel;
            bvel *= ball_decay;
            b.setX( opt.screenX( bpos.x ) );
            b.setY( opt.screenY( bpos.y ) );
            painter.drawEllipse( QRectF( b.x() - 2, b.y() - 2, 4, 4 ) );
        }

        painter.drawLine( QLineF( p, b ) );
    }
#else
    {
        QPainterPath path;
        painter.setPen( dconf.debugBallTracePen() );
        painter.setBrush( dconf.debugBallTraceBrush() );

        //QPointF b = p;
        double bx = p.x();
        double by = p.y();

        const double len = r * 0.8;
        const double half = len * 0.5;

        for ( int i = 0; i < 10; ++i )
        {
            bpos += bvel;
            bvel *= ball_decay;
            // b.setX( opt.screenX( bpos.x ) );
            // b.setY( opt.screenY( bpos.y ) );
            // path.addEllipse( b, rr, rr );
            bx = opt.screenX( bpos.x );
            by = opt.screenY( bpos.y );
            path.addEllipse( bx - half, by - half, len, len );
        }
        path.moveTo( p );
        path.lineTo( QPointF( bx, by ) );

        painter.drawPath( path );
    }
#endif


    if ( opt.antiAliasing() )
    {
#ifdef USE_HIGH_QUALITY_ANTIALIASING
        painter.setRenderHints( QPainter::HighQualityAntialiasing |
                                QPainter::Antialiasing,
                                true );
#else
        painter.setRenderHint( QPainter::Antialiasing, true );
#endif
    }

    // draw comment
    if ( opt.showDebugViewComment()
         && ! ball->comment_.empty() )
    {
        painter.setFont( dconf.debugCommentFont() );
        painter.setPen( dconf.debugCommentFontPen() );

        painter.drawText( QPointF( p.x() - r,
                                   p.y() + r + painter.fontMetrics().ascent() ),
                          QString::fromStdString( ball->comment_ ) );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
DebugPainter::drawPlayers( QPainter & painter,
                           const rcsc::SideID self_side,
                           // const DebugViewData & view,
                           const DebugViewData::PlayerCont & players,
                           // const int target_number,
                           const QBrush & body_brush ) const
{
    const Options & opt = Options::instance();
    const DrawConfig & dconf = DrawConfig::instance();

    const double reverse = ( self_side == rcsc::RIGHT
                             ? -1.0
                             : 1.0 );
    const bool comment = opt.showDebugViewComment();
    // const double r = std::max( 2.0,
    //                          ( opt.enlargeMode()
    //                            ? opt.scale( 1.0 )
    //                            : opt.scale( 0.3 ) ) );

    for ( DebugViewData::PlayerCont::const_reference pl : players )
    {
        const rcsc::PlayerType & ptype = M_main_data.viewHolder().playerType( pl->ptype_ );
        const double kr = std::max( 2.0, opt.scale( ptype.kickableArea() ) );
        const double r = std::max( 2.0,
                                   ( opt.enlargeMode()
                                     ? kr
                                     : opt.scale( ptype.playerSize() ) ) );

        const QPointF pos( opt.screenX( pl->x() * reverse ),
                           opt.screenY( pl->y() * reverse ) );

        if ( pl->hasBody() )
        {
            double body_start_dir = - ( pl->body() + 90.0 );
            if ( opt.reverseSide() ) body_start_dir += 180.0;
            if ( self_side == rcsc::RIGHT ) body_start_dir -= 180.0;
            //double body_end_dir = body_start_dir + 180.0;
            // draw body half circle
            painter.setPen( dconf.transparentPen() );
            painter.setBrush( body_brush );
            painter.drawPie( QRectF( pos.x() - r, pos.y() - r,
                                     r * 2, r * 2 ),
                             static_cast< int >( rint( body_start_dir * 16 ) ),
                             180 * 16 );
            // draw edge
            painter.setPen( dconf.debugPlayerPen() );
            painter.setBrush( dconf.transparentBrush() );
            painter.drawEllipse( QRectF( pos.x() - r, pos.y() - r, r * 2, r * 2 ) );
        }
        else
        {
            // draw simple circle
            painter.setPen( dconf.transparentPen() );
            painter.setBrush( body_brush );
            painter.drawEllipse( QRectF( pos.x() - r, pos.y() - r, r * 2, r * 2 ) );
        }

        if ( ! opt.enlargeMode() )
        {
            painter.setPen( dconf.debugPlayerPen() );
            painter.setBrush( dconf.transparentBrush() );
            painter.drawEllipse( QRectF( pos.x() - kr, pos.y() - kr, kr * 2, kr * 2 ) );
        }

        if ( pl->hasPointto() )
        {
            double pointto_angle = pl->pointto();

            QPointF pointto_pos( opt.screenX( ( pl->x() + 5.0 * rcsc::AngleDeg::cos_deg( pointto_angle ) ) * reverse ),
                                 opt.screenY( ( pl->y() + 5.0 * rcsc::AngleDeg::sin_deg( pointto_angle ) ) * reverse ) );

            painter.setPen( dconf.debugPointtoPen() );
            painter.setBrush( dconf.transparentBrush() );
            painter.drawLine( pos, pointto_pos );
        }

        if ( pl->unum_ > 0 )
        {
            painter.setFont( dconf.debugCommentFont() );
            painter.setPen( dconf.debugCommentFontPen() );

            painter.drawText( QPointF( pos.x() + r, pos.y() + 4 ),
                              //QString::number( pl->unum_ ) );
                              QString( "%1t%2" ).arg( pl->unum_ ).arg( pl->ptype_ ) );
        }

        // if ( target_number == pl->unum_ )
        // {
        //     painter.setPen( dconf.debugTargetPen() );
        //     painter.setBrush( dconf.transparentBrush() );

        //     const std::shared_ptr< DebugViewData::SelfT > self = view.self();
        //     if ( self )
        //     {
        //         QPointF s( opt.screenX( (double)self->x_ / rcsc::rcg::SHOWINFO_SCALE2 * reverse ),
        //                    opt.screenY( (double)self->y_ / rcsc::rcg::SHOWINFO_SCALE2 * reverse ) );
        //         painter.drawLine( QLineF( s, p ) );
        //     }

        //     double rr = r + 2;
        //     painter.drawEllipse( QRectF( pos.x() - rr, pos.y() - rr,
        //                                  rr * 2, rr * 2 ) );
        // }

        if ( comment
             && ! pl->comment_.empty() )
        {
            painter.setFont( dconf.debugCommentFont() );
            painter.setPen( dconf.debugCommentFontPen() );

            painter.drawText( QPointF( pos.x() - r,
                                       pos.y() + r + painter.fontMetrics().ascent() ),
                              QString::fromStdString( pl->comment_ ) );
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
DebugPainter::drawTarget( QPainter & painter,
                          const rcsc::SideID self_side,
                          const DebugViewData & view ) const
{
    const Options & opt = Options::instance();

    if ( ! opt.showDebugViewTarget() )
    {
        return;
    }

    const DrawConfig & dconf = DrawConfig::instance();
    const double reverse = ( self_side == rcsc::RIGHT
                             ? -1.0
                             : 1.0 );

    const double r = std::max( 2.0,
                               ( opt.enlargeMode()
                                 ? opt.scale( 0.25 )
                                 : opt.scale( 0.07 ) ) );

    //
    // target point
    //
    if ( view.targetPoint() )
    {
        double tx = opt.screenX( view.targetPoint()->x * reverse );
        double ty = opt.screenY( view.targetPoint()->y * reverse );

        painter.setPen( dconf.debugTargetPen() );
        painter.setBrush( dconf.transparentBrush() );

        painter.drawLine( QLineF( tx - r, ty - r,
                                  tx + r, ty + r ) );
        painter.drawLine( QLineF( tx - r, ty + r,
                                  tx + r, ty - r ) );
    }

    //
    // target teammate
    //
    const int target_unum = view.targetTeammate();
    if ( 1 <= target_unum
         && target_unum <= 11 )
    {
        for ( DebugViewData::PlayerCont::const_reference pl : view.teammates() )
        {
            if ( target_unum == pl->unum_ )
            {
                const QPointF pos( opt.screenX( (double)pl->x_ / rcsc::rcg::SHOWINFO_SCALE2 * reverse ),
                                   opt.screenY( (double)pl->y_ / rcsc::rcg::SHOWINFO_SCALE2 * reverse ) );

                painter.setPen( dconf.debugTargetPen() );
                painter.setBrush( dconf.transparentBrush() );

                const std::shared_ptr< DebugViewData::SelfT > self = view.self();
                if ( self
                     && self->unum_ != 12 )
                {
                    QPointF self_pos( opt.screenX( (double)self->x_ / rcsc::rcg::SHOWINFO_SCALE2 * reverse ),
                                      opt.screenY( (double)self->y_ / rcsc::rcg::SHOWINFO_SCALE2 * reverse ) );
                    painter.drawLine( QLineF( self_pos, pos ) );
                }

                double rr = r + 2;
                painter.drawEllipse( QRectF( pos.x() - rr, pos.y() - rr,
                                             rr * 2, rr * 2 ) );

                break;
            }
        }


    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
DebugPainter::drawLines( QPainter & painter,
                         const rcsc::SideID self_side,
                         const DebugViewData & view ) const
{
    if ( view.lines().empty() )
    {
        return;
    }

    const Options & opt = Options::instance();
    const DrawConfig & dconf = DrawConfig::instance();

    const double reverse = ( self_side == rcsc::RIGHT
                             ? -1.0
                             : 1.0 );

    // painter.setPen( dconf.debugShapePen() );
    painter.setBrush( dconf.transparentBrush() );

// #ifdef USE_GL_WIDGET
    for ( const DebugViewData::LineT & line : view.lines() )
    {
        if ( line.color_.empty() )
        {
            painter.setPen( dconf.debugShapePen() );
        }
        else
        {
            QColor col( line.color_.c_str() );
            if ( col.isValid() )
            {
                painter.setPen( col );
            }
            else
            {
                painter.setPen( dconf.debugShapePen() );
            }
        }
        painter.drawLine( QLineF( opt.screenX( line.x1_ * reverse ),
                                  opt.screenY( line.y1_ * reverse ),
                                  opt.screenX( line.x2_ * reverse ),
                                  opt.screenY( line.y2_ * reverse ) ) );
    }
// #else
//     QPainterPath path;

//     for ( const DebugViewData::LineT & line : view.lines() )
//     {
//         path.moveTo( opt.screenX( line.x1_ * reverse ),
//                      opt.screenY( line.y1_ * reverse ) );
//         path.lineTo( opt.screenX( line.x2_ * reverse ),
//                      opt.screenY( line.y2_ * reverse ) );
//     }

//     painter.drawPath( path );
// #endif
}

/*-------------------------------------------------------------------*/
/*!

*/
void
DebugPainter::drawTriangles( QPainter & painter,
                             const rcsc::SideID self_side,
                             const DebugViewData & view ) const
{
    if ( view.triangles().empty() )
    {
        return;
    }

    const Options & opt = Options::instance();
    const DrawConfig & dconf = DrawConfig::instance();

    const double reverse = ( self_side == rcsc::RIGHT
                             ? -1.0
                             : 1.0 );

    // painter.setPen( dconf.debugShapePen() );
    painter.setBrush( dconf.transparentBrush() );

// #ifdef USE_GLWIDGET
    for ( const DebugViewData::TriangleT & tri : view.triangles() )
    {
        std::cerr << "triangle (" << tri.x1_ << ',' << tri.y1_ << ")("
                  << '(' << tri.x2_ << ',' << tri.y2_ << ")("
                  << '(' << tri.x3_ << ',' << tri.y3_ << ")" << std::endl;
        QPointF points[4] = { QPointF( opt.screenX( tri.x1_ * reverse ),
                                       opt.screenY( tri.y1_ * reverse ) ),
                              QPointF( opt.screenX( tri.x2_ * reverse ),
                                       opt.screenY( tri.y2_ * reverse ) ),
                              QPointF( opt.screenX( tri.x3_ * reverse ),
                                       opt.screenY( tri.y3_ * reverse ) ) };
        if ( tri.color_.empty() )
        {
            painter.setPen( dconf.debugShapePen() );
        }
        else
        {
            QColor col( tri.color_.c_str() );
            if ( col.isValid() )
            {
                painter.setPen( col );
            }
            else
            {
                painter.setPen( dconf.debugShapePen() );
            }
        }
        //painter.drawPolygon( points, 3 );
        painter.drawLine( points[0], points[1] );
        painter.drawLine( points[1], points[2] );
        painter.drawLine( points[2], points[0] );
    }
// #else
//     QPainterPath path;

//     for ( const DebugViewData::TriangleT & tri : view.triangles() )
//     {
//         double x1 = opt.screenX( tri.x1_ * reverse );
//         double y1 = opt.screenY( tri.y1_ * reverse );
//         double x2 = opt.screenX( tri.x2_ * reverse );
//         double y2 = opt.screenY( tri.y2_ * reverse );
//         double x3 = opt.screenX( tri.x3_ * reverse );
//         double y3 = opt.screenY( tri.y3_ * reverse );

//         path.moveTo( x1, y1 );
//         path.lineTo( x2, y2 );
//         path.lineTo( x3, y3 );
//         path.lineTo( x1, y1 );
//     }

//     painter.drawPath( path );
// #endif
}

/*-------------------------------------------------------------------*/
/*!

*/
void
DebugPainter::drawRectangles( QPainter & painter,
                              const rcsc::SideID self_side,
                              const DebugViewData & view ) const
{
    if ( view.rectangles().empty() )
    {
        return;
    }

    const Options & opt = Options::instance();
    const DrawConfig & dconf = DrawConfig::instance();

    const double reverse = ( self_side == rcsc::RIGHT
                             ? -1.0
                             : 1.0 );

    // painter.setPen( dconf.debugShapePen() );
    painter.setBrush( dconf.transparentBrush() );

// #ifdef USE_GLWIDGET
    for ( const DebugViewData::RectT & rect : view.rectangles() )
    {
        double left_x = opt.screenX( rect.left_x_ * reverse );
        double top_y = opt.screenY( rect.top_y_ * reverse );
        double right_x = opt.screenX( rect.right_x_ * reverse );
        double bottom_y = opt.screenY( rect.bottom_y_ * reverse );

        if ( rect.color_.empty() )
        {
            painter.setPen( dconf.debugShapePen() );
        }
        else
        {
            QColor col( rect.color_.c_str() );
            if ( col.isValid() )
            {
                painter.setPen( col );
            }
            else
            {
                painter.setPen( dconf.debugShapePen() );
            }
        }
        painter.drawRect( QRectF( left_x,
                                  top_y,
                                  ( right_x - left_x ),
                                  ( bottom_y - top_y ) ) );
    }
// #else
//     QPainterPath path;

//     for ( const DebugViewData::RectT & rect : view.rectangles() )
//     {
//         double left_x = opt.screenX( rect.left_x_ * reverse );
//         double top_y = opt.screenY( rect.top_y_ * reverse );
//         double right_x = opt.screenX( rect.right_x_ * reverse );
//         double bottom_y = opt.screenY( rect.bottom_y_ * reverse );

//         path.addRect( QRectF( left_x,
//                               top_y,
//                               ( right_x - left_x ),
//                               ( bottom_y - top_y ) ) );
//     }

//     painter.drawPath( path );
// #endif
}

/*-------------------------------------------------------------------*/
/*!

*/
void
DebugPainter::drawCircles( QPainter & painter,
                           const rcsc::SideID self_side,
                           const DebugViewData & view ) const
{
    if ( view.circles().empty() )
    {
        return;
    }

    const Options & opt = Options::instance();
    const DrawConfig & dconf = DrawConfig::instance();

    const double reverse = ( self_side == rcsc::RIGHT
                             ? -1.0
                             : 1.0 );

    // painter.setPen( dconf.debugShapePen() );
    painter.setBrush( dconf.transparentBrush() );

// #ifdef USE_GLWIDGET
    for ( const DebugViewData::CircleT & c : view.circles() )
    {
        double r = opt.scale( c.radius_ );

        if ( c.color_.empty() )
        {
            painter.setPen( dconf.debugShapePen() );
        }
        else
        {
            QColor col( c.color_.c_str() );
            if ( col.isValid() )
            {
                painter.setPen( col );
            }
            else
            {
                painter.setPen( dconf.debugShapePen() );
            }
        }
        painter.drawEllipse( QRectF( opt.screenX( c.center_x_ * reverse ) - r,
                                     opt.screenY( c.center_y_ * reverse ) - r,
                                     r * 2,
                                     r * 2 ) );
    }
// #else
//     QPainterPath path;

//     for ( const DebugViewData::CircleT & c : view.circles() )
//     {
//         double r = opt.scale( c.radius_ );
//         path.addEllipse( QRectF( opt.screenX( c.center_x_ * reverse ) - r,
//                                  opt.screenY( c.center_y_ * reverse ) - r,
//                                  r * 2,
//                                  r * 2 ) );
//     }

//     painter.drawPath( path );
// #endif
}

/*-------------------------------------------------------------------*/
/*!

*/
void
DebugPainter::drawMessage( QPainter & painter,
                           const DebugViewData & view ) const
{
    painter.setFont( DrawConfig::instance().debugMessageFont() );
    painter.setPen( DrawConfig::instance().debugMessageFontPen() );

    const QRect win = painter.window();
    QRect rect( 10, Options::instance().scoreBoardHeight(),
                win.width() - 20,
                win.height() - Options::instance().scoreBoardHeight() );
    QRect bounding_rect;

    if ( ! view.message().empty() )
    {
        painter.drawText( rect,
                          Qt::AlignLeft | Qt::TextWordWrap,
                          QString::fromStdString( view.message() ),
                          &bounding_rect );
        rect.setRect( 10, bounding_rect.bottom() + 1,
                      win.width() - 20,
                      win.height() - bounding_rect.bottom() - 1 );
    }

    if ( ! view.sayMessage().empty() )
    {
        QString text = QObject::tr( "Say: " );
        text += QString::fromStdString( view.sayMessage() );

        painter.drawText( rect, Qt::AlignLeft, text, &bounding_rect );
        rect.setRect( 10, bounding_rect.bottom() + 1,
                      win.width() - 20,
                      win.height() - bounding_rect.bottom() - 1 );
    }

    if ( ! view.hearMessages().empty() )
    {
        QString text = QObject::tr( "Hear: " );

        for ( std::map< int, std::string >::const_reference hear : view.hearMessages() )
        {
            text += QString( "(%1" ).arg( hear.first );
            text += QString::fromStdString( hear.second );
            text += QObject::tr( ")" );
        }

        painter.drawText( rect, Qt::AlignLeft, text );
    }
}
