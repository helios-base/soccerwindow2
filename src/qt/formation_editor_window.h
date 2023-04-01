// -*-c++-*-

/*!
  \file formation_editor_window.h
  \brief formation editor window class Header File.
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

#ifndef SOCCERWINDOW2_FORMATION_EDITOR_WINDOW_H
#define SOCCERWINDOW2_FORMATION_EDITOR_WINDOW_H

#include <QMainWindow>

#include <memory>

class FormationEditData;

class FormationEditorWindow
    : public QMainWindow {

    Q_OBJECT

private:

    std::weak_ptr< FormationEditData > M_edit_data;

    // not used
    FormationEditorWindow() = delete;
    FormationEditorWindow( const FormationEditorWindow & ) = delete;
    FormationEditorWindow & operator=( const FormationEditorWindow & ) = delete;

public:

    FormationEditorWindow( QWidget * parent );
    ~FormationEditorWindow();

    void setEditData( std::shared_ptr< FormationEditData > data );

};

#endif
