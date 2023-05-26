// -*-c++-*-

/*!
  \file draw_data_holder.cpp
  \brief draw data holder class Source File.
*/

/*
 *Copyright:

 Copyright (C) Hidehisa Akiyama

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

#include "draw_data_holder.h"

#include "draw_data_handler.h"
#include "draw_data_parser.h"

#include <iostream>
#include <fstream>

using namespace rcsc;

class MyHandler
    : public DrawDataHandler {
public:

    DrawDataHolder & holder_;
    GameTime time_;

    MyHandler( DrawDataHolder & holder )
        : holder_( holder ),
          time_( -1, 0 )
      { }


    bool handleText( const DrawText & text )
      {
          holder_.addText( time_, text );
          return true;
      }

    bool handlePoint( const DrawPoint & point )
      {
          holder_.addPoint( time_, point );
          return true;
      }

    bool handleLine( const DrawLine & line )
      {
          holder_.addLine( time_, line );
          return true;
      }

    bool handleRect( const DrawRect & rect )
      {
          holder_.addRect( time_, rect );
          return true;
      }

    bool handleCircle( const DrawCircle & circle )
      {
          holder_.addCircle( time_, circle );
          return true;
      }
};


/*-------------------------------------------------------------------*/
/*!

*/
DrawDataHolder::DrawDataHolder()
{

}

/*-------------------------------------------------------------------*/
/*!

*/
DrawDataHolder::~DrawDataHolder()
{

}

/*-------------------------------------------------------------------*/
/*!

*/
void
DrawDataHolder::clear()
{
    M_data_map.clear();
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
DrawDataHolder::open( const std::string & filepath )
{
    std::ifstream fin( filepath.c_str() );

    if ( ! fin )
    {
        std::cerr << "(DrawDataHolder::open) could not open the file [" << filepath << "]" << std::endl;
        return false;
    }

    clear();

    MyHandler handler( *this );
    DrawDataParser parser( handler );

    std::string line_buf;
    int n_line = 0;
    while ( std::getline( fin, line_buf ) )
    {
        ++n_line;

        if ( line_buf.empty()
             || line_buf[0] == '#' )
        {
            continue;
        }

        const char * buf = line_buf.c_str();
        int n_read = 0;
        int cycle, stopped;
        if ( std::sscanf( buf, " %d,%d %n ", &cycle, &stopped, &n_read ) != 2 )
        {
            handler.time_.assign( cycle, stopped );
        }
        buf += n_read;

        if ( ! parser.parse( buf ) )
        {
            std::cerr << "(DrawDataHolder::open) Error at line " << n_line << std::endl;
            continue;
        }
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
DrawDataHolder::addText( const rcsc::GameTime & time,
                         const DrawText & text )
{
    M_data_map[time].texts_.push_back( text );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
DrawDataHolder::addPoint( const rcsc::GameTime & time,
                          const DrawPoint & point )
{
    M_data_map[time].points_.push_back( point );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
DrawDataHolder::addLine( const rcsc::GameTime & time,
                         const DrawLine & line )
{
    M_data_map[time].lines_.push_back( line );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
DrawDataHolder::addRect( const rcsc::GameTime & time,
                         const DrawRect & rect )
{
    M_data_map[time].rects_.push_back( rect );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
DrawDataHolder::addCircle( const rcsc::GameTime & time,
                           const DrawCircle & circle )
{
    M_data_map[time].circles_.push_back( circle );
}
