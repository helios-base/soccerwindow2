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

#include <rcsc/geom/triangle_2d.h>
#include <rcsc/common/server_param.h>

using namespace rcsc;

/*-------------------------------------------------------------------*/
FormationEditorPainter::FormationEditorPainter( const MainData & main_data )
    : M_main_data( main_data ),
      M_fedit_font( "Sans Serif", 10 ),
      M_ball_pen( QColor( 255, 255, 255 ), 0, Qt::SolidLine ),
      M_ball_brush( QColor( 155, 155, 155 ), Qt::SolidPattern ),
      M_player_pen( QColor( 0, 0, 0 ), 0, Qt::SolidLine ),
      M_selected_pen( Qt::white, 0, Qt::SolidLine ),
      M_player_brush( QColor( 255, 215, 0 ), Qt::SolidPattern ),
      M_paired_brush( QColor( 0, 255, 95 ), Qt::SolidPattern ),
      M_triangle_pen( QColor( 255, 0, 0 ), 0, Qt::SolidLine ),
      M_contained_area_brush( QColor( 15, 143, 15 ), Qt::SolidPattern ),
      M_background_triangle_pen( QColor( 0, 127, 255 ), 0, Qt::SolidLine ),
      M_background_contained_area_brush( QColor( 31, 143, 31 ), Qt::SolidPattern ),
      M_background_font_pen( QColor( 0, 63, 127 ), 0, Qt::SolidLine )
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

    painter.setOpacity( Options::instance().feditOpacity() );

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
    painter.setOpacity( 1.0 );
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

    //
    // draw index of vertices
    //

    if ( Options::instance().feditShowIndex() )
    {
        painter.setPen( Qt::red );
        painter.setFont( M_fedit_font );

        const QTransform transform = painter.worldTransform();
        painter.setWorldMatrixEnabled( false );

        int count = 0;
        for ( const FormationData::Data & data : ptr->formationData()->dataCont() )
        {
            painter.drawText( transform.map( QPointF( data.ball_.x + 0.7, data.ball_.y - 0.7 ) ),
                              QString::number( count ) );
            ++count;
        }

        painter.setWorldMatrixEnabled( true );
    }

    //
    // draw selected sample
    //

    if ( 0 <= ptr->currentIndex() )
    {
        FormationData::DataCont::const_iterator it = ptr->formationData()->dataCont().begin();
        std::advance( it, ptr->currentIndex() );

        painter.setPen( QPen( Qt::yellow, 0, Qt::SolidLine) );
        painter.setBrush( Qt::NoBrush );
        painter.drawRect( QRectF( it->ball_.x - 1.0, it->ball_.y - 1.0, 2.0, 2.0 ) );
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
    std::shared_ptr< FormationEditData > ptr = M_edit_data.lock();
    if ( ! ptr )
    {
        return;
    }

    // std::shared_ptr< const Formation > f = ptr->formation();
    // if ( ! f )
    // {
    //     return;
    // }

    if ( ptr->triangulation().triangles().empty() )
    {
        return;
    }

    const Triangulation::Triangle * tri = ptr->triangulation().findTriangleContains( ptr->currentState().ball_ );
    if ( ! tri )
    {
        return;
    }


    const Triangulation::PointCont & points = ptr->triangulation().points();
    const Vector2D vertex_0 = points.at( tri->v0_ );
    const Vector2D vertex_1 = points.at( tri->v1_ );
    const Vector2D vertex_2 = points.at( tri->v2_ );

    const QPointF vertices[3] = {
        QPointF( vertex_0.x, vertex_0.y ),
        QPointF( vertex_1.x, vertex_1.y ),
        QPointF( vertex_2.x, vertex_2.y )
    };

    painter.setPen( Qt::NoPen );
    painter.setBrush( M_contained_area_brush );
    painter.drawConvexPolygon( vertices, 3 );

    // draw center point
    Vector2D center = Triangle2D::centroid( vertex_0, vertex_1, vertex_2 );
    const double pix = M_transform.inverted().map( QLineF( 0.0, 0.0, 1.0, 0.0 ) ).length();

    painter.setPen( QPen( Qt::red, 0, Qt::SolidLine ) );
    painter.setBrush( QBrush( Qt::red, Qt::SolidPattern ) );
    painter.drawRect( QRectF( center.x - pix, center.y - pix,
                              pix*2.0, pix*2.0 ) );

    // draw the triangule's circumcircle
    if ( Options::instance().feditShowCircumcircle() )
    {
        const Vector2D circumcenter = Triangle2D::circumcenter( vertex_0, vertex_1, vertex_2 );
        double x = circumcenter.x;
        double y = circumcenter.y;
        double r = circumcenter.dist( vertex_0 );

        painter.setPen( QPen( Qt::cyan, 0, Qt::SolidLine ) );
        painter.setBrush( QBrush( Qt::cyan, Qt::SolidPattern ) );
        painter.drawRect( QRectF( x - pix, y - pix, pix * 2.0, pix * 2.0 ) );
        //painter.drawPoint( QPointF( x, y ) );

        painter.setBrush( Qt::NoBrush );
        painter.drawEllipse( x - r, y - r,
                             r * 2.0, r * 2.0 );
    }
}

/*-------------------------------------------------------------------*/
void
FormationEditorPainter::drawBall( QPainter & painter )
{
    std::shared_ptr< FormationEditData > ptr = M_edit_data.lock();
    if ( ! ptr )
    {
        return;
    }

    painter.setPen( M_ball_pen );
    painter.setBrush( M_ball_brush );

    const Vector2D bpos = ptr->currentState().ball_;
    const bool enlarge = Options::instance().enlargeMode();
    const double r = ( enlarge
                       ? ( ptr->selectType() == FormationEditData::SELECT_BALL
                           ? Options::instance().ballSize() * 2.0
                           : Options::instance().ballSize() * 0.9 )
                       : ( ptr->selectType() == FormationEditData::SELECT_BALL
                           ? ServerParam::i().ballSize() * 2.0
                           : ServerParam::i().ballSize() * 0.9 )
                       );

    painter.drawEllipse( QRectF( bpos.x - r, bpos.y - r,
                                 r * 2.0, r * 2.0 ) );

    if ( ! enlarge )
    {
        painter.setBrush( Qt::NoBrush );

        double kr = ServerParam::i().defaultKickableArea();
        painter.drawEllipse( QRectF( bpos.x - kr, bpos.y - kr,
                                     kr * 2.0, kr * 2.0 ) );
    }
}

/*-------------------------------------------------------------------*/
void
FormationEditorPainter::drawPlayers( QPainter & painter )
{
    std::shared_ptr< FormationEditData > ptr = M_edit_data.lock();
    if ( ! ptr )
    {
        return;
    }

    Formation::ConstPtr f = ptr->formation();
    if ( ! f )
    {
        return;
    }

    painter.setFont( M_fedit_font );

    const bool enlarge = Options::instance().enlargeMode();
    const double r = ( enlarge
                       ? ServerParam::instance().defaultKickableArea() * 0.9
                       : ServerParam::instance().defaultPlayerSize() * 0.9 ); // body radius
    const double d = r * 2.0;   // body diameter
    const double kr = ServerParam::i().defaultKickableArea(); // kickable radius
    const double kd = kr * 2.0;    // kickable diameter
    const double cr = ServerParam::i().catchableArea();  // catchable radius
    const double cd = cr * 2.0;    // catchable diameter

    const QTransform & transform = painter.worldTransform();

    const Vector2D selected_pos
        = ( ptr->selectType() == FormationEditData::SELECT_PLAYER
            ? ptr->currentState().players_[ptr->selectIndex()]
            : Vector2D::INVALIDATED );

    int unum = 0;
    for ( const Vector2D & p : ptr->currentState().players_ )
    {
        ++unum;

        const bool paired = ( f->pairedNumber( unum ) > 0
                              && f->roleType( unum ).side() == RoleType::Right );

        if ( p == selected_pos )
        {
            painter.setPen( M_selected_pen );
            painter.setBrush( paired
                              ? M_paired_brush
                              : M_player_brush );
            painter.drawEllipse( QRectF( p.x - r - 0.5, p.y - r - 0.5,
                                         d + 1.0, d + 1.0 ) );
        }
        else
        {
            painter.setPen( M_player_pen );
            painter.setBrush( paired
                              ? M_paired_brush
                              : M_player_brush );
            painter.drawEllipse( QRectF( p.x - r, p.y - r, d, d ) );
        }

        painter.setBrush( Qt::NoBrush );

        if ( unum == 1 )
        {
            // catchable area circle
            painter.drawEllipse( QRectF( p.x - cr, p.y - cr, cd, cd ) );
        }
        else if ( ! enlarge )
        {
            // kickable area circle
            painter.drawEllipse( QRectF( p.x - kr, p.y - kr, kd, kd ) );
        }

        painter.setPen( Qt::white );
        painter.setWorldMatrixEnabled( false );
        painter.drawText( transform.map( QPointF( p.x + r, p.y ) ),
                          QString::number( unum ) );
        painter.setWorldMatrixEnabled( true );
    }
}

/*-------------------------------------------------------------------*/
void
FormationEditorPainter::drawBackgroundData( QPainter & painter )
{
    std::shared_ptr< FormationEditData > ptr = M_edit_data.lock();
    if ( ! ptr )
    {
        return;
    }

    if ( ptr->backgroundTriangulation().triangles().empty() )
    {
        // too few kernel points
        // no valid triangulation
        return;
    }

    if ( Options::instance().antiAliasing() )
    {
        setAntialiasFlag( painter, false );
    }

    if ( Options::instance().feditShowTriangulation() )
    {

        const Triangulation::PointCont & points = ptr->backgroundTriangulation().points();

        // triangulation

        painter.setPen( M_background_triangle_pen );
        painter.setBrush( Qt::NoBrush );

        for ( const auto & e : ptr->backgroundTriangulation().edges() )
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

        for ( const Vector2D & p : ptr->backgroundTriangulation().points() )
        {
            painter.drawRect( QRectF( p.x - r, p.y - r, d, d ) );
        }
    }

    //
    // draw index of vertices

    //
    if ( Options::instance().feditShowIndex() )
    {
        //painter.setPen( Qt::darkRed );
        painter.setPen( Qt::darkMagenta );
        painter.setFont( M_fedit_font );

        const QTransform transform = painter.worldTransform();
        painter.setWorldMatrixEnabled( false );

        int count = 0;
        for ( const Vector2D & p : ptr->backgroundTriangulation().points() )
        {
            painter.drawText( transform.map( QPointF( p.x + 0.7, p.y - 0.7 ) ),
                              QString::number( count ) );
            ++count;
        }

        painter.setWorldMatrixEnabled( true );
    }

    if ( Options::instance().antiAliasing() )
    {
        setAntialiasFlag( painter, true );
    }
}

/*-------------------------------------------------------------------*/
void
FormationEditorPainter::drawBackgroundContainedTriangle( QPainter & painter )
{
   std::shared_ptr< FormationEditData > ptr = M_edit_data.lock();
    if ( ! ptr )
    {
        return ;
    }

    if ( ptr->backgroundTriangulation().triangles().empty() )
    {
        return;
    }

    const Triangulation::Triangle * tri = ptr->backgroundTriangulation().findTriangleContains( ptr->currentState().ball_ );

    if ( ! tri )
    {
        return;
    }

    const Triangulation::PointCont & points = ptr->backgroundTriangulation().points();
    const Vector2D vertex_0 = points.at( tri->v0_ );
    const Vector2D vertex_1 = points.at( tri->v1_ );
    const Vector2D vertex_2 = points.at( tri->v2_ );

    const QPointF vertices[3] = {
        QPointF( vertex_0.x, vertex_0.y ),
        QPointF( vertex_1.x, vertex_1.y ),
        QPointF( vertex_2.x, vertex_2.y )
    };

    painter.setPen( Qt::NoPen );
    painter.setBrush( M_background_contained_area_brush );
    painter.drawConvexPolygon( vertices, 3 );
}

/*-------------------------------------------------------------------*/
void
FormationEditorPainter::drawGoalieMovableArea( QPainter & painter )
{
    std::shared_ptr< FormationEditData > ptr = M_edit_data.lock();
    if ( ! ptr )
    {
        return;
    }

    if ( ptr->currentState().players_.empty() )
    {
        return;
    }

    const Vector2D goalie_pos = ptr->currentState().players_.front();
    const double catch_area = ServerParam::i().catchableArea();
    const double max_accel = ServerParam::i().maxDashPower() * ServerParam::i().defaultDashPowerRate() * ServerParam::i().defaultEffortMax();
    const double decay = ServerParam::i().defaultPlayerDecay();

    const QTransform transform = painter.worldTransform();
    QColor base_color = DrawConfig::instance().fieldBrush().color();
    base_color.setAlphaF( 0.4 );

    double radius[10];

    double dist = 0.0;
    double speed = 0.0;
    for ( int i = 0; i < 10; ++i )
    {
        speed += max_accel;
        dist += speed;
        speed *= decay;

        radius[i] = dist + catch_area;
    }

    painter.setPen( QPen( Qt::black, 0, Qt::SolidLine ) );
    for ( int i = 9; i >= 0; --i )
    {
        painter.setBrush( QBrush( base_color.darker( 300 - 20*i ), Qt::SolidPattern ) );
        painter.drawEllipse( QRectF( goalie_pos.x - radius[i],
                                     goalie_pos.y - radius[i],
                                     radius[i]*2.0,
                                     radius[i]*2.0 ) );
    }

    painter.setPen( Qt::white );
    painter.setWorldMatrixEnabled( false );
    for ( int i = 0; i < 10; ++i )
    {
        QString str = QString::number( i + 1 );
        painter.drawText( transform.map( QPointF( goalie_pos.x - radius[i], goalie_pos.y ) ), str );
        painter.drawText( transform.map( QPointF( goalie_pos.x + radius[i], goalie_pos.y ) ), str );
        painter.drawText( transform.map( QPointF( goalie_pos.x, goalie_pos.y - radius[i] ) ), str );
        painter.drawText( transform.map( QPointF( goalie_pos.x, goalie_pos.y + radius[i] ) ), str );
    }
    painter.setWorldMatrixEnabled( true );
}

/*-------------------------------------------------------------------*/
void
FormationEditorPainter::drawShootLines( QPainter & painter )
{
    std::shared_ptr< FormationEditData > ptr = M_edit_data.lock();
    if ( ! ptr )
    {
        return;
    }

    painter.setPen( QPen( QColor( 255, 140, 0 ), 0, Qt::SolidLine ) );
    painter.setBrush( Qt::NoBrush );

    const Vector2D ball = ptr->currentState().ball_;
    const double goal_line_x = -ServerParam::i().pitchHalfLength() - 0.001;
    const double goal_half_width = ServerParam::i().goalHalfWidth() - 0.1;
    const double bdecay = ServerParam::i().ballDecay();

    const QTransform & transform = painter.worldTransform();

    painter.drawLine( QLineF( ball.x, ball.y, goal_line_x, -goal_half_width ) );
    painter.drawLine( QLineF( ball.x, ball.y, goal_line_x, +goal_half_width ) );

    Vector2D ball_pos = ball;
    Vector2D ball_vel = Vector2D( goal_line_x, -goal_half_width ) - ball_pos;
    ball_vel.setLength( ServerParam::i().ballSpeedMax() );

    const int left_angle = qRound( ball_vel.th().degree() * -16 );
    int max_count = 0;
    int count = 1;
    while ( ball_pos.x > goal_line_x )
    {
        ball_pos += ball_vel;
        ball_vel *= bdecay;

        painter.drawRect( QRectF( ball_pos.x - 0.05, ball_pos.y - 0.05, 0.1, 0.1 ) );
        painter.setWorldMatrixEnabled( false );
        painter.drawText( transform.map( QPointF( ball_pos.x + 0.1, ball_pos.y ) ),
                          QString::number( count ) );
        painter.setWorldMatrixEnabled( true );

        if ( ++count > 15 )
        {
            break;
        }
    }
    max_count = std::max( max_count, count - 1 );

    ball_pos = ball;
    ball_vel = Vector2D( goal_line_x, +goal_half_width ) - ball_pos;
    ball_vel.setLength( ServerParam::i().ballSpeedMax() );

    const int right_angle = qRound( ball_vel.th().degree() * -16 );
    count = 1;
    while ( ball_pos.x > goal_line_x )
    {
        ball_pos += ball_vel;
        ball_vel *= bdecay;

        painter.drawRect( QRectF( ball_pos.x - 0.05, ball_pos.y - 0.05, 0.1, 0.1 ) );
        painter.setWorldMatrixEnabled( false );
        painter.drawText( transform.map( QPointF( ball_pos.x + 0.1, ball_pos.y ) ),
                          QString::number( count ) );
        painter.setWorldMatrixEnabled( true );

        if ( ++count > 15 )
        {
            break;
        }
    }
    max_count = std::max( max_count, count - 1 );

    int span_angle = 360*16 + ( right_angle - left_angle );
    if ( span_angle > 360*16 ) span_angle -= 360*16;

    double ball_move = 0.0;
    double ball_speed = ServerParam::i().ballSpeedMax();
    for ( int i = 0; i < max_count; ++i )
    {
        ball_move += ball_speed;
        painter.drawArc( QRectF( ball.x - ball_move, ball.y - ball_move, ball_move*2, ball_move*2 ),
                         left_angle, span_angle );
        ball_speed *= bdecay;
    }
}

/*-------------------------------------------------------------------*/
void
FormationEditorPainter::drawFreeKickCircle( QPainter & painter )
{
    std::shared_ptr< FormationEditData > ptr = M_edit_data.lock();
    if ( ! ptr )
    {
        return;
    }

    painter.setPen( QPen( Qt::magenta, 0, Qt::SolidLine ) );
    painter.setBrush( Qt::NoBrush );

    const Vector2D ball = ptr->currentState().ball_;
    const double r = ServerParam::DEFAULT_CENTER_CIRCLE_R;

    painter.drawEllipse( QRectF( ball.x - r, ball.y - r, r*2, r*2 ) );
}
