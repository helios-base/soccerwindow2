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

#include "draw_data_handler.h"

#include <iostream>
#include <cstring>
// #include <cctype>
// #include <climits>
// #include <cmath>
// #include <cstdlib>
#include <cstdio>

/*-------------------------------------------------------------------*/
DrawDataParser::DrawDataParser( DrawDataHandler & handler )
    : M_handler( handler )
{

}

/*-------------------------------------------------------------------*/
bool
DrawDataParser::parse( const char * buf )
{
    /*
      DATA := t "color" "string"
      | p x y "color"
      | l x1 y1 x2 y2 "color"
      | r left top width height "color" "fill"
      | c x y r "color" "fill"
     */

    while ( *buf != '\0' )
    {
        while ( *buf == ' ' ) ++buf;

        int n_read = 0;
        char type;
        if ( std::sscanf( buf, " ( %c %n ", &type, &n_read ) != 1 )
        {
            std::cerr << "(DrawDataParser::parse) no type character" << std::endl;
            return false;
        }

        switch ( type ) {
        case 't':
            if ( ( n_read = parseText( buf ) ) <= 0 )
            {
                return false;
            }
            break;
        case 'p':
            if ( ( n_read = parsePoint( buf ) ) <= 0 )
            {
                return false;
            }
            break;
        case 'l':
            if ( ( n_read = parseLine( buf ) ) <= 0 )
            {
                return false;
            }
            break;
        case 'r':
            if ( ( n_read = parseRect( buf ) ) <= 0 )
            {
                return false;
            }
            break;
        case 'R':
            if ( ( n_read = parseFilledRect( buf ) ) <= 0 )
            {
                return false;
            }
            break;
        case 'c':
            if ( ( n_read = parseCircle( buf ) ) <= 0 )
            {
                return false;
            }
            break;
        case 'C':
            if ( ( n_read = parseFilledCircle( buf ) ) <= 0 )
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
DrawDataParser::parseText( const char * buf )
{
    double x, y;
    char color[32];
    char text[8192];
    int n_read = 0;

    if ( std::sscanf( buf, " ( t %lf %lf \"%31[^\"]\" \"%8191[^\"]\" ) %n ",
                      &x, &y, color, text, &n_read ) != 4 )
    {
        std::cerr << "(DrawDataParser::parseText) illegal line [" << buf << "]" << std::endl;
        return 0;
    }

    M_handler.handleText( DrawText( x, y, color, text ) );
    return n_read;
}

/*-------------------------------------------------------------------*/
int
DrawDataParser::parsePoint( const char * buf )
{
    double x, y;
    char color[32];
    int n_read = 0;

    if ( std::sscanf( buf, " ( p %lf %lf \"%31[^\"]\" ) %n ",
                      &x, &y, color, &n_read ) != 3 )
    {
        std::cerr << "(DrawDataParser::parsePoint) illegal data [" << buf << "]" << std::endl;
        return false;
    }

    M_handler.handlePoint( DrawPoint( x, y, color ) );

    return n_read;
}

/*-------------------------------------------------------------------*/
int
DrawDataParser::parseLine( const char * buf )
{
    double x1, y1, x2, y2;
    char color[32];
    int n_read = 0;
    if ( std::sscanf( buf, " ( l %lf %lf %lf %lf \"%31[^\"]\" ) %n ",
                      &x1, &y1, &x2, &y2, color, &n_read ) != 5 )
    {
        std::cerr << "(DrawDataParser::parseLine) illegal data [" << buf << "]" << std::endl;
        return 0;
    }

    M_handler.handleLine( DrawLine( x1, y1, x2, y2, color ) );

    return n_read;
}

/*-------------------------------------------------------------------*/
int
DrawDataParser::parseRect( const char * buf )
{
    double top, left, width, height;
    char line_color[32];
    int n_read = 0;

    if ( std::sscanf( buf, " ( r %lf %lf %lf %lf \"%31[^\"]\" ) %n ",
                      &top, &left, &width, &height, line_color, &n_read )  != 5 )
    {
        std::cerr << "(DrawDataParser::parseRect) illegal data [" << buf << "]" << std::endl;
        return 0;
    }

    M_handler.handleRect( DrawRect( top, left, width, height, line_color, "" ) );

    return n_read;
}


/*-------------------------------------------------------------------*/
int
DrawDataParser::parseFilledRect( const char * buf )
{
    double top, left, width, height;
    char line_color[32];
    char fill_color[32];
    int n_read = 0;

    if ( std::sscanf( buf, " ( R %lf %lf %lf %lf  \"%31[^\"]\" \"%31[^\"]\" ) %n ",
                      &top, &left, &width, &height, line_color, fill_color, &n_read ) != 6 )
    {
        std::cerr << "(DrawDataParser::parseFilledRect) illegal data [" << buf << "]" << std::endl;
        return 0;
    }

    M_handler.handleRect( DrawRect( top, left, width, height, line_color, fill_color ) );

    return n_read;
}

/*-------------------------------------------------------------------*/
int
DrawDataParser::parseCircle( const char * buf )
{
    double x, y, r;
    char line_color[32];
    int n_read = 0;

    if ( std::sscanf( buf, " ( c %lf %lf %lf \"%31[^\"]\" ) %n ",
                      &x, &y, &r, line_color, &n_read ) != 4 )
    {
        std::cerr << "(DrawDataParser::parseCircle) illegal data [" << buf << "]" << std::endl;
    }

    M_handler.handleCircle( DrawCircle( x, y, r, line_color, "" ) );

    return n_read;
}

/*-------------------------------------------------------------------*/
int
DrawDataParser::parseFilledCircle( const char * buf )
{
    double x, y, r;
    char line_color[32];
    char fill_color[32];
    int n_read = 0;

    if ( std::sscanf( buf, " ( C %lf %lf %lf  \"%31[^\"]\" \"%31[^\"]\" ) %n ",
                      &x, &y, &r, line_color, fill_color, &n_read ) != 5 )
    {
        std::cerr << "(DrawDataParser::parseFilledCircle) illegal data [" << buf << "]" << std::endl;
        return 0;
    }

    M_handler.handleCircle( DrawCircle( x, y, r, line_color, fill_color ) );

    return n_read;
}
