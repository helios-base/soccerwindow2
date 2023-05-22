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

  (TASKNAME) VALUES DESC

  VALUES := (h N M name1,name2,...)
            | (f N M X Y label float1 float2 ... floatN str1 str2 ... strM)

  DESC := (d "description")
 */

/*-------------------------------------------------------------------*/
std::shared_ptr< FeaturesLogHolder >
FeaturesLogParser::parse( std::istream & is ) const
{
    std::string line;

    std::shared_ptr< FeaturesLogHolder > holder( new FeaturesLogHolder() );

    while ( std::getline( is, line ) )
    {
        if ( line.empty() ) continue;

        const char * msg = line.c_str();

        int n_read = 0;
        char task_name[128];

        if ( std::sscanf( msg, " ( %127[^) ] ) %n ", task_name, &n_read ) != 1 )
        {
            std::cerr << __FILE__ << ": No task name [" << line << "]" << std::endl;
            continue;
        }
        msg += n_read;

        char type;
        size_t float_count = 0, cat_count = 0;
        if ( std::sscanf( msg, " ( %c %zd %zd %n ", &type, &float_count, &cat_count, &n_read ) != 2 )
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

        if ( type == 'f' )
        {
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

            if ( *msg != ')' )
            {
                std::cerr << __FILE__ << ": Could not find the end paren [" << line << "]" << std::endl;
                continue;
            }
            ++msg;

            char desc;
            if ( std::sscanf( msg, " ( %c %n ", &desc, &n_read ) == 1 )
            {
                msg += n_read;

                if ( desc == 'd' )
                {
                    char buf[512];
                    if ( std::sscanf( msg, " \"%511[^\"]\" %n ", buf, &n_read ) != 1 )
                    {
                        std::cerr << __FILE__ << ": Could not complete the description [" << line << "]" << std::endl;
                    }
                    msg += n_read;

                    if ( *msg != ')' )
                    {
                        std::cerr << __FILE__ << ": Could not find the end paren [" << line << "]" << std::endl;
                    }
                    ++msg;

                    features_log->setDescription( buf );
                }
                else
                {
                    std::cerr << __FILE__ << ": Illegal description [" << line << "]" << std::endl;
                }
            }

            holder->addFeaturesLog( task_name, features_log );
        }
        else if ( type == 'h' )
        {
            const size_t count = float_count + cat_count;
            std::vector< std::string > header;
            header.reserve( count );

            for ( size_t i = 0; i < count; ++i )
            {
                char name[128];
                if ( std::sscanf( msg, " %127[^)] %n ", name, &n_read ) != 1 )
                {
                    break;
                }
                msg += n_read;
                header.push_back( name );
            }

            if ( header.size() != count )
            {
                std::cerr << __FILE__ << ": Could not complete the header [" << line << "]" << std::endl;
                continue;
            }

            if ( *msg != ')' )
            {
                std::cerr << __FILE__ << ": Could not find the end paren [" << line << "]" << std::endl;
                continue;
            }
            ++msg;

            holder->setHeader( task_name, header );
        }
        else
        {
            std::cerr << __FILE__ << ": Unknown type [" << line << "]" << std::endl;
        }
    }

    return holder;
}
