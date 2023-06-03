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

#include "draw_data_handler.h"
#include "draw_data_parser.h"

#include <iostream>
#include <string>
#include <cstring>

class DrawGroupHandler
    : public DrawDataHandler {
private:

    DrawGroupHandler()
      { }
public:

    std::shared_ptr< DrawGroup > group_;

    explicit
    DrawGroupHandler( std::shared_ptr< DrawGroup > group )
        : group_( group )
      { }

    bool handleText( const DrawText & text )
      {
          group_->texts_.push_back( text );
          return true;
      }

    bool handlePoint( const DrawPoint & point )
      {
          group_->points_.push_back( point );
          return true;
      }

    bool handleLine( const DrawLine & line )
      {
          group_->lines_.push_back( line );
          return true;
      }

    bool handleRect( const DrawRect & rect )
      {
          group_->rects_.push_back( rect );
          return true;
      }

    bool handleCircle( const DrawCircle & circle )
      {
          group_->circles_.push_back( circle );
          return true;
      }

};


/*

  HEADER
  COLUMN_NAMES
  VALUES DRAW

  HEADER := task TASKNAME unum UNUM float <int:NumFloat> cat <int:NumCat>
  FEATURE_NAMES := names "str1" "str2" ... "strN+M"
  VALUES := TIME label float1 float2 ... floatN str1 str2 ... strM
  TIME := int-int (GameTime)
  DRAW = draw group
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

    if ( ! parseFeatureNamesLine( is, holder ) )
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
        int unum = -1;
        size_t float_count = 0, cat_count = 0;
        if ( std::sscanf( line.c_str(), " task %127s unum %d float %zd cat %zd ", task_name, &unum,  &float_count, &cat_count ) != 4 )
        {
            std::cerr << __FILE__ << ": Illegal header [" << line << "]" << std::endl;
            return false;
        }

        holder->setTaskName( task_name );
        holder->setUnum( unum );
        holder->setFeaturesCount( float_count, cat_count );
        return true;
    }

    return false;
}

/*-------------------------------------------------------------------*/
bool
FeaturesLogParser::parseFeatureNamesLine( std::istream & is,
                                          std::shared_ptr< WholeFeaturesLog > holder ) const
{
    std::string line;
    while ( std::getline( is, line ) )
    {
        if ( line.empty() ) continue;
        if ( line[0] == '#' ) continue;

        std::vector< std::string > names;

        const char * msg = line.c_str();
        while ( *msg == ' ' ) ++msg;

        if ( std::strncmp( msg, "names", 5 ) != 0 )
        {
            std::cerr << __FILE__ << ": (parseColumnNames) no 'names' block [" << line << "]" << std::endl;
            return false;
        }
        msg += 5;

        while ( *msg != '\0' )
        {
            int n_read = 0;
            char name[128];
            // read quated string
            if ( std::sscanf( msg, " \"%127[^\"]\" %n ", name, &n_read ) != 1 )
            {
                std::cerr << __FILE__ << ": (parseColumnNames) Could not read the quoted string [" << msg << "]" << std::endl;
                return false;
            }
            msg += n_read;

            names.push_back( name );
            while ( *msg == ' ' ) ++msg;
        }

        if ( names.empty() )
        {
            return false;
        }

        holder->setFeatureNames( names );
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
    int index = 0;

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
            holder->addGroupedFeaturesLog( group->time(), group );
            group.reset();
            index = 0;
        }

        features_log->setIndex( ++index );
        if ( ! group )
        {
            group = GroupedFeaturesLog::Ptr( new GroupedFeaturesLog() );
            group->setTime( features_log->time() );
        }

        group->addFeaturesLog( features_log );
    }

    if ( group )
    {
        holder->addGroupedFeaturesLog( group->time(), group );
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

    // time, label value
    {
        int time = -1, stopped = 0;
        double score = 0.0;
        if ( std::sscanf( msg, " %d,%d %lf %n ", &time, &stopped, &score, &n_read ) != 3 )
        {
            std::cerr << "(FeaturesLogParser::parseValueLine) Could not read the time and label [" << line << "]" << std::endl;
            return FeaturesLog::Ptr();
        }
        msg += n_read;

        features_log->setTime( rcsc::GameTime( time, stopped ) );
        features_log->setScore( score );
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

    while ( *msg == ' ' ) ++msg;

    // read draw data
    if ( *msg == '(' )
    {
        std::shared_ptr< DrawGroup > draw_group( new DrawGroup );
        DrawGroupHandler handler( draw_group );
        DrawDataParser draw_data_parser( handler );
        if ( draw_data_parser.parse( msg ) )
        {
            features_log->setDrawData( draw_group );
        }
    }

    return features_log;
}
