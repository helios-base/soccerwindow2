// -*-c++-*-

/*!
  \file chain_action_data.cpp
  \brief data contains chain actions Source File.
*/

/*
 *Copyright:

 Copyright (C) Hiroki SHIMORA

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

#include "chain_action_data.h"

#include <iostream>

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
ChainDescription::print( std::ostream & os,
                         const double evaluation ) const
{
    os << chainID() << ": evaluation=" << evaluation << '\n';

    int i = 0;
    for ( ActionSequenceType::const_iterator it = actions().begin(), end = actions().end();
          it != end;
          ++it, ++i )
    {
        it->print( os, i );
    }

    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
ActionDescription::print( std::ostream & os,
                          int index ) const
{
    os << "__ " << index << ": ";

    switch ( actionCategory() ) {
    case PASS:
        {
            os << "pass (" << actionName() << "[" << actionNumber() << "])"
               << " t=" << durationTime()
               << " from[" << fromUnum() << "]"
               << "(" << fromPos().x << " " << fromPos().y << ")"
               << "-to[" << toUnum() << "]"
               << "(" << toPos().x << " " << toPos().y << "), "
               << "safe=" << safeLevel() << '\n';
        }
        break;

    case DRIBBLE:
        {
            os << "dribble (" << actionName() << "[" << actionNumber() << "])"
               << " t=" << durationTime()
               << " from[" << fromUnum() << "]"
               << "(" << fromPos().x << " " << fromPos().y << ")"
               << "-to"
               << "(" << toPos().x << " " << toPos().y << "), "
               << "safe=" << safeLevel() << '\n';
        }
        break;

    case SHOOT:
        {
            os << "shoot (" << actionName() << ")"
               << " t=" << durationTime()
               << " from[" << fromUnum() << "]"
               << "(" << fromPos().x << " " << fromPos().y << ")"
               << "-to"
               << "(" << toPos().x << " " << toPos().y << "), "
               << "safe=" << safeLevel() << '\n';
        }
        break;

    case HOLD:
        {
            os << "hold (" << actionName() << "[" << actionNumber() << "])"
               << " t=" << durationTime()
               << " from[" << fromUnum() << "]"
               << "(" << fromPos().x << " " << fromPos().y << "), "
               << "safe=" << safeLevel() << '\n';
        }
        break;

    case MOVE:
        {
            os << "move (" << actionName() << ")"
               << " t=" << durationTime()
               << " from[" << fromUnum() << "]"
               << "(" << fromPos().x << " " << fromPos().y << ")"
               << "-to[" << toUnum() << "]"
               << "(" << toPos().x << " " << toPos().y << "),"
               << "safe=" << safeLevel() << '\n';
        }
        break;

    default:
        {
            os << "???? (" << actionName() << "[" << actionNumber() << "])"
               << " t=" << durationTime()
               << " from[" << fromUnum() << "]"
               << "(" << fromPos().x << " " << fromPos().y << ")"
               << "-to[" << toUnum() << "]"
               << "(" << toPos().x << " " << toPos().y << "),"
               << "safe=" << safeLevel() << '\n';
        }
        break;
    }

    return os;
}
