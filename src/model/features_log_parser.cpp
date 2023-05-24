// -*-c++-*-

/*!
  \file features_log_parser.cpp
  \brief parser for features log Source File.
*/

/*
 *Copyright:

 Copyright (C) Hiroki SHIMORA, Hidehisa AKIYAMA

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
  COLUMN_NAMES := "str1" "str2" ... "strN"
  VALUES := GROUPID label float1 float2 ... floatN str1 str2 ... strM
  GROUPID := unquoted string (GameTime string?)
  DESC := "description"
 */

/*-------------------------------------------------------------------*/
std::shared_ptr< FeaturesLogHolder >
FeaturesLogParser::parse( std::istream & is ) const
{
    std::shared_ptr< FeaturesLogHolder > holder( new FeaturesLogHolder() );

    if ( ! parseHeaderLine( is, holder ) )
    {
        return std::shared_ptr< FeaturesLogHolder >();
    }

    if ( ! parseColumnNamesLine( is, holder ) )
    {
        return std::shared_ptr< FeaturesLogHolder >();
    }


#if 0
    std::string line;
    while ( std::getline( is, line ) )
    {
        if ( line.empty() ) continue;

        const char * msg = line.c_str();

        int n_read = 0;
        char type;

        if ( std::sscanf( msg, " %c %n ", &type, &n_read ) != 2 )
        {
            std::cerr << __FILE__ << ": No type id [" << line << "]" << std::endl;
            continue;
        }
        msg += n_read;

        if ( type == 'v' )
        {
            FeaturesLog::Ptr features_log = parseValueLine( msg );
            if ( ! features_log )
            {
                continue;
            }

            size_t float_count = 0, cat_count = 0;
            if ( std::sscanf( msg, " %zd %zd %n ", &float_count, &cat_count, &n_read ) != 2 )
            {
                std::cerr << __FILE__ << ": Illegal values header [" << line << "]" << std::endl;
                continue;
            }
            msg += n_read;

            if ( float_count == 0
                 && cat_count == 0 )
            {
                std::cerr << __FILE__ << ": Illegal number of features [" << line << "]" << std::endl;
                continue;
            }

            FeaturesLog::Ptr features_log( new FeaturesLog( float_count, cat_count ) );

            {
                double pos_x, pos_y;
                double value = 0.0;
                if ( std::sscanf( msg, " %lf %lf %lf %n ", &pos_x, & pos_y, &value, &n_read ) != 3 )
                {
                    continue;
                }
                msg += n_read;
                features_log->setPos( pos_x, pos_y );
                features_log->setValue( value );
            }

            for ( size_t i = 0; i < float_count; ++i )
            {
                double float_value;
                if ( std::sscanf( msg, " %lf %n ", &float_value, &n_read ) != 1 )
                {
                    break;
                }
                msg += n_read;
                features_log->addFeature( float_value );
            }

            if ( features_log->floatFeatures().size() != float_count )
            {
                std::cerr << __FILE__ << ": Could not complete the float features [" << line << "]" << std::endl;
                continue;
            }

            for ( size_t i = 0; i < cat_count; ++i )
            {
                char cat_value[128];
                if ( std::sscanf( msg, " %127s %n ", cat_value, &n_read ) != 1 )
                {
                    break;
                }
                msg += n_read;
                features_log->addFeature( cat_value );
            }

            if ( features_log->catFeatures().size() != cat_count )
            {
                std::cerr << __FILE__ << ": Could not complete the cat features [" << line << "]" << std::endl;
                continue;
            }

            {
                char description[512];
                if ( std::sscanf( msg, " \"%511[^\"]\" %n ", description, &n_read ) == 1 )
                {
                    msg += n_read;
                    features_log->setDescription( description );
                }
            }

            holder->addFeaturesLog( task_name, features_log );
        }
    }
#endif
    return holder;
}


/*-------------------------------------------------------------------*/
bool
FeaturesLogParser::parseHeaderLine( std::istream & is,
                                    std::shared_ptr< FeaturesLogHolder > holder ) const
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
                                         std::shared_ptr< FeaturesLogHolder > holder ) const
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
                                    std::shared_ptr< FeaturesLogHolder > holder ) const
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
             && group->groupId() != features_log->groupId() )
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
        int group_id = -1;
        double label = 0.0;
        if ( std::sscanf( msg, " %d %lf %n ", &group_id, &label, &n_read ) != 2 )
        {
            return FeaturesLog::Ptr();
        }
        msg += n_read;

        features_log->setGroupId( group_id );
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
