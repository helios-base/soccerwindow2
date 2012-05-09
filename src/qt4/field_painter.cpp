// -*-c++-*-

/*!
  \file field_painter.cpp
  \brief field painter class Source File.
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

#include <QtGui>

#include "field_painter.h"

#include "draw_config.h"
#include "options.h"

#include <rcsc/common/server_param.h>

#include <iostream>

/*-------------------------------------------------------------------*/
/*!

 */
FieldPainter::FieldPainter()
    : M_center_x( 0 )
    , M_center_y( 0 )
{

}

/*-------------------------------------------------------------------*/
/*!

 */
void
FieldPainter::draw( QPainter & painter )
{
    if ( Options::instance().antiAliasing() )
    {
#ifdef USE_HIGH_QUALITY_ANTIALIASING
        painter.setRenderHints( QPainter::HighQualityAntialiasing |
                                QPainter::Antialiasing,
                                false );
#else
        painter.setRenderHint( QPainter::Antialiasing, false );
#endif
    }

    drawBackGround( painter );
    drawLines( painter );
    drawPenaltyAreaLines( painter );
    drawGoalAreaLines( painter );
    drawGoals( painter );

    if ( Options::instance().showFlags() )
    {
        drawFlags( painter );
    }

    if ( Options::instance().gridStep() > 0 )
    {
        drawGrid( painter );
    }

    if ( Options::instance().antiAliasing() )
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

/*-------------------------------------------------------------------*/
/*!

 */
void
FieldPainter::drawBackGround( QPainter & painter ) const
{
    // fill the whole region
    //M_pixmap->fill( DrawConfig::instance().fieldBrush().color() );
    painter.fillRect( painter.window(),
                      DrawConfig::instance().fieldBrush() );

    ////////////////////////////////////////////////////////////////
    switch ( Options::instance().fieldGrassType() ) {
    case Options::GRASS_LINES:
        drawLinesGrass( painter );
        break;
    case Options::GRASS_CHECKER:
        drawCheckerGrass( painter );
        break;
    default:
        break;
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
FieldPainter::drawLinesGrass( QPainter & painter ) const
{
    const rcsc::ServerParam & SP = rcsc::ServerParam::i();
    const Options & opt = Options::instance();
    const DrawConfig & dconf = DrawConfig::instance();

    const double left_x = opt.fieldCenterF().x - opt.scaleF( SP.pitchHalfLength() );
    const double top_y = opt.fieldCenterF().y - opt.scaleF( SP.pitchHalfWidth() );
    const double right_x = opt.fieldCenterF().x + opt.scaleF( SP.pitchHalfLength() );
    const double bottom_y = opt.fieldCenterF().y + opt.scaleF( SP.pitchHalfWidth() );

    const double grid = opt.scaleF( 10.0 ); // grid length

    const int line_top = top_y - grid/2;
    const int line_height = bottom_y - top_y + grid;

    //    const bool use_gradient = opt.gradient();

    painter.setPen( dconf.transparentPen() );
    //painter.setBrush( dconf.fieldDarkBrush() );

    double i = opt.fieldCenterF().x - grid*0.5;
    int cnt = 0;
    while ( i > left_x )
    {
        if ( ! ( cnt % 2 ) )
        {
            double left = i - grid;

//             if ( use_gradient )
//             {
//                 QLinearGradient grad( left + grid/2, 0,
//                                       left - grid/2, 0 );
//                 grad.setSpread( QGradient::ReflectSpread );
//                 grad.setColorAt( 1.0, dconf.fieldBrush().color() );
//                 grad.setColorAt( 0.2, dconf.fieldDarkBrush().color() );
//                 grad.setColorAt( 0.0, dconf.fieldDarkBrush().color() );
//                 painter.fillRect( left - grid/2,
//                                   line_top,
//                                   grid*2,
//                                   line_height,
//                                   grad );
//             }
//             else
            {
                painter.fillRect( left,
                                  line_top,
                                  grid,
                                  line_height,
                                  dconf.fieldDarkBrush() );
            }
        }
        i -= grid;
        ++cnt;
    }

    i = opt.fieldCenterF().x + grid*0.5;
    cnt = 0;
    while ( i < right_x )
    {
        if ( ! ( cnt % 2 ) )
        {
            //int right = i + grid;
            double left = i;

//             if ( use_gradient )
//             {
//                 QLinearGradient grad( left + grid/2, 0,
//                                       left - grid/2, 0 );
//                 grad.setSpread( QGradient::ReflectSpread );
//                 grad.setColorAt( 1.0, dconf.fieldBrush().color() );
//                 grad.setColorAt( 0.2, dconf.fieldDarkBrush().color() );
//                 grad.setColorAt( 0.0, dconf.fieldDarkBrush().color() );
//                 painter.fillRect( left - grid/2,
//                                   line_top,
//                                   grid*2, //right - i,
//                                   line_height,
//                                   grad );
//             }
//             else
            {
                painter.fillRect( left,
                                  line_top,
                                  grid,
                                  line_height,
                                  dconf.fieldDarkBrush() );
            }
        }

        i += grid;
        ++cnt;
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
FieldPainter::drawCheckerGrass( QPainter & painter )const
{
    const rcsc::ServerParam & SP = rcsc::ServerParam::i();
    const Options & opt = Options::instance();
    const DrawConfig & dconf = DrawConfig::instance();

    painter.setPen( dconf.transparentPen() );
    painter.setBrush( dconf.fieldDarkBrush() );

    const double left_x = opt.fieldCenterF().x - opt.scaleF( SP.pitchHalfLength() );
    const double top_y = opt.fieldCenterF().y - opt.scaleF( SP.pitchHalfWidth() );
    const double right_x = opt.fieldCenterF().x + opt.scaleF( SP.pitchHalfLength() );
    const double bottom_y = opt.fieldCenterF().y + opt.scaleF( SP.pitchHalfWidth() );

    const double grid = opt.scaleF( 10.0 ); // grid length

    //const bool gradient = opt.gradient();

    int flag = 0;
    for ( double i = opt.fieldCenterF().x; i > left_x; i -= grid )
    {
        int cnt = flag;
        double left = i - grid;
        for ( double j = opt.fieldCenterF().y; j > top_y; j -= grid )
        {
            if ( cnt % 2 )
            {
                double top = j - grid;
//                 if ( gradient )
//                 {
//                     QRadialGradient gradient( left + grid/2,
//                                               top + grid/2,
//                                               grid*13/20,
//                                               left + grid/2,
//                                               top + grid/2  );
//                     gradient.setColorAt( 1.0, dconf.fieldBrush().color() );
//                     gradient.setColorAt( 0.0, dconf.fieldDarkBrush().color() );
//                     painter.fillRect( left, top, i - left, j - top,
//                                       gradient );
//                 }
//                else
                {
                    painter.fillRect( left, top, i - left, j - top,
                                      dconf.fieldDarkBrush() );
                }
            }
            ++cnt;
        }
        ++flag;
    }

    flag = 1;
    for ( double i = opt.fieldCenterF().x;  i > left_x; i -= grid )
    {
        int cnt = flag;
        double left = i - grid;
        for ( double j = opt.fieldCenterF().y; j < bottom_y; j += grid )
        {
            if ( cnt % 2 )
            {
                double bottom = j + grid;
//                 if ( gradient )
//                 {
//                     QRadialGradient gradient( left + grid/2,
//                                               j + grid/2,
//                                               grid*6/10,
//                                               left + grid/2,
//                                               j + grid/2  );
//                     gradient.setColorAt( 1.0, dconf.fieldBrush().color() );
//                     gradient.setColorAt( 0.0, dconf.fieldDarkBrush().color() );
//                     painter.fillRect( left, j, i - left, bottom - j,
//                                       gradient );
//                 }
//                else
                {
                    painter.fillRect( left, j, i - left, bottom - j,
                                      dconf.fieldDarkBrush() );
                }
            }
            ++cnt;
        }
        ++flag;
    }
    flag = 0;
    for ( double i = opt.fieldCenterF().x;  i < right_x; i += grid )
    {
        int cnt = flag;
        double right = i + grid;
        for ( double j = opt.fieldCenterF().y; j < bottom_y; j += grid )
        {
            if ( cnt % 2 )
            {
                double bottom = j + grid;
//                 if ( gradient )
//                 {
//                     QRadialGradient gradient( i + grid/2,
//                                               j + grid/2,
//                                               grid*6/10,
//                                               i + grid/2,
//                                               j + grid/2  );
//                     gradient.setColorAt( 1.0, dconf.fieldBrush().color() );
//                     gradient.setColorAt( 0.0, dconf.fieldDarkBrush().color() );
//                     painter.fillRect( i, j, right - i, bottom - j,
//                                       gradient );
//                 }
//                 else
                {
                    painter.fillRect( i, j, right - i, bottom - j,
                                      dconf.fieldDarkBrush() );
                }
            }
            ++cnt;
        }
        ++flag;
    }
    flag = 1;
    for ( double i = opt.fieldCenterF().x;  i < right_x; i += grid )
    {
        int cnt = flag;
        double right = i + grid;
        for ( double j = opt.fieldCenterF().y; j > top_y; j -= grid )
        {
            if ( cnt % 2 )
            {
                double top = j - grid;
//                 if ( gradient )
//                 {
//                     QRadialGradient gradient( i + grid/2,
//                                               top + grid/2,
//                                               grid*6/10,
//                                               i + grid/2,
//                                               top + grid/2  );
//                     gradient.setColorAt( 1.0, dconf.fieldBrush().color() );
//                     gradient.setColorAt( 0.0, dconf.fieldDarkBrush().color() );
//                     painter.fillRect( i, top, right - i, j - top,
//                                       gradient );
//                 }
//                 else
                {
                    painter.fillRect( i, top, right - i, j - top,
                                      dconf.fieldDarkBrush() );
                }
            }
            ++cnt;
        }
        ++flag;
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
FieldPainter::drawLines( QPainter & painter ) const
{
    const rcsc::ServerParam & SP = rcsc::ServerParam::i();
    const Options & opt = Options::instance();
    const DrawConfig & dconf = DrawConfig::instance();

    // set gdi objects
    painter.setPen( dconf.linePen() );
    painter.setBrush( dconf.transparentBrush() );

    // set screen coordinates of field
    double left_x   = opt.absScreenXF( - SP.pitchHalfLength() );
    double right_x  = opt.absScreenXF( + SP.pitchHalfLength() );
    double top_y    = opt.absScreenYF( - SP.pitchHalfWidth() );
    double bottom_y = opt.absScreenYF( + SP.pitchHalfWidth() );

    // side lines & goal lines
    painter.drawLine( QLineF( left_x, top_y, right_x, top_y ) );
    painter.drawLine( QLineF( right_x, top_y, right_x, bottom_y ) );
    painter.drawLine( QLineF( right_x, bottom_y, left_x, bottom_y ) );
    painter.drawLine( QLineF( left_x, bottom_y, left_x, top_y ) );

    if ( opt.keepawayMode()
         || SP.keepawayMode() )
    {
        // keepaway area
        double ka_left = opt.absScreenXF( - SP.keepawayLength() * 0.5 );
        double ka_top = opt.absScreenYF( - SP.keepawayWidth() * 0.5 );
        double ka_length = opt.scaleF( SP.keepawayLength() );
        double ka_width = opt.scaleF( SP.keepawayWidth() );

        painter.drawRect( QRectF( ka_left, ka_top, ka_length, ka_width ) );
    }
    else
    {
        // center line
        painter.drawLine( QLineF( opt.fieldCenterF().x, top_y,
                                  opt.fieldCenterF().x, bottom_y ) );

        // center circle
        double center_radius = opt.scaleF( SP.centerCircleR() );
        painter.drawEllipse( QRectF( opt.fieldCenterF().x - center_radius,
                                     opt.fieldCenterF().y - center_radius,
                                     center_radius * 2,
                                     center_radius * 2 ) );
    }

    // corner arc
    {
        double r = opt.scaleF( SP.cornerArcR() );

        painter.drawArc( QRectF( left_x - r, top_y - r, r * 2, r * 2 ),
                         -90*16, 90*16 );
        painter.drawArc( QRectF( left_x - r, bottom_y - r, r * 2, r * 2 ),
                         0*16, 90*16 );
        painter.drawArc( QRectF( right_x - r, bottom_y - r, r * 2, r * 2 ),
                         90*16, 90*16 );
        painter.drawArc( QRectF( right_x - r, top_y - r, r * 2, r * 2 ),
                         180*16, 90*16 );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
FieldPainter::drawPenaltyAreaLines( QPainter & painter ) const
{
    const rcsc::ServerParam & SP = rcsc::ServerParam::i();
    const Options & opt = Options::instance();
    const DrawConfig & dconf = DrawConfig::instance();

    // set gdi objects
    painter.setPen( dconf.linePen() );
    painter.setBrush( dconf.transparentBrush() );

    // set screen coordinates of field
    double left_x   = opt.absScreenXF( - SP.pitchHalfLength() );
    double right_x  = opt.absScreenXF( + SP.pitchHalfLength() );

    // set penalty area params
    double pen_top_y    = opt.absScreenYF( - SP.penaltyAreaHalfWidth() );
    double pen_bottom_y = opt.absScreenYF( + SP.penaltyAreaHalfWidth() );
    double pen_circle_y_degree_abs
        = std::acos( ( SP.penaltyAreaLength()
                       - rcsc::ServerParam::DEFAULT_PENALTY_SPOT_DIST )
                     / rcsc::ServerParam::DEFAULT_PENALTY_CIRCLE_R )
        * ( 180.0 / M_PI );
    int span_angle = qRound( pen_circle_y_degree_abs * 2.0 * 16 );
    double pen_circle_r = opt.scaleF( rcsc::ServerParam::DEFAULT_PENALTY_CIRCLE_R );
    double pen_circle_size = opt.scaleF( rcsc::ServerParam::DEFAULT_PENALTY_CIRCLE_R * 2.0 );

    // left penalty area X
    double pen_x = opt.absScreenXF( -( SP.pitchHalfLength()
                                       - SP.penaltyAreaLength() ) );
    // left arc
    double pen_spot_x = opt.absScreenXF( -( SP.pitchHalfLength()
                                            - rcsc::ServerParam::DEFAULT_PENALTY_SPOT_DIST ) );
    painter.drawArc( QRectF( pen_spot_x - pen_circle_r + 1,
                             opt.fieldCenterF().y - pen_circle_r,
                             pen_circle_size,
                             pen_circle_size ),
                     qRound( -pen_circle_y_degree_abs * 16 ),
                     span_angle );
    // left rectangle
    painter.drawLine( QLineF( left_x, pen_top_y,
                              pen_x, pen_top_y ) );
    painter.drawLine( QLineF( pen_x, pen_top_y,
                              pen_x, pen_bottom_y ) );
    painter.drawLine( QLineF( pen_x, pen_bottom_y,
                              left_x, pen_bottom_y ) );
    // left spot
    painter.drawPoint( QPointF( pen_spot_x, opt.fieldCenterF().y ) );

    // right penalty area X
    pen_x = opt.absScreenXF( +( SP.pitchHalfLength()
                                - SP.penaltyAreaLength() ) );
    // right arc
    pen_spot_x = opt.absScreenXF( +( SP.pitchHalfLength()
                                     - rcsc::ServerParam::DEFAULT_PENALTY_SPOT_DIST ) );
    painter.drawArc( QRectF( pen_spot_x - pen_circle_r,
                             opt.fieldCenterF().y - pen_circle_r,
                             pen_circle_size, pen_circle_size ),
                     qRound( ( 180.0 - pen_circle_y_degree_abs + 0.5 ) * 16 ),
                     span_angle );
    // right rectangle
    painter.drawLine( QLineF( right_x, pen_top_y,
                              pen_x, pen_top_y ) );
    painter.drawLine( QLineF( pen_x, pen_top_y,
                              pen_x, pen_bottom_y ) );
    painter.drawLine( QLineF( pen_x, pen_bottom_y,
                              right_x, pen_bottom_y ) );
    // right spot
    painter.drawPoint( QPointF( pen_spot_x, opt.fieldCenterF().y ) );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
FieldPainter::drawGoalAreaLines( QPainter & painter ) const
{
    const rcsc::ServerParam & SP = rcsc::ServerParam::i();
    const Options & opt = Options::instance();
    const DrawConfig & dconf = DrawConfig::instance();

    // set gdi objects
    painter.setPen( dconf.linePen() );
    painter.setBrush( dconf.transparentBrush() );

    // set screen coordinates of field
    double left_x   = opt.absScreenXF( - SP.pitchHalfLength() );
    double right_x  = opt.absScreenXF( + SP.pitchHalfLength() );

    // set coordinates M_confs
    double goal_area_y_abs = opt.scaleF( SP.goalAreaHalfWidth() );
    double goal_area_top_y = opt.fieldCenterF().y - goal_area_y_abs;
    double goal_area_bottom_y = opt.fieldCenterF().y + goal_area_y_abs;

    // left goal area
    double goal_area_x = opt.absScreenXF( - SP.pitchHalfLength()
                                          + SP.goalAreaLength() );
    painter.drawLine( QLineF( left_x, goal_area_top_y,
                              goal_area_x, goal_area_top_y ) );
    painter.drawLine( QLineF( goal_area_x, goal_area_top_y,
                              goal_area_x, goal_area_bottom_y ) );
    painter.drawLine( QLineF( goal_area_x, goal_area_bottom_y,
                              left_x, goal_area_bottom_y ) );

    // right goal area
    goal_area_x = opt.absScreenXF( SP.pitchHalfLength()
                                   - SP.goalAreaLength() );
    painter.drawLine( QLineF( right_x, goal_area_top_y ,
                              goal_area_x, goal_area_top_y ) );
    painter.drawLine( QLineF( goal_area_x, goal_area_top_y,
                              goal_area_x, goal_area_bottom_y ) );
    painter.drawLine( QLineF( goal_area_x, goal_area_bottom_y,
                              right_x, goal_area_bottom_y ) );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
FieldPainter::drawGoals( QPainter & painter ) const
{
    const rcsc::ServerParam & SP = rcsc::ServerParam::i();
    const Options & opt = Options::instance();

    // set gdi objects
    painter.setPen( Qt::black );
    painter.setBrush( Qt::black );

    // set coordinates param
    double goal_top_y = opt.absScreenYF( - SP.goalHalfWidth() );
    double goal_size_x = opt.scaleF( SP.goalDepth() );
    double goal_size_y = opt.scaleF( SP.goalWidth() );

    double post_top_y = opt.absScreenYF( - SP.goalHalfWidth()
                                         - SP.goalPostRadius() * 2.0 );
    double post_bottom_y = opt.absScreenYF( + SP.goalHalfWidth() );
    double post_diameter = opt.scaleF( SP.goalPostRadius() * 2.0 );

    // left goal
    painter.drawRect( QRectF( opt.absScreenXF( - SP.pitchHalfLength() - SP.goalDepth() ) - 1,
                              goal_top_y,
                              goal_size_x,
                              goal_size_y ) );
    if ( post_diameter >= 1 )
    {
        double post_x = opt.absScreenXF( - SP.pitchHalfLength() );
        painter.drawEllipse( QRectF( post_x,
                                     post_top_y,
                                     post_diameter,
                                     post_diameter ) );
        painter.drawEllipse( QRectF( post_x,
                                     post_bottom_y,
                                     post_diameter,
                                     post_diameter ) );
    }
    // right goal
    painter.drawRect( QRectF( opt.absScreenXF( SP.pitchHalfLength() ) + 1,
                              goal_top_y,
                              goal_size_x,
                              goal_size_y ) );
    if ( post_diameter >= 1 )
    {
        double post_x = opt.absScreenXF( SP.pitchHalfLength() - SP.goalPostRadius() * 2.0 );
        painter.drawEllipse( QRectF( post_x,
                                     post_top_y,
                                     post_diameter,
                                     post_diameter ) );
        painter.drawEllipse( QRectF( post_x,
                                     post_bottom_y,
                                     post_diameter,
                                     post_diameter ) );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
FieldPainter::drawFlags( QPainter & painter ) const
{
    const rcsc::ServerParam & SP = rcsc::ServerParam::i();
    const Options & opt = Options::instance();

    // set gdi objects
    painter.setPen( DrawConfig::instance().linePen() );
    painter.setBrush( DrawConfig::instance().transparentBrush() );

    // set size or coordinates params
    double flag_radius = opt.scaleF( 0.5 );
    if ( flag_radius < 2 ) flag_radius = 2;
    if ( flag_radius > 5 ) flag_radius = 5;
    double flag_diameter = flag_radius * 2;

    double x, y;
    double pitch_half_length = opt.scaleF( SP.pitchHalfLength() );
    double pitch_half_width = opt.scaleF( SP.pitchHalfWidth() );
    double pitch_margin_x = opt.scaleF( SP.pitchHalfLength() + rcsc::ServerParam::DEFAULT_PITCH_MARGIN );
    double pitch_margin_y = opt.scaleF( SP.pitchHalfWidth() + rcsc::ServerParam::DEFAULT_PITCH_MARGIN );
    double penalty_x = opt.scaleF( SP.pitchHalfLength() - SP.penaltyAreaLength() );
    double penalty_y = opt.scaleF( SP.penaltyAreaHalfWidth() );
    double goal_y = opt.scaleF( SP.goalHalfWidth() );
    double scale10 = opt.scaleF( 10.0 );
    double scale20 = opt.scaleF( 20.0 );
    double scale30 = opt.scaleF( 30.0 );
    double scale40 = opt.scaleF( 40.0 );
    double scale50 = opt.scaleF( 50.0 );

    QPainterPath path;

    // goal left
    x = opt.fieldCenterF().x - pitch_half_length - flag_radius;
    y = opt.fieldCenterF().y - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );
    // goal right
    x = opt.fieldCenterF().x + pitch_half_length - flag_radius;
    y = opt.fieldCenterF().y - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );
    // flag c
    x  = opt.fieldCenterF().x - flag_radius;
    y = opt.fieldCenterF().y - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );
    // flag c t
    x = opt.fieldCenterF().x - flag_radius;
    y = opt.fieldCenterF().y - pitch_half_width - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );

    // flag c b
    x = opt.fieldCenterF().x - flag_radius;
    y = opt.fieldCenterF().y + pitch_half_width - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );
    // flag l t
    x = opt.fieldCenterF().x - pitch_half_length - flag_radius;
    y = opt.fieldCenterF().y - pitch_half_width - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );
    // flag l b
    x = opt.fieldCenterF().x - pitch_half_length - flag_radius;
    y = opt.fieldCenterF().y + pitch_half_width - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );
    // flag r t
    x = opt.fieldCenterF().x + pitch_half_length - flag_radius;
    y = opt.fieldCenterF().y - pitch_half_width - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );
    // flag r b
    x = opt.fieldCenterF().x + pitch_half_length - flag_radius;
    y = opt.fieldCenterF().y + pitch_half_width - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );
    // flag p l t
    x = opt.fieldCenterF().x - penalty_x - flag_radius;
    y = opt.fieldCenterF().y - penalty_y - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );
    // flag p l c
    x = opt.fieldCenterF().x - penalty_x - flag_radius;
    y = opt.fieldCenterF().y - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );
    // flag p l b
    x = opt.fieldCenterF().x - penalty_x - flag_radius;
    y = opt.fieldCenterF().y + penalty_y - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );
    // flag p r t
    x = opt.fieldCenterF().x + penalty_x - flag_radius;
    y = opt.fieldCenterF().y - penalty_y - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );
    // flag p r c
    x = opt.fieldCenterF().x + penalty_x - flag_radius;
    y = opt.fieldCenterF().y - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );
    // flag p r b
    x = opt.fieldCenterF().x + penalty_x - flag_radius;
    y = opt.fieldCenterF().y + penalty_y - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );

    // flag g l t
    x = opt.fieldCenterF().x - pitch_half_length - flag_radius;
    y = opt.fieldCenterF().y - goal_y - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );
    // flag g l b
    x = opt.fieldCenterF().x - pitch_half_length - flag_radius;
    y = opt.fieldCenterF().y + goal_y - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );
    // flag g r t
    x = opt.fieldCenterF().x + pitch_half_length - flag_radius;
    y = opt.fieldCenterF().y - goal_y - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );
    // flag g r b
    x = opt.fieldCenterF().x + pitch_half_length - flag_radius;
    y = opt.fieldCenterF().y + goal_y - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );

    // flag t ...

    y = opt.fieldCenterF().y - pitch_margin_y - flag_radius;
    // flag t l 50
    x = opt.fieldCenterF().x - scale50 - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );
    // flag t l 40
    x = opt.fieldCenterF().x - scale40 - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );
    // flag t l 30
    x = opt.fieldCenterF().x - scale30 - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );
    // flag t l 20
    x = opt.fieldCenterF().x - scale20 - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );
    // flag t l 10
    x = opt.fieldCenterF().x - scale10 - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );
    // flag t 0
    x = opt.fieldCenterF().x - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );
    // flag t r 10
    x = opt.fieldCenterF().x + scale10 - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );
    // flag t r 20
    x = opt.fieldCenterF().x + scale20 - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );
    // flag t r 30
    x = opt.fieldCenterF().x + scale30 - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );
    // flag t r 40
    x = opt.fieldCenterF().x + scale40 - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );
    // flag t r 50
    x = opt.fieldCenterF().x + scale50 - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );

    // flag b ...

    y = opt.fieldCenterF().y + pitch_margin_y - flag_radius;
    // flag b l 50
    x = opt.fieldCenterF().x - scale50 - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );
    // flag b l 40
    x = opt.fieldCenterF().x - scale40 - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );
    // flag b l 30
    x = opt.fieldCenterF().x - scale30 - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );
    // flag b l 20
    x = opt.fieldCenterF().x - scale20 - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );
    // flag b l 10
    x = opt.fieldCenterF().x - scale10 - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );
    // flag b 0
    x = opt.fieldCenterF().x - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );
    // flag b r 10
    x = opt.fieldCenterF().x + scale10 - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );
    // flag b r 20
    x = opt.fieldCenterF().x + scale20 - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );
    // flag b r 30
    x = opt.fieldCenterF().x + scale30 - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );
    // flag b r 40
    x = opt.fieldCenterF().x + scale40 - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );
    // flag b r 50
    x = opt.fieldCenterF().x + scale50 - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );

    // flag l ...

    x = opt.fieldCenterF().x - pitch_margin_x - flag_radius;
    // flag l t 30
    y = opt.fieldCenterF().y - scale30 - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );
    // flag l t 20
    y = opt.fieldCenterF().y - scale20 - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );
    // flag l t 10
    y = opt.fieldCenterF().y - scale10 - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );
    // flag l 0
    y = opt.fieldCenterF().y - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );
    // flag l b 10
    y = opt.fieldCenterF().y + scale10 - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );
    // flag l b 20
    y = opt.fieldCenterF().y + scale20 - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );
    // flag l b 30
    y = opt.fieldCenterF().y + scale30 - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );

    // flag r ...

    x = opt.fieldCenterF().x + pitch_margin_x - flag_radius;
    // flag r t 30
    y = opt.fieldCenterF().y - scale30 - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );
    // flag r t 20
    y = opt.fieldCenterF().y - scale20 - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );
    // flag r t 10
    y = opt.fieldCenterF().y - scale10 - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );
    // flag r 0
    y = opt.fieldCenterF().y - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );
    // flag r b 10
    y = opt.fieldCenterF().y + scale10 - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );
    // flag r b 20
    y = opt.fieldCenterF().y + scale20 - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );
    // flag r b 30
    y = opt.fieldCenterF().y + scale30 - flag_radius;
    path.addEllipse( x, y, flag_diameter, flag_diameter );

    painter.drawPath( path );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
FieldPainter::drawGrid( QPainter & painter ) const
{
    const Options & opt = Options::instance();

    const double grid_step = 0.1 * opt.gridStep();
    const double istep = opt.scaleInt( grid_step );

    if ( istep <= 4 )
    {
        return;
    }

    const QFontMetrics metrics = painter.fontMetrics();
    const int text_step_x = ( opt.showGridCoord()
                              ? metrics.width( QObject::tr( "-00.0" ) )
                              : 100000 );
    const int text_step_y = ( opt.showGridCoord()
                              ? metrics.ascent()
                              : 100000 );

    const QRect win = painter.window();

    const int max_ix = win.right();
    const int min_ix = win.left();
    const int max_iy = win.bottom();
    const int min_iy = win.top();
    const double max_x = opt.fieldX( max_ix );
    const double min_x = opt.fieldX( min_ix );
    const double max_y = opt.fieldY( max_iy );
    const double min_y = opt.fieldY( min_iy );

    //     std::cerr << "drawGrid  min_x = " << min_x
    //               << "  max_x = " << max_x
    //               << "  min_y = " << min_y
    //               << "  max_y = " << max_y
    //               << std::endl;

    painter.setPen( DrawConfig::instance().linePen() );
    painter.setBrush( DrawConfig::instance().transparentBrush() );

    double x = 0.0;
    while ( x < max_x )
    {
        int ix = opt.absScreenXInt( x );
        if ( istep > text_step_x )
        {
            painter.drawText( QPoint( ix, max_iy ), QString::number( x ) );
        }
        painter.drawLine( QLine( ix, max_iy, ix, min_iy ) );
        x += grid_step;
    }

    x = -grid_step;
    while ( min_x < x )
    {
        int ix = opt.absScreenXInt( x );
        if ( istep > text_step_x )
        {
            painter.drawText( QPoint( ix, max_iy ), QString::number( x ) );
        }
        painter.drawLine( QLine( ix, max_iy, ix, min_iy ) );
        x -= grid_step;
    }


    double y = 0.0;
    while ( y < max_y )
    {
        int iy = opt.absScreenYInt( y );
        if ( istep > text_step_y )
        {
            painter.drawText( QPoint( min_ix, iy ), QString::number( y ) );
        }
        painter.drawLine( QLine( max_ix, iy, min_ix, iy ) );
        y += grid_step;
    }

    y = -grid_step;
    while ( min_y < y )
    {
        int iy = opt.absScreenYInt( y );
        if ( istep > text_step_y )
        {
            painter.drawText( QPoint( min_ix, iy ), QString::number( y ) );
        }
        painter.drawLine( QLine( max_ix, iy, min_ix, iy ) );
        y -= grid_step;
    }
}
