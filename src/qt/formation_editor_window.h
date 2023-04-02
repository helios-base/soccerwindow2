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

class QComboBox;
class QLineEdit;
class QSplitter;
class QWidget;

class FormationEditData;

class FormationEditorWindow
    : public QMainWindow {

    Q_OBJECT

private:

    static const QString ROLE_CENTER;
    static const QString ROLE_LEFT;
    static const QString ROLE_RIGHT;


    std::weak_ptr< FormationEditData > M_edit_data;

    QSplitter * M_splitter;

    QLineEdit * M_method_name; //! formation method type

    QLineEdit * M_ball_x;
    QLineEdit * M_ball_y;

    QLineEdit * M_paired_number[11];
    QComboBox * M_role_type[11];
    QComboBox * M_role_side[11];
    QLineEdit * M_role_name[11];

    QLineEdit * M_player_x[11];
    QLineEdit * M_player_y[11];

    // not used
    FormationEditorWindow() = delete;
    FormationEditorWindow( const FormationEditorWindow & ) = delete;
    FormationEditorWindow & operator=( const FormationEditorWindow & ) = delete;

public:

    FormationEditorWindow( QWidget * parent );
    ~FormationEditorWindow();

    void setEditData( std::shared_ptr< FormationEditData > data );

private:

    void createActions();
    void createMenus();
    void createWidgets();
    QWidget * createInputPanel();

    bool checkConsistency();

protected:
    void showEvent( QShowEvent * event );
    //void closeEvent( QCloseEvent * event );

public slots:
    void updateView();

private slots:
    void validateBallCoordinate();
    void applyToField();
    void resetChanges();

signals:
    void editorUpdated();

};

#endif
