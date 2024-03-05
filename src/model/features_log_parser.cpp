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
FeaturesLog::Ptr
FeaturesLogParser::parse( std::istream & is ) const
{
    FeaturesLog::Ptr holder( new FeaturesLog() );

    if ( ! parseHeaderLine( is, holder ) )
    {
        return FeaturesLog::Ptr();
    }

    if ( ! parseValueLines( is, holder ) )
    {
        return FeaturesLog::Ptr();
    }

    return holder;
}


/*-------------------------------------------------------------------*/
// bool
// FeaturesLogParser::parseHeaderLine( std::istream & is,
//                                     FeaturesLog::Ptr holder ) const
// {
//     std::string line;
//     while ( std::getline( is, line ) )
//     {
//         if ( line.empty() ) continue;
//         if ( line[0] == '#' ) continue;

//         char task_name[128];
//         int unum = -1;
//         size_t float_count = 0, cat_count = 0;
//         if ( std::sscanf( line.c_str(), " task %127s unum %d float %zd cat %zd ", task_name, &unum,  &float_count, &cat_count ) != 4 )
//         {
//             std::cerr << __FILE__ << ": Illegal header [" << line << "]" << std::endl;
//             return false;
//         }

//         holder->setTaskName( task_name );
//         holder->setUnum( unum );
//         holder->setFeaturesCount( float_count, cat_count );
//         return true;
//     }

//     return false;
// }

/*-------------------------------------------------------------------*/
bool
FeaturesLogParser::parseHeaderLine( std::istream & is,
                                    FeaturesLog::Ptr holder ) const
{
    std::string line;
    while ( std::getline( is, line ) )
    {
        if ( line.empty() ) continue;

        const char * msg = line.c_str();
        while ( *msg == ' ' ) ++msg;

        {
            char time[128], label[128], value[128];
            int n_read = 0;

            // The name of the first 3 columns must be 'Time', 'Label', and 'Value'.
            if ( std::sscanf( msg, " \"%127[^\"]\" \"%127[^\"]\" \"%127[^\"]\" %n ", time, label, value, &n_read ) != 3 )
            {
                std::cerr << __FILE__ << ": (parseHeader) ERROR: could not read the name of the first 3 columns [" << line << "]" << std::endl;
                return false;
            }

            if ( std::strncmp( time, "Time", 4 ) != 0
                 || std::strncmp( label, "Label", 5 ) != 0
                 || std::strncmp( value, "Value", 5 ) != 0 )
            {
                std::cerr << __FILE__ << ": (parseHeader) the first 3 columns must be 'Time', 'Label', and 'Value' [" << line << "]" << std::endl;
                return false;
            }

            msg += n_read;
        }

        std::vector< std::string > names;

        while ( *msg != '\0' )
        {
            int n_read = 0;
            char name[128];
            // read quated string
            if ( std::sscanf( msg, " \"%127[^\"]\" %n ", name, &n_read ) != 1 )
            {
                std::cerr << __FILE__ << ": (parseHeader) Could not read the quoted string [" << msg << "]" << std::endl;
                return false;
            }
            msg += n_read;

            if ( std::strncmp( name, "Draw", 4 ) == 0 )
            {
                continue;
            }

            names.push_back( name );
            while ( *msg == ' ' ) ++msg;
        }

        if ( names.empty() )
        {
            std::cerr << __FILE__ << ": (parseHader) No names [" << msg << "]" << std::endl;
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
                                    FeaturesLog::Ptr holder ) const
{
    std::string line;
    FeaturesGroup::Ptr group;
    int index = 0;

    while ( std::getline( is, line ) )
    {
        if ( line.empty() ) continue;
        if ( line[0] == '#' ) continue;

        Features::Ptr features = parseValueLine( line, holder->floatFeaturesSize(), holder->catFeaturesSize() );
        if ( ! features )
        {
            continue;
        }

        if ( group
             && group->time() != features->time() )
        {
            if ( ! holder->addFeaturesGroup( group->time(), group ) )
            {
                std::cerr << "(FeaturesLogParser::parseValueLines) ERROR: Could not add FeaturesGroup."
                          << line << std::endl;
                return false;
            }
            group.reset();
            index = 0;
        }

        features->setIndex( ++index );
        if ( ! group )
        {
            group = FeaturesGroup::Ptr( new FeaturesGroup() );
            group->setTime( features->time() );
        }

        if ( ! group->featuresList().empty() )
        {
            if ( group->floatFeaturesSize() != features->floatFeatures().size()
                 || group->catFeaturesSize() != features->catFeatures().size() )
            {
                std::cerr << "(FeaturesLogParser::parseValueLines) ERROR: Illegal feature size."
                          << line << std::endl;
                return false;
            }
        }

        group->addFeatures( features );
    }

    if ( group )
    {
        if ( ! holder->addFeaturesGroup( group->time(), group ) )
        {
            std::cerr << "(FeaturesLogParser::parseValueLines) ERROR: Could not add FeaturesGroup."
                      << line << std::endl;
            return false;
        }
        group.reset();
    }

    return true;
}

/*-------------------------------------------------------------------*/
Features::Ptr
FeaturesLogParser::parseValueLine( const std::string & line,
                                   const size_t float_features_size,
                                   const size_t cat_features_size ) const
{
    const char * msg = line.c_str();
    int n_read = 0;

    Features::Ptr features( new Features( float_features_size, cat_features_size ) );

    // time, label value
    {
        int time = -1, stopped = 0;
        int label = 0;
        double value = 0.0;
        if ( std::sscanf( msg, " %d:%d %d %lf %n ", &time, &stopped, &label, &value, &n_read ) != 4 )
        {
            std::cerr << "(FeaturesLogParser::parseValueLine) Could not read the time, label, and value [" << line << "]" << std::endl;
            return Features::Ptr();
        }
        msg += n_read;

        features->setTime( rcsc::GameTime( time, stopped ) );
        features->setEditableLabel( label );
        features->setValue( value );
    }

    while ( *msg == ' ' ) ++msg;

    while ( *msg )
    {
        int n_read = 0;

        if ( *msg == '\"' )
        {
            // categorical feature
            char cat_value[128];
            if ( std::sscanf( msg, " \"%127[^\"]\" %n ", cat_value, &n_read ) == 1 )
            {
                features->addFeature( cat_value );
                msg += n_read;
            }
            else
            {
                std::cerr << "(FeaturesLogParser::parseValueLine) parse error [" << msg << "]" << std::endl;
                return Features::Ptr();
            }
        }
        else if ( *msg == '(' )
        {
            // draw data
            std::shared_ptr< DrawGroup > draw_group( new DrawGroup );
            DrawGroupHandler handler( draw_group );
            DrawDataParser draw_data_parser( handler );
            if ( draw_data_parser.parse( msg ) )
            {
                features->setDrawData( draw_group );
            }
        }
        else
        {
            // float feature
            float float_value = 0.0;
            if ( std::sscanf( msg, " %f %n ", &float_value, &n_read ) == 1 )
            {
                features->addFeature( float_value );
                msg += n_read;
            }
            else
            {
                std::cerr << "(FeaturesLogParser::parseValueLine) parse error [" << msg << "]" << std::endl;
                return Features::Ptr();
            }
        }

        while ( *msg == ' ' ) ++msg;
    }

    return features;
}
