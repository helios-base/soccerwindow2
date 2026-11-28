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

#include <algorithm>

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
