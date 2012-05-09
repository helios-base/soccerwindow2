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
void
ChainDescription::debugPrint( const ChainDescription & c,
                              const double evaluation,
                              std::ostream & out )
{
    out << c.chainID() << ": evaluation=" << evaluation << std::endl;

    int i = 0;
    for ( ActionSequenceType::const_iterator it = c.actions().begin(), end = c.actions().end();
          it != end;
          ++it, ++i )
    {
        ActionDescription::debugPrint( (*it), i, out );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ActionDescription::debugPrint( const ActionDescription & a,
                               int index,
                               std::ostream & out )
{
    out << "__ " << index << ": ";

    switch ( a.actionCategory() ) {
    case PASS:
        {
            out << "pass (" << a.actionName() << "[" << a.actionNumber() << "])"
                << " t=" << a.spendTime()
                << " from[" << a.fromUnum() << "]"
                << "(" << a.fromPos().x << " " << a.fromPos().y << ")"
                << "-to[" << a.toUnum() << "]"
                << "(" << a.toPos().x << " " << a.toPos().y << "), "
                << "safe=" << a.safeLevel() << std::endl;
        }
        break;

    case DRIBBLE:
        {
            out << "dribble (" << a.actionName() << "[" << a.actionNumber() << "])"
                << " t=" << a.spendTime()
                << " from[" << a.fromUnum() << "]"
                << "(" << a.fromPos().x << " " << a.fromPos().y << ")"
                << "-to"
                << "(" << a.toPos().x << " " << a.toPos().y << "), "
                << "safe=" << a.safeLevel() << std::endl;
        }
        break;

    case SHOOT:
        {
            out << "shoot (" << a.actionName() << ")"
                << " t=" << a.spendTime()
                << " from[" << a.fromUnum() << "]"
                << "(" << a.fromPos().x << " " << a.fromPos().y << ")"
                << "-to"
                << "(" << a.toPos().x << " " << a.toPos().y << "), "
                << "safe=" << a.safeLevel() << std::endl;
        }
        break;

    case HOLD:
        {
            out << "hold (" << a.actionName() << "[" << a.actionNumber() << "])"
                << " t=" << a.spendTime()
                << " from[" << a.fromUnum() << "]"
                << "(" << a.fromPos().x << " " << a.fromPos().y << "), "
                << "safe=" << a.safeLevel() << std::endl;
        }
        break;

    case MOVE:
        {
            out << "move (" << a.actionName() << ")"
                << " t=" << a.spendTime()
                << " from[" << a.fromUnum() << "]"
                << "(" << a.fromPos().x << " " << a.fromPos().y << ")"
                << "-to[" << a.toUnum() << "]"
                << "(" << a.toPos().x << " " << a.toPos().y << "),"
                << "safe=" << a.safeLevel() << std::endl;
        }
        break;

    default:
        {
            out << "???? (" << a.actionName() << "[" << a.actionNumber() << "])"
                << " t=" << a.spendTime()
                << " from[" << a.fromUnum() << "]"
                << "(" << a.fromPos().x << " " << a.fromPos().y << ")"
                << "-to[" << a.toUnum() << "]"
                << "(" << a.toPos().x << " " << a.toPos().y << "),"
                << "safe=" << a.safeLevel() << std::endl;
        }
        break;
    }
}
