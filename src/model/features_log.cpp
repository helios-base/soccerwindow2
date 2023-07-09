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
Features::printCSV( std::ostream & os ) const
{
    os << ( M_time.cycle() * 100 ) + ( M_time.stopped() % 100 );
    os << ',' << M_editable_label;
    os << ',' << M_value;

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
std::ostream &
Features::printLog( std::ostream & os ) const
{
    os << time().cycle() << ','  << time().stopped();
    os << ' ' << editableLabel();
    os << ' ' << value();

    for ( double v : floatFeatures() )
    {
        os << ' ' << v;
    }

    for ( const std::string & v : catFeatures() )
    {
        os << ' ' << std::quoted( v );
    }

    if ( drawData() )
    {
        for ( const DrawText & t : drawData()->texts_ )
        {
            os << " (t " << t.x_ << ' ' << t.y_ << ' ' << std::quoted( t.msg_ ) << ' ' << std::quoted( t.color_ ) << ')';
        }

        for ( const DrawPoint & t : drawData()->points_ )
        {
            os << " (p " << t.x_ << ' ' << t.y_ << ' ' << std::quoted( t.color_ ) << ')';
        }

        for ( const DrawLine & t : drawData()->lines_ )
        {
            os << " (l " << t.x1_ << ' ' << t.y1_ << ' ' << t.x2_ << ' ' << t.y2_ << ' ' << std::quoted( t.color_ ) << ')';
        }

        for ( const DrawRect & t : drawData()->rects_ )
        {
            if ( t.fill_color_.empty() )
            {
                os << " (r " << t.left_ << ' ' << t.top_ << ' ' << t.width_ << ' ' << t.height_ << ' ' << std::quoted( t.line_color_ ) << ')';
            }
            else
            {
                os << " (R " << t.left_ << ' ' << t.top_ << ' ' << t.width_ << ' ' << t.height_ << ' ' << std::quoted( t.line_color_ ) << ' ' << std::quoted( t.fill_color_ ) << ')';
            }
        }

        for ( const DrawCircle & t : drawData()->circles_ )
        {
            if ( t.fill_color_.empty() )
            {
                os << " (c " << t.x_ << ' ' << t.y_ << ' ' << t.r_ << ' ' << std::quoted( t.line_color_ ) << ')';
            }
            else
            {
                os << " (C " << t.x_ << ' ' << t.y_ << ' ' << t.r_ << ' ' << std::quoted( t.line_color_ ) << ' ' << std::quoted( t.fill_color_ ) << ')';
            }
        }
    }

    os << '\n';
    return os;
}

/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/

/*-------------------------------------------------------------------*/
bool
FeaturesGroup::updateLabelValue( const int index,
                                 const int new_value )
{
    for ( const Features::Ptr & f : M_features_list )
    {
        if ( f->index() == index )
        {
            f->setEditableLabel( new_value );
            return true;
        }
    }

    return false;
}


/*-------------------------------------------------------------------*/
Features::ConstPtr
FeaturesGroup::findFeaturesLog( const int index ) const
{
    for ( const Features::Ptr & f : M_features_list )
    {
        if ( f->index() == index )
        {
            return f;
        }
    }

    return Features::ConstPtr();
}

/*-------------------------------------------------------------------*/
std::ostream &
FeaturesGroup::printCSV( std::ostream & os ) const
{
    for ( const Features::Ptr & f : M_features_list )
    {
        f->printCSV( os );
    }

    return os;
}

/*-------------------------------------------------------------------*/
std::ostream &
FeaturesGroup::printLog( std::ostream & os ) const
{
    for ( const Features::Ptr & f : M_features_list )
    {
        f->printLog( os );
    }

    return os;
}

/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/

/*-------------------------------------------------------------------*/
bool
FeaturesLog::updateLabelValue( const rcsc::GameTime & time,
                               const int index,
                               const int new_value )
{
    Map::iterator it = M_timed_map.find( time );
    if ( it == M_timed_map.end() )
    {
        return false;
    }

    return it->second->updateLabelValue( index, new_value );
}

/*-------------------------------------------------------------------*/
FeaturesGroup::ConstPtr
FeaturesLog::findGroup( const rcsc::GameTime & time ) const
{
    Map::const_iterator it = M_timed_map.find( time );
    if ( it == M_timed_map.end() )
    {
        return FeaturesGroup::ConstPtr();
    }

    return it->second;
}

/*-------------------------------------------------------------------*/
std::ostream &
FeaturesLog::printCSV( std::ostream & os ) const
{
    for ( const Map::value_type & v : M_timed_map )
    {
        v.second->printCSV( os );
    }

    return os;
}

/*-------------------------------------------------------------------*/
std::ostream &
FeaturesLog::printLog( std::ostream & os ) const
{
    // print header
    {
        os << "task " << taskName();
        os << " unum " << unum();
        os << " float " << floatFeaturesSize();
        os << " cat " << catFeaturesSize();
        os << '\n';
    }

    // print feature names
    {
        os << "names";
        for ( const std::string & s : featureNames() )
        {
            os << ' ' << '"' << s << '"';
        }
        os << '\n';
    }

    // print value lines
    for ( const Map::value_type & v : M_timed_map )
    {
        v.second->printLog( os );
    }

    return os;
}
