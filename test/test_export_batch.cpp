// -*-c++-*-

/*!
  \file test_export_batch.cpp
  \brief unit test of ExportBatch: the export from the command line
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

#include "export_batch.h"
#include "main_data.h"
#include "options.h"
#include "view_holder.h"

#include "test_environment.h"

#include <QTemporaryDir>

#include <sstream>
#include <string>
#include <vector>

#ifndef SOCCERWINDOW2_TEST_DATA_DIR
#error "SOCCERWINDOW2_TEST_DATA_DIR is not defined."
#endif

/*
  docs/plan/2026-11-28-00_frame-export.md 4.5, S13, S14, D4, D14, §6.
  Ported from rcssmonitor. test/data/head_v5.rcg has 3 frames: the cycles
  1, 2 and 4. The test runs with HOME in a temporary directory and the
  offscreen platform (test_environment.h).

  The options are parsed by Options::parseCmdLine() of the singleton, as
  main() does: what a switch (--auto-image-save) or an option out of the
  export (--canvas-size, --image-save-dir, ...) sets is kept for the next
  calls, so they are given only in the last test (autoImageSave()), and no
  game log is given to it (a second one is an error).
*/

namespace {

const QString LOG = QStringLiteral( SOCCERWINDOW2_TEST_DATA_DIR "/head_v5.rcg" );

//! a command line (the program's name first) as argc and argv
class CommandLine {
private:
    std::vector< QByteArray > M_args;
    std::vector< char * > M_argv;
public:
    explicit
    CommandLine( const QStringList & args )
      {
          M_args.push_back( QByteArray( "soccerwindow2" ) );
          for ( const QString & a : args ) M_args.push_back( a.toLocal8Bit() );
          for ( QByteArray & a : M_args ) M_argv.push_back( a.data() );
          M_argv.push_back( nullptr );
      }
    int argc() const { return static_cast< int >( M_args.size() ); }
    char ** argv() { return M_argv.data(); }
};

//! the options of a command line read as main() reads them
bool
readArgs( const QStringList & args,
          ExportBatch::Settings * settings,
          QString * error )
{
    CommandLine command_line( args );
    if ( ! Options::instance().parseCmdLine( command_line.argc(), command_line.argv() ) )
    {
        *error = QStringLiteral( "Options::parseCmdLine() failed" );
        return false;
    }
    return ExportBatch::readOptions( Options::instance(), settings, error );
}

//! whether the command line asks for an export
bool
asked( const QStringList & args )
{
    CommandLine command_line( args );
    return ExportBatch::isAsked( command_line.argc(), command_line.argv() );
}

/*!
  a game as a game log (rcg v5) in a directory: the headers of
  test/data/head_v5.rcg (halves of 300 s), and the cycles 0 to 6000 with a
  goal of the left at 100 and of the right at 3100 (as test_export_dialog)
  \return the path of the game log, or an empty one
*/
QString
writeGame( const QString & dir )
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

    const std::vector< std::pair< int, const char * > > frames = {
        { 0, "before_kick_off" }, { 50, "play_on" }, { 100, "goal_l" }, { 101, "kick_off_r" }, { 140, "play_on" },
        { 160, "play_on" }, { 2990, "play_on" }, { 3000, "play_on" }, { 3001, "kick_off_r" }, { 3100, "goal_r" },
        { 3101, "kick_off_l" }, { 3200, "play_on" }, { 6000, "time_over" } };
    for ( const auto & f : frames )
    {
        const int left = ( f.first >= 100 ? 1 : 0 );
        const int right = ( f.first >= 3100 ? 1 : 0 );
        lines << QString( "(playmode %1 %2)" ).arg( f.first ).arg( f.second );
        lines << QString( "(team %1 LEFT RIGHT %2 %3)" ).arg( f.first ).arg( left ).arg( right );
        lines << QString( "(show %1" ).arg( f.first ) + show;
    }

    const QString path = QDir( dir ).filePath( "game.rcg" );
    QFile file( path );
    if ( ! file.open( QIODevice::WriteOnly | QIODevice::Text ) ) return QString();
    file.write( ( lines.join( QLatin1Char( '\n' ) ) + QLatin1Char( '\n' ) ).toUtf8() );
    return path;
}

//! the ranges as "NAME:FIRST-LAST" (frame indices), a space between
QString
textOf( const std::vector< ExportBatch::NamedRange > & ranges )
{
    QStringList list;
    for ( const ExportBatch::NamedRange & r : ranges )
    {
        list << QString( "%1:%2-%3" ).arg( r.name ).arg( r.range.first ).arg( r.range.last );
    }
    return list.join( ' ' );
}

//! a fake ffmpeg: its version and encoders, its arguments into FILE.args, the frames into FILE
QString
fakeFfmpeg( const QTemporaryDir & dir )
{
    const QString path = dir.filePath( "ffmpeg" );
    QFile file( path );
    if ( ! file.open( QIODevice::WriteOnly | QIODevice::Text ) ) return QString();
    file.write( "#!/bin/sh\n"
                "case \"$1 $2\" in\n"
                "  \"-hide_banner -version\") echo 'ffmpeg version 9.9-fake Copyright (c) the tests'; exit 0 ;;\n"
                "  \"-hide_banner -encoders\") printf '%s\\n' 'Encoders:' ' V....D libx264    the encoder libx264'; exit 0 ;;\n"
                "esac\n"
                "for a in \"$@\"; do last=\"$a\"; done\n"
                "printf '%s\\n' \"$@\" > \"$last.args\"\n"
                "exec cat > \"$last\"\n" );
    file.close();
    file.setPermissions( file.permissions() | QFileDevice::ExeOwner | QFileDevice::ExeUser );
    return path;
}

}

/*!
  \brief the export from the command line (docs/plan/2026-11-28-00_frame-export.md 4.5)
*/
class TestExportBatch
    : public QObject {
    Q_OBJECT

private slots:

    //! the options that make main() export without a window
    void isAsked()
      {
          QVERIFY( ! asked( {} ) );
          QVERIFY( ! asked( { "game.rcg" } ) );
          QVERIFY( ! asked( { "--canvas-size", "320x240", "game.rcg" } ) );
          // a value is not an option
          QVERIFY( ! asked( { "--image-name-prefix", "export-video", "game.rcg" } ) );
          QVERIFY( asked( { "--export-video", "a.mp4", "game.rcg" } ) );
          QVERIFY( asked( { "--export-video=a.mp4", "game.rcg" } ) );
          QVERIFY( asked( { "--export-images", "dir" } ) );
          QVERIFY( asked( { "--auto-image-save", "game.rcg" } ) );
          // an option of the export without the export: told as an error, without a window
          QVERIFY( asked( { "--export-fps", "30" } ) );
          QVERIFY( asked( { "--ffmpeg=/opt/ffmpeg" } ) );
      }

    //! the options: the defaults, all of them, and the wrong ones
    void options()
      {
          ExportBatch::Settings s;
          QString error;
          QVERIFY2( readArgs( {}, &s, &error ), qPrintable( error ) );
          QVERIFY( ! s.enabled() );

          s = ExportBatch::Settings();
          QVERIFY2( readArgs( { "--export-video", "out.mp4" }, &s, &error ), qPrintable( error ) );
          QVERIFY( s.enabled() );
          QCOMPARE( s.video, QString( "out.mp4" ) );
          QVERIFY( s.images.isEmpty() );
          QVERIFY( s.ranges.isEmpty() );
          QVERIFY( ! s.join );
          QCOMPARE( s.fps, 10.0 );
          QCOMPARE( s.speed, 1.0 );
          QCOMPARE( s.width, 1280 );
          QCOMPARE( s.height, 720 );
          QCOMPARE( s.scale, 0.0 );
          QCOMPARE( s.crf, 18 );
          QCOMPARE( s.preset, QString( "medium" ) );
          QCOMPARE( s.image_format, QString( "png" ) );
          QCOMPARE( s.image_prefix, QString( "image-" ) );
          QVERIFY( s.ffmpeg.isEmpty() );

          // all of them; --export-range again for more ranges, --name=value too (S13)
          s = ExportBatch::Settings();
          QVERIFY2( readArgs( { "--export-images", "dir", "--export-range", "goals:5:3",
                                "--export-range=100-200", "--export-range", "-50", "--export-join", "--export-fps", "60",
                                "--export-speed", "0.5", "--export-size", "1920x1080", "--export-scale", "2",
                                "--export-image-format", "JPEG", "--export-image-prefix", "f-", "--export-crf", "23",
                                "--export-preset", "slow", "--ffmpeg", "/opt/ffmpeg" }, &s, &error ),
                    qPrintable( error ) );
          QCOMPARE( s.images, QString( "dir" ) );
          QCOMPARE( s.ranges, QStringList( { "goals:5:3", "100-200", "-50" } ) );
          QVERIFY( s.join );
          QCOMPARE( s.fps, 60.0 );
          QCOMPARE( s.speed, 0.5 );
          QCOMPARE( s.width, 1920 );
          QCOMPARE( s.height, 1080 );
          QCOMPARE( s.scale, 2.0 );
          QCOMPARE( s.image_format, QString( "jpg" ) );
          QCOMPARE( s.image_prefix, QString( "f-" ) );
          QCOMPARE( s.crf, 23 );
          QCOMPARE( s.preset, QString( "slow" ) );
          QCOMPARE( s.ffmpeg, QString( "/opt/ffmpeg" ) );

          // the options of the last command line only
          s = ExportBatch::Settings();
          QVERIFY2( readArgs( { "--export-images", "dir" }, &s, &error ), qPrintable( error ) );
          QVERIFY( s.ranges.isEmpty() );
          QVERIFY( ! s.join );
          QCOMPARE( s.image_prefix, QString( "image-" ) );

          const QList< QPair< QStringList, QString > > wrong = {
              { { "--export-video", "a.mp4", "--export-images", "d" }, "not both" },
              { { "--export-fps", "30" }, "--export-fps is an option of the export" },
              { { "--export-video", "", "--export-range", "all" }, "--export-video is an option of the export" },
              { { "--export-images", "d", "--export-fps", "0" }, "--export-fps 0" },
              { { "--export-images", "d", "--export-speed", "fast" }, "--export-speed fast: a number" },
              { { "--export-images", "d", "--export-scale", "20" }, "--export-scale 20" },
              { { "--export-images", "d", "--export-size", "1920" }, "WxH" },
              { { "--export-images", "d", "--export-size", "8x8" }, "WxH" },
              { { "--export-images", "d", "--export-image-format", "gif" }, "png or jpg" },
              { { "--export-video", "a.mp4", "--export-crf", "52" }, "0 to 51" },
              { { "--export-video", "a.mp4", "--export-preset", "fastest" }, "fastest" },
          };
          for ( const auto & w : wrong )
          {
              s = ExportBatch::Settings();
              error.clear();
              QVERIFY2( ! readArgs( w.first, &s, &error ), qPrintable( w.first.join( ' ' ) ) );
              QVERIFY2( error.contains( w.second ), qPrintable( error ) );
          }
      }

    //! the ranges: positions, all, the parts of the game, the goals; the wrong ones
    void ranges()
      {
          QTemporaryDir tmp;
          const QString game = writeGame( tmp.path() );
          QVERIFY( ! game.isEmpty() );
          MainData data;
          QVERIFY( data.openRCG( game.toStdString() ) );
          const ViewHolder & holder = data.viewHolder();
          QCOMPARE( holder.monitorViewCont().size(), std::size_t( 13 ) );
          std::vector< ExportBatch::NamedRange > ranges;
          QString error;

          QVERIFY2( ExportBatch::resolveRanges( {}, holder, &ranges, &error ), qPrintable( error ) );
          QCOMPARE( textOf( ranges ), QString( "all:0-12" ) );

          QVERIFY2( ExportBatch::resolveRanges( { "50-140", " -100 ", "3001-", "first-half", "second-half" }, holder, &ranges, &error ),
                    qPrintable( error ) );
          QCOMPARE( textOf( ranges ), QString( "50-140:1-4 -100:0-2 3001-:8-12 first-half:0-7 second-half:8-12" ) );

          // a range for each goal, named as the export dialog's, by the frames
          // from the frame of the goal (D12): 10 s before and 5 s after (100
          // and 50 frames) are out of the log of 13 frames
          QVERIFY2( ExportBatch::resolveRanges( { "goals" }, holder, &ranges, &error ), qPrintable( error ) );
          QCOMPARE( textOf( ranges ), QString( "goal1-c100:0-12 goal2-c3100:0-12" ) );
          QVERIFY2( ExportBatch::resolveRanges( { "goals:0:0" }, holder, &ranges, &error ), qPrintable( error ) );
          QCOMPARE( textOf( ranges ), QString( "goal1-c100:2-2 goal2-c3100:9-9" ) );
          QVERIFY2( ExportBatch::resolveRanges( { "goals:0.2:0.1", "all" }, holder, &ranges, &error ), qPrintable( error ) );
          QCOMPARE( textOf( ranges ), QString( "goal1-c100:0-3 goal2-c3100:7-10 all:0-12" ) );

          const QList< QPair< QString, QString > > wrong = {
              { "100", "START-END" },
              { "goals:5", "goals:BEFORE:AFTER" },
              { "goals:-1:5", "goals:BEFORE:AFTER" },
              { "extra-1", "no such part" },
              { "9000-9100", "after the last frame" },
              { "half", "START-END" },
          };
          for ( const auto & w : wrong )
          {
              error.clear();
              QVERIFY2( ! ExportBatch::resolveRanges( { w.first }, holder, &ranges, &error ), qPrintable( w.first ) );
              QVERIFY2( error.contains( w.second ), qPrintable( error ) );
          }

          // no frame, no goal
          MainData none;
          QVERIFY( ! ExportBatch::resolveRanges( {}, none.viewHolder(), &ranges, &error ) );
          QCOMPARE( error, QString( "The game log has no frame." ) );
          MainData scoreless;
          QVERIFY( scoreless.openRCG( LOG.toStdString() ) );
          QVERIFY( ! ExportBatch::resolveRanges( { "goals" }, scoreless.viewHolder(), &ranges, &error ) );
          QCOMPARE( error, QString( "There is no goal in the game log." ) );
      }

    //! images of the game log: the files, the progress and the end told, the exit code 0
    void images()
      {
          QTemporaryDir tmp;
          ExportBatch::Settings s;
          s.images = tmp.filePath( "out" );
          s.width = 320;
          s.height = 240;
          std::ostringstream log;
          QCOMPARE( ExportBatch( s, log ).run( LOG ), 0 );
          const QString told = QString::fromStdString( log.str() );
          QVERIFY2( told.contains( "Exporting 3 images of 3 frames, 320 x 240 pixels, 10 images/s, to " ), qPrintable( told ) );
          QVERIFY2( told.contains( "Exported 3 images in " ), qPrintable( told ) );
          QVERIFY2( told.contains( "Finished in " ), qPrintable( told ) );
          QDir dir( s.images );
          QCOMPARE( dir.entryList( QDir::Files ), QStringList( { "image-00000.png", "image-00001.png", "image-00002.png" } ) );
          QCOMPARE( QImage( dir.filePath( "image-00000.png" ) ).size(), QSize( 320, 240 ) );

          // two ranges, each in its own directory; joined, the numbers going on
          s.images = tmp.filePath( "two" );
          s.ranges = QStringList( { "-1", "1-" } );
          s.image_prefix = "f-";
          std::ostringstream log2;
          QCOMPARE( ExportBatch( s, log2 ).run( LOG ), 0 );
          QVERIFY2( QString::fromStdString( log2.str() ).contains( "in 2 ranges" ), log2.str().c_str() );
          QCOMPARE( QDir( s.images ).entryList( QDir::Dirs | QDir::NoDotAndDotDot ), QStringList( { "-1", "1-" } ) );
          // the cycles 1, 2 and 4: -1 is the first frame, 1- all three
          QCOMPARE( QDir( s.images + "/-1" ).entryList( QDir::Files ), QStringList( "f-00000.png" ) );
          QCOMPARE( QDir( s.images + "/1-" ).entryList( QDir::Files ).size(), 3 );
          s.images = tmp.filePath( "joined" );
          s.join = true;
          std::ostringstream log3;
          QCOMPARE( ExportBatch( s, log3 ).run( LOG ), 0 );
          QCOMPARE( QDir( s.images ).entryList( QDir::Files ).size(), 4 );
          QVERIFY( QFile::exists( s.images + "/f-00003.png" ) );

          // twice as fast: every other frame (the cycles 1 and 4)
          s.images = tmp.filePath( "fast" );
          s.ranges.clear();
          s.join = false;
          s.speed = 2.0;
          s.image_format = "jpg";
          std::ostringstream log4;
          QCOMPARE( ExportBatch( s, log4 ).run( LOG ), 0 );
          QCOMPARE( QDir( s.images ).entryList( QDir::Files ), QStringList( { "f-00000.jpg", "f-00001.jpg" } ) );

          // nothing written in ~/.soccerwindow2 (S14)
          QVERIFY( ! QFile::exists( QDir::homePath() + "/.soccerwindow2" ) );
      }

    //! a video by ffmpeg: one file, the arguments of the 2D view; a video for each range
    void video()
      {
#ifdef Q_OS_WIN
          QSKIP( "the fake ffmpeg is a shell script" );
#endif
          QTemporaryDir tmp;
          ExportBatch::Settings s;
          s.video = tmp.filePath( "game" );
          s.width = 161;
          s.height = 121;
          s.ffmpeg = fakeFfmpeg( tmp );
          s.crf = 21;
          std::ostringstream log;
          QCOMPARE( ExportBatch( s, log ).run( LOG ), 0 );
          const QString told = QString::fromStdString( log.str() );
          QVERIFY2( told.contains( "ffmpeg version 9.9-fake; H.264 by libx264" ), qPrintable( told ) );
          QVERIFY2( told.contains( "160 x 120 pixels" ), qPrintable( told ) );
          QVERIFY2( told.contains( "finishing the video" ), qPrintable( told ) );
          const QString file = tmp.filePath( "game.mp4" );
          QCOMPARE( QFileInfo( file ).size(), qint64( 160 * 120 * 4 * 3 ) );
          QFile args( file + ".args" );
          QVERIFY( args.open( QIODevice::ReadOnly ) );
          const QString text = QString::fromUtf8( args.readAll() );
          QVERIFY2( text.contains( "animation" ), qPrintable( text ) );
          QVERIFY2( text.contains( "\n21\n" ), qPrintable( text ) );

          // FILE-NAME.ext for each range
          s.video = tmp.filePath( "part.mkv" );
          s.ranges = QStringList( { "-1", "2-" } );
          std::ostringstream log2;
          QCOMPARE( ExportBatch( s, log2 ).run( LOG ), 0 );
          QCOMPARE( QFileInfo( tmp.filePath( "part--1.mkv" ) ).size(), qint64( 160 * 120 * 4 * 1 ) );
          QCOMPARE( QFileInfo( tmp.filePath( "part-2-.mkv" ) ).size(), qint64( 160 * 120 * 4 * 2 ) );
      }

    //! what cannot be exported: told, the exit code 1
    void failures()
      {
          QTemporaryDir tmp;
          ExportBatch::Settings s;
          s.images = tmp.filePath( "out" );
          const auto failure = [&]( const QString & game_log, const QString & expected ) {
              std::ostringstream log;
              const int code = ExportBatch( s, log ).run( game_log );
              const QString told = QString::fromStdString( log.str() );
              if ( code != 1
                   || ! told.contains( "soccerwindow2: " )
                   || ! told.contains( expected ) )
              {
                  qWarning( "exit code %d: %s", code, qPrintable( told ) );
                  return false;
              }
              return true;
          };
          QVERIFY( failure( QString(), "No game log" ) );
          QVERIFY( failure( tmp.filePath( "none.rcg" ), "Could not read the game log" ) );
          s.ranges = QStringList( "100" );
          QVERIFY( failure( LOG, "The range 100: " ) );
          s.ranges = QStringList( "goals" );
          QVERIFY( failure( LOG, "There is no goal in the game log." ) );
          s.ranges.clear();
          s.images.clear();
          QVERIFY( failure( LOG, "Nothing to export" ) );
          s.video = tmp.filePath( "a.avi" );
          QVERIFY( failure( LOG, "A video is a .mp4, .mov or .mkv file: not .avi." ) );
          s.video = tmp.filePath( "a.mp4" );
          s.ffmpeg = tmp.filePath( "no-ffmpeg" );
          QVERIFY( failure( LOG, "A video needs ffmpeg" ) );
          QVERIFY( ! QFile::exists( tmp.filePath( "a.mp4" ) ) );
          // a directory that cannot be made
          QFile file( tmp.filePath( "file" ) );
          QVERIFY( file.open( QIODevice::WriteOnly ) );
          file.close();
          s.video.clear();
          s.images = tmp.filePath( "file/out" );
          QVERIFY( failure( LOG, tmp.filePath( "file/out" ) ) );
      }

    /*!
      --auto-image-save (D4, D14): the images of the whole game log, by the
      options of the images and the size of the canvas. Last: the switch
      and the options are kept by the options for the next calls.
    */
    void autoImageSave()
      {
          ExportBatch::Settings s;
          QString error;
          QVERIFY2( readArgs( { "--auto-image-save" }, &s, &error ), qPrintable( error ) );
          QCOMPARE( s.images, QDir::currentPath() );
          QVERIFY( s.video.isEmpty() );
          QVERIFY( s.ranges.isEmpty() );
          QCOMPARE( s.image_prefix, QString( "image-" ) );
          QCOMPARE( s.image_format, QString( "png" ) );
          QCOMPARE( s.width, 1280 );
          QCOMPARE( s.height, 720 );
          QCOMPARE( s.scale, 1.0 );
          QCOMPARE( s.fps, 10.0 );
          QCOMPARE( s.speed, 1.0 );

          QTemporaryDir tmp;
          s = ExportBatch::Settings();
          QVERIFY2( readArgs( { "--auto-image-save", "--image-save-dir", tmp.filePath( "auto" ),
                                "--image-name-prefix", "shot//", "--image-save-format", "JPEG", "--canvas-size", "320x240" },
                              &s, &error ),
                    qPrintable( error ) );
          QCOMPARE( s.images, tmp.filePath( "auto" ) );
          QCOMPARE( s.image_prefix, QString( "shot" ) );
          QCOMPARE( s.image_format, QString( "jpg" ) );
          QCOMPARE( s.width, 320 );
          QCOMPARE( s.height, 240 );
          QCOMPARE( s.scale, 1.0 );

          // the names numbered by the frames from 0, as ImageSaveDialog named them
          std::ostringstream log;
          QCOMPARE( ExportBatch( s, log ).run( LOG ), 0 );
          QCOMPARE( QDir( s.images ).entryList( QDir::Files ), QStringList( { "shot00000.jpg", "shot00001.jpg", "shot00002.jpg" } ) );
          QCOMPARE( QImage( s.images + "/shot00000.jpg" ).size(), QSize( 320, 240 ) );

          const QList< QPair< QStringList, QString > > wrong = {
              { { "--auto-image-save", "--export-images", "d" }, "only one of them" },
              { { "--auto-image-save", "--export-fps", "30" }, "--export-fps is an option of the export" },
              { { "--auto-image-save", "--canvas-size", "8x8" }, "--canvas-size 8x8" },
              { { "--auto-image-save", "--canvas-size", "320x240", "--image-save-format", "BMP" }, "--image-save-format BMP: png or jpg" },
          };
          for ( const auto & w : wrong )
          {
              s = ExportBatch::Settings();
              error.clear();
              QVERIFY2( ! readArgs( w.first, &s, &error ), qPrintable( w.first.join( ' ' ) ) );
              QVERIFY2( error.contains( w.second ), qPrintable( error ) );
          }
      }
};

SOCCERWINDOW2_GUI_TEST_MAIN( TestExportBatch )
#include "test_export_batch.moc"
