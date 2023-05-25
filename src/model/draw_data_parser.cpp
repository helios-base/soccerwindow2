// -*-c++-*-

/*!
  \file draw_data_parser.cpp
  \brief draw data parser class Source File.
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

#include "draw_data_parser.h"
#include "draw_data_holder.h"

#include <iostream>
#include <cstring>
// #include <cctype>
// #include <climits>
// #include <cmath>
// #include <cstdlib>
#include <cstdio>

/*-------------------------------------------------------------------*/
DrawDataParser::DrawDataParser( DrawDataHolder & holder )
    : M_holder( holder )
{

}

/*-------------------------------------------------------------------*/
bool
DrawDataParser::parse( const char * buf )
{
    /*
      MSG := TIME DATA
      TIME := long,long
      DATA := TYPE INFO
      TYPE := t|p|l|r|c
      INFO := string
      | x y color
      | x1 y1 x2 y2 color [fill]
      | left top width height color [fill]
      | x y r color [fill]
     */

    long cycle;
    long stopped;
    int n_read = 0;

    if ( std::sscanf( buf, " %ld %ld %n", &cycle, &stopped, &n_read ) != 2
         || n_read == 0 )
    {
        return false;
    }
    buf += n_read;

    const rcsc::GameTime time( cycle, stopped );

    while ( *buf != '\0' )
    {
        while ( *buf == ' ' ) ++buf;

        char type;
        if ( std::sscanf( buf, " ( %c %n ", &type, &n_read ) != 1 )
        {
            std::cerr << "(DrawDataParser::parse) no type character" << std::endl;
            return false;
        }

        switch ( type ) {
        case 't':
            if ( ( n_read = parseText( time, buf ) ) <= 0 )
            {
                return false;
            }
            break;
        case 'p':
            if ( ( n_read = parsePoint( time, buf ) ) <= 0 )
            {
                return false;
            }
            break;
        case 'l':
            if ( ( n_read = parseLine( time, buf ) ) <= 0 )
            {
                return false;
            }
            break;
        case 'r':
            if ( ( n_read = parseRect( time, buf ) ) <= 0 )
            {
                return false;
            }
            break;
        case 'R':
            if ( ( n_read = parseFilledRect( time, buf ) ) <= 0 )
            {
                return false;
            }
            break;
        case 'c':
            if ( ( n_read = parseCircle( time, buf ) ) <= 0 )
            {
                return false;
            }
            break;
        case 'C':
            if ( ( n_read = parseFilledCircle( time, buf ) ) <= 0 )
            {
                return false;
            }
            break;
        default:
            std::cerr << "(DrawDataParser::parse) unknown type [" << type << "]" << std::endl;
            return false;
            break;
        }

        buf += n_read;
    }

    return true;
}

/*-------------------------------------------------------------------*/
int
DrawDataParser::parseText( const rcsc::GameTime & time,
                           const char * buf )
{
    double x, y;
    char color[32];
    char text[8192];
    int n_read = 0;

    if ( std::sscanf( buf, " ( t %lf %lf %31s \"%[^\"]\" ) %n ", &x, &y, color, text, &n_read ) != 4 )
    {
        std::cerr << "(DrawDataParser::parseText) illegal line [" << buf << "]" << std::endl;
        return 0;
    }

    M_holder.addText( time, DrawText( x, y, color, text ) );
    return n_read;
}

/*-------------------------------------------------------------------*/
int
DrawDataParser::parsePoint( const rcsc::GameTime & time,
                            const char * buf )
{
    double x, y;
    char color[32];
    int n_read = 0;

    if ( std::sscanf( buf, " ( p %lf %lf %31[^)] ) %n ", &x, &y, color, &n_read ) != 3 )
    {
        std::cerr << "(DrawDataParser::parsePoint) illegal data [" << buf << "]" << std::endl;
        return false;
    }

    M_holder.addPoint( time, DrawPoint( x, y, color ) );

    return n_read;
}

/*-------------------------------------------------------------------*/
int
DrawDataParser::parseLine( const rcsc::GameTime & time,
                           const char * buf )
{
    double x1, y1, x2, y2;
    char color[32];
    int n_read = 0;
    if ( std::sscanf( buf, " ( l %lf %lf %lf %lf %31[^\"] ) %n ", &x1, &y1, &x2, &y2, color, &n_read ) != 5 )
    {
        std::cerr << "(DrawDataParser::parseLine) illegal data [" << buf << "]" << std::endl;
        return 0;
    }

    M_holder.addLine( time, DrawLine( x1, y1, x2, y2, color ) );

    return n_read;
}

/*-------------------------------------------------------------------*/
int
DrawDataParser::parseRect( const rcsc::GameTime & time,
                           const char * buf )
{
    double top, left, width, height;
    char line_color[32];
    int n_read = 0;

    if ( std::sscanf( buf, " ( r %lf %lf %lf %lf %31[^\"] ) %n ", &top, &left, &width, &height, line_color, &n_read )  != 5 )
    {
        std::cerr << "(DrawDataParser::parseRect) illegal data [" << buf << "]" << std::endl;
        return 0;
    }

    M_holder.addRect( time, DrawRect( top, left, width, height, line_color, "" ) );

    return n_read;
}


/*-------------------------------------------------------------------*/
int
DrawDataParser::parseFilledRect( const rcsc::GameTime & time,
                                 const char * buf )
{
    double top, left, width, height;
    char line_color[32];
    char fill_color[32];
    int n_read = 0;

    if ( std::sscanf( buf, " ( R %lf %lf %lf %lf %s %31[^\"] ) %n ", &top, &left, &width, &height, line_color, fill_color, &n_read ) != 6 )
    {
        std::cerr << "(DrawDataParser::parseRect) illegal data [" << buf << "]" << std::endl;
        return 0;
    }

    M_holder.addRect( time, DrawRect( top, left, width, height, line_color, fill_color ) );

    return n_read;
}

/*-------------------------------------------------------------------*/
int
DrawDataParser::parseCircle( const rcsc::GameTime & time,
                             const char * buf )
{
    double x, y, r;
    char line_color[32];
    int n_read = 0;

    if ( std::sscanf( buf, " ( c %lf %lf %lf %31[^\"] ) %n ", &x, &y, &r, line_color, &n_read ) != 5 )
    {
        std::cerr << "(DrawDataParser::parseCircle) illegal data [" << buf << "]" << std::endl;
    }

    M_holder.addCircle( time, DrawCircle( x, y, r, line_color, "" ) );
    return n_read;
}

/*-------------------------------------------------------------------*/
int
DrawDataParser::parseFilledCircle( const rcsc::GameTime & time,
                                   const char * buf )
{
    double x, y, r;
    char line_color[32];
    char fill_color[32];
    int n_read = 0;

    if ( std::sscanf( buf, " ( C %lf %lf %lf %s %31[^\"] ) %n ", &x, &y, &r, line_color, fill_color, &n_read ) != 5 )
    {
        std::cerr << "(DrawDataParser::parseRect) illegal data [" << buf << "]" << std::endl;
        return 0;
    }

    M_holder.addCircle( time, DrawCircle( x, y, r, line_color, fill_color ) );
    return n_read;
}
