// -*-c++-*-

/*!
  \file draw_data_holder.h
  \brief draw data holder class Header File.
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

#ifndef SOCCERWINDOW2_DRAW_DATA_HOLDER_H
#define SOCCERWINDOW2_DRAW_DATA_HOLDER_H

#include "draw_types.h"

#include <rcsc/game_time.h>

#include <vector>
#include <map>
#include <string>

class DrawDataHolder {
public:

    using Map = std::map< rcsc::GameTime, DrawGroup, rcsc::GameTime::Less >;

private:

    Map M_data_map;

public:

    DrawDataHolder();
    ~DrawDataHolder();

    void clear();
    bool open( const std::string & filepath );

    const Map & dataMap() const { return M_data_map; }

    void addText( const rcsc::GameTime & time,
                  const DrawText & text );
    void addPoint( const rcsc::GameTime & time,
                   const DrawPoint & point );
    void addLine( const rcsc::GameTime & time,
                  const DrawLine & line );
    void addRect( const rcsc::GameTime & time,
                  const DrawRect & rect );
    void addCircle( const rcsc::GameTime & time,
                    const DrawCircle & circle );

};

#endif
