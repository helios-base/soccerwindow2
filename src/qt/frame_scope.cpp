// -*-c++-*-

/*!
  \file frame_scope.cpp
  \brief guard around drawing a frame other than the one shown Source File.
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

#include "frame_scope.h"

#include "main_data.h"

/*-------------------------------------------------------------------*/
/*!

*/
FrameScope::FrameScope( MainData & main_data,
                        const std::size_t index )
    : M_main_data( main_data ),
      M_saved_index( main_data.viewIndex() ),
      M_saved_view_state( Options::instance().viewState() )
{
    // an index out of the frames is refused: the frame shown is drawn
    if ( index < M_main_data.viewHolder().monitorViewCont().size() )
    {
        M_main_data.setViewDataIndex( static_cast< int >( index ) );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
FrameScope::~FrameScope()
{
    M_main_data.setViewDataIndex( static_cast< int >( M_saved_index ) );
    Options::instance().setViewState( M_saved_view_state );
}
