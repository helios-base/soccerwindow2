// -*-c++-*-

/*!
  \file frame_scope.h
  \brief guard around drawing a frame other than the one shown Header File.
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

#ifndef SOCCERWINDOW2_QT_FRAME_SCOPE_H
#define SOCCERWINDOW2_QT_FRAME_SCOPE_H

#include "options.h"

#include <cstddef>

class MainData;

/*!
  \class FrameScope
  \brief a guard around drawing a frame other than the one shown, e.g. into
  an image (docs/plan/2026-11-28-00_frame-export.md 4.1, S4): it keeps the
  current index of MainData and Options::viewState(), makes the index the
  one given, and sets both back when it is destroyed.
*/
class FrameScope {
private:

    MainData & M_main_data;
    const std::size_t M_saved_index;
    const Options::ViewState M_saved_view_state;

    // not used
    FrameScope( const FrameScope & ) = delete;
    FrameScope & operator=( const FrameScope & ) = delete;

public:

    /*!
      \param main_data the frames and the current index
      \param index the frame to draw (an index out of the frames is ignored:
      the frame shown is drawn)
    */
    FrameScope( MainData & main_data,
                const std::size_t index );
    ~FrameScope();
};

#endif
