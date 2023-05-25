// -*-c++-*-

/*!
  \file draw_data.h
  \brief draw data types Header File.
*/

/*
 *Copyright:

 Copyright (C) Hidehisa Akiyama

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

#ifndef SOCCERWINDOW2_DRAW_TYPES_H
#define SOCCERWINDOW2_DRAW_TYPES_H

#include <string>

struct DrawText {
    double x_;
    double y_;
    std::string color_;
    std::string msg_;

    DrawText( const double x,
              const double y,
              const std::string & color,
              const std::string & msg )
        : x_( x ),
          y_( y ),
          color_( color ),
          msg_( msg )
      { }
};

struct DrawPoint {
    double x_;
    double y_;
    std::string color_;

    DrawPoint( const double x,
               const double y,
               const std::string & color )
        : x_( x ),
          y_( y ),
          color_( color )
      { }
};

struct DrawLine {
    double x1_;
    double y1_;
    double x2_;
    double y2_;
    std::string color_;

    DrawLine( const double x1,
              const double y1,
              const double x2,
              const double y2,
              const std::string & color )
        : x1_( x1 ),
          y1_( y1 ),
          x2_( x2 ),
          y2_( y2 ),
          color_( color )
      { }

};

struct DrawRect {
    double left_;
    double top_;
    double width_;
    double height_;
    std::string line_color_;
    std::string fill_color_;

    DrawRect( const double left,
              const double top,
              const double width,
              const double height,
              const std::string & line_color,
              const std::string & fill_color )
        : left_( left ),
          top_( top ),
          width_( width ),
          height_( height ),
          line_color_( line_color ),
          fill_color_( fill_color )
      { }
};

struct DrawCircle {
    double x_;
    double y_;
    double r_;
    std::string line_color_;
    std::string fill_color_;

    DrawCircle( const double x,
                const double y,
                const double r,
                const std::string & line_color,
                const std::string & fill_color )
        : x_( x ),
          y_( y ),
          r_( r ),
          line_color_( line_color ),
          fill_color_( fill_color )
      { }
};

#endif
