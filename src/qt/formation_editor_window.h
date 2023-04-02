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

    //
    // actions
    //

    // file
    //QAction * M_new_file_act;
    QAction * M_open_conf_act;
    QAction * M_open_background_conf_act;
    QAction * M_open_data_act;
    QAction * M_save_act;
    QAction * M_save_as_act;
    QAction * M_save_data_as_act;

    // edit
    QAction * M_toggle_player_auto_move_act;
    QAction * M_toggle_data_auto_select_act;
    QAction * M_toggle_pair_mode_act;

    QAction * M_add_data_act;
    QAction * M_insert_data_act;
    QAction * M_replace_data_act;
    QAction * M_delete_data_act;
    QAction * M_reverse_y_act;
    QAction * M_fit_model_act;

    // view
    QAction * M_show_tool_bar_act;

    QAction * M_show_background_data_act;
    QAction * M_show_index_act;
    QAction * M_show_free_kick_circle_act;
    QAction * M_show_triangulation_act;
    QAction * M_show_circumcircle_act;
    QAction * M_show_shoot_lines_act;
    QAction * M_show_goalie_movable_area_act;

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
    void createActionsFile();
    void createActionsEdit();
    void createActionsView();

    void createMenus();
    void createMenuFile();
    void createMenuEdit();
    void createMenuOptions();
    void createMenuView();

    void createWidgets();
    QWidget * createInputPanel();

    bool checkConsistency();

protected:
    void showEvent( QShowEvent * event );
    //void closeEvent( QCloseEvent * event );

public slots:
    void updateView();

private slots:

    // file
    void newFile();
    void openConf();
    void openBackgroundConf();
    void saveConf();
    void saveConfAs();
    void openData();
    void saveDataAs();

    // edit
    void setPlayerAutoMove( bool onoff );
    void setDataAutoSelect( bool on );
    void setPairMode( bool on );

    void addData();
    void insertData();
    void replaceData();
    void deleteData();
    void changeSampleIndex( int old_visual_index,
                            int new_visual_index );
    void reverseY();
    void fitModel();

    void deleteSample( int index );
    void replaceBall( int index,
                      double x,
                      double y );
    void replacePlayer( int index,
                        int unum,
                        double x,
                        double y );


    // view
    void setShowIndex( bool on );
    void setShowFreeKickCircle( bool on );
    void setShowTriangulation( bool on );
    void setShowCircumcircle( bool on );
    void setShowShootLines( bool on );
    void setShowGoalieMovableArea( bool on );
    void setShowBackgroundData( bool on );

    //
    //

    void validateBallCoordinate();
    void applyToField();
    void resetChanges();

    //void editMenuAboutToShow();


    void selectSample( int index );
    void selectSampleVisualIndex( int value );

    void updateDataIndex();


signals:
    void editorUpdated();

};

#endif
