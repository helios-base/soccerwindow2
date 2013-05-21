// -*-c++-*-

/*!
  \file debug_message_window.h
  \brief debug message viewer class Header File.
*/

/*
 *Copyright:

 Copyright (C) Hidehisa AKIYAMA, Hiroki SHIMORA

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

#ifndef SOCCERWINDOW2_QT_DEBUG_MESSAGE_FRAME_H
#define SOCCERWINDOW2_QT_DEBUG_MESSAGE_FRAME_H

#include <QMainWindow>

#include <rcsc/game_time.h>
#include <rcsc/types.h>

#include <iostream>

class QAction;
class QProcess;
class QLineEdit;
class QRadioButton;
class QTabBar;
class QTabWidget;
class QTextEdit;
class QToolBar;

class ActionSequenceSelector;
class TabWidget;
class EvaluatorControlPanel;
class MainData;

//! debug manager frame class
class DebugMessageWindow
    : public QMainWindow {

    Q_OBJECT

private:

    MainData & M_main_data;

    QToolBar * M_tool_bar;
    QLineEdit * M_find_box;
    QRadioButton * M_find_forward_rb;
    QRadioButton * M_find_backward_rb;

    TabWidget * M_tab_widget;
    QTextEdit * M_message[12];

    EvaluatorControlPanel * M_evaluator_control_panel;
    ActionSequenceSelector * M_action_sequence_selector;

    QAction * M_show_debug_view_all_act;
    QAction * M_show_debug_view_self_act;
    QAction * M_show_debug_view_ball_act;
    QAction * M_show_debug_view_teammates_act;
    QAction * M_show_debug_view_opponents_act;
    QAction * M_show_debug_view_comment_act;
    QAction * M_show_debug_view_shape_act;
    QAction * M_show_debug_view_target_act;
    QAction * M_show_debug_view_message_act;

    QAction * M_show_debug_log_objects_act;

    QAction * M_open_debug_log_dir_act;
    QAction * M_clear_data_act;

    QAction * M_sync_act;
    QAction * M_decrement_act;
    QAction * M_increment_act;

    QString M_debug_text[32];
    QAction * M_debug_level_act[32];


    QAction * M_intercept_ok_act;
    QAction * M_intercept_ng_act;

    //! not used
    DebugMessageWindow();
    //! noncopyable
    DebugMessageWindow( const DebugMessageWindow & );
    //! noncopyable
    DebugMessageWindow & operator=( const DebugMessageWindow & );

public:

    DebugMessageWindow( QWidget * parent,
                        MainData & main_data );

    ~DebugMessageWindow();

protected:

    void showEvent( QShowEvent * event );
    void closeEvent( QCloseEvent * event );

private:

    void readSettings();
    void saveSettings();

    void createActions();
    void createMenus();
    void createWindows();

    void createToolBar();
    void createControlToolBar();
    void createDebugViewToolBar();
    void createTeachingToolBar();
    void createDebugLevelToolBar();

    void runOfflineClientImpl();

    void createDebugEvaluatorToolBar();

    bool openDebugLogDir( const rcsc::SideID side,
                          const std::string & dir_path );

    std::ostream * openOrCreateInterceptDecisionFile();
    std::ostream * openInterceptDecisionFile();
    std::ostream * createInterceptDecisionFile();
    void saveInterceptDecision( const bool positive );

private slots:

    void openDebugLogDir();
    void changeCurrentTab( int index );

    void toggleShowDebugViewAll( bool checked );
    void toggleShowDebugViewSelf( bool checked );
    void toggleShowDebugViewBall( bool checked );
    void toggleShowDebugViewTeammates( bool checked );
    void toggleShowDebugViewOpponents( bool checked );
    void toggleShowDebugViewComment( bool checked );
    void toggleShowDebugViewShape( bool checked );
    void toggleShowDebugViewTarget( bool checked );
    void toggleShowDebugViewMessage( bool checked );

    void toggleShowDebugLogObjects( bool checked );

    void showDebugLogDirDialog();

    void syncCycle();
    void decrementCycle();
    void incrementCycle();

    void syncAll();

    void findExistString();
    void findString( const QString & expr );

    void toggleDebugLevel( int level );

    void onInterceptOK();
    void onInterceptNG();


    void runOfflineClient();

    void selectActionSequence( int id );

public slots:

    void clearAll();
    void updateMessage();

signals:

    void configured();

    void timeSelected( const rcsc::GameTime & time );

};

#endif
