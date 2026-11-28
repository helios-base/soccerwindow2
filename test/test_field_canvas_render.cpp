// -*-c++-*-

/*!
  \file test_field_canvas_render.cpp
  \brief unit test of FieldCanvas::renderFrame(), FrameScope and File >
  Save Image of This Frame.

  docs/plan/2026-11-28-00_frame-export.md 4.1, §6, D5, S4, S5.
  test/data/head_v5.rcg has 3 frames: the cycles 1, 2 and 4. The ball moves
  only in the last one.

  HOME, XDG_CONFIG_HOME and XDG_RUNTIME_DIR are set to a temporary directory
  before QApplication is created, so the user's ~/.soccerwindow2 is neither
  read nor modified (MainWindow reads and writes it). The test always uses
  the offscreen platform (the device pixel ratio 1).
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

#include <QtWidgets>
#include <QtTest>

#include "draw_config.h"
#include "field_canvas.h"
#include "formation_edit_data.h"
#include "frame_scope.h"
#include "main_data.h"
#include "main_window.h"
#include "options.h"
#include "view_holder.h"

#include "test_environment.h"

#include <cstdlib>
#include <memory>

#ifndef SOCCERWINDOW2_TEST_DATA_DIR
#error "SOCCERWINDOW2_TEST_DATA_DIR is not defined."
#endif

namespace {

const QString LOG = QStringLiteral( SOCCERWINDOW2_TEST_DATA_DIR "/head_v5.rcg" );

//! the image as RGB32, to compare
QImage
rgb32( const QImage & image )
{
    return image.convertToFormat( QImage::Format_RGB32 );
}

/*!
  the view drawn by its paintEvent() into an image (QWidget::render()). The
  view draws the texts without antialiasing and the images with grayscale
  antialiasing (D5): compared with renderFrame() without texts (showTexts()).
*/
QImage
drawn( QWidget & widget )
{
    const qreal dpr = widget.devicePixelRatioF();
    QImage image( widget.size() * dpr, QImage::Format_RGB32 );
    image.setDevicePixelRatio( dpr );
    widget.render( &image );
    return image;
}

//! how two images differ: the number of the pixels and their bounding box, or "same"
QString
differenceOf( const QImage & a,
              const QImage & b )
{
    if ( a.size() != b.size() )
    {
        return QString( "sizes %1x%2 and %3x%4" )
            .arg( a.width() ).arg( a.height() ).arg( b.width() ).arg( b.height() );
    }
    const QImage x = rgb32( a );
    const QImage y = rgb32( b );
    int count = 0;
    QRect box;
    for ( int j = 0; j < x.height(); ++j )
    {
        for ( int i = 0; i < x.width(); ++i )
        {
            if ( x.pixel( i, j ) != y.pixel( i, j ) )
            {
                ++count;
                box |= QRect( i, j, 1, 1 );
            }
        }
    }
    if ( count == 0 )
    {
        return QStringLiteral( "same" );
    }
    if ( ! qEnvironmentVariableIsEmpty( "SOCCERWINDOW2_TEST_DUMP_DIR" ) )
    {
        static int s_dumped = 0;
        const QString dir = qEnvironmentVariable( "SOCCERWINDOW2_TEST_DUMP_DIR" );
        ++s_dumped;
        x.save( QString( "%1/diff%2-a.png" ).arg( dir ).arg( s_dumped ) );
        y.save( QString( "%1/diff%2-b.png" ).arg( dir ).arg( s_dumped ) );
    }
    return QString( "%1 pixels differ in (%2,%3) %4x%5" )
        .arg( count ).arg( box.x() ).arg( box.y() ).arg( box.width() ).arg( box.height() );
}

//! the texts of the field: of the players (the numbers, the types and the stamina) and the score board
struct Texts {
    bool number;
    bool type;
    bool stamina;
    bool capacity;
    bool board;
};

//! the texts shown now
Texts
textsShown()
{
    const Options & opt = Options::instance();
    return Texts{ opt.showPlayerNumber(), opt.showPlayerType(),
                  opt.showStamina(), opt.showStaminaCapacity(),
                  opt.showScoreBoard() };
}

void
showTexts( const Texts & t )
{
    Options & opt = Options::instance();
    if ( opt.showPlayerNumber() != t.number ) opt.toggleShowPlayerNumber();
    if ( opt.showPlayerType() != t.type ) opt.toggleShowPlayerType();
    if ( opt.showStamina() != t.stamina ) opt.toggleShowStamina();
    if ( opt.showStaminaCapacity() != t.capacity ) opt.toggleShowStaminaCapacity();
    if ( opt.showScoreBoard() != t.board ) opt.toggleShowScoreBoard();
}

void
showTexts( const bool on )
{
    showTexts( Texts{ on, on, on, on, on } );
}

//! the state of the view, to compare
QString
textOf( const Options::ViewState & s )
{
    return QString( "scale %1 center (%2 %3) canvas %4x%5 board %6 (%7 %8) focus (%9 %10) selected %11" )
        .arg( s.field_scale ).arg( s.field_center.x ).arg( s.field_center.y )
        .arg( s.canvas_width ).arg( s.canvas_height )
        .arg( s.score_board_height )
        .arg( s.sized_with_score_board ).arg( s.sized_paint_style )
        .arg( s.focus_point.x ).arg( s.focus_point.y )
        .arg( s.selected_number );
}

//! the leftmost pixel of a row much lighter than the field (the left goal line), or -1.
//! In an image smaller than the layout, the lines are blended with the field.
int
leftLine( const QImage & image,
          const int y )
{
    const int field = DrawConfig::instance().fieldBrush().color().lightness();
    for ( int x = 0; x < image.width(); ++x )
    {
        if ( image.pixelColor( x, y ).lightness() > field + 40 )
        {
            return x;
        }
    }
    return -1;
}

}

/*!
  \brief unit test of drawing a frame into an image
*/
class TestFieldCanvasRender
    : public QObject {
    Q_OBJECT

private:

    MainData M_main_data;
    //! the texts shown by default: set back for each test
    Texts M_texts = { true, false, true, false, true };

private slots:

    void initTestCase()
      {
          QVERIFY( M_main_data.openRCG( LOG.toStdString() ) );
          QCOMPARE( M_main_data.viewHolder().monitorViewCont().size(), std::size_t( 3 ) );
          M_texts = textsShown();
      }

    void init()
      {
          Options & opt = Options::instance();
          opt.fitToScreen();
          opt.setFocusType( Options::FOCUS_POINT );
          opt.updateFocusPoint( 0.0, 0.0 );
          opt.unselectAgent();
          showTexts( M_texts );
          QVERIFY( M_main_data.setViewDataIndex( 1 ) || M_main_data.viewIndex() == 1 );
      }

    void cleanup()
      {
          showTexts( M_texts );
      }

    void sameAsView();
    void otherFrame();
    void stateKept();
    void formationEditKept();
    void devicePixelRatio();
    void zoomed();
    void emptySize();
    void saveImage();
};

/*-------------------------------------------------------------------*/
/*!
  The frame shown, as large as the view: the same image as the view draws,
  without the texts (the view draws them without antialiasing, the image
  with grayscale antialiasing. D5).
  The score board is hidden after the view was drawn: the scale is fit
  again for the image, and the view's is set back (the items shown when the
  scale was fit are a part of Options::ViewState, not static variables).
*/
void
TestFieldCanvasRender::sameAsView()
{
    FieldCanvas canvas( M_main_data );
    canvas.resize( 640, 480 );
    canvas.show();
    QVERIFY( QTest::qWaitForWindowExposed( &canvas ) );

    showTexts( false );
    const QImage image = canvas.renderFrame( 1, canvas.size(), 1.0 );
    QCOMPARE( image.size(), QSize( 640, 480 ) );
    QCOMPARE( image.format(), QImage::Format_RGB32 );
    QCOMPARE( differenceOf( image, drawn( canvas ) ), QString( "same" ) );
    QCOMPARE( differenceOf( image, canvas.grab().toImage() ), QString( "same" ) );

    // the texts drawn, the same each time
    showTexts( true );
    const QImage texts = canvas.renderFrame( 1, canvas.size(), 1.0 );
    QVERIFY( texts != image );
    QVERIFY( canvas.renderFrame( 1, canvas.size(), 1.0 ) == texts );
}

/*-------------------------------------------------------------------*/
//! another frame: as the view draws it when it is shown; the frame shown stays
void
TestFieldCanvasRender::otherFrame()
{
    showTexts( false );
    FieldCanvas canvas( M_main_data );
    canvas.resize( 640, 480 );
    canvas.show();
    QVERIFY( QTest::qWaitForWindowExposed( &canvas ) );

    const QImage shown = drawn( canvas );
    const QImage other = canvas.renderFrame( 2, canvas.size(), 1.0 );
    QCOMPARE( M_main_data.viewIndex(), std::size_t( 1 ) );
    QVERIFY( other != shown );
    QCOMPARE( differenceOf( drawn( canvas ), shown ), QString( "same" ) );

    QVERIFY( M_main_data.setViewDataIndex( 2 ) );
    QCOMPARE( differenceOf( drawn( canvas ), other ), QString( "same" ) );

    // an index out of the frames: the frame shown
    QVERIFY( canvas.renderFrame( 100, canvas.size(), 1.0 ) == other );
    QCOMPARE( M_main_data.viewIndex(), std::size_t( 2 ) );
}

/*-------------------------------------------------------------------*/
/*!
  The state of the view after drawing other frames in other sizes: the
  focus on the ball, the zoom and the player selected automatically.
*/
void
TestFieldCanvasRender::stateKept()
{
    Options & opt = Options::instance();
    opt.setFocusType( Options::FOCUS_BALL );
    opt.setAgentSelectType( Options::SELECT_AUTO_ALL );
    opt.zoomIn();
    showTexts( false );

    FieldCanvas canvas( M_main_data );
    canvas.resize( 640, 480 );
    canvas.show();
    QVERIFY( QTest::qWaitForWindowExposed( &canvas ) );
    const QImage view = drawn( canvas );
    const QString state = textOf( opt.viewState() );

    QVERIFY( ! canvas.renderFrame( 0, QSize( 320, 240 ), 1.0 ).isNull() );
    QVERIFY( ! canvas.renderFrame( 2, QSize( 1920, 1080 ), 1.0 ).isNull() );
    QVERIFY( ! canvas.renderFrame( 2, QSize( 1280, 960 ), 2.0 ).isNull() );

    QCOMPARE( textOf( opt.viewState() ), state );
    QCOMPARE( M_main_data.viewIndex(), std::size_t( 1 ) );
    QVERIFY( opt.zoomed() );
    QCOMPARE( opt.focusType(), Options::FOCUS_BALL );
    QCOMPARE( opt.agentSelectType(), Options::SELECT_AUTO_ALL );
    QVERIFY( drawn( canvas ) == view );

    // FrameScope alone
    {
        FrameScope scope( M_main_data, 0 );
        QCOMPARE( M_main_data.viewIndex(), std::size_t( 0 ) );
        opt.updateFieldSize( 100, 100 );
        opt.updateFocusPoint( 10.0, 10.0 );
        opt.setSelectedNumber( rcsc::LEFT, 3 );
    }
    QCOMPARE( M_main_data.viewIndex(), std::size_t( 1 ) );
    QCOMPARE( textOf( opt.viewState() ), state );
}

/*-------------------------------------------------------------------*/
/*!
  The ball of the formation editor synchronized with the frame
  (feditBallSyncMove): drawing another frame into an image does not move it
  (S5); the view does.
*/
void
TestFieldCanvasRender::formationEditKept()
{
    Options & opt = Options::instance();
    const bool sync = opt.feditBallSyncMove();
    const bool auto_select = opt.feditDataAutoSelect();
    const bool auto_move = opt.feditPlayerAutoMove();
    opt.setFeditBallSyncMove( true );
    opt.setFeditDataAutoSelect( false );
    opt.setFeditPlayerAutoMove( false );

    std::shared_ptr< FormationEditData > edit_data = std::make_shared< FormationEditData >();
    M_main_data.setFormationEditData( edit_data );

    FieldCanvas canvas( M_main_data );
    canvas.resize( 640, 480 );

    // the ball of the frame 2 is at (-1.577, 1.074); of the frame 1 at (0, 0)
    QVERIFY( ! canvas.renderFrame( 2, canvas.size(), 1.0 ).isNull() );
    QCOMPARE( edit_data->currentState().ball_.x, 0.0 );
    QCOMPARE( edit_data->currentState().ball_.y, 0.0 );

    // the view synchronizes it
    QVERIFY( M_main_data.setViewDataIndex( 2 ) );
    M_main_data.update( canvas.width(), canvas.height() );
    QVERIFY( edit_data->currentState().ball_.x < -1.0 );
    QVERIFY( edit_data->currentState().ball_.y > 0.5 ); // rounded to 0.5

    M_main_data.clearFormationEditData();
    opt.setFeditBallSyncMove( sync );
    opt.setFeditDataAutoSelect( auto_select );
    opt.setFeditPlayerAutoMove( auto_move );
}

/*-------------------------------------------------------------------*/
/*!
  A device pixel ratio of 2: the frame laid out in the logical size (the
  score board at the top), drawn with twice the pixels. Of 0.5 (an image
  smaller than the view, the texts as large as in it): the same layout,
  half as large (QPainter takes no ratio below 1: the window is mapped
  onto the pixels).
*/
void
TestFieldCanvasRender::devicePixelRatio()
{
    QVERIFY( Options::instance().showScoreBoard() );

    FieldCanvas canvas( M_main_data );
    canvas.resize( 640, 480 );
    const QImage one = canvas.renderFrame( 1, QSize( 640, 480 ), 1.0 );
    const QImage two = canvas.renderFrame( 1, QSize( 1280, 960 ), 2.0 );
    QCOMPARE( two.size(), QSize( 1280, 960 ) );
    QCOMPARE( two.devicePixelRatio(), 2.0 );

    // the score board at the top left of both (the box of the left team)
    const QColor board = DrawConfig::instance().leftTeamBrush().color();
    QCOMPARE( one.pixelColor( 1, 1 ), board );
    QCOMPARE( two.pixelColor( 2, 2 ), board );

    // the same layout: the left goal line twice as far
    const int y1 = 240 + 60;
    const int x1 = leftLine( one, y1 );
    const int x2 = leftLine( two, y1 * 2 );
    QVERIFY( x1 > 0 );
    QVERIFY2( std::abs( x2 - 2 * x1 ) <= 2, qPrintable( QString( "%1 %2" ).arg( x1 ).arg( x2 ) ) );

    const QImage half = canvas.renderFrame( 1, QSize( 320, 240 ), 0.5 );
    QCOMPARE( half.size(), QSize( 320, 240 ) );
    QCOMPARE( half.pixelColor( 0, 0 ), board );
    const int x_half = leftLine( half, y1 / 2 );
    QVERIFY2( std::abs( 2 * x_half - x1 ) <= 3, qPrintable( QString( "%1 %2" ).arg( x1 ).arg( x_half ) ) );
}

/*-------------------------------------------------------------------*/
//! zoomed: an image of another size shows the same range as the view (the scale by the heights)
void
TestFieldCanvasRender::zoomed()
{
    Options & opt = Options::instance();
    opt.zoomOut(); // the whole field, smaller
    showTexts( false );

    FieldCanvas canvas( M_main_data );
    canvas.resize( 640, 480 );
    canvas.show();
    QVERIFY( QTest::qWaitForWindowExposed( &canvas ) );
    const QImage view = drawn( canvas );
    const QImage twice = canvas.renderFrame( 1, QSize( 1280, 960 ), 1.0 );

    const int y = 240 + 60;
    const int x1 = leftLine( view, y );
    const int x2 = leftLine( twice, y * 2 );
    QVERIFY( x1 > 0 );
    QVERIFY2( std::abs( x2 - 2 * x1 ) <= 3, qPrintable( QString( "%1 %2" ).arg( x1 ).arg( x2 ) ) );
    QVERIFY( opt.zoomed() );
    QVERIFY( drawn( canvas ) == view );
}

/*-------------------------------------------------------------------*/
void
TestFieldCanvasRender::emptySize()
{
    FieldCanvas canvas( M_main_data );
    QVERIFY( canvas.renderFrame( 1, QSize( 0, 100 ), 1.0 ).isNull() );
    QVERIFY( canvas.renderFrame( 1, QSize(), 1.0 ).isNull() );
}

/*-------------------------------------------------------------------*/
/*!
  File > Save Image of This Frame (Ctrl+Shift+I): the frame shown, as large
  as the view; the name of the log and the cycle; PNG without an extension;
  JPEG; no frame; a directory not there.
*/
void
TestFieldCanvasRender::saveImage()
{
    MainWindow window;
    window.init();
    window.resize( 800, 600 );
    window.show();
    QVERIFY( QTest::qWaitForWindowExposed( &window ) );

    QAction * act = window.findChild< QAction * >( "save_image" );
    QVERIFY( act );
    QCOMPARE( act->shortcut(), QKeySequence( Qt::CTRL | Qt::SHIFT | Qt::Key_I ) );
    QCOMPARE( act->text(), QString( "Save Image of This Frame..." ) );
    QVERIFY( act->isEnabled() );
    bool in_file_menu = false;
    for ( QAction * menu : window.menuBar()->actions() )
    {
        if ( menu->menu() && menu->text() == "&File" )
        {
            in_file_menu = menu->menu()->actions().contains( act );
        }
    }
    QVERIFY( in_file_menu );

    QTemporaryDir dir;
    QVERIFY( dir.isValid() );
    QString error;
    QVERIFY( ! window.saveImage( dir.filePath( "none.png" ), &error ) );
    QCOMPARE( error, QString( "There is no frame to save." ) );
    QVERIFY( ! QFile::exists( dir.filePath( "none.png" ) ) );
    QCOMPARE( window.defaultImageName(), QString( "soccerwindow2.png" ) );

    window.openRCG( LOG );
    QCOMPARE( window.defaultImageName(), QString( "head_v5-1.png" ) );

    FieldCanvas * canvas = window.findChild< FieldCanvas * >();
    QVERIFY( canvas );
    showTexts( false );
    QVERIFY( window.saveImage( dir.filePath( "frame.png" ), &error ) );
    QVERIFY( error.isEmpty() );
    const QImage saved( dir.filePath( "frame.png" ) );
    QCOMPARE( saved.size(), canvas->size() * canvas->devicePixelRatioF() );
    QCOMPARE( differenceOf( saved, drawn( *canvas ) ), QString( "same" ) );

    // no extension: PNG
    QVERIFY( window.saveImage( dir.filePath( "frame" ), &error ) );
    QCOMPARE( QImageReader( dir.filePath( "frame.png" ) ).format(), QByteArray( "png" ) );
    QVERIFY( window.saveImage( dir.filePath( "frame.jpg" ), &error ) );
    QCOMPARE( QImageReader( dir.filePath( "frame.jpg" ) ).format(), QByteArray( "jpeg" ) );

    // a directory not there
    QVERIFY( ! window.saveImage( dir.filePath( "no/such/dir/frame.png" ), &error ) );
    QVERIFY2( error.contains( "Could not write the image" ), qPrintable( error ) );
}

SOCCERWINDOW2_GUI_TEST_MAIN( TestFieldCanvasRender )

#include "test_field_canvas_render.moc"
