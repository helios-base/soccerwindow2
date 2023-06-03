// -*-c++-*-

/*!
  \file features_logr.cpp
  \brief features log Source File.
*/

/*
 *Copyright:

 Copyright (C) Hidehisa AKIYAMA

 This code is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2, or (at your option)
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

#include "features_log.h"

#include "features_log_parser.h"

#include <iomanip>
#include <iostream>
#include <fstream>
#include <string>

/*-------------------------------------------------------------------*/
std::ostream &
FeaturesLog::printCSV( std::ostream & os ) const
{
    os << ( M_time.cycle() * 100 ) + ( M_time.stopped() % 100 );
    os << ',' << M_rank_label;
    os << ',' << M_score;

    for ( double v : M_float_features )
    {
        os << ',' << v;
    }

    for ( const std::string & v : M_cat_features )
    {
        os << ',' << std::quoted( v );
    }

    os << '\n';
    return os;
}

/*-------------------------------------------------------------------*/
FeaturesLog::ConstPtr
GroupedFeaturesLog::findFeaturesLog( const int index ) const
{
    for ( FeaturesLog::ConstPtr f : M_features_list )
    {
        if ( f->index() == index )
        {
            return f;
        }
    }

    return FeaturesLog::ConstPtr();
}

/*-------------------------------------------------------------------*/
std::ostream &
GroupedFeaturesLog::printCSV( std::ostream & os ) const
{
    for ( const FeaturesLog::Ptr & f : M_features_list )
    {
        f->printCSV( os );
    }

    return os;
}

/*-------------------------------------------------------------------*/
GroupedFeaturesLog::ConstPtr
WholeFeaturesLog::findGroup( const rcsc::GameTime & time ) const
{
    Map::const_iterator it = M_timed_map.find( time );
    if ( it == M_timed_map.end() )
    {
        return GroupedFeaturesLog::ConstPtr();
    }

    return it->second;
}

/*-------------------------------------------------------------------*/
std::ostream &
WholeFeaturesLog::printCSV( std::ostream & os ) const
{
    for ( const Map::value_type & v : M_timed_map )
    {
        v.second->printCSV( os );
    }

    return os;
}

#if 0
/*-------------------------------------------------------------------*/
bool
FeaturesLogHolder::open( const std::string & filepath )
{
    std::ifstream fin( filepath );
    if ( ! fin )
    {
        return false;
    }

    FeaturesLogParser parser;
    WholeFeaturesLog::Ptr log = parser.parse( fin );

    if ( ! log )
    {
        return false;
    }

    if ( log->unum() < 1 || 12 < log->unum() )
    {
        return false;
    }

    M_features_data[ log->unum() - 1 ] = log;

    return true;
}

/*-------------------------------------------------------------------*/
void
FeaturesLogHolder::clear()
{
    for ( int i = 0; i < 12; ++i )
    {
        M_features_data[i].reset();
    }
}

/*-------------------------------------------------------------------*/
GroupedFeaturesLog::ConstPtr
FeaturesLogHolder::getGroupedData( const int unum,
                                   const rcsc::GameTime & time ) const
{
    if ( unum < 1 || 12 < unum )
    {
        return GroupedFeaturesLog::ConstPtr();
    }

    if ( ! M_features_data[unum - 1] )
    {
        return GroupedFeaturesLog::ConstPtr();
    }

    WholeFeaturesLog::Map::const_iterator it = M_features_data[unum - 1]->timedMap().find( time );
    if ( it == M_features_data[unum - 1]->timedMap().end() )
    {
        return GroupedFeaturesLog::ConstPtr();
    }

    return it->second;
}

/*-------------------------------------------------------------------*/
WholeFeaturesLog::ConstPtr
FeaturesLogHolder::getWholeData( const int unum ) const
{
    if ( unum < 1 || 12 < unum )
    {
        return WholeFeaturesLog::ConstPtr();
    }

    return M_features_data[unum - 1];
}
#endif
