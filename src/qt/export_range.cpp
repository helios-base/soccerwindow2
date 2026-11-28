// -*-c++-*-

/*!
  \file export_range.cpp
  \brief the ranges of the frames to export Source File.
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

#include "export_range.h"

#include <QCoreApplication>
#include <QRegularExpression>

#include <rcsc/common/server_param.h>

#include <algorithm>
#include <cmath>

namespace {

//! the order of the frames: the cycle, then the stopped frame
struct Key {
    long long cycle;
    long long stopped;
};

bool
before( const MonitorViewData::ConstPtr & view,
        const Key & key )
{
    const rcsc::GameTime & t = view->time();
    return t.cycle() < key.cycle
        || ( t.cycle() == key.cycle && t.stopped() < key.stopped );
}

bool
after( const Key & key,
       const MonitorViewData::ConstPtr & view )
{
    const rcsc::GameTime & t = view->time();
    return key.cycle < t.cycle()
        || ( key.cycle == t.cycle() && key.stopped < t.stopped() );
}

bool
at( const MonitorViewData::ConstPtr & view,
    const ExportPosition & pos )
{
    const rcsc::GameTime & t = view->time();
    return t.cycle() == pos.cycle
        && ( pos.stopped < 0 || t.stopped() == pos.stopped );
}

}

/*-------------------------------------------------------------------*/
bool
ExportRange::parsePosition( const QString & text,
                            ExportPosition * pos,
                            QString * error )
{
    // the stopped frame after "." (the status bar of rcssmonitor) or ","
    // (the score board of soccerwindow2. S9)
    static const QRegularExpression re( QStringLiteral( "^([0-9]{1,9})(?:[.,]([0-9]{1,9}))?$" ) );
    const QRegularExpressionMatch m = re.match( text.trimmed() );
    if ( ! m.hasMatch() )
    {
        if ( error ) *error = QCoreApplication::translate( "ExportRange", "Not a position: \"%1\" (a cycle, e.g. 1234, or a cycle and a stopped frame, e.g. 1234.3)" ).arg( text.trimmed() );
        return false;
    }
    if ( pos )
    {
        pos->cycle = m.captured( 1 ).toInt();
        pos->stopped = ( m.capturedLength( 2 ) > 0 ? m.captured( 2 ).toInt() : -1 );
    }
    if ( error ) error->clear();
    return true;
}

/*-------------------------------------------------------------------*/
QString
ExportRange::positionText( const rcsc::GameTime & time )
{
    return time.stopped() > 0
        ? QString( "%1.%2" ).arg( time.cycle() ).arg( time.stopped() )
        : QString::number( time.cycle() );
}

/*-------------------------------------------------------------------*/
QString
ExportRange::positionText( const MonitorViewData::Cont & frames,
                           const std::size_t index )
{
    return index < frames.size()
        ? positionText( frames[index]->time() )
        : QString();
}

/*-------------------------------------------------------------------*/
QString
ExportRange::endText( const MonitorViewData::Cont & frames,
                      const std::size_t index )
{
    if ( index >= frames.size() )
    {
        return QString();
    }
    const rcsc::GameTime & time = frames[index]->time();
    if ( time.stopped() == 0
         && index + 1 < frames.size()
         && frames[index + 1]->time().cycle() == time.cycle() )
    {
        return QString( "%1.0" ).arg( time.cycle() );
    }
    return positionText( time );
}

/*-------------------------------------------------------------------*/
std::size_t
ExportRange::startIndex( const MonitorViewData::Cont & frames,
                         const ExportPosition & pos,
                         bool * snapped )
{
    if ( snapped ) *snapped = false;
    if ( ! pos.isValid() )
    {
        return INVALID_INDEX;
    }

    // the first frame at or after the position
    const Key key{ pos.cycle, std::max( 0, pos.stopped ) };
    const MonitorViewData::Cont::const_iterator it = std::lower_bound( frames.begin(), frames.end(), key, before );
    if ( it == frames.end() )
    {
        return INVALID_INDEX;
    }
    if ( snapped ) *snapped = ! at( *it, pos );
    return std::distance( frames.begin(), it );
}

/*-------------------------------------------------------------------*/
std::size_t
ExportRange::endIndex( const MonitorViewData::Cont & frames,
                       const ExportPosition & pos,
                       bool * snapped )
{
    if ( snapped ) *snapped = false;
    if ( ! pos.isValid() )
    {
        return INVALID_INDEX;
    }

    // the last frame at or before the position: all the frames stopped in
    // the cycle for a cycle
    const Key key{ pos.cycle,
                   pos.stopped < 0
                   ? std::numeric_limits< long long >::max()
                   : pos.stopped };
    MonitorViewData::Cont::const_iterator it = std::upper_bound( frames.begin(), frames.end(), key, after );
    if ( it == frames.begin() )
    {
        return INVALID_INDEX;
    }
    --it;
    if ( snapped ) *snapped = ! at( *it, pos );
    return std::distance( frames.begin(), it );
}

/*-------------------------------------------------------------------*/
QString
ExportRange::gameTime( const int cycle,
                       const int step_ms )
{
    const long long seconds = static_cast< long long >( std::max( 0, cycle ) ) * std::max( 1, step_ms ) / 1000;
    return QString( "%1:%2" ).arg( seconds / 60 ).arg( seconds % 60, 2, 10, QLatin1Char( '0' ) );
}

/*-------------------------------------------------------------------*/
ExportGameParam
ExportGameParam::current()
{
    const rcsc::ServerParam & sp = rcsc::ServerParam::i();
    ExportGameParam param;
    param.half_time = sp.halfTime();
    param.nr_normal_halfs = sp.nrNormalHalfs();
    param.extra_half_time = sp.extraHalfTime();
    param.nr_extra_halfs = sp.nrExtraHalfs();
    param.step_ms = std::max( 1, sp.simulatorStep() );
    return param;
}

/*-------------------------------------------------------------------*/
int
ExportRange::cyclesOf( const double seconds,
                       const int step_ms )
{
    return static_cast< int >( std::lround( seconds * 1000.0 / std::max( 1, step_ms ) ) );
}

/*-------------------------------------------------------------------*/
std::vector< ExportPeriod >
ExportRange::periods( const MonitorViewData::Cont & frames,
                      const ExportGameParam & param )
{
    std::vector< ExportPeriod > list;
    if ( frames.empty() )
    {
        return list;
    }

    // the penalty shootout: from its first play mode to the end
    std::size_t penalties = frames.size();
    for ( std::size_t i = 0; i < frames.size(); ++i )
    {
        const rcsc::PlayMode pm = frames[i]->playmode();
        if ( rcsc::PM_PenaltySetup_Left <= pm && pm <= rcsc::PM_PenaltyScore_Right )
        {
            penalties = i;
            break;
        }
    }

    // the ends of the halves (cycles), with their keys and labels
    struct End {
        int cycle;
        QString key;
        QString label;
    };
    std::vector< End > ends;
    const int step = std::max( 1, param.step_ms );
    const int half = cyclesOf( param.half_time, step );
    if ( half > 0 )
    {
        const int normal = std::max( 1, param.nr_normal_halfs );
        for ( int k = 1; k <= normal; ++k )
        {
            ends.push_back( { half * k,
                              k == 1 ? QStringLiteral( "first-half" ) : k == 2 ? QStringLiteral( "second-half" ) : QString( "half-%1" ).arg( k ),
                              k == 1 ? QCoreApplication::translate( "ExportRange", "First half" )
                              : k == 2 ? QCoreApplication::translate( "ExportRange", "Second half" )
                              : QCoreApplication::translate( "ExportRange", "Half %1" ).arg( k ) } );
        }
        const int extra = cyclesOf( param.extra_half_time, step );
        for ( int j = 1; extra > 0 && j <= param.nr_extra_halfs; ++j )
        {
            ends.push_back( { half * normal + extra * j,
                              QString( "extra-%1" ).arg( j ),
                              QCoreApplication::translate( "ExportRange", "Extra time %1" ).arg( j ) } );
        }
    }

    // the frames before the shootout, by the end of each part (the frames
    // after the last end in the last part)
    std::size_t first = 0;
    std::size_t part = 0;
    for ( std::size_t i = 0; i <= penalties && first < penalties; ++i )
    {
        std::size_t p = ends.size();
        if ( i < penalties )
        {
            const int cycle = static_cast< int >( frames[i]->time().cycle() );
            p = 0;
            while ( p + 1 < ends.size() && cycle > ends[p].cycle ) ++p;
        }
        if ( i == penalties || p != part )
        {
            if ( i > first )
            {
                ExportPeriod period;
                period.first = first;
                period.last = i - 1;
                period.key = ( ends.empty() ? QStringLiteral( "game" ) : ends[part].key );
                period.label = ( ends.empty() ? QCoreApplication::translate( "ExportRange", "The game" ) : ends[part].label );
                list.push_back( period );
            }
            first = i;
            part = p;
        }
    }

    if ( penalties < frames.size() )
    {
        ExportPeriod period;
        period.first = penalties;
        period.last = frames.size() - 1;
        period.key = QStringLiteral( "penalties" );
        period.label = QCoreApplication::translate( "ExportRange", "Penalty shootout" );
        list.push_back( period );
    }
    return list;
}

/*-------------------------------------------------------------------*/
std::vector< ExportGoal >
ExportRange::goals( const MonitorViewData::Cont & frames,
                    const std::vector< std::size_t > & score_changed )
{
    std::vector< ExportGoal > list;
    // the score before each change: from the change before it (not from
    // the frame at the index, which may have the new score in some logs)
    int left = 0;
    int right = 0;
    for ( const std::size_t index : score_changed )
    {
        if ( index >= frames.size() )
        {
            continue;
        }
        const MonitorViewData::ConstPtr & after = frames[std::min( index + 1, frames.size() - 1 )];
        const int new_left = after->leftTeam().score_;
        const int new_right = after->rightTeam().score_;
        for ( int side = 0; side < 2; ++side )
        {
            if ( ( side == 0 ? new_left > left : new_right > right ) )
            {
                ExportGoal goal;
                goal.index = index;
                goal.frame = std::min( index + 1, frames.size() - 1 );
                goal.cycle = static_cast< int >( after->time().cycle() );
                goal.side = side;
                goal.left = new_left;
                goal.right = new_right;
                goal.team = QString::fromStdString( side == 0 ? after->leftTeam().name_ : after->rightTeam().name_ );
                list.push_back( goal );
            }
        }
        left = new_left;
        right = new_right;
    }
    return list;
}

/*-------------------------------------------------------------------*/
ExportRange
ExportRange::around( const MonitorViewData::Cont & frames,
                     const std::size_t center,
                     const double before_s,
                     const double after_s,
                     const int step_ms )
{
    if ( center >= frames.size() )
    {
        return ExportRange();
    }

    // the seconds of the playback (D12): a frame a step, the stopped ones
    // too, as in the exported images (ExportTiming: a frame an image at 1x)
    const std::size_t before = static_cast< std::size_t >( std::max( 0, cyclesOf( before_s, step_ms ) ) );
    const std::size_t after = static_cast< std::size_t >( std::max( 0, cyclesOf( after_s, step_ms ) ) );

    ExportRange r;
    r.first = ( center > before ? center - before : 0 );
    r.last = std::min( frames.size() - 1, center + after );
    return r;
}
