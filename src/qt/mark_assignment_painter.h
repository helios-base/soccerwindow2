// -*-c++-*-

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

#ifndef MARK_ASSIGNMENT_PAINTER_H
#define MARK_ASSIGNMENT_PAINTER_H

#include "painter_interface.h"

#include <QColor>
#include <QBrush>
#include <QPen>

class MainData;

class MarkAssignmentPainter
    : public PainterInterface {
private:
    const MainData & M_main_data;

    // not used
    MarkAssignmentPainter( const MarkAssignmentPainter & ) = delete;
    MarkAssignmentPainter & operator=( const MarkAssignmentPainter & ) = delete;

public:
    explicit
    MarkAssignmentPainter( const MainData & main_data );

    virtual
    ~MarkAssignmentPainter()
      { }

    void draw( QPainter & painter ) override;

};

#endif
