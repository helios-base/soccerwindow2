// -*-c++-*-

/*!
  \file trainer_data.h
  \brief trainer relate data holder class Header File.
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

#ifndef SOCCERWINDOW2_MODEL_TRAINER_DATA_H
#define SOCCERWINDOW2_MODEL_TRAINER_DATA_H

#include <rcsc/geom/vector_2d.h>
#include <rcsc/types.h>

class TrainerData {
private:

    rcsc::PlayMode M_playmode;

    rcsc::Vector2D M_ball_move_pos;
    rcsc::Vector2D M_ball_move_vel;

    rcsc::Vector2D M_left_move_pos[11];
    rcsc::AngleDeg M_left_body[11];

    rcsc::Vector2D M_right_move_pos[11];
    rcsc::AngleDeg M_right_body[11];


public:

    TrainerData();

    rcsc::PlayMode playmode() const
      {
          return M_playmode;
      }
    void setPlayMode( const rcsc::PlayMode mode )
      {
          M_playmode = mode;
      }
    void setPlayMode( const std::string & playmode_str );


    const
    rcsc::Vector2D & ballPos() const
      {
          return M_ball_move_pos;
      }
    const
    rcsc::Vector2D & ballVel() const
      {
          return M_ball_move_vel;
      }
    void setBall( const rcsc::Vector2D & pos )
      {
          M_ball_move_pos = pos;
      }
    void setBall( const rcsc::Vector2D & pos,
                  const rcsc::Vector2D & vel )
      {
          M_ball_move_pos = pos;
          M_ball_move_vel = vel;
      }

    const
    rcsc::Vector2D & playerMovePos( const rcsc::SideID side,
                                    const int number ) const;

    const
    rcsc::AngleDeg & playerBody( const rcsc::SideID side,
                                 const int number ) const;

    void disablePlayer( const rcsc::SideID side,
                        const int unum );

    void setPlayer( const rcsc::SideID side,
                    const int unum,
                    const rcsc::Vector2D & pos,
                    const rcsc::AngleDeg & body );
};

#endif
