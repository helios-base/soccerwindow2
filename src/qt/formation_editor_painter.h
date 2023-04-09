// -*-c++-*-

/*!
  \file formation_editor_painter.h
  \brief formation edit data painter class Header File.
*/

/*
 *Copyright:

 Copyright (C) 2004 Hidehisa AKIYAMA

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

#ifndef FORMATION_EDITOR_PAINTER_H
#define FORMATION_EDITOR_PAINTER_H

#include "painter_interface.h"

#include <QColor>
#include <QBrush>
#include <QPen>

#include <memory>

class FormationEditData;
class MainData;

class FormationEditorPainter
    : public PainterInterface {
private:

    const MainData & M_main_data;

    QTransform M_transform;

    // graphic context
    QFont M_fedit_font;

    QPen M_ball_pen;
    QBrush M_ball_brush;

    QPen M_player_pen;
    QPen M_selected_pen;
    QBrush M_player_brush;
    QBrush M_paired_brush;

    QPen M_triangle_pen;
    QBrush M_contained_area_brush;

    QPen M_background_player_pen;
    QBrush M_background_player_brush;
    QBrush M_background_paired_brush;

    QPen M_background_triangle_pen;
    QBrush M_background_contained_area_brush;
    QPen M_background_font_pen;


    // not used
    FormationEditorPainter( const FormationEditorPainter & ) = delete;
    const FormationEditorPainter & operator=( const FormationEditorPainter & ) = delete;
public:

    explicit
    FormationEditorPainter( const MainData & main_data );

    /*!
      \brief draw soccer field and stationary objects
      \param painter reference to the painter object
    */
    void draw( QPainter & painter );

private:

    void setAntialiasFlag( QPainter & painter,
                           bool on );

    void drawTriangulation( QPainter & painter );
    void drawContainedTriangle( QPainter & painter );
    void drawBall( QPainter & painter );
    void drawPlayers( QPainter & painter );

    void drawBackgroundTriangulation( QPainter & painter );
    void drawBackgroundContainedTriangle( QPainter & painter );
    void drawBackgroundPlayers( QPainter & painter );

    void drawGoalieMovableArea( QPainter & painter );
    void drawShootLines( QPainter & painter );
    void drawFreeKickCircle( QPainter & painter );

};

#endif
