// -*-c++-*-

/*!
  \file field_canvas.cpp
  \brief main field canvas class Source File.
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

#include "field_canvas.h"

#include "draw_config.h"
#include "field_painter.h"
#include "score_board_painter.h"
#include "score_board_painter_rcss.h"
#include "ball_painter.h"
#include "ball_painter_rcss.h"
#include "player_painter.h"
#include "player_painter_rcss.h"
#include "offside_line_painter.h"
#include "ball_trace_painter.h"
#include "player_control_painter.h"
#include "player_trace_painter.h"
#include "voronoi_diagram_painter.h"
#include "debug_log_painter.h"
#include "debug_painter.h"
//#include "field_evaluation_painter.h"
#include "team_graphic_painter.h"
#include "draw_data_painter.h"
#include "formation_editor_painter.h"

// model
#include "main_data.h"
#include "options.h"
#include "formation_edit_data.h"

#include <rcsc/common/server_param.h>

#include <iostream>
#include <memory>
#include <cmath>
#include <cassert>

/*-------------------------------------------------------------------*/
/*!

*/
FieldCanvas::FieldCanvas( MainData & main_data )
    :
#ifdef USE_GLWIDGET
    QGLWidget( QGLFormat( QGL::SampleBuffers ) ),
#endif
    M_main_data( main_data ),
    M_redraw_all( true ),
    M_normal_menu( static_cast< QMenu * >( 0 ) ),
    M_system_menu( static_cast< QMenu * >( 0 ) ),
    M_monitor_menu( static_cast< QMenu * >( 0 ) ),
    M_offline_menu( static_cast< QMenu * >( 0 ) ),
    M_cursor_timer( new QTimer( this ) ),
    M_paint_style( -1 )
{
    //this->setPalette( M_main_data.drawConfig().fieldBrush().color() );
    //this->setAutoFillBackground( true );

    // need for the MouseMoveEvent
    this->setMouseTracking( true );

    this->setFocusPolicy( Qt::WheelFocus );

    // paint directory
    //this->setAttribute( Qt::WA_PaintOnScreen );

    M_field_painter = std::shared_ptr< FieldPainter >( new FieldPainter() );
    M_formation_editor_painter = std::shared_ptr< FormationEditorPainter >( new FormationEditorPainter( M_main_data ) );

    createPainters();

    M_cursor_timer->setSingleShot( true );
    connect( M_cursor_timer, SIGNAL( timeout() ),
             this, SLOT( hideCursor() ) );
}

/*-------------------------------------------------------------------*/
/*!

*/
FieldCanvas::~FieldCanvas()
{
    //std::cerr << "delete FieldCanvas" << std::endl;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
FieldCanvas::createPainters()
{
    Options::PaintStyle paint_style = Options::instance().paintStyle();
    if ( M_paint_style == static_cast< int >( paint_style ) )
    {
        return;
    }

    M_paint_style = static_cast< int >( paint_style );
    M_painters.clear();

    if ( paint_style == Options::PAINT_RCSSMONITOR )
    {
        //M_painters.push_back( std::shared_ptr< PainterInterface >( new FieldEvaluationPainter( M_main_data ) ) );
        M_painters.push_back( std::shared_ptr< PainterInterface >( new TeamGraphicPainter( M_main_data ) ) );
        M_painters.push_back( std::shared_ptr< PainterInterface >( new BallTracePainter( M_main_data ) ) );
        M_painters.push_back( std::shared_ptr< PainterInterface >( new PlayerTracePainter( M_main_data ) ) );
        M_painters.push_back( std::shared_ptr< PainterInterface >( new PlayerPainterRCSS( M_main_data ) ) );
        M_painters.push_back( std::shared_ptr< PainterInterface >( new BallPainterRCSS( M_main_data ) ) );
        M_painters.push_back( std::shared_ptr< PainterInterface >( new OffsideLinePainter( M_main_data ) ) );
        M_painters.push_back( std::shared_ptr< PainterInterface >( new PlayerControlPainter( M_main_data ) ) );
        M_painters.push_back( std::shared_ptr< PainterInterface >( new VoronoiDiagramPainter( M_main_data ) ) );
        M_painters.push_back( std::shared_ptr< PainterInterface >( new DebugPainter( M_main_data ) ) );
        M_painters.push_back( std::shared_ptr< PainterInterface >( new DebugLogPainter( M_main_data ) ) );
        M_painters.push_back( std::shared_ptr< PainterInterface >( new DrawDataPainter( M_main_data ) ) );
        M_painters.push_back( std::shared_ptr< PainterInterface >( new ScoreBoardPainterRCSS( M_main_data ) ) );
    }
    else
    {
        if ( paint_style != Options::PAINT_DEFAULT )
        {
            std::cerr << __FILE__ << ": ***WARNING*** (createPainters) "
                      << "Unsupported paint style : " << paint_style << std::endl;
        }

        //M_painters.push_back( std::shared_ptr< PainterInterface >( new FieldEvaluationPainter( M_main_data ) ) );
        M_painters.push_back( std::shared_ptr< PainterInterface >( new TeamGraphicPainter( M_main_data ) ) );
        M_painters.push_back( std::shared_ptr< PainterInterface >( new BallTracePainter( M_main_data ) ) );
        M_painters.push_back( std::shared_ptr< PainterInterface >( new PlayerTracePainter( M_main_data ) ) );
        M_painters.push_back( std::shared_ptr< PainterInterface >( new PlayerPainter( M_main_data ) ) );
        M_painters.push_back( std::shared_ptr< PainterInterface >( new BallPainter( M_main_data ) ) );
        M_painters.push_back( std::shared_ptr< PainterInterface >( new OffsideLinePainter( M_main_data ) ) );
        M_painters.push_back( std::shared_ptr< PainterInterface >( new VoronoiDiagramPainter( M_main_data ) ) );
        M_painters.push_back( std::shared_ptr< PainterInterface >( new DebugPainter( M_main_data ) ) );
        M_painters.push_back( std::shared_ptr< PainterInterface >( new DebugLogPainter( M_main_data ) ) );
        M_painters.push_back( std::shared_ptr< PainterInterface >( new DrawDataPainter( M_main_data ) ) );
        M_painters.push_back( std::shared_ptr< PainterInterface >( new ScoreBoardPainter( M_main_data ) ) );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
FieldCanvas::setNormalMenu( QMenu * menu )
{
    if ( M_normal_menu )
    {
        delete M_normal_menu;
        M_normal_menu = static_cast< QMenu * >( 0 );
    }

    M_normal_menu = menu;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
FieldCanvas::setSystemMenu( QMenu * menu )
{
    if ( M_system_menu )
    {
        delete M_system_menu;
        M_system_menu = static_cast< QMenu * >( 0 );
    }

    M_system_menu = menu;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
FieldCanvas::setMonitorMenu( QMenu * menu )
{
    if ( M_monitor_menu )
    {
        delete M_monitor_menu;
        M_monitor_menu = static_cast< QMenu * >( 0 );
    }

    M_monitor_menu = menu;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
FieldCanvas::setOfflineMenu( QMenu * menu )
{
    if ( M_offline_menu )
    {
        delete M_offline_menu;
        M_offline_menu = static_cast< QMenu * >( 0 );
    }

    M_offline_menu = menu;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
FieldCanvas::setRedrawAllFlag()
{
    M_redraw_all = true;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
FieldCanvas::redrawAll()
{
    M_redraw_all = true;
    this->update();
}

/*-------------------------------------------------------------------*/
/*!

*/
void
FieldCanvas::dropBall()
{
    emit dropBall( M_mouse_state[0].pressedPoint() );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
FieldCanvas::freeKickLeft()
{
    emit freeKickLeft( M_mouse_state[0].pressedPoint() );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
FieldCanvas::freeKickRight()
{
    emit freeKickRight( M_mouse_state[0].pressedPoint() );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
FieldCanvas::changePlayMode( int mode )
{
    emit playModeChanged( mode, M_mouse_state[0].pressedPoint() );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
FieldCanvas::paintEvent( QPaintEvent * )
{
    QPainter painter( this );

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

    painter.setRenderHint( QPainter::TextAntialiasing, false );

    M_main_data.update( this->width(), this->height() );

    {
        const Options & opt = Options::instance();
        const double offset_x = painter.viewport().width() * 0.5 - opt.focusPoint().x * opt.fieldScale();
        const double offset_y = ( painter.viewport().height() - opt.scoreBoardHeight() ) * 0.5
            - opt.focusPoint().y * opt.fieldScale()
            + opt.scoreBoardHeight();
        M_transform.reset();
        M_transform.translate( offset_x, offset_y );
        M_transform.scale( opt.fieldScale(), opt.fieldScale() );
    }

    draw( painter );

    // draw mouse measure

    if ( M_mouse_state[2].isDragged() )
    {
        drawMouseMeasure( painter );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
FieldCanvas::mouseDoubleClickEvent( QMouseEvent * event )
{
    this->unsetCursor();

    if ( event->button() == Qt::LeftButton )
    {
        emit focusChanged( event->pos() );
    }

    if ( Options::instance().cursorHide() )
    {
        M_cursor_timer->start( Options::CURSOR_TIMEOUT );
    }

    QWidget::mouseDoubleClickEvent( event );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
FieldCanvas::mousePressEvent( QMouseEvent * event )
{
    if ( event->button() == Qt::LeftButton )
    {
        M_mouse_state[0].pressed( event->pos() );

        if ( Options::instance().feditMode() )
        {
            if ( event->modifiers() == 0 )
            {
                if ( std::shared_ptr< FormationEditData > ptr = M_main_data.formationEditData() )
                {
                    QPointF field_pos = M_transform.inverted().map( QPointF( event->pos() ) );
                    if ( ptr->selectObject( field_pos.x(), field_pos.y() ) )
                    {
                        this->update();
                    }
                }
            }
        }
        else
        {
            if ( M_mouse_state[2].isPressed()
                 && M_mouse_state[2].isDragged() )
            {
                M_measure_first_length_point = event->pos();
            }
        }

    }
    else if ( event->button() == Qt::MidButton )
    {
        M_mouse_state[1].pressed( event->pos() );
    }
    else if ( event->button() == Qt::RightButton )
    {
        M_mouse_state[2].pressed( event->pos() );

        if ( Options::instance().feditMode() )
        {
            if ( event->modifiers() == 0 )
            {
                if ( std::shared_ptr< FormationEditData > ptr = M_main_data.formationEditData() )
                {
                    QPointF field_pos = M_transform.inverted().map( QPointF( event->pos() ) );
                    ptr->moveBallTo( field_pos.x(), field_pos.y() );
                    emit feditObjectMoved();
                    this->update();
                }
            }
        }
        else
        {
            M_measure_first_length_point = event->pos();
        }
    }

    if ( Options::instance().cursorHide() )
    {
        M_cursor_timer->stop();
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
FieldCanvas::mouseReleaseEvent( QMouseEvent * event )
{
    this->unsetCursor();

    if ( event->button() == Qt::LeftButton )
    {
        M_mouse_state[0].released();
        if ( M_mouse_state[0].isMenuFailed() )
        {
            M_mouse_state[0].setMenuFailed( false );
        }

        if ( Options::instance().feditMode() )
        {
            if ( std::shared_ptr< FormationEditData > ptr = M_main_data.formationEditData() )
            {
                if ( ptr->releaseObject() )
                {
                    this->update();
                }
            }
        }
        else if ( Options::instance().monitorClientMode() )
        {
            if ( M_mouse_state[0].pressedPoint() == event->pos()
                 && M_monitor_menu )
            {
                M_monitor_menu->exec( event->globalPos() );
            }
        }
    }
    else if ( event->button() == Qt::MidButton )
    {
        M_mouse_state[1].released();
        if ( M_mouse_state[1].isMenuFailed() )
        {
            M_mouse_state[1].setMenuFailed( false );
        }
    }
    else if ( event->button() == Qt::RightButton )
    {
        M_mouse_state[2].released();
        if ( M_mouse_state[2].isMenuFailed() )
        {
            M_mouse_state[2].setMenuFailed( false );
        }

        if ( Options::instance().feditMode() )
        {

        }
        else if ( Options::instance().monitorClientMode() )
        {
            if ( M_system_menu
                 && M_mouse_state[2].pressedPoint() == event->pos() )
            {
                M_system_menu->exec( event->globalPos() );
            }
        }
        else
        {
            if ( M_normal_menu
                 && M_mouse_state[2].pressedPoint() == event->pos() )
            {
                M_normal_menu->exec( event->globalPos() );
            }
        }
    }

    if ( Options::instance().cursorHide() )
    {
        M_cursor_timer->start( Options::CURSOR_TIMEOUT );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
FieldCanvas::mouseMoveEvent( QMouseEvent * event )
{
    if ( this->cursor().shape() == Qt::BlankCursor )
    {
        this->unsetCursor();
    }

    const QPointF field_pos = M_transform.inverted().map( QPointF( event->pos() ) );

    if ( M_mouse_state[0].isDragged() )
    {
        if ( Options::instance().feditMode() )
        {
            if ( event->modifiers() == 0 )
            {
                if ( std::shared_ptr< FormationEditData > ptr = M_main_data.formationEditData() )
                {
                    if ( ptr->moveSelectObjectTo( field_pos.x(), field_pos.y() ) )
                    {
                        emit feditObjectMoved();
                        this->update();
                    }
                }
            }
            else if ( event->modifiers() & Qt::ControlModifier )
            {
                if ( this->cursor().shape() != Qt::ClosedHandCursor )
                {
                    this->setCursor( QCursor( Qt::ClosedHandCursor ) );
                }

                double new_x = Options::instance().absScreenX( Options::instance().focusPoint().x );
                double new_y = Options::instance().absScreenY( Options::instance().focusPoint().y );
                new_x -= ( event->pos().x() - M_mouse_state[0].draggedPoint().x() );
                new_y -= ( event->pos().y() - M_mouse_state[0].draggedPoint().y() );
                emit focusChanged( QPoint( new_x, new_y ) );
            }
        }
        else
        {
            if ( this->cursor().shape() != Qt::ClosedHandCursor )
            {
                this->setCursor( QCursor( Qt::ClosedHandCursor ) );
            }

            double new_x = Options::instance().absScreenX( Options::instance().focusPoint().x );
            double new_y = Options::instance().absScreenY( Options::instance().focusPoint().y );
            new_x -= ( event->pos().x() - M_mouse_state[0].draggedPoint().x() );
            new_y -= ( event->pos().y() - M_mouse_state[0].draggedPoint().y() );
            emit focusChanged( QPoint( new_x, new_y ) );
        }
    }


    if ( M_mouse_state[2].isDragged() )
    {
        if ( Options::instance().feditMode() )
        {

        }
        else
        {
            static QRect s_last_rect;
            QRect new_rect = QRect( M_mouse_state[2].pressedPoint(), M_mouse_state[2].draggedPoint() ).normalized();
            new_rect.adjust( -128, -128, 128, 128 );
            if ( new_rect.right() < M_mouse_state[2].draggedPoint().x() + 512 )
            {
                new_rect.setRight( M_mouse_state[2].draggedPoint().x() + 512 );
            }
            // draw mouse measure
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
            this->update( s_last_rect.united( new_rect ) );
#else
            this->update( s_last_rect.unite( new_rect ) );
#endif
            s_last_rect = new_rect;
        }
    }

    for ( int i = 0; i < 3; ++i )
    {
        M_mouse_state[i].moved( event->pos() );
    }

    if ( Options::instance().cursorHide() )
    {
        if ( M_mouse_state[0].isPressed()
             || M_mouse_state[1].isPressed()
             || M_mouse_state[2].isPressed() )
        {
            M_cursor_timer->stop();
        }
        else
        {
            M_cursor_timer->start( Options::CURSOR_TIMEOUT );
        }
    }

    emit mouseMoved( event->pos() );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
FieldCanvas::draw( QPainter & painter )
{
    createPainters();

    M_field_painter->draw( painter );

    if ( M_main_data.getCurrentViewData() )
    {
        for ( auto p : M_painters )
        {
            p->draw( painter );
        }
    }

    if ( Options::instance().feditMode() )
    {
        M_formation_editor_painter->draw( painter );
    }

    M_redraw_all = false;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
FieldCanvas::drawMouseMeasure( QPainter & painter )
{
    const Options & opt = Options::instance();

    QPoint start_point = M_mouse_state[2].pressedPoint();
    QPoint end_point = M_mouse_state[2].draggedPoint();

    // draw straight line
    painter.setPen( DrawConfig::instance().measurePen() );
    painter.setBrush( DrawConfig::instance().transparentBrush() );
    painter.drawLine( QLine( start_point, end_point ) );

    QPainterPath mark_path;
    QPainterPath text_path;
    mark_path.addEllipse( start_point.x() - 2,
                          start_point.y() - 2,
                          4,
                          4 );
    mark_path.addEllipse( end_point.x() - 2,
                          end_point.y() - 2,
                          4,
                          4 );

    if ( opt.mouseMeasureMode() == Options::MEASURE_BALL_MOVE )
    {
        createBallMovePath( start_point, end_point, mark_path, text_path );
    }
    else if ( opt.mouseMeasureMode() == Options::MEASURE_PLAYER_MOVE )
    {
        createPlayerMovePath( start_point, end_point, mark_path, text_path );
    }

    painter.setPen( QPen( Qt::red, 0, Qt::SolidLine ) );
    painter.drawPath( mark_path );

    // draw distance & angle text
    painter.setFont( DrawConfig::instance().measureFont() );
    painter.setPen( DrawConfig::instance().measureFontPen() );
    //painter.setBackgroundMode( Qt::TransparentMode );
    painter.drawPath( text_path );

    //
    // draw text
    //

    rcsc::Vector2D start_real( opt.fieldX( start_point.x() ),
                               opt.fieldY( start_point.y() ) );
    rcsc::Vector2D end_real( opt.fieldX( end_point.x() ),
                             opt.fieldY( end_point.y() ) );

    char buf[64];
    snprintf( buf, 64,
              "(%.2f,%.2f)",
              start_real.x,
              start_real.y );

    painter.drawText( start_point,
                      QString::fromLatin1( buf ) );

    if ( std::abs( start_point.x() - end_point.x() ) < 1
         && std::abs( start_point.y() - end_point.y() ) < 1 )
    {
        return;
    }

    snprintf( buf, 64,
              "(%.2f,%.2f)",
              end_real.x,
              end_real.y );

    painter.drawText( end_point, QString::fromLatin1( buf ) );

    painter.setPen( QColor( 224, 224, 192 ) );
    rcsc::Vector2D rel( end_real - start_real );
    snprintf( buf, 64,
              "rel(%.2f,%.2f) r%.2f th%.1f",
              rel.x, rel.y, rel.r(), rel.th().degree() );

    int dist_add_y = ( end_point.y() > start_point.y()
                       ? + painter.fontMetrics().height()
                       : - painter.fontMetrics().height() );
    painter.drawText( QPoint( end_point.x(),
                              end_point.y() + dist_add_y ),
                      QString::fromLatin1( buf ) );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
FieldCanvas::createBallMovePath( const QPoint & start_point,
                                 const QPoint & end_point,
                                 QPainterPath & mark_path,
                                 QPainterPath & text_path )
{
    const rcsc::ServerParam & SP = rcsc::ServerParam::i();
    const Options & opt = Options::instance();


    rcsc::Vector2D start_real( opt.fieldX( start_point.x() ),
                               opt.fieldY( start_point.y() ) );
    rcsc::Vector2D end_real( opt.fieldX( end_point.x() ),
                             opt.fieldY( end_point.y() ) );


    double ball_speed = ( opt.mouseMeasureFirstLength() < 0.0
                          ? SP.ballSpeedMax()
                          : std::min( opt.mouseMeasureFirstLength(), SP.ballSpeedMax() ) );

    if ( ! M_measure_first_length_point.isNull()
         && M_measure_first_length_point != start_point )
    {
        rcsc::Vector2D vel( std::abs( M_measure_first_length_point.x()
                                      - start_point.x() )
                            / opt.fieldScale(),
                            std::abs( M_measure_first_length_point.y()
                                      - start_point.y() )
                            / opt.fieldScale() );
        ball_speed = vel.r();
        if ( ball_speed > SP.ballSpeedMax() )
        {
            ball_speed = SP.ballSpeedMax();
        }
    }

    rcsc::Vector2D ball_pos = start_real;
    rcsc::Vector2D ball_vel = end_real - start_real;
    ball_vel.setLength( ball_speed );

    const double max_length = start_real.dist( end_real );
    double total_travel = 0.0;

    QPointF last_pos( opt.absScreenX( ball_pos.x ),
                      opt.absScreenY( ball_pos.y ) );

    QFont font = DrawConfig::instance().measureFont();
    font.setPointSize( 6 );

    int count = 0;
    for ( int i = 1; i <= 100; ++i )
    {
        ++count;
        total_travel += ball_speed;

        if ( total_travel >= max_length )
        {
            break;
        }

        if ( ball_speed < 0.001 )
        {
            break;
        }

        ball_pos += ball_vel;
        ball_vel *= SP.ballDecay();
        ball_speed *= SP.ballDecay();

        QPoint pos( opt.absScreenX( ball_pos.x ),
                    opt.absScreenY( ball_pos.y ) );
        double d2
            = std::pow( pos.x() - last_pos.x(), 2 )
            + std::pow( pos.y() - last_pos.y(), 2 );
        if ( d2 < 16.0 )
        {
            //continue;
            break;
        }

        mark_path.addEllipse( pos.x() - 1.0, pos.y() - 1.0, 2.0, 2.0 );
        //mark_path.addEllipse( pos, 2.0, 2.0 );
        if ( i <= 10
             || d2 > std::pow( 13.0, 2 ) )
        {
#if 1
            text_path.addText( pos, font,
                               tr( "%1:%2")
                               .arg( i )
                               .arg( ball_speed, 0, 'g', 3  ) );
#else
            text_path.addText( pos, font, QString::number( i ) );
#endif
        }

        last_pos = pos;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
FieldCanvas::createPlayerMovePath( const QPoint & start_point,
                                   const QPoint & end_point,
                                   QPainterPath & mark_path,
                                   QPainterPath & text_path )
{
    const rcsc::ServerParam & SP = rcsc::ServerParam::i();
    const Options & opt = Options::instance();

    if ( opt.selectedNumber() == 0 )
    {
        // no selected player
        //std::cerr << __FILE__ << ": (createPlayerMovePath) "
        //          << "no selected player." << std::endl;
        return;
    }

    //
    // decide player type
    //

    MonitorViewData::ConstPtr view = M_main_data.getCurrentViewData();

    if ( ! view )
    {
        //std::cerr << __FILE__ <<  ": (createPlayerMovePath) "
        //          << "no view data." << std::endl;
        return;
    }

    rcsc::SideID side = ( opt.selectedNumber() > 0
                          ? rcsc::LEFT
                          : rcsc::RIGHT );
    int unum = std::abs( opt.selectedNumber() );
    int player_type = -1;
    for ( const rcsc::rcg::PlayerT & p : view->players() )
    {
        if ( p.side() == side
             && p.unum() == unum )
        {
            player_type = p.type();
            break;
        }
    }

    if ( player_type < 0 )
    {
        std::cerr << __FILE__ << ": (createPlayerMovePath) "
                  << "invalid player type id=" << player_type << std::endl;
        return;
    }

    const rcsc::PlayerType & ptype = M_main_data.viewHolder().playerType( player_type );

    if ( opt.scale( ptype.realSpeedMax() ) <= 4.0 )
    {
        return;
    }

    const rcsc::Vector2D start_real( opt.fieldX( start_point.x() ),
                                     opt.fieldY( start_point.y() ) );
    const rcsc::Vector2D end_real( opt.fieldX( end_point.x() ),
                                   opt.fieldY( end_point.y() ) );

    const rcsc::Vector2D player_accel
        = ( end_real - start_real ).setLengthVector( SP.maxDashPower()
                                                     * ptype.dashPowerRate()
                                                     * ptype.effortMax() );

    const double max_length = start_real.dist( end_real );
    double total_travel = 0.0;

    rcsc::Vector2D player_pos = start_real;
#if 1
    rcsc::Vector2D player_vel( 0.0, 0.0 );
#else
    double player_speed = ( opt.mouseMeasureFirstLength() < 0.0
                            ? 0.0
                            : std::min( opt.mouseMeasureFirstLength(), ptype.realSpeedMax() ) );

    if ( ! M_measure_first_length_point.isNull()
         && M_measure_first_length_point != start_point )
    {
        rcsc::Vector2D len( std::abs( M_measure_first_length_point.x()
                                      - start_point.x() )
                            / opt.fieldScale(),
                            std::abs( M_measure_first_length_point.y()
                                      - start_point.y() )
                            / opt.fieldScale() );
        player_speed = len.r();
        if ( player_speed > ptype.realSpeedMax() )
        {
            player_speed = ptype.realSpeedMax();
        }
    }

    rcsc::Vector2D player_vel = ( end_real - start_real ).setLengthVector( player_speed );

    player_pos += player_vel;
    player_vel *= ptype.playerDecay();
    total_travel += player_vel.r();

    {
        QPointF pt( opt.absScreenX( player_pos.x ),
                    opt.absScreenY( player_pos.y ) );
        mark_path.addEllipse( pt.x() - 1.0, pt.y() - 1.0, 2.0, 2.0 );
    }
#endif

    // std::cerr << "pos=" << player_pos
    //           << " vel=" << player_vel.r()
    //           << " accel=" << player_accel << ":" << player_accel.r()
    //           << " max_length=" << max_length
    //           << std::endl;

    QFont font = DrawConfig::instance().measureFont();
    font.setPointSize( 6 );

    const bool draw_text = ( opt.scale( ptype.realSpeedMax() ) > 8.0 );

    for ( int i = 1; i <= 100; ++i )
    {
        player_vel += player_accel;
        total_travel += player_vel.r();

        if ( total_travel > max_length )
        {
            break;
        }

        player_pos += player_vel;
        player_vel *= ptype.playerDecay();

        QPointF pos( opt.absScreenX( player_pos.x ),
                     opt.absScreenY( player_pos.y ) );

        mark_path.addEllipse( pos.x() - 1.0, pos.y() - 1.0,
                              2.0, 2.0 );
        //mark_path.addEllipse( pos, 2.0, 2.0 );
        if ( draw_text
             && i <= 30 )
        {
            text_path.addText( pos, font,
                               tr( "%1:%2" ).arg( i ).arg( total_travel, 0, 'g', 3 ) );
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
FieldCanvas::hideCursor()
{
    this->setCursor( QCursor( Qt::BlankCursor ) );
}
