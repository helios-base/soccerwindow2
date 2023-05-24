// -*-c++-*-

/*!
  \file features_log_parser.cpp
  \brief parser for features log Source File.
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

#include "features_log_parser.h"

#include <iostream>
#include <string>

/*

  HEADER
  COLUMN_NAMES
  VALUES DESC

  HEADER := TASKNAME <int:NumFloat> <int:NumCat>
  COLUMN_NAMES := "str1" "str2" ... "strN+M"
  VALUES := TIME label float1 float2 ... floatN str1 str2 ... strM
  TIME := int-int (GameTime)
  DESC := "description"
 */

/*-------------------------------------------------------------------*/
std::shared_ptr< WholeFeaturesLog >
FeaturesLogParser::parse( std::istream & is ) const
{
    std::shared_ptr< WholeFeaturesLog > holder( new WholeFeaturesLog() );

    if ( ! parseHeaderLine( is, holder ) )
    {
        return std::shared_ptr< WholeFeaturesLog >();
    }

    if ( ! parseColumnNamesLine( is, holder ) )
    {
        return std::shared_ptr< WholeFeaturesLog >();
    }

    if ( ! parseValueLines( is, holder ) )
    {
        return std::shared_ptr< WholeFeaturesLog >();
    }

    return holder;
}


/*-------------------------------------------------------------------*/
bool
FeaturesLogParser::parseHeaderLine( std::istream & is,
                                    std::shared_ptr< WholeFeaturesLog > holder ) const
{
    std::string line;
    while ( std::getline( is, line ) )
    {
        if ( line.empty() ) continue;
        if ( line[0] == '#' ) continue;

        char task_name[128];
        size_t float_count = 0, cat_count = 0;
        if ( std::sscanf( line.c_str(), " %127s %zd %zd ", task_name, &float_count, &cat_count ) != 3 )
        {
            std::cerr << __FILE__ << ": Illegal header [" << line << "]" << std::endl;
            return false;
        }

        holder->setTaskName( task_name );
        holder->setFeaturesCount( float_count, cat_count );
        return true;
    }

    return false;
}

/*-------------------------------------------------------------------*/
bool
FeaturesLogParser::parseColumnNamesLine( std::istream & is,
                                         std::shared_ptr< WholeFeaturesLog > holder ) const
{
    std::string line;
    while ( std::getline( is, line ) )
    {
        if ( line.empty() ) continue;
        if ( line[0] == '#' ) continue;

        std::vector< std::string > header;

        const char * msg = line.c_str();
        while ( *msg == ' ' ) ++msg;

        while ( msg )
        {
            int n_read = 0;
            char name[128];
            // read quated string
            if ( std::sscanf( msg, " \"%127[^\"]\" %n ", name, &n_read ) != 1 )
            {
                std::cerr << __FILE__ << ": (parseHeader) Could not read the quated string [" << line << "]" << std::endl;
                return false;
            }
            msg += n_read;

            header.push_back( name );
            while ( *msg == ' ' ) ++msg;
        }

        if ( header.empty() )
        {
            return false;
        }

        holder->setHeader( header );
        return true;
    }

    return false;
}

/*-------------------------------------------------------------------*/
bool
FeaturesLogParser::parseValueLines( std::istream & is,
                                    std::shared_ptr< WholeFeaturesLog > holder ) const
{
    std::string line;
    GroupedFeaturesLog::Ptr group;

    while ( std::getline( is, line ) )
    {
        if ( line.empty() ) continue;
        if ( line[0] == '#' ) continue;

        FeaturesLog::Ptr features_log = parseValueLine( line, holder->floatFeaturesSize(), holder->catFeaturesSize() );
        if ( ! features_log )
        {
            continue;
        }

        if ( group
             && group->time() != features_log->time() )
        {
            holder->addGroupedFeaturesLog( group );
            group.reset();
        }

        if ( ! group )
        {
            group = GroupedFeaturesLog::Ptr( new GroupedFeaturesLog() );
        }

        group->addFeaturesLog( features_log );
    }

    if ( group )
    {
        holder->addGroupedFeaturesLog( group );
        group.reset();
    }

    return true;
}

/*-------------------------------------------------------------------*/
FeaturesLog::Ptr
FeaturesLogParser::parseValueLine( const std::string & line,
                                   const size_t float_features_size,
                                   const size_t cat_features_size ) const
{
    const char * msg = line.c_str();
    int n_read = 0;

    FeaturesLog::Ptr features_log( new FeaturesLog( float_features_size, cat_features_size ) );

    // group id, label value
    {
        int time = -1, stopped = 0;
        double label = 0.0;
        if ( std::sscanf( msg, " %d-%d %lf %n ", &time, &stopped, &label, &n_read ) != 2 )
        {
            return FeaturesLog::Ptr();
        }
        msg += n_read;

        features_log->setTime( rcsc::GameTime( time, stopped ) );
        features_log->setLabel( label );
    }

    // read float values
    for ( size_t i = 0; i < float_features_size; ++i )
    {
        double value = 0.0;
        if ( std::sscanf( msg, " %lf %n ", &value, &n_read ) != 1 )
        {
            return FeaturesLog::Ptr();
        }
        msg += n_read;

        features_log->addFeature( value );
    }

    // read categorical values
    for ( size_t i = 0; i < cat_features_size; ++i )
    {
        char value[128];
        if ( std::sscanf( msg, " \"%[^\"]\" %n ", value, &n_read ) != 1 )
        {
            return FeaturesLog::Ptr();
        }
        msg += n_read;

        features_log->addFeature( value );
    }

    // read description
    while ( *msg == ' ' ) ++msg;
    features_log->setDescription( msg );

    return features_log;
}
