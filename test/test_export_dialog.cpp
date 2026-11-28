// -*-c++-*-

/*!
  \file test_export_dialog.cpp
  \brief unit test of the export dialog (File > Export Images / Video).
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

#include "export_dialog.h"
#include "export_range_list.h"
#include "export_settings.h"
#include "field_canvas.h"
#include "log_player_slider.h"
#include "main_data.h"
#include "main_window.h"
#include "options.h"

#include "test_environment.h"

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QStatusBar>
#include <QSettings>
#include <QTemporaryDir>

#include <vector>

#ifndef SOCCERWINDOW2_TEST_DATA_DIR
#error "SOCCERWINDOW2_TEST_DATA_DIR is not defined."
#endif

/*
  docs/plan/2026-11-28-00_frame-export.md 4.2 to 4.4, S3, S8, S10, D3, D7, D15,
  D9 to D12, §6.
  Ported from rcssmonitor. test/data/head_v5.rcg has 3 frames: the cycles
  1, 2 and 4. The test runs with HOME in a temporary directory and the
  offscreen platform (test_environment.h).
*/

namespace {

const QString LOG = QStringLiteral( SOCCERWINDOW2_TEST_DATA_DIR "/head_v5.rcg" );

template < typename T >
T *
child( const QObject & parent,
       const char * name )
{
    return parent.findChild< T * >( QString::fromLatin1( name ) );
}

//! the dialog, with the images small, in a directory
ExportDialog *
openDialog( MainWindow & window,
            const QString & dir )
{
    QAction * act = window.findChild< QAction * >( "export" );
    if ( ! act ) return nullptr;
    act->trigger();
    ExportDialog * dialog = window.findChild< ExportDialog * >( "export_dialog" );
    if ( ! dialog ) return nullptr;
    QComboBox * size = child< QComboBox >( *dialog, "size" );
    size->setCurrentIndex( size->findData( "custom" ) );
    child< QSpinBox >( *dialog, "width" )->setValue( 160 );
    child< QSpinBox >( *dialog, "height" )->setValue( 120 );
    child< QCheckBox >( *dialog, "scale_texts" )->setChecked( false );
    child< QDoubleSpinBox >( *dialog, "fps" )->setValue( 10.0 );
    child< QDoubleSpinBox >( *dialog, "speed" )->setValue( 1.0 );
    child< QCheckBox >( *dialog, "name_by_cycle" )->setChecked( false );
    child< QLineEdit >( *dialog, "prefix" )->setText( "img-" );
    dialog->setDirectory( dir );
    dialog->setVideo( false );
    dialog->clearRange();
    return dialog;
}

//! export and wait for the end: ok and the message, or "not started"
QString
exportNow( ExportDialog & dialog,
           const bool ask = false )
{
    QSignalSpy finished( &dialog, &ExportDialog::exportFinished );
    if ( ! dialog.startExport( ask ) ) return QStringLiteral( "not started" );
    if ( ! finished.wait( 10000 ) ) return QStringLiteral( "no end" );
    return ( finished.first().at( 0 ).toBool() ? QStringLiteral( "ok: " ) : QStringLiteral( "failed: " ) )
        + finished.first().at( 1 ).toString();
}


//! a frame of a game log written by writeLog(): the cycle, the play mode and the score
struct LogFrame {
    int cycle;
    const char * mode;
    int left;
    int right;
};

/*!
  a game log (rcg v5) in a directory: the headers of test/data/head_v5.rcg
  (halves of 300 s), and the frames given, each with the players and the
  ball of its first frame. The frames of a cycle after the first one are
  numbered as stopped by ViewHolder in the play modes of a stop (goal_l, ...).
  \return the path of the game log, or an empty one
*/
QString
writeLog( const QString & dir,
          const std::vector< LogFrame > & frames )
{
    QFile head( LOG );
    if ( ! head.open( QIODevice::ReadOnly | QIODevice::Text ) ) return QString();
    QStringList lines;
    QString show; // the players and the ball of the first frame
    while ( ! head.atEnd() )
    {
        const QString line = QString::fromUtf8( head.readLine() ).trimmed();
        if ( line.startsWith( QLatin1String( "(show " ) ) )
        {
            if ( show.isEmpty() ) show = line.mid( line.indexOf( QLatin1Char( ' ' ), 6 ) );
            continue;
        }
        if ( line.startsWith( QLatin1String( "(team " ) )
             || line.startsWith( QLatin1String( "(playmode " ) ) )
        {
            continue;
        }
        lines << line;
    }
    if ( show.isEmpty() ) return QString();

    for ( const LogFrame & f : frames )
    {
        lines << QString( "(playmode %1 %2)" ).arg( f.cycle ).arg( f.mode );
        lines << QString( "(team %1 LEFT RIGHT %2 %3)" ).arg( f.cycle ).arg( f.left ).arg( f.right );
        lines << QString( "(show %1" ).arg( f.cycle ) + show;
    }

    const QString path = QDir( dir ).filePath( "game.rcg" );
    QFile file( path );
    if ( ! file.open( QIODevice::WriteOnly | QIODevice::Text ) ) return QString();
    file.write( ( lines.join( QLatin1Char( '\n' ) ) + QLatin1Char( '\n' ) ).toUtf8() );
    return path;
}

/*!
  a game as a game log: the cycles 0 to 6000 with a goal of the left at 100
  and of the right at 3100 (the frames of rcssmonitor's test, written as a
  game log: soccerwindow2 reads no frames of JSON one by one)
*/
QString
writeGame( const QString & dir )
{
    std::vector< LogFrame > frames;
    for ( const auto & f : std::vector< std::pair< int, const char * > >{
            { 0, "before_kick_off" }, { 50, "play_on" }, { 100, "goal_l" }, { 101, "kick_off_r" }, { 140, "play_on" },
            { 160, "play_on" }, { 2990, "play_on" }, { 3000, "play_on" }, { 3001, "kick_off_r" }, { 3100, "goal_r" },
            { 3101, "kick_off_l" }, { 3200, "play_on" }, { 6000, "time_over" } } )
    {
        frames.push_back( LogFrame{ f.first, f.second, f.first >= 100 ? 1 : 0, f.first >= 3100 ? 1 : 0 } );
    }
    return writeLog( dir, frames );
}

/*!
  a game with the frames stopped after a goal, as rcssmonitor's test: the
  cycles 0 to 199, a goal of the left at 200 and the 50 frames stopped
  after it (200.0 to 200.50, the frames 200 to 250), and the cycles 201 to
  300 after the kick-off (the frames 251 to 350): 351 frames
*/
QString
writeGoalGame( const QString & dir )
{
    std::vector< LogFrame > frames;
    for ( int c = 0; c < 200; ++c ) frames.push_back( LogFrame{ c, c == 0 ? "before_kick_off" : "play_on", 0, 0 } );
    for ( int n = 0; n <= 50; ++n ) frames.push_back( LogFrame{ 200, "goal_l", 1, 0 } );
    for ( int c = 201; c <= 300; ++c ) frames.push_back( LogFrame{ c, c == 201 ? "kick_off_r" : "play_on", 1, 0 } );
    return writeLog( dir, frames );
}

/*!
  \brief a fake ffmpeg: it answers -version and -encoders, writes its
  arguments to OUT.args and its input (the raw frames) to OUT
  \param encoders the video encoders it has
*/
QString
fakeFfmpeg( const QTemporaryDir & dir,
            const QString & name,
            const QStringList & encoders )
{
    QString list = "Encoders:\n";
    for ( const QString & e : encoders ) list += " V....D " + e + "    the encoder " + e + "\n";
    const QString path = dir.filePath( name );
    QFile file( path );
    if ( ! file.open( QIODevice::WriteOnly | QIODevice::Text ) ) return QString();
    file.write( ( "#!/bin/sh\n"
                  "case \"$1 $2\" in\n"
                  "  \"-hide_banner -version\") echo 'ffmpeg version 9.9-fake Copyright (c) the tests'; exit 0 ;;\n"
                  "  \"-hide_banner -encoders\") printf '%s' '" + list + "'; exit 0 ;;\n"
                  "esac\n"
                  "for a in \"$@\"; do last=\"$a\"; done\n"
                  "printf '%s\\n' \"$@\" > \"$last.args\"\n"
                  "exec cat > \"$last\"\n" ).toUtf8() );
    file.close();
    file.setPermissions( file.permissions() | QFileDevice::ExeOwner | QFileDevice::ExeUser );
    return path;
}

//! the arguments the fake ffmpeg was run with, a line each
QString
argsOf( const QString & video )
{
    QFile file( video + ".args" );
    if ( ! file.open( QIODevice::ReadOnly | QIODevice::Text ) ) return QString();
    return QString::fromUtf8( file.readAll() );
}

//! the ranges as "name[start,end]", the current one with "*"
QString
textOf( const ExportRangeList & list )
{
    QStringList texts;
    for ( int i = 0; i < list.size(); ++i )
    {
        const ExportRangeSpec & s = list.ranges()[i];
        texts << ( i == list.current() ? "*" : "" ) + s.name + "[" + s.start + "," + s.end + "]";
    }
    return texts.join( ' ' );
}

}

/*!
  \brief unit test of the export dialog
*/
class TestExportDialog
    : public QObject {
    Q_OBJECT

private slots:

    void action();
    void range();
    void exportImages();
    void namesByCycle();
    void sizes();
    void filesThere();
    void connected();
    void imageOptions();
    void marks();
    void presets();
    void presetsStopped();
    void severalRanges();
    void exportVideo();
    void severalVideos();
    void noFfmpeg();
    void playbackSpeed();
};

/*-------------------------------------------------------------------*/
/*!
  File > Export Images / Video > Open the Export Dialog (Ctrl+Shift+E): the
  submenu after Save Image of This Frame (rcssmonitor's D11), instead of the
  old Save Image (D3); enabled with a game log; a modeless dialog, once.
*/
void
TestExportDialog::action()
{
    MainWindow window;
    window.init();
    window.show();
    QVERIFY( QTest::qWaitForWindowExposed( &window ) );

    QAction * act = window.findChild< QAction * >( "export" );
    QVERIFY( act );
    QCOMPARE( act->shortcut(), QKeySequence( Qt::CTRL | Qt::SHIFT | Qt::Key_E ) );
    QVERIFY( ! act->isEnabled() );
    QAction * save_image = window.findChild< QAction * >( "save_image" );
    QVERIFY( save_image );
    QCOMPARE( save_image->text(), QString( "Save Image of This Frame..." ) );
    QCOMPARE( act->text(), QString( "Open the Export Dialog..." ) );
    QMenu * submenu = window.findChild< QMenu * >( "export_menu" );
    QVERIFY( submenu );
    QCOMPARE( submenu->title(), QString( "Export Images / Video" ) );
    QCOMPARE( submenu->actions().first(), act );
    // the old File > Save Image is replaced (D3)
    QVERIFY( ! window.findChild< QAction * >( "show_image_save_dialog" ) );
    for ( QAction * menu : window.menuBar()->actions() )
    {
        if ( menu->menu() && menu->text() == "&File" )
        {
            const QList< QAction * > items = menu->menu()->actions();
            QCOMPARE( items.indexOf( submenu->menuAction() ), items.indexOf( save_image ) + 1 );
        }
    }

    window.openRCG( LOG );
    QVERIFY( act->isEnabled() );
    QVERIFY( ! window.findChild< ExportDialog * >( "export_dialog" ) );
    act->trigger();
    ExportDialog * dialog = window.findChild< ExportDialog * >( "export_dialog" );
    QVERIFY( dialog );
    QVERIFY( dialog->isVisible() );
    QVERIFY( ! dialog->isModal() );
    act->trigger();
    QCOMPARE( window.findChild< ExportDialog * >( "export_dialog" ), dialog );
    QCOMPARE( window.findChildren< ExportDialog * >().size(), 1 );
}

/*-------------------------------------------------------------------*/
/*!
  The range: empty fields for the whole log, the positions now, the
  reasons of the wrong ones (the export disabled).
*/
void
TestExportDialog::range()
{
    MainWindow window;
    window.init();
    window.show();
    QVERIFY( QTest::qWaitForWindowExposed( &window ) );
    window.openRCG( LOG );
    QTemporaryDir tmp;
    ExportDialog * dialog = openDialog( window, tmp.path() );
    QVERIFY( dialog );
    QPushButton * button = child< QPushButton >( *dialog, "export" );

    QString error;
    ExportRange r = dialog->range( &error );
    QCOMPARE( r.first, std::size_t( 0 ) );
    QCOMPARE( r.last, std::size_t( 2 ) );
    QVERIFY2( dialog->summaryText().startsWith( "3 images of 3 frames" ), qPrintable( dialog->summaryText() ) );
    QVERIFY( button->isEnabled() );

    // the positions now: the first frame shown after opening
    child< QPushButton >( *dialog, "start_now" )->click();
    QCOMPARE( child< QLineEdit >( *dialog, "start" )->text(), QString( "1" ) );
    QTest::keyClick( &window, Qt::Key_Right );
    child< QPushButton >( *dialog, "end_now" )->click();
    QCOMPARE( child< QLineEdit >( *dialog, "end" )->text(), QString( "2" ) );
    r = dialog->range( &error );
    QCOMPARE( r.first, std::size_t( 0 ) );
    QCOMPARE( r.last, std::size_t( 1 ) );
    QVERIFY2( dialog->summaryText().startsWith( "2 images of 2 frames" ), qPrintable( dialog->summaryText() ) );

    // a cycle missing: the next frame
    dialog->setStart( "3" );
    dialog->setEnd( "4" );
    r = dialog->range( &error );
    QCOMPARE( r.first, std::size_t( 2 ) );
    QVERIFY( dialog->findChildren< QLabel * >().size() > 0 );
    QVERIFY2( dialog->summaryText().startsWith( "1 image of 1 frame" ), qPrintable( dialog->summaryText() ) );

    // wrong ones
    dialog->setStart( "5" );
    QVERIFY( ! dialog->range( &error ).isValid() );
    QVERIFY2( error.contains( "after the last frame" ), qPrintable( error ) );
    QVERIFY( ! button->isEnabled() );
    dialog->setStart( "4" );
    dialog->setEnd( "2" );
    QVERIFY( ! dialog->range( &error ).isValid() );
    QCOMPARE( error, QString( "The start is after the end." ) );
    QCOMPARE( dialog->summaryText(), error );
    QVERIFY( ! button->isEnabled() );
    dialog->setStart( "abc" );
    QVERIFY( ! dialog->range( &error ).isValid() );
    QVERIFY2( error.contains( "Not a position" ), qPrintable( error ) );
    QVERIFY( ! dialog->startExport( false ) );
    QVERIFY2( dialog->messageText().contains( "Not a position" ), qPrintable( dialog->messageText() ) );

    // Clear: the whole log
    child< QPushButton >( *dialog, "clear_range" )->click();
    QVERIFY( child< QLineEdit >( *dialog, "start" )->text().isEmpty() );
    QVERIFY( child< QLineEdit >( *dialog, "end" )->text().isEmpty() );
    QVERIFY( dialog->range( &error ).isValid() );
    QVERIFY( button->isEnabled() );

    // a stopped frame written as the score board shows it (S9)
    dialog->setStart( "1,0" );
    QCOMPARE( dialog->range( &error ).first, std::size_t( 0 ) );
    dialog->clearRange();
}

/*-------------------------------------------------------------------*/
/*!
  The images of the range, named by their numbers; the playback stopped;
  the frame shown and the view unchanged; the settings kept.
*/
void
TestExportDialog::exportImages()
{
    MainWindow window;
    window.init();
    window.show();
    QVERIFY( QTest::qWaitForWindowExposed( &window ) );
    window.openRCG( LOG );
    QTemporaryDir tmp;
    ExportDialog * dialog = openDialog( window, tmp.filePath( "images" ) );
    QVERIFY( dialog );
    FieldCanvas * canvas = window.findChild< FieldCanvas * >();
    QVERIFY( canvas );

    QTest::keyClick( &window, Qt::Key_Right ); // the frame shown: 2
    QTest::qWait( 200 ); // the rows of the tool bars laid out again after opening the log
    const QSize view_size = canvas->size();
    const QImage view = canvas->grab().toImage();
    const Options::ViewState state = Options::instance().viewState();

    QSignalSpy stop( dialog, &ExportDialog::stopRequested );
    const QString result = exportNow( *dialog );
    QVERIFY2( result.startsWith( "ok: 3 images in " ), qPrintable( result ) );
    QCOMPARE( stop.count(), 1 );
    QVERIFY2( dialog->messageText().startsWith( "Exported 3 images" ), qPrintable( dialog->messageText() ) );
    QVERIFY2( window.statusBar()->currentMessage().startsWith( "Exported 3 images" ), qPrintable( window.statusBar()->currentMessage() ) );

    const QDir dir( tmp.filePath( "images" ) );
    QCOMPARE( dir.entryList( QDir::Files ), QStringList( { "img-00000.png", "img-00001.png", "img-00002.png" } ) );
    QCOMPARE( QImage( dir.filePath( "img-00002.png" ) ).size(), QSize( 160, 120 ) );
    QVERIFY( QImage( dir.filePath( "img-00000.png" ) ) != QImage( dir.filePath( "img-00002.png" ) ) );

    // nothing changed in the view
    QCOMPARE( canvas->size(), view_size );
    QVERIFY( canvas->grab().toImage() == view );
    QCOMPARE( Options::instance().viewState().canvas_width, state.canvas_width );
    QCOMPARE( Options::instance().viewState().field_scale, state.field_scale );

    // the settings written, in ~/.soccerwindow2 too
    const ExportSettings s = ExportSettings::read();
    QCOMPARE( s.prefix, QString( "img-" ) );
    QCOMPARE( s.size, QString( "custom" ) );
    QCOMPARE( s.width, 160 );
    QCOMPARE( s.directory, QDir::cleanPath( tmp.filePath( "images" ) ) );
    QSettings settings( QDir::homePath() + "/.soccerwindow2", QSettings::IniFormat );
    QCOMPARE( settings.value( "Export/prefix" ).toString(), QString( "img-" ) );
    QCOMPARE( settings.value( "Export/width" ).toInt(), 160 );
}

/*-------------------------------------------------------------------*/
//! named by the cycles when an image is a frame; the frames twice at half the speed
void
TestExportDialog::namesByCycle()
{
    MainWindow window;
    window.init();
    window.show();
    QVERIFY( QTest::qWaitForWindowExposed( &window ) );
    window.openRCG( LOG );
    QTemporaryDir tmp;
    ExportDialog * dialog = openDialog( window, tmp.path() );
    QVERIFY( dialog );
    QCheckBox * by_cycle = child< QCheckBox >( *dialog, "name_by_cycle" );

    QVERIFY( dialog->canNameByCycle() );
    QVERIFY( by_cycle->isEnabled() );
    by_cycle->setChecked( true );
    QString error;
    QCOMPARE( dialog->imageNames( dialog->imageFrames( dialog->range( &error ) ) ),
              QStringList( { "img-c00001.00.png", "img-c00002.00.png", "img-c00004.00.png" } ) );

    // half the speed: 2 images a frame, by the numbers
    child< QDoubleSpinBox >( *dialog, "speed" )->setValue( 0.5 );
    QVERIFY( ! dialog->canNameByCycle() );
    QVERIFY( ! by_cycle->isEnabled() );
    const std::vector< std::size_t > frames = dialog->imageFrames( dialog->range( &error ) );
    QCOMPARE( frames.size(), std::size_t( 5 ) );
    QCOMPARE( dialog->imageNames( frames ).last(), QString( "img-00004.png" ) );
    QVERIFY2( dialog->summaryText().startsWith( "5 images of 3 frames" ), qPrintable( dialog->summaryText() ) );

    // the same speed at 20 images/s: one frame an image again
    child< QDoubleSpinBox >( *dialog, "fps" )->setValue( 20.0 );
    child< QDoubleSpinBox >( *dialog, "speed" )->setValue( 2.0 );
    QVERIFY( dialog->canNameByCycle() );
    const QString result = exportNow( *dialog );
    QVERIFY2( result.startsWith( "ok: " ), qPrintable( result ) );
    QCOMPARE( QDir( tmp.path() ).entryList( QDir::Files ),
              QStringList( { "img-c00001.00.png", "img-c00002.00.png", "img-c00004.00.png" } ) );
}

/*-------------------------------------------------------------------*/
//! the sizes: as the view, a fixed one; enlarged as the view or not
void
TestExportDialog::sizes()
{
    MainWindow window;
    window.init();
    window.resize( 800, 600 );
    window.show();
    QVERIFY( QTest::qWaitForWindowExposed( &window ) );
    window.openRCG( LOG );
    QTemporaryDir tmp;
    ExportDialog * dialog = openDialog( window, tmp.path() );
    QVERIFY( dialog );
    FieldCanvas * canvas = window.findChild< FieldCanvas * >();
    QComboBox * size = child< QComboBox >( *dialog, "size" );
    QCheckBox * scale = child< QCheckBox >( *dialog, "scale_texts" );
    QCOMPARE( scale->text(), QString( "Enlarge the texts, logos and lines with the image" ) );

    size->setCurrentIndex( size->findData( "view" ) );
    QCOMPARE( dialog->imageSize(), canvas->size() * canvas->devicePixelRatioF() );
    QVERIFY( ! child< QSpinBox >( *dialog, "width" )->isEnabled() );
    QCOMPARE( child< QSpinBox >( *dialog, "width" )->value(), dialog->imageSize().width() );
    scale->setChecked( true );
    QCOMPARE( dialog->imageRatio(), canvas->devicePixelRatioF() );

    size->setCurrentIndex( size->findData( "1920x1080" ) );
    QCOMPARE( dialog->imageSize(), QSize( 1920, 1080 ) );
    QCOMPARE( dialog->imageRatio(), 1080.0 / canvas->height() );
    scale->setChecked( false );
    QCOMPARE( dialog->imageRatio(), 1.0 );
    QVERIFY2( dialog->summaryText().contains( "1920 x 1080 pixels" ), qPrintable( dialog->summaryText() ) );
}

/*-------------------------------------------------------------------*/
//! the files there: asked once (cancel: not exported); without asking, overwritten
void
TestExportDialog::filesThere()
{
    MainWindow window;
    window.init();
    window.show();
    QVERIFY( QTest::qWaitForWindowExposed( &window ) );
    window.openRCG( LOG );
    QTemporaryDir tmp;
    ExportDialog * dialog = openDialog( window, tmp.path() );
    QVERIFY( dialog );
    QVERIFY( exportNow( *dialog ).startsWith( "ok: " ) );
    const QDateTime written = QFileInfo( tmp.filePath( "img-00000.png" ) ).lastModified();

    int asked = 0;
    QString question;
    QTimer::singleShot( 200, [&]() {
        QMessageBox * box = qobject_cast< QMessageBox * >( QApplication::activeModalWidget() );
        if ( ! box ) return;
        ++asked;
        question = box->text();
        box->button( QMessageBox::Cancel )->click();
    } );
    QCOMPARE( exportNow( *dialog, true ), QString( "not started" ) );
    QCOMPARE( asked, 1 );
    QVERIFY2( question.startsWith( "3 of the 3 files are already in " ), qPrintable( question ) );
    QCOMPARE( QFileInfo( tmp.filePath( "img-00000.png" ) ).lastModified(), written );

    QTest::qWait( 20 );
    QVERIFY( exportNow( *dialog ).startsWith( "ok: " ) );
}

/*-------------------------------------------------------------------*/
//! connected to a server: no export (S3)
void
TestExportDialog::connected()
{
    MainWindow window;
    window.init();
    window.show();
    QVERIFY( QTest::qWaitForWindowExposed( &window ) );
    window.openRCG( LOG );
    QTemporaryDir tmp;
    ExportDialog * dialog = openDialog( window, tmp.path() );
    QVERIFY( dialog );

    dialog->setExportAllowed( false );
    QVERIFY( ! child< QPushButton >( *dialog, "export" )->isEnabled() );
    QVERIFY( ! dialog->startExport( false ) );
    QVERIFY2( dialog->messageText().contains( "connected to a server" ), qPrintable( dialog->messageText() ) );
    QVERIFY( QDir( tmp.path() ).entryList( QDir::Files ).isEmpty() );

    dialog->setExportAllowed( true );
    QVERIFY( child< QPushButton >( *dialog, "export" )->isEnabled() );
}

/*-------------------------------------------------------------------*/
/*!
  The directory, the prefix and the format of the images (D7, D15): the
  dialog starts with the saved ones, but the prefix and the format given on
  the command line (--export-image-prefix and --export-image-format, without
  an export) for that run; an export saves the ones used.
*/
void
TestExportDialog::imageOptions()
{
    QTemporaryDir tmp;
    {
        QSettings settings( QDir::homePath() + "/.soccerwindow2", QSettings::IniFormat );
        settings.setValue( "Export/directory", tmp.path() );
        settings.setValue( "Export/prefix", "saved-" );
        settings.setValue( "Export/format", "jpg" );
    }
    ExportSettings saved = ExportSettings::read();
    QCOMPARE( saved.directory, tmp.path() );
    QCOMPARE( saved.prefix, QString( "saved-" ) );
    QCOMPARE( saved.format, QString( "jpg" ) );

    // the command line parsed after MainWindow is made (as main() does)
    const auto parse = []( QStringList args ) {
        args.prepend( "soccerwindow2" );
        std::vector< QByteArray > bytes;
        for ( const QString & a : args ) bytes.push_back( a.toLocal8Bit() );
        std::vector< char * > argv;
        for ( QByteArray & b : bytes ) argv.push_back( b.data() );
        argv.push_back( nullptr );
        return Options::instance().parseCmdLine( static_cast< int >( bytes.size() ), argv.data() );
    };
    MainWindow window;
    {
        QSettings settings( QDir::homePath() + "/.soccerwindow2", QSettings::IniFormat );
        settings.setValue( "Export/format", "png" );
    }
    QVERIFY( parse( { "--export-image-prefix", "cmd-", "--export-image-format", "JPEG" } ) );
    saved = ExportSettings::read();
    QCOMPARE( saved.directory, tmp.path() );
    QCOMPARE( saved.prefix, QString( "cmd-" ) );
    QCOMPARE( saved.format, QString( "jpg" ) );

    window.init();
    window.show();
    QVERIFY( QTest::qWaitForWindowExposed( &window ) );
    window.openRCG( LOG );
    window.findChild< QAction * >( "export" )->trigger();
    ExportDialog * dialog = window.findChild< ExportDialog * >( "export_dialog" );
    QVERIFY( dialog );
    QCOMPARE( dialog->directory(), QDir::cleanPath( tmp.path() ) );
    QCOMPARE( child< QLineEdit >( *dialog, "prefix" )->text(), QString( "cmd-" ) );
    QCOMPARE( child< QComboBox >( *dialog, "format" )->currentData().toString(), QString( "jpg" ) );

    // an export saves the ones used
    child< QComboBox >( *dialog, "format" )->setCurrentIndex( child< QComboBox >( *dialog, "format" )->findData( "png" ) );
    child< QLineEdit >( *dialog, "end" )->setText( "1" );
    QVERIFY( exportNow( *dialog ).startsWith( "ok: " ) );
    QVERIFY( QFile::exists( tmp.filePath( "cmd-00000.png" ) ) );
    {
        QSettings settings( QDir::homePath() + "/.soccerwindow2", QSettings::IniFormat );
        QCOMPARE( settings.value( "Export/directory" ).toString(), QDir::cleanPath( tmp.path() ) );
        QCOMPARE( settings.value( "Export/prefix" ).toString(), QString( "cmd-" ) );
        QCOMPARE( settings.value( "Export/format" ).toString(), QString( "png" ) );
    }

    // the options of the export are made again by the next command line:
    // not left for the other tests
    QVERIFY( parse( {} ) );
    QCOMPARE( ExportSettings::read().prefix, QString( "cmd-" ) ); // the saved one
    QVERIFY( Options::instance().exportOptions().given.empty() );
}

/*-------------------------------------------------------------------*/
/*!
  The marks of the main window (D9): [ and ] at the position shown, in the
  dialog and on the slider; cleared by the menu, and by another log.
*/
void
TestExportDialog::marks()
{
    MainWindow window;
    window.init();
    window.show();
    QVERIFY( QTest::qWaitForWindowExposed( &window ) );
    QAction * start = window.findChild< QAction * >( "mark_start" );
    QAction * end = window.findChild< QAction * >( "mark_end" );
    QAction * clear = window.findChild< QAction * >( "clear_marks" );
    QVERIFY( start && end && clear );
    QCOMPARE( start->shortcut(), QKeySequence( Qt::Key_BracketLeft ) );
    QCOMPARE( end->shortcut(), QKeySequence( Qt::Key_BracketRight ) );
    QVERIFY( clear->shortcut().isEmpty() );
    // in the submenu of the export, after its dialog (rcssmonitor's D11)
    QMenu * submenu = window.findChild< QMenu * >( "export_menu" );
    QVERIFY( submenu );
    QList< QAction * > items;
    for ( QAction * a : submenu->actions() ) if ( ! a->isSeparator() ) items << a;
    QCOMPARE( items, QList< QAction * >( { window.findChild< QAction * >( "export" ), start, end, clear } ) );
    QCOMPARE( start->text(), QString( "Start the Range Here" ) );
    QCOMPARE( end->text(), QString( "End the Range Here" ) );
    QCOMPARE( clear->text(), QString( "Clear the Range" ) );
    QVERIFY( ! start->isEnabled() );

    window.openRCG( LOG );
    QVERIFY( start->isEnabled() );
    ExportRangeList * ranges = window.findChild< ExportRangeList * >( "export_ranges" );
    LogPlayerSlider * slider = window.findChild< LogPlayerSlider * >();
    QVERIFY( ranges && slider );
    QVERIFY( slider->bands().empty() ); // the whole log: no band

    QTest::keyClick( &window, Qt::Key_BracketLeft );
    QTest::keyClick( &window, Qt::Key_Right );
    QTest::keyClick( &window, Qt::Key_Right );
    QTest::keyClick( &window, Qt::Key_BracketRight );
    QCOMPARE( textOf( *ranges ), QString( "*range1[1,4]" ) );
    QCOMPARE( slider->bands().size(), std::size_t( 1 ) );
    QCOMPARE( slider->bands().front().first, std::size_t( 0 ) );
    QCOMPARE( slider->bands().front().last, std::size_t( 2 ) );
    QVERIFY( slider->bands().front().current );
    QVERIFY2( window.statusBar()->currentMessage().contains( "4" ), qPrintable( window.statusBar()->currentMessage() ) );

    // the dialog shows them, and changes them
    QTemporaryDir tmp;
    ExportDialog * dialog = openDialog( window, tmp.path() );
    QVERIFY( dialog );
    QCOMPARE( textOf( *ranges ), QString( "*range1[,]" ) ); // openDialog() takes the whole log
    QTest::keyClick( &window, Qt::Key_BracketRight );
    QCOMPARE( child< QLineEdit >( *dialog, "end" )->text(), QString( "4" ) );
    QTest::keyClick( &window, Qt::Key_Left );
    QTest::keyClick( &window, Qt::Key_BracketLeft );
    QCOMPARE( child< QLineEdit >( *dialog, "start" )->text(), QString( "2" ) );
    QCOMPARE( slider->bands().front().first, std::size_t( 1 ) );

    clear->trigger();
    QCOMPARE( textOf( *ranges ), QString( "*range1[,]" ) );
    QVERIFY( slider->bands().empty() );
    QCOMPARE( child< QLineEdit >( *dialog, "start" )->text(), QString() );

    // the button Clear of the dialog does the same (rcssmonitor's D12); Now tells the keys
    QTest::keyClick( &window, Qt::Key_BracketLeft );
    QVERIFY( ! slider->bands().empty() );
    QPushButton * clear_button = child< QPushButton >( *dialog, "clear_range" );
    QVERIFY( clear_button );
    QCOMPARE( clear_button->text(), QString( "Clear" ) );
    clear_button->click();
    QCOMPARE( textOf( *ranges ), QString( "*range1[,]" ) );
    QVERIFY( slider->bands().empty() );
    QVERIFY2( child< QPushButton >( *dialog, "start_now" )->toolTip().contains( "[" ), qPrintable( child< QPushButton >( *dialog, "start_now" )->toolTip() ) );
    QVERIFY( child< QPushButton >( *dialog, "start_now" )->toolTip().contains( "Start the Range Here" ) );
    QVERIFY( child< QPushButton >( *dialog, "end_now" )->toolTip().contains( "]" ) );

    // another log: the ranges of the one before are gone
    QTest::keyClick( &window, Qt::Key_BracketLeft );
    dialog->addRange();
    QCOMPARE( ranges->size(), 2 );
    window.openRCG( LOG );
    QCOMPARE( textOf( *ranges ), QString( "*range1[,]" ) );
}

/*-------------------------------------------------------------------*/
/*!
  The presets: the halves, a goal scene, all the goals as ranges, around
  the position shown, by the seconds of the fields (D10).
*/
void
TestExportDialog::presets()
{
    QTemporaryDir tmp;
    const QString game = writeGame( tmp.path() );
    QVERIFY( ! game.isEmpty() );
    MainData data;
    QVERIFY( data.openRCG( game.toStdString() ) );
    QCOMPARE( data.viewHolder().monitorViewCont().size(), std::size_t( 13 ) );
    FieldCanvas canvas( data );
    ExportRangeList ranges;
    ExportDialog dialog( data, canvas, ranges );

    QCOMPARE( child< QDoubleSpinBox >( dialog, "goal_before" )->value(), 10.0 );
    QCOMPARE( child< QDoubleSpinBox >( dialog, "goal_after" )->value(), 5.0 );
    QCOMPARE( child< QDoubleSpinBox >( dialog, "around_before" )->value(), 5.0 );
    QCOMPARE( child< QDoubleSpinBox >( dialog, "around_after" )->value(), 5.0 );

    // the menu of the presets
    QMenu * menu = child< QMenu >( dialog, "presets_menu" );
    QVERIFY( menu );
    emit menu->aboutToShow();
    QStringList names;
    for ( QAction * act : menu->actions() ) if ( ! act->objectName().isEmpty() ) names << act->objectName();
    QCOMPARE( names, QStringList( { "preset_first-half", "preset_second-half", "preset_goal1", "preset_goal2",
                                    "preset_all_goals", "preset_around" } ) );

    // the halves
    dialog.choosePeriod( 1 );
    QCOMPARE( textOf( ranges ), QString( "*second-half[3001,6000]" ) );
    dialog.choosePeriod( 0 );
    QCOMPARE( textOf( ranges ), QString( "*first-half[0,3000]" ) );

    // a goal: 10 s before, 5 s after (the frames there)
    // the seconds are of the playback, a frame a step (D12): the frames of
    // this game are far apart, so a tenth of a second is a frame
    child< QDoubleSpinBox >( dialog, "goal_before" )->setValue( 0.2 );
    child< QDoubleSpinBox >( dialog, "goal_after" )->setValue( 0.2 );
    dialog.chooseGoal( 0 );
    QCOMPARE( textOf( ranges ), QString( "*goal1-c100[0,140]" ) );
    child< QDoubleSpinBox >( dialog, "goal_before" )->setValue( 0.1 );
    dialog.chooseGoal( 1 );
    QCOMPARE( textOf( ranges ), QString( "*goal2-c3100[3001,3200]" ) );

    // all the goals: in place of the whole log
    child< QDoubleSpinBox >( dialog, "goal_before" )->setValue( 0.2 );
    ranges.clear();
    dialog.chooseAllGoals();
    QCOMPARE( textOf( ranges ), QString( "*goal1-c100[0,140] goal2-c3100[3000,3200]" ) );
    // after the ranges given
    ranges.setRanges( { ExportRangeSpec{ "50", "100", "mine" } } );
    dialog.chooseAllGoals();
    QCOMPARE( textOf( ranges ), QString( "*mine[50,100] goal1-c100[0,140] goal2-c3100[3000,3200]" ) );

    // around the position shown
    ranges.clear();
    QVERIFY( data.setViewDataIndex( 9 ) ); // 3100
    child< QDoubleSpinBox >( dialog, "around_before" )->setValue( 0.1 );
    child< QDoubleSpinBox >( dialog, "around_after" )->setValue( 0.1 );
    dialog.chooseAround();
    QCOMPARE( textOf( ranges ), QString( "*around-c3100[3001,3101]" ) );
}

/*-------------------------------------------------------------------*/
/*!
  The presets and the ends by the frames stopped after a goal, ported from
  rcssmonitor's PR #79: a goal scene is counted by the seconds of the
  playback (D12), and an end at the frame before the stopped ones is
  written "200.0" (D17), by "Around the position shown", Now and the text
  beside the end.
*/
void
TestExportDialog::presetsStopped()
{
    QTemporaryDir tmp;
    const QString game = writeGoalGame( tmp.path() );
    QVERIFY( ! game.isEmpty() );
    MainData data;
    QVERIFY( data.openRCG( game.toStdString() ) );
    const MonitorViewData::Cont & frames = data.viewHolder().monitorViewCont();
    QCOMPARE( frames.size(), std::size_t( 351 ) );
    QCOMPARE( ExportRange::positionText( frames, 250 ), QString( "200.50" ) );
    FieldCanvas canvas( data );
    ExportRangeList ranges;
    ExportDialog dialog( data, canvas, ranges );
    child< QDoubleSpinBox >( dialog, "goal_before" )->setValue( 10.0 );
    child< QDoubleSpinBox >( dialog, "goal_after" )->setValue( 5.0 );
    child< QDoubleSpinBox >( dialog, "around_before" )->setValue( 5.0 );
    child< QDoubleSpinBox >( dialog, "around_after" )->setValue( 5.0 );
    QString error;

    // the goal scene by default: 151 frames, to the last stopped frame
    // before the kick-off (not 5 s after the kick-off)
    dialog.chooseGoal( 0 );
    QCOMPARE( textOf( ranges ), QString( "*goal1-c200[100,200.50]" ) );
    QCOMPARE( dialog.range( &error ).size(), std::size_t( 151 ) );

    // around the position shown, 5 s before the goal: to the frame of the
    // goal, not to the frames stopped after it
    QVERIFY( data.setViewDataIndex( 150 ) );
    dialog.chooseAround();
    QCOMPARE( textOf( ranges ), QString( "*around-c150[100,200.0]" ) );
    QCOMPARE( dialog.range( &error ).size(), std::size_t( 101 ) );
    QCOMPARE( child< QLabel >( dialog, "end_info" )->text().section( ' ', 0, 0 ), QString( "200.0" ) );

    // around a stopped frame (200.25): by the frames
    QVERIFY( data.setViewDataIndex( 225 ) );
    dialog.chooseAround();
    QCOMPARE( textOf( ranges ), QString( "*around-c200[175,225]" ) );
    QCOMPARE( dialog.range( &error ).size(), std::size_t( 101 ) );

    // the end at the frame of the goal shown: that frame
    QVERIFY( data.setViewDataIndex( 200 ) );
    child< QPushButton >( dialog, "end_now" )->click();
    QCOMPARE( child< QLineEdit >( dialog, "end" )->text(), QString( "200.0" ) );
    QCOMPARE( dialog.range( &error ).last, std::size_t( 200 ) );
    // a stopped frame and a frame not followed by stopped ones: as they are
    QVERIFY( data.setViewDataIndex( 210 ) );
    child< QPushButton >( dialog, "end_now" )->click();
    QCOMPARE( child< QLineEdit >( dialog, "end" )->text(), QString( "200.10" ) );
    QVERIFY( data.setViewDataIndex( 199 ) );
    child< QPushButton >( dialog, "end_now" )->click();
    QCOMPARE( child< QLineEdit >( dialog, "end" )->text(), QString( "199" ) );
}

/*-------------------------------------------------------------------*/
/*!
  Several ranges: each in its own directory by its name, or all in one
  sequence (the numbers go on; the names by the cycles must not repeat).
*/
void
TestExportDialog::severalRanges()
{
    MainWindow window;
    window.init();
    window.show();
    QVERIFY( QTest::qWaitForWindowExposed( &window ) );
    window.openRCG( LOG );
    QTemporaryDir tmp;
    ExportDialog * dialog = openDialog( window, tmp.filePath( "separate" ) );
    QVERIFY( dialog );
    ExportRangeList * ranges = window.findChild< ExportRangeList * >( "export_ranges" );
    QRadioButton * separate = child< QRadioButton >( *dialog, "separate_ranges" );
    QRadioButton * one = child< QRadioButton >( *dialog, "one_sequence" );
    QVERIFY( ! separate->isEnabled() ); // one range

    ranges->setRanges( { ExportRangeSpec{ "1", "2", "a b" }, ExportRangeSpec{ "4", "", "c" } } );
    QVERIFY( separate->isEnabled() );
    separate->setChecked( true );
    QVERIFY2( dialog->summaryText().startsWith( "3 images of 3 frames, in 2 ranges" ), qPrintable( dialog->summaryText() ) );
    QString result = exportNow( *dialog );
    QVERIFY2( result.startsWith( "ok: " ), qPrintable( result ) );
    QCOMPARE( QDir( tmp.filePath( "separate/a_b" ) ).entryList( QDir::Files ), QStringList( { "img-00000.png", "img-00001.png" } ) );
    QCOMPARE( QDir( tmp.filePath( "separate/c" ) ).entryList( QDir::Files ), QStringList( { "img-00000.png" } ) );
    QVERIFY( ExportSettings::read().separate_ranges );

    // the same names: made unique
    ranges->setRanges( { ExportRangeSpec{ "1", "1", "x" }, ExportRangeSpec{ "2", "2", "x" } } );
    QCOMPARE( dialog->allImageNames( { { 0 }, { 1 } }, nullptr ), QStringList( { "x/img-00000.png", "x-2/img-00000.png" } ) );

    // one sequence
    dialog->setDirectory( tmp.filePath( "one" ) );
    one->setChecked( true );
    ranges->setRanges( { ExportRangeSpec{ "4", "", "c" }, ExportRangeSpec{ "1", "2", "a" } } );
    result = exportNow( *dialog );
    QVERIFY2( result.startsWith( "ok: 3 images" ), qPrintable( result ) );
    QCOMPARE( QDir( tmp.filePath( "one" ) ).entryList( QDir::Files ), QStringList( { "img-00000.png", "img-00001.png", "img-00002.png" } ) );
    QVERIFY( ! ExportSettings::read().separate_ranges );

    // by the cycles in one sequence: the same frames twice is wrong
    child< QCheckBox >( *dialog, "name_by_cycle" )->setChecked( true );
    ranges->setRanges( { ExportRangeSpec{ "1", "2", "a" }, ExportRangeSpec{ "2", "4", "b" } } );
    QVERIFY2( dialog->summaryText().contains( "names by the cycles repeat" ), qPrintable( dialog->summaryText() ) );
    QVERIFY( ! child< QPushButton >( *dialog, "export" )->isEnabled() );
    QCOMPARE( exportNow( *dialog ), QString( "not started" ) );

    // a wrong range: told with its name
    child< QCheckBox >( *dialog, "name_by_cycle" )->setChecked( false );
    ranges->setRanges( { ExportRangeSpec{ "1", "2", "a" }, ExportRangeSpec{ "4", "2", "b" } } );
    QCOMPARE( dialog->summaryText(), QString( "b: The start is after the end." ) );
    QVERIFY( child< QListWidget >( *dialog, "ranges" )->item( 1 )->text().contains( "The start is after the end." ) );

    // the buttons of the list
    ranges->setRanges( { ExportRangeSpec{ "1", "2", "a" }, ExportRangeSpec{ "4", "", "b" } } );
    ranges->setCurrent( 0 );
    child< QPushButton >( *dialog, "range_down" )->click();
    QCOMPARE( textOf( *ranges ), QString( "b[4,] *a[1,2]" ) );
    child< QPushButton >( *dialog, "add_range" )->click();
    QCOMPARE( textOf( *ranges ), QString( "b[4,] a[1,2] *range1[,]" ) );
    child< QPushButton >( *dialog, "remove_range" )->click();
    QCOMPARE( textOf( *ranges ), QString( "b[4,] *a[1,2]" ) );
    child< QLineEdit >( *dialog, "range_name" )->setText( "" );
    QTest::keyClicks( child< QLineEdit >( *dialog, "range_name" ), "z" );
    QCOMPARE( textOf( *ranges ), QString( "b[4,] *z[1,2]" ) );
    child< QListWidget >( *dialog, "ranges" )->setCurrentRow( 0 );
    QCOMPARE( child< QLineEdit >( *dialog, "start" )->text(), QString( "4" ) );
}

/*-------------------------------------------------------------------*/
/*!
  A video by ffmpeg (a fake one: the raw frames written to the file, D14):
  the fields of the video shown; .mp4 when the file has no extension; the
  quality and the speed given; the size even; the settings kept; a wrong
  extension and a file there.
*/
void
TestExportDialog::exportVideo()
{
#ifdef Q_OS_WIN
    QSKIP( "the fake ffmpeg is a shell script" );
#endif
    MainWindow window;
    window.init();
    window.show();
    QVERIFY( QTest::qWaitForWindowExposed( &window ) );
    window.openRCG( LOG );
    QTemporaryDir tmp;
    ExportDialog * dialog = openDialog( window, tmp.filePath( "images" ) );
    QVERIFY( dialog );
    const QString program = fakeFfmpeg( tmp, "ffmpeg", { "mpeg4", "libx264" } );

    QVERIFY( child< QLineEdit >( *dialog, "directory" )->isVisible() );
    QVERIFY( ! child< QLineEdit >( *dialog, "video_file" )->isVisible() );
    child< QRadioButton >( *dialog, "output_video" )->setChecked( true );
    QVERIFY( dialog->isVideo() );
    QVERIFY( ! child< QLineEdit >( *dialog, "directory" )->isVisible() );
    QVERIFY( child< QLineEdit >( *dialog, "video_file" )->isVisible() );
    QCOMPARE( child< QRadioButton >( *dialog, "separate_ranges" )->text(), QString( "A video for each, by its name" ) );
    QCOMPARE( child< QRadioButton >( *dialog, "one_sequence" )->text(), QString( "All in one video" ) );
    // the defaults (D15)
    QCOMPARE( child< QSpinBox >( *dialog, "crf" )->value(), 18 );
    QCOMPARE( child< QComboBox >( *dialog, "preset" )->currentText(), QString( "medium" ) );

    dialog->setFfmpeg( program );
    QCOMPARE( child< QLabel >( *dialog, "ffmpeg_status" )->text(), QString( "ffmpeg version 9.9-fake; H.264 by libx264." ) );
    QCOMPARE( dialog->ffmpegInfo().encoder, QString( "libx264" ) );
    dialog->setVideoFile( tmp.filePath( "out" ) );
    QCOMPARE( dialog->videoFile(), tmp.filePath( "out.mp4" ) );
    child< QSpinBox >( *dialog, "crf" )->setValue( 20 );
    child< QComboBox >( *dialog, "preset" )->setCurrentText( "fast" );

    // the size even (yuv420p)
    child< QSpinBox >( *dialog, "width" )->setValue( 161 );
    child< QSpinBox >( *dialog, "height" )->setValue( 121 );
    QCOMPARE( dialog->imageSize(), QSize( 160, 120 ) );
    QVERIFY2( dialog->summaryText().startsWith( "3 images of 3 frames, " ), qPrintable( dialog->summaryText() ) );
    QVERIFY2( dialog->summaryText().endsWith( "160 x 120 pixels; a video of 0.3 s at 10 images/s." ), qPrintable( dialog->summaryText() ) );

    QSignalSpy stop( dialog, &ExportDialog::stopRequested );
    const QString video = tmp.filePath( "out.mp4" );
    QCOMPARE( exportNow( *dialog ), "ok: the video " + QDir::toNativeSeparators( video ) );
    QCOMPARE( stop.count(), 1 );
    QCOMPARE( dialog->messageText(), "Exported the video " + QDir::toNativeSeparators( video ) + "." );
    QVERIFY2( window.statusBar()->currentMessage().startsWith( "Exported the video " ), qPrintable( window.statusBar()->currentMessage() ) );
    QCOMPARE( QFileInfo( video ).size(), qint64( 3 ) * 160 * 120 * 4 );
    const QString args = argsOf( video );
    QVERIFY2( args.contains( "-video_size\n160x120\n-framerate\n10\n" ), qPrintable( args ) );
    QVERIFY2( args.contains( "-c:v\nlibx264\n-preset\nfast\n-crf\n20\n-tune\nanimation\n" ), qPrintable( args ) );
    QVERIFY( ! QDir( tmp.filePath( "images" ) ).exists() );

    // the settings written
    const ExportSettings s = ExportSettings::read();
    QVERIFY( s.video );
    QCOMPARE( s.video_file, tmp.filePath( "out" ) );
    QCOMPARE( s.crf, 20 );
    QCOMPARE( s.preset, QString( "fast" ) );
    QCOMPARE( s.ffmpeg, program );

    // a wrong extension
    dialog->setVideoFile( tmp.filePath( "out.avi" ) );
    QCOMPARE( dialog->summaryText(), QString( "A video is a .mp4, .mov or .mkv file: not .avi." ) );
    QVERIFY( ! child< QPushButton >( *dialog, "export" )->isEnabled() );
    QCOMPARE( exportNow( *dialog ), QString( "not started" ) );
    dialog->setVideoFile( tmp.filePath( "out.MKV" ) );
    QVERIFY( child< QPushButton >( *dialog, "export" )->isEnabled() );

    // the file there: asked (cancel: not exported)
    dialog->setVideoFile( video );
    int asked = 0;
    QString question;
    QTimer::singleShot( 200, [&]() {
        QMessageBox * box = qobject_cast< QMessageBox * >( QApplication::activeModalWidget() );
        if ( ! box ) return;
        ++asked;
        question = box->text();
        box->button( QMessageBox::Cancel )->click();
    } );
    QCOMPARE( exportNow( *dialog, true ), QString( "not started" ) );
    QCOMPARE( asked, 1 );
    QCOMPARE( question, QDir::toNativeSeparators( video ) + " is already there." );
}

/*-------------------------------------------------------------------*/
//! several ranges: a video for each (FILE-NAME.mp4), or one for all
void
TestExportDialog::severalVideos()
{
#ifdef Q_OS_WIN
    QSKIP( "the fake ffmpeg is a shell script" );
#endif
    MainWindow window;
    window.init();
    window.show();
    QVERIFY( QTest::qWaitForWindowExposed( &window ) );
    window.openRCG( LOG );
    QTemporaryDir tmp;
    ExportDialog * dialog = openDialog( window, tmp.path() );
    QVERIFY( dialog );
    dialog->setVideo( true );
    dialog->setFfmpeg( fakeFfmpeg( tmp, "ffmpeg", { "libx264" } ) );
    dialog->setVideoFile( tmp.filePath( "game.mp4" ) );
    ExportRangeList * ranges = window.findChild< ExportRangeList * >( "export_ranges" );

    ranges->setRanges( { ExportRangeSpec{ "1", "2", "a b" }, ExportRangeSpec{ "4", "", "c" } } );
    child< QRadioButton >( *dialog, "separate_ranges" )->setChecked( true );
    QCOMPARE( dialog->videoFiles( 2 ), QStringList( { tmp.filePath( "game-a_b.mp4" ), tmp.filePath( "game-c.mp4" ) } ) );
    QVERIFY2( dialog->summaryText().endsWith( "2 videos, 0.3 s in all at 10 images/s." ), qPrintable( dialog->summaryText() ) );
    QCOMPARE( exportNow( *dialog ), "ok: 2 videos in " + QDir::toNativeSeparators( tmp.path() ) );
    QCOMPARE( QFileInfo( tmp.filePath( "game-a_b.mp4" ) ).size(), qint64( 2 ) * 160 * 120 * 4 );
    QCOMPARE( QFileInfo( tmp.filePath( "game-c.mp4" ) ).size(), qint64( 1 ) * 160 * 120 * 4 );
    QVERIFY( ! QFile::exists( tmp.filePath( "game.mp4" ) ) );

    // both kept, and read again: a video, a video for each range
    QVERIFY( ExportSettings::read().video );
    QVERIFY( ExportSettings::read().separate_ranges );
    {
        MainWindow other;
        other.init();
        other.openRCG( LOG );
        other.findChild< QAction * >( "export" )->trigger();
        QVERIFY( other.findChild< ExportDialog * >( "export_dialog" ) );
        QVERIFY( child< QRadioButton >( *other.findChild< ExportDialog * >( "export_dialog" ), "output_video" )->isChecked() );
        QVERIFY( child< QRadioButton >( *other.findChild< ExportDialog * >( "export_dialog" ), "separate_ranges" )->isChecked() );
    }

    // the same names: made unique
    ranges->setRanges( { ExportRangeSpec{ "1", "1", "x" }, ExportRangeSpec{ "2", "2", "x" } } );
    QCOMPARE( dialog->videoFiles( 2 ), QStringList( { tmp.filePath( "game-x.mp4" ), tmp.filePath( "game-x-2.mp4" ) } ) );

    // one for all
    child< QRadioButton >( *dialog, "one_sequence" )->setChecked( true );
    ranges->setRanges( { ExportRangeSpec{ "4", "", "c" }, ExportRangeSpec{ "1", "2", "a" } } );
    QCOMPARE( dialog->videoFiles( 2 ), QStringList( tmp.filePath( "game.mp4" ) ) );
    QVERIFY2( dialog->summaryText().endsWith( "a video of 0.3 s at 10 images/s." ), qPrintable( dialog->summaryText() ) );
    QCOMPARE( exportNow( *dialog ), "ok: the video " + QDir::toNativeSeparators( tmp.filePath( "game.mp4" ) ) );
    QCOMPARE( QFileInfo( tmp.filePath( "game.mp4" ) ).size(), qint64( 3 ) * 160 * 120 * 4 );
}

/*-------------------------------------------------------------------*/
//! no ffmpeg: told, no export; another encoder (mpeg4) told, and used (D14)
void
TestExportDialog::noFfmpeg()
{
#ifdef Q_OS_WIN
    QSKIP( "the fake ffmpeg is a shell script" );
#endif
    MainWindow window;
    window.init();
    window.show();
    QVERIFY( QTest::qWaitForWindowExposed( &window ) );
    window.openRCG( LOG );
    QTemporaryDir tmp;
    ExportDialog * dialog = openDialog( window, tmp.path() );
    QVERIFY( dialog );
    dialog->setVideo( true );
    dialog->setVideoFile( tmp.filePath( "v.mp4" ) );
    QLabel * status = child< QLabel >( *dialog, "ffmpeg_status" );
    QPushButton * button = child< QPushButton >( *dialog, "export" );

    dialog->setFfmpeg( tmp.filePath( "missing" ) );
    QVERIFY( ! dialog->ffmpegInfo().found );
    QVERIFY2( status->text().contains( "is not found" ), qPrintable( status->text() ) );
    QVERIFY2( status->text().endsWith( "A video needs ffmpeg: install it, or choose it." ), qPrintable( status->text() ) );
    QVERIFY2( dialog->summaryText().endsWith( "is not found, or cannot be run." ), qPrintable( dialog->summaryText() ) );
    QVERIFY( ! button->isEnabled() );
    QCOMPARE( exportNow( *dialog ), QString( "not started" ) );
    QVERIFY2( dialog->messageText().contains( "is not found" ), qPrintable( dialog->messageText() ) );

    // the images need no ffmpeg
    dialog->setVideo( false );
    QVERIFY( button->isEnabled() );
    dialog->setVideo( true );
    QVERIFY( ! button->isEnabled() );

    // no H.264: MPEG-4 Part 2, the quality and the speed not used
    dialog->setFfmpeg( fakeFfmpeg( tmp, "ffmpeg", { "mpeg4" } ) );
    QVERIFY( dialog->ffmpegInfo().found );
    QVERIFY2( status->text().contains( "no H.264: MPEG-4 Part 2 by mpeg4" ), qPrintable( status->text() ) );
    QVERIFY( ! child< QSpinBox >( *dialog, "crf" )->isEnabled() );
    QVERIFY( ! child< QComboBox >( *dialog, "preset" )->isEnabled() );
    QVERIFY( button->isEnabled() );
    QVERIFY( exportNow( *dialog ).startsWith( "ok: the video " ) );
    const QString args = argsOf( tmp.filePath( "v.mp4" ) );
    QVERIFY2( args.contains( "-c:v\nmpeg4\n-q:v\n2\n" ), qPrintable( args ) );
    QVERIFY( ! args.contains( "-crf" ) );

    dialog->setFfmpeg( fakeFfmpeg( tmp, "ffmpeg264", { "libx264" } ) );
    QVERIFY( child< QSpinBox >( *dialog, "crf" )->isEnabled() );
}

/*-------------------------------------------------------------------*/
//! the speed as the monitor plays the log: the step / the interval of the playback
void
TestExportDialog::playbackSpeed()
{
    MainWindow window;
    window.init();
    window.show();
    QVERIFY( QTest::qWaitForWindowExposed( &window ) );
    window.openRCG( LOG );
    QTemporaryDir tmp;
    ExportDialog * dialog = openDialog( window, tmp.path() );
    QVERIFY( dialog );
    QDoubleSpinBox * speed = child< QDoubleSpinBox >( *dialog, "speed" );
    speed->setValue( 3.0 );
    child< QPushButton >( *dialog, "playback_speed" )->click();
    const double expected = std::round( 100.0 / Options::instance().timerInterval() * 100.0 ) / 100.0;
    QCOMPARE( speed->value(), expected );
}

SOCCERWINDOW2_GUI_TEST_MAIN( TestExportDialog )
#include "test_export_dialog.moc"
