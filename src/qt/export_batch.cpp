// -*-c++-*-

/*!
  \file export_batch.cpp
  \brief the export from the command line, without a window. Source File.
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

#include "export_batch.h"

#include "export_job.h"
#include "export_range_list.h"
#include "export_settings.h"
#include "ffmpeg_sink.h"
#include "field_canvas.h"
#include "image_sequence_sink.h"

#include "options.h"
#include "view_holder.h"

#include <rcsc/param/cmd_line_parser.h>

#include <QDir>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QObject>
#include <QRegularExpression>

#include <algorithm>
#include <iterator>
#include <list>
#include <memory>
#include <set>
#include <string>

namespace {

//! the extensions of the videos: their formats (rcssmonitor's D14)
const char * const VIDEO_EXTENSIONS[] = { "mp4", "mov", "mkv" };

//! the options only of the export from the command line (Options::parseCmdLine()), but --export-video and --export-images
const char * const OPTION_NAMES[] = {
    "export-range", "export-join", "export-fps", "export-speed", "export-size", "export-scale",
    "export-crf", "export-preset", "ffmpeg",
};

//! the options that are also the first values of the export dialog (D15)
const char * const DIALOG_OPTION_NAMES[] = {
    "export-image-format", "export-image-prefix",
};

//! the size of the images: from 16x16 to 7680x4320
bool
isSizeValid( const int width,
             const int height )
{
    return 16 <= width && width <= 7680
        && 16 <= height && height <= 4320;
}

//! the names of the ranges in the names of files: safe, and made unique (name, name-2, ...)
QStringList
fileNames( const std::vector< ExportBatch::NamedRange > & ranges )
{
    QStringList names;
    std::set< QString > used;
    for ( const ExportBatch::NamedRange & r : ranges )
    {
        const QString base = ExportRangeList::safeName( r.name );
        QString name = base;
        for ( int n = 2; used.count( name ) > 0; ++n )
        {
            name = base + QString( "-%1" ).arg( n );
        }
        used.insert( name );
        names << name;
    }
    return names;
}

//! the text of an option given
QString
textOf( const std::string & value )
{
    return QString::fromLocal8Bit( value.c_str() );
}

//! a number of an option given, in its range
bool
readNumber( const Options::ExportOptions & given,
            const char * name,
            const std::string & text,
            const double min,
            const double max,
            double * value,
            QString * error )
{
    if ( ! given.isGiven( name ) )
    {
        return true;
    }
    bool ok = false;
    const double v = textOf( text ).trimmed().toDouble( &ok );
    if ( ! ok || v < min || max < v )
    {
        *error = QObject::tr( "--%1 %2: a number from %3 to %4 is expected." )
            .arg( QLatin1String( name ), textOf( text ) ).arg( min ).arg( max );
        return false;
    }
    *value = v;
    return true;
}

}

/*-------------------------------------------------------------------*/
bool
ExportBatch::isAsked( const int argc,
                      const char * const * argv )
{
    // the arguments split as the parser splits them (--name value, --name=value)
    const rcsc::CmdLineParser split( argc, argv );
    for ( const std::string & arg : split.args() )
    {
        if ( arg.compare( 0, 2, "--" ) != 0 )
        {
            continue;
        }
        const std::string name = arg.substr( 2 );
        if ( name == "export-video"
             || name == "export-images"
             || std::find( std::begin( OPTION_NAMES ), std::end( OPTION_NAMES ), name ) != std::end( OPTION_NAMES ) )
        {
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------*/
bool
ExportBatch::readOptions( const Options & options,
                          Settings * settings,
                          QString * error )
{
    const Options::ExportOptions & b = options.exportOptions();
    Settings & s = *settings;
    s.video = textOf( b.video );
    s.images = textOf( b.images );
    if ( ! s.video.isEmpty() && ! s.images.isEmpty() )
    {
        *error = QObject::tr( "Give --export-video or --export-images, not both." );
        return false;
    }
    if ( ! s.enabled() )
    {
        // the options of the export dialog may be given alone (D15)
        for ( const std::string & name : b.given )
        {
            if ( std::find( std::begin( DIALOG_OPTION_NAMES ), std::end( DIALOG_OPTION_NAMES ), name ) != std::end( DIALOG_OPTION_NAMES ) )
            {
                continue;
            }
            *error = QObject::tr( "--%1 is an option of the export: give --export-video FILE or --export-images DIR too." )
                .arg( textOf( name ) );
            return false;
        }
        return true;
    }

    for ( const std::string & range : b.ranges )
    {
        s.ranges << textOf( range );
    }
    s.join = b.join;
    if ( ! readNumber( b, "export-fps", b.fps, 0.1, 240.0, &s.fps, error )
         || ! readNumber( b, "export-speed", b.speed, 0.01, 64.0, &s.speed, error )
         || ! readNumber( b, "export-scale", b.scale, 0.1, 16.0, &s.scale, error ) )
    {
        return false;
    }
    if ( b.isGiven( "export-size" ) )
    {
        static const QRegularExpression size( QStringLiteral( "^\\s*(\\d+)\\s*[xX]\\s*(\\d+)\\s*$" ) );
        const QRegularExpressionMatch m = size.match( textOf( b.size ) );
        const int w = ( m.hasMatch() ? m.captured( 1 ).toInt() : 0 );
        const int h = ( m.hasMatch() ? m.captured( 2 ).toInt() : 0 );
        if ( ! isSizeValid( w, h ) )
        {
            *error = QObject::tr( "--export-size %1: WxH from 16x16 to 7680x4320 is expected." ).arg( textOf( b.size ) );
            return false;
        }
        s.width = w;
        s.height = h;
    }
    if ( b.isGiven( "export-image-format" ) )
    {
        // png, jpg or jpeg (Options::parseCmdLine() has checked it)
        s.image_format = ExportSettings::formatOf( textOf( b.image_format ) );
    }
    if ( b.isGiven( "export-image-prefix" ) )
    {
        s.image_prefix = textOf( b.image_prefix );
    }
    if ( b.isGiven( "export-crf" ) )
    {
        bool ok = false;
        const int crf = textOf( b.crf ).trimmed().toInt( &ok );
        if ( ! ok || crf < 0 || 51 < crf )
        {
            *error = QObject::tr( "--export-crf %1: 0 to 51 is expected." ).arg( textOf( b.crf ) );
            return false;
        }
        s.crf = crf;
    }
    if ( b.isGiven( "export-preset" ) )
    {
        const QString preset = textOf( b.preset ).trimmed();
        if ( ! FfmpegSink::PRESETS.contains( preset ) )
        {
            *error = QObject::tr( "--export-preset %1: one of %2 is expected." ).arg( preset, FfmpegSink::PRESETS.join( ", " ) );
            return false;
        }
        s.preset = preset;
    }
    if ( b.isGiven( "ffmpeg" ) )
    {
        s.ffmpeg = textOf( b.ffmpeg );
    }
    return true;
}

/*-------------------------------------------------------------------*/
bool
ExportBatch::resolveRanges( const QStringList & texts,
                            const ViewHolder & holder,
                            std::vector< NamedRange > * ranges,
                            QString * error )
{
    ranges->clear();
    const MonitorViewData::Cont & frames = holder.monitorViewCont();
    if ( frames.empty() )
    {
        *error = QObject::tr( "The game log has no frame." );
        return false;
    }
    const ExportGameParam param = ExportGameParam::current();

    const QStringList list = ( texts.isEmpty() ? QStringList( QStringLiteral( "all" ) ) : texts );
    bool goals_asked = false;
    for ( const QString & given : list )
    {
        const QString text = given.trimmed();
        if ( text == QLatin1String( "all" ) )
        {
            NamedRange r;
            r.range.first = 0;
            r.range.last = frames.size() - 1;
            r.name = text;
            ranges->push_back( r );
            continue;
        }

        // goals[:BEFORE:AFTER]: a range for each goal (D10: 10 s before to 5 s
        // after), counted in the frames from the frame of the goal (D12), as
        // the presets of the export dialog
        if ( text == QLatin1String( "goals" )
             || text.startsWith( QLatin1String( "goals:" ) ) )
        {
            double before = 10.0;
            double after = 5.0;
            if ( text != QLatin1String( "goals" ) )
            {
                const QStringList parts = text.split( QLatin1Char( ':' ) );
                bool ok_before = false, ok_after = false;
                if ( parts.size() == 3 )
                {
                    before = parts[1].toDouble( &ok_before );
                    after = parts[2].toDouble( &ok_after );
                }
                if ( ! ok_before || ! ok_after || before < 0.0 || after < 0.0 )
                {
                    *error = QObject::tr( "The range %1: goals or goals:BEFORE:AFTER (seconds) is expected." ).arg( text );
                    return false;
                }
            }
            goals_asked = true;
            const std::vector< ExportGoal > goals = ExportRange::goals( frames, holder.scoreChangeIndices() );
            for ( std::size_t i = 0; i < goals.size(); ++i )
            {
                NamedRange r;
                r.range = ExportRange::around( frames, goals[i].frame, before, after, param.step_ms );
                r.name = QString( "goal%1-c%2" ).arg( i + 1 ).arg( goals[i].cycle );
                if ( r.range.isValid() ) ranges->push_back( r );
            }
            continue;
        }

        // a part of the game
        bool found = false;
        for ( const ExportPeriod & p : ExportRange::periods( frames, param ) )
        {
            if ( p.key == text )
            {
                NamedRange r;
                r.range.first = p.first;
                r.range.last = p.last;
                r.name = p.key;
                ranges->push_back( r );
                found = true;
                break;
            }
        }
        if ( found )
        {
            continue;
        }
        static const QRegularExpression period( QStringLiteral( "^(first-half|second-half|half-\\d+|extra-\\d+|penalties|game)$" ) );
        if ( period.match( text ).hasMatch() )
        {
            *error = QObject::tr( "The range %1: the game log has no such part." ).arg( text );
            return false;
        }

        // START-END
        const int dash = text.indexOf( QLatin1Char( '-' ) );
        if ( dash < 0 )
        {
            *error = QObject::tr( "The range %1: START-END, all, first-half, second-half, half-N, extra-N, penalties, game or goals[:BEFORE:AFTER] is expected." ).arg( text );
            return false;
        }
        ExportRangeSpec spec;
        spec.start = text.left( dash ).trimmed();
        spec.end = text.mid( dash + 1 ).trimmed();
        spec.name = text;
        QString why;
        NamedRange r;
        r.range = ExportRangeList::resolve( spec, frames, &why );
        if ( ! r.range.isValid() )
        {
            *error = QObject::tr( "The range %1: %2" ).arg( text, why );
            return false;
        }
        r.name = text;
        ranges->push_back( r );
    }

    if ( ranges->empty() )
    {
        *error = ( goals_asked
                   ? QObject::tr( "There is no goal in the game log." )
                   : QObject::tr( "There is no frame to export." ) );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------*/
ExportBatch::ExportBatch( const Settings & settings,
                          std::ostream & log )
    : M_settings( settings ),
      M_log( log )
{

}

/*-------------------------------------------------------------------*/
int
ExportBatch::fail( const QString & message )
{
    M_log << PACKAGE_NAME << ": " << message.toStdString() << std::endl;
    return 1;
}

/*-------------------------------------------------------------------*/
QStringList
ExportBatch::videoFiles( const std::vector< NamedRange > & ranges ) const
{
    QString file = QDir::cleanPath( M_settings.video );
    if ( QFileInfo( file ).suffix().isEmpty() )
    {
        file += QStringLiteral( ".mp4" );
    }
    if ( ranges.size() <= 1
         || M_settings.join )
    {
        return QStringList( file );
    }

    // FILE-NAME.ext: a video for each range, by its name (as the export dialog)
    const QFileInfo info( file );
    const QString base = QDir::cleanPath( QDir( info.path() ).filePath( info.completeBaseName() ) );
    QStringList files;
    for ( const QString & name : fileNames( ranges ) )
    {
        files << base + QLatin1Char( '-' ) + name + QLatin1Char( '.' ) + info.suffix();
    }
    return files;
}

/*-------------------------------------------------------------------*/
int
ExportBatch::run( const QString & game_log )
{
    const Settings & s = M_settings;
    if ( ! s.enabled() )
    {
        return fail( QObject::tr( "Nothing to export: give --export-video FILE or --export-images DIR." ) );
    }
    if ( game_log.isEmpty() )
    {
        return fail( QObject::tr( "No game log: give the game log (.rcg or .rcg.gz) to export." ) );
    }

    // the game log, without a window (no message box)
    if ( ! M_main_data.openRCG( QFile::encodeName( game_log ).toStdString() ) )
    {
        return fail( QObject::tr( "Could not read the game log %1." ).arg( game_log ) );
    }
    const ViewHolder & holder = M_main_data.viewHolder();

    // the ranges, and the frames of their images
    std::vector< NamedRange > ranges;
    QString error;
    if ( ! resolveRanges( s.ranges, holder, &ranges, &error ) )
    {
        return fail( error );
    }
    const double per_image = ExportTiming::framesPerImage( s.speed, ExportGameParam::current().step_ms, s.fps );
    std::vector< std::vector< std::size_t > > frames;
    std::size_t total = 0;
    std::size_t frame_count = 0;
    for ( const NamedRange & r : ranges )
    {
        frames.push_back( ExportTiming::frames( r.range.first, r.range.last, per_image ) );
        total += frames.back().size();
        frame_count += r.range.size();
    }
    if ( total == 0 )
    {
        return fail( QObject::tr( "There is no frame to export." ) );
    }

    // the size: even for a video (yuv420p); the layout as a view of the height / the scale
    const bool video = ! s.video.isEmpty();
    QSize pixels( s.width, s.height );
    if ( video )
    {
        pixels = QSize( std::max( 2, pixels.width() & ~1 ), std::max( 2, pixels.height() & ~1 ) );
    }
    const qreal dpr = ( s.scale > 0.0
                        ? s.scale
                        : std::max( 0.1, pixels.height() / LAYOUT_HEIGHT ) );

    // where they go
    const bool separate = ( ranges.size() > 1 && ! s.join );
    std::vector< ExportPart > parts;
    const auto all_in_one = [&]( FrameSink * sink ) {
        ExportPart part{ sink, {} };
        for ( const std::vector< std::size_t > & f : frames )
        {
            part.frames.insert( part.frames.end(), f.begin(), f.end() );
        }
        return part;
    };
    QString target;
    if ( video )
    {
        const QStringList files = videoFiles( ranges );
        const QString suffix = QFileInfo( files.front() ).suffix().toLower();
        if ( std::find_if( std::begin( VIDEO_EXTENSIONS ), std::end( VIDEO_EXTENSIONS ),
                           [&]( const char * e ) { return suffix == QLatin1String( e ); } ) == std::end( VIDEO_EXTENSIONS ) )
        {
            return fail( QObject::tr( "A video is a .mp4, .mov or .mkv file: not .%1." ).arg( suffix ) );
        }
        const FfmpegSink::Info info = FfmpegSink::probe( s.ffmpeg );
        if ( ! info.found )
        {
            return fail( info.error + QObject::tr( " A video needs ffmpeg: install it, or give --ffmpeg PATH." ) );
        }
        // the version without its copyright, as the export dialog shows it
        QString version = info.version;
        const int copyright = version.indexOf( QLatin1String( " Copyright" ) );
        if ( copyright > 0 ) version.truncate( copyright );
        M_log << version.toStdString() << "; H.264 by " << info.encoder.toStdString() << std::endl;
        // a video of the 2D view: flat colors (-tune animation)
        const auto sink = [&]( const QString & file ) {
            return new FfmpegSink( info.program, file, info.encoder, s.crf, s.preset, true );
        };
        if ( separate )
        {
            for ( std::size_t r = 0; r < ranges.size(); ++r )
            {
                parts.push_back( ExportPart{ sink( files[static_cast< int >( r )] ), frames[r] } );
            }
            target = QObject::tr( "%1 videos (%2, ...)" ).arg( files.size() ).arg( files.front() );
        }
        else
        {
            parts.push_back( all_in_one( sink( files.front() ) ) );
            target = files.front();
        }
    }
    else
    {
        // each range in its own directory by its name, or the numbers going on over them
        QStringList names;
        if ( separate )
        {
            const QStringList dirs = fileNames( ranges );
            for ( std::size_t r = 0; r < ranges.size(); ++r )
            {
                const int digits = ExportTiming::nameDigits( frames[r].size() );
                for ( std::size_t n = 0; n < frames[r].size(); ++n )
                {
                    names << dirs[static_cast< int >( r )] + QLatin1Char( '/' )
                        + ExportTiming::imageName( s.image_prefix, n, digits, s.image_format );
                }
            }
        }
        else
        {
            const int digits = ExportTiming::nameDigits( total );
            for ( std::size_t n = 0; n < total; ++n )
            {
                names << ExportTiming::imageName( s.image_prefix, n, digits, s.image_format );
            }
        }
        parts.push_back( all_in_one( new ImageSequenceSink( s.images, names, s.image_format.toLatin1() ) ) );
        target = s.images;
    }

    M_log << "Exporting " << total << " images of " << frame_count << " frames"
          << ( ranges.size() > 1 ? QString( " in %1 ranges" ).arg( ranges.size() ).toStdString() : std::string() )
          << ", " << pixels.width() << " x " << pixels.height() << " pixels, " << s.fps << " images/s, to "
          << target.toStdString() << std::endl;

    // drawn by a field canvas that is not shown
    FieldCanvas canvas( M_main_data );
    ExportJob job( canvas, parts, pixels, dpr, s.fps );

    // the progress at most every second, and every tenth
    QEventLoop loop;
    bool ok = false;
    QString message;
    QElapsedTimer clock;
    clock.start();
    qint64 told_at = 0;
    std::size_t told_tenth = 0;
    QObject::connect( &job, &ExportJob::progressed,
                      [&]( std::size_t done, std::size_t all ) {
                          const std::size_t tenth = ( all > 0 ? done * 10 / all : 10 );
                          if ( done < all
                               && tenth == told_tenth
                               && clock.elapsed() - told_at < 1000 )
                          {
                              return;
                          }
                          told_tenth = tenth;
                          told_at = clock.elapsed();
                          M_log << ( done < all
                                     ? QString( "Exported %1 of %2 images (%3%)" ).arg( done ).arg( all ).arg( all > 0 ? done * 100 / all : 100 )
                                     : video ? QString( "Exported %1 images; finishing the video" ).arg( all )
                                     : QString( "Exported %1 images" ).arg( all ) ).toStdString() << std::endl;
                      } );
    QObject::connect( &job, &ExportJob::finished,
                      [&]( bool o, const QString & m ) {
                          ok = o;
                          message = m;
                          loop.quit();
                      } );
    job.start();
    if ( job.isRunning() )
    {
        loop.exec();
    }

    if ( ! ok )
    {
        return fail( message );
    }
    for ( const QString & line : message.split( QLatin1Char( '\n' ), Qt::SkipEmptyParts ) )
    {
        M_log << "Exported " << line.toStdString() << std::endl;
    }
    M_log << "Finished in " << clock.elapsed() / 1000.0 << " s" << std::endl;
    return 0;
}
