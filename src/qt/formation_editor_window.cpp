// -*-c++-*-

/*!
  \file formation_editor_window.cpp
  \brief formation editor window class Source File.
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <QtGlobal>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtWidgets>
#else
#include <QtGui>
#endif

#include "formation_editor_window.h"

#include "options.h"
#include "main_data.h"
#include "formation_edit_data.h"
#include "formation_data_view.h"

#include <rcsc/formation/formation_dt.h>

#include <iostream>

#include "xpm/fedit2.xpm"
#include "xpm/chase.xpm"
#include "xpm/data_auto_select.xpm"
#include "xpm/delete.xpm"
#include "xpm/insert.xpm"
#include "xpm/new.xpm"
#include "xpm/open.xpm"
#include "xpm/record.xpm"
#include "xpm/replace.xpm"
#include "xpm/reverse.xpm"
#include "xpm/save.xpm"
#include "xpm/symmetry.xpm"
#include "xpm/sync-ball.xpm"
#include "xpm/train.xpm"

// using namespace rcsc;

const QString FormationEditorWindow::ROLE_CENTER( "C" );
const QString FormationEditorWindow::ROLE_LEFT( "L" );
const QString FormationEditorWindow::ROLE_RIGHT( "R" );

/*-------------------------------------------------------------------*/
FormationEditorWindow::FormationEditorWindow( MainData & main_data,
                                              QWidget * parent )
    : QMainWindow( parent ),
      M_main_data( main_data )
{
    this->setWindowIcon( QIcon( QPixmap( fedit2_xpm ) ) );
    this->setWindowTitle( tr( "Formation Editor" ) );
    this->setMinimumWidth( 600 );

    this->setWindowFlags( this->windowFlags() | Qt::WindowStaysOnTopHint );

    M_tool_bar = new QToolBar( tr( "Edit tools" ), this );
    this->addToolBar( Qt::TopToolBarArea, M_tool_bar );

    createWidgets();
    createActions();
    createMenus();

    addToolBarActions();
    this->statusBar()->showMessage( tr( "Ready" ) );

    // this->setGeometry( parent->x() + ( parent->width() - this->width() ) / 2,
    //                    parent->y() + ( parent->height() - this->height() ) / 2,
    //                    this->width(),
    //                    this->height() );
}

/*-------------------------------------------------------------------*/
FormationEditorWindow::~FormationEditorWindow()
{
    // std::cerr << "delete FormationEditorWindow" << std::endl;
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::createActions()
{
    createActionsFile();
    createActionsEdit();
    createActionsView();
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::createActionsFile()
{
    //
    M_new_file_act = new QAction( QIcon( QPixmap( new_xpm ) ),
                                  tr( "New formation" ),
                                  this );
    M_new_file_act->setShortcut( Qt::CTRL + Qt::Key_N );
    M_new_file_act->setToolTip( tr( "Create a new formation." ) );
    M_new_file_act->setStatusTip( tr( "Create new formation data. (" )
                                  + M_new_file_act->shortcut().toString()
                                  + tr( ")" ) );
    connect( M_new_file_act, SIGNAL( triggered() ), this, SLOT( newFile() ) );
    //
    M_clear_act = new QAction( tr( "Clear All" ),
                               this );
    M_clear_act->setToolTip( tr( "Clear all formation model and data." ) );
    M_clear_act->setStatusTip( tr( "Clear all formation model and data." ) );
    connect( M_clear_act, SIGNAL( triggered() ), this, SLOT( clearAll() ) );
    //
    M_open_conf_act = new QAction( QIcon( QPixmap( open_xpm ) ),
                                   tr( "Open formation" ),
                                   this );
    M_open_conf_act->setShortcut( Qt::CTRL + Qt::Key_O );
    M_open_conf_act->setStatusTip( tr( "Open the formation file." ) );
    connect( M_open_conf_act, SIGNAL( triggered() ), this, SLOT( openConf() ) );
    //
    M_open_background_conf_act = new QAction( QIcon( QPixmap( open_xpm ) ),
                                              tr( "Open background formation" ),
                                              this );
    M_open_background_conf_act->setShortcut( Qt::CTRL + Qt::SHIFT + Qt::Key_O );
    M_open_background_conf_act->setStatusTip( tr( "Open background formation file." ) );

    connect( M_open_background_conf_act, SIGNAL( triggered() ), this, SLOT( openBackgroundConf() ) );
    //
    M_open_data_act = new QAction( QIcon( QPixmap( open_xpm ) ),
                                   tr( "Open training data" ),
                                   this );
    M_open_data_act->setShortcut( Qt::CTRL + Qt::ALT + Qt::Key_O );
    M_open_data_act->setToolTip( tr( "Open the training data file." ) );
    M_open_data_act->setStatusTip( tr( "Open the training data file." ) );
    connect( M_open_data_act, SIGNAL( triggered() ), this, SLOT( openData() ) );
    //
    M_save_act = new QAction( QIcon( QPixmap( save_xpm ) ),
                              tr( "Save formation" ),
                              this );
    M_save_act->setShortcut( Qt::CTRL + Qt::Key_S );
    M_save_act->setToolTip( tr( "Save the current model." ) );
    M_save_act->setStatusTip( tr( "Save the current model." ) );
    connect( M_save_act, SIGNAL( triggered() ), this, SLOT( saveConf() ) );
    //
    M_save_as_act = new QAction( tr( "Save formation as..." ),
                                 this );
    M_save_as_act->setStatusTip( tr( "Save the current model to a new file." ) );
    connect( M_save_as_act, SIGNAL( triggered() ), this, SLOT( saveConfAs() ) );
    //
    M_save_data_as_act = new QAction( tr( "Save training data as..." ),
                                      this );
    M_save_data_as_act->setStatusTip( tr( "Save the training data to a new file" ) );
    connect( M_save_data_as_act, SIGNAL( triggered() ), this, SLOT( saveDataAs() ) );
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::createActionsEdit()
{
    M_toggle_ball_sync_move_act = new QAction( QIcon( QPixmap( sync_ball_xpm ) ),
                                               tr( "Ball Sync Move" ),
                                               this );
    M_toggle_ball_sync_move_act->setToolTip( tr( "Toggle ball auto move." ) );
    M_toggle_ball_sync_move_act->setStatusTip( tr( "Toggle ball auto move mode, synchronized with the field." ) );
    connect( M_toggle_ball_sync_move_act, &QAction::toggled,
             []( bool onoff )
               {
                   Options::instance().setFeditBallSyncMove( onoff );
               } );
    M_toggle_ball_sync_move_act->setCheckable( true );
    M_toggle_ball_sync_move_act->setChecked( Options::instance().feditBallSyncMove() );
    this->addAction( M_toggle_ball_sync_move_act );
    //
    M_toggle_player_auto_move_act = new QAction( QIcon( QPixmap( chase_xpm ) ),
                                                 tr( "Player Auto Move" ),
                                                 this );
    M_toggle_player_auto_move_act->setToolTip( tr( "Toggle player auto move mode." ) );
    M_toggle_player_auto_move_act->setStatusTip( tr( "Toggle player auto move mode." ) );
    // connect( M_toggle_player_auto_move_act, SIGNAL( toggled( bool ) ),
    //          this, SLOT( setPlayerAutoMove( bool ) ) );
    connect( M_toggle_player_auto_move_act, &QAction::toggled,
             []( bool onoff )
               {
                   Options::instance().setFeditPlayerAutoMove( onoff );
               } );
    M_toggle_player_auto_move_act->setCheckable( true );
    M_toggle_player_auto_move_act->setChecked( Options::instance().feditPlayerAutoMove() );
    this->addAction( M_toggle_player_auto_move_act );

    //
    M_toggle_data_auto_select_act = new QAction(  QIcon( QPixmap( data_auto_select_xpm ) ),
                                                  tr( "Data Auto Select" ),
                                                 this );
    M_toggle_data_auto_select_act->setToolTip( tr( "Toggle auto data selection." ) );
    M_toggle_data_auto_select_act->setStatusTip( tr( "Toggle auto data selection." ) );
    // connect( M_toggle_data_auto_select_act, SIGNAL( toggled( bool ) ),
    //          this, SLOT( setDataAutoSelect( bool ) ) );
    connect( M_toggle_data_auto_select_act, &QAction::toggled,
             []( bool onoff )
               {
                   Options::instance().setFeditDataAutoSelect( onoff );
               } );
    M_toggle_data_auto_select_act->setCheckable( true );
    M_toggle_data_auto_select_act->setChecked( Options::instance().feditDataAutoSelect() );
    this->addAction( M_toggle_data_auto_select_act );

    //
    M_toggle_pair_mode_act = new QAction( QIcon( QPixmap( symmetry_xpm ) ),
                                          tr( "Pair Mode" ),
                                          this );
    M_toggle_pair_mode_act->setToolTip( tr( "Toggle pair mode." ) );
    M_toggle_pair_mode_act->setStatusTip( tr( "Toggle pair mode." ) );
    // connect( M_toggle_pair_mode_act, SIGNAL( toggled( bool ) ),
    //          this, SLOT( setPairMode( bool ) ) );
    connect( M_toggle_pair_mode_act, &QAction::toggled,
             []( bool onoff )
               {
                   Options::instance().setFeditPairMode( onoff );
               } );
    M_toggle_pair_mode_act->setCheckable( true );
    M_toggle_pair_mode_act->setChecked( Options::instance().feditPairMode() );
    this->addAction( M_toggle_pair_mode_act );
    //
    M_toggle_snap_mode_act = new QAction( tr( "Snap" ),
                                          this );
    M_toggle_snap_mode_act->setToolTip( tr( "Toggle snap mode." ) );
    M_toggle_snap_mode_act->setStatusTip( tr( "Toggle snap mode." ) );
    connect( M_toggle_snap_mode_act, &QAction::toggled,
             []( bool onoff )
               {
                   Options::instance().setFeditSnapMode( onoff );
               } );
    M_toggle_snap_mode_act->setCheckable( true );
    M_toggle_snap_mode_act->setChecked( Options::instance().feditSnapMode() );
    this->addAction( M_toggle_snap_mode_act );

    //
    M_add_data_act = new QAction( QIcon( QPixmap( record_xpm ) ),
                                  tr( "Add Data" ),
                                  this );
    M_add_data_act->setToolTip( tr( "Add a new data." ) );
    M_add_data_act->setStatusTip( tr( "Add the current field state as a new data." ) );
    connect( M_add_data_act, SIGNAL( triggered() ), this, SLOT( addData() ) );
    this->addAction( M_add_data_act );

    //
    M_insert_data_act = new QAction( QIcon( QPixmap( insert_xpm ) ),
                                     tr( "Insert Data" ),
                                     this );
    M_insert_data_act->setToolTip( tr( "Insert a new data." ) );
    M_insert_data_act->setStatusTip( tr( "Insert the field state as a new data after the current index." ) );
    connect( M_insert_data_act, SIGNAL( triggered() ), this, SLOT( insertData() ) );
    this->addAction( M_insert_data_act );

    //
    M_replace_data_act = new QAction( QIcon( QPixmap( replace_xpm ) ),
                                      tr( "Replace Data" ),
                                      this );
    M_replace_data_act->setToolTip( tr( "Replace the current index data." ) );
    M_replace_data_act->setStatusTip( tr( "Replace the current index data by the field state." ) );
    connect( M_replace_data_act, SIGNAL( triggered() ), this, SLOT( replaceCurrentData() ) );
    this->addAction( M_replace_data_act );

    //
    M_delete_data_act = new QAction( QIcon( QPixmap( delete_xpm ) ),
                                     tr( "Delete Data" ),
                                     this );
    //M_delete_data_act->setShortcut( Qt::Key_Delete );
    M_delete_data_act->setToolTip( tr( "Delete the current index data." ) );
    M_delete_data_act->setStatusTip( tr( "Delete the current index data." ) );
    connect( M_delete_data_act, SIGNAL( triggered() ), this, SLOT( deleteCurrentData() ) );
    this->addAction( M_delete_data_act );

    //
    M_reverse_y_act = new QAction( QIcon( QPixmap( reverse_xpm ) ),
                                   tr( "ReverseY" ),
                                   this );
    M_reverse_y_act->setToolTip( tr( "Reverse Y positions." ) );
    M_reverse_y_act->setStatusTip( tr( "Reverse Y positions." ) );
    connect( M_reverse_y_act, SIGNAL( triggered() ), this, SLOT( reverseY() ) );
    this->addAction( M_reverse_y_act );

    //
    M_fit_model_act = new QAction( QIcon( QPixmap( train_xpm ) ),
                                   tr( "Fit Model" ),
                                   this );
    M_fit_model_act->setStatusTip( tr( "Fit a formation model." ) );
    M_fit_model_act->setStatusTip( tr( "Fit a formation model to the current trainig data." ) );
    connect( M_fit_model_act, SIGNAL( triggered() ), this, SLOT( fitModel() ) );
    this->addAction( M_fit_model_act );
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::createActionsView()
{
    //
    M_show_tool_bar_act = new QAction( tr( "Tool Bar" ), this );
    M_show_tool_bar_act->setStatusTip( tr( "Show/Hide tool bar." ) );
    M_show_tool_bar_act->setCheckable( true );
    M_show_tool_bar_act->setChecked( true );
    connect( M_show_tool_bar_act, SIGNAL( toggled( bool ) ),
             M_tool_bar, SLOT( setVisible( bool ) ) );
    this->addAction( M_show_tool_bar_act );
    //
    // M_show_data_list_act = new QAction( tr( "Data List" ), this );
    // M_show_data_list_act->setStatusTip( tr( "Show/Hide data list." ) );
    // M_show_data_list_act->setCheckable( true );
    // M_show_data_list_act->setChecked( false );
    // connect( M_show_data_list_act, SIGNAL( toggled( bool ) ),
    //          M_data_list_dialog, SLOT( setVisible( bool ) ) );
    // this->addAction( M_show_data_list_act );

    //
    M_show_background_data_act = new QAction( tr( "Background Data" ),
                                              this );
    M_show_background_data_act->setStatusTip( tr( "Show/Hide Background Data" ) );
    connect( M_show_background_data_act, SIGNAL( toggled( bool ) ),
             this, SLOT( showBackgroundData( bool ) ) );
    M_show_background_data_act->setCheckable( true );
    M_show_background_data_act->setChecked( true );
    this->addAction( M_show_background_data_act );

    //
    M_show_index_act = new QAction( tr( "Show Index" ),
                                    this );
    M_show_index_act->setStatusTip( tr( "Toggle index painting." ) );
    connect( M_show_index_act, SIGNAL( toggled( bool ) ),
             this, SLOT( showIndex( bool ) ) );
    M_show_index_act->setCheckable( true );
    M_show_index_act->setChecked( true );
    this->addAction( M_show_index_act );

    //
    M_show_free_kick_circle_act = new QAction( tr( "Show Free Kick Circle" ),
                                                      this );
    M_show_free_kick_circle_act->setStatusTip( tr( "Show/Hide free kick circle." ) );
    connect( M_show_free_kick_circle_act, SIGNAL( toggled( bool ) ),
             this, SLOT( showFreeKickCircle( bool ) ) );
    M_show_free_kick_circle_act->setCheckable( true );
    M_show_free_kick_circle_act->setChecked( false );
    this->addAction( M_show_free_kick_circle_act );

    //
    //QAction * M_show_triangulation_act;
    M_show_triangulation_act = new QAction( tr( "Show Triangulation" ),
                                                   this );
    M_show_triangulation_act->setStatusTip( tr( "Show/Hide triangulation." ) );
    connect( M_show_triangulation_act, SIGNAL( toggled( bool ) ),
             this, SLOT( showTriangulation( bool ) ) );
    M_show_triangulation_act->setCheckable( true );
    M_show_triangulation_act->setChecked( true );
    this->addAction( M_show_triangulation_act );

    //
    M_show_circumcircle_act = new QAction( tr( "Show Circumcircle" ),
                                                  this );
    M_show_circumcircle_act->setStatusTip( tr( "Show/Hide circumcircles." ) );
    connect( M_show_circumcircle_act, SIGNAL( toggled( bool ) ),
             this, SLOT( showCircumcircle( bool ) ) );
    M_show_circumcircle_act->setCheckable( true );
    M_show_circumcircle_act->setChecked( false );
    this->addAction( M_show_circumcircle_act );

    //
    M_show_shoot_lines_act = new QAction( tr( "Show Shoot Lines" ), this );
    M_show_shoot_lines_act->setStatusTip( tr( "Show/Hide shoot lines." ) );
    connect( M_show_shoot_lines_act, SIGNAL( toggled( bool ) ),
             this, SLOT( showShootLines( bool ) ) );
    M_show_shoot_lines_act->setCheckable( true );
    M_show_shoot_lines_act->setChecked( false );
    this->addAction( M_show_shoot_lines_act );

    //
    M_show_goalie_movable_area_act = new QAction( tr( "Show Goalie Movalble Area" ), this );
    M_show_goalie_movable_area_act->setStatusTip( tr( "Show/Hide goalie reachable area." ) );
    connect( M_show_goalie_movable_area_act, SIGNAL( toggled( bool ) ),
             this, SLOT( showGoalieMovableArea( bool ) ) );
    M_show_goalie_movable_area_act->setCheckable( true );
    M_show_goalie_movable_area_act->setChecked( false );
    this->addAction( M_show_goalie_movable_area_act );
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::createMenus()
{
    createMenuFile();
    createMenuEdit();
    createMenuView();
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::createMenuFile()
{
    QMenu * menu = menuBar()->addMenu( tr( "&File" ) );

    menu->addAction( M_new_file_act );
    menu->addAction( M_clear_act );

    menu->addSeparator();

    menu->addAction( M_open_conf_act );
    menu->addAction( M_open_data_act );
    menu->addAction( M_open_background_conf_act );

    menu->addSeparator();

    menu->addAction( M_save_act );
    menu->addAction( M_save_as_act );

    menu->addSeparator();

    menu->addAction( M_save_data_as_act );

    menu->addSeparator();

    menu->addAction( tr( "Close" ), this, SLOT( close() ), Qt::CTRL + Qt::Key_W );

}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::createMenuEdit()
{
    QMenu * menu = menuBar()->addMenu( tr( "&Edit" ) );

    // connect( menu, SIGNAL( aboutToShow() ),
    //          this, SLOT( editMenuAboutToShow() ) );

    // menu->addAction( M_undo_act );
    // menu->addAction( M_redo_act );

    // menu->addSeparator();

    menu->addAction( M_toggle_ball_sync_move_act );
    menu->addAction( M_toggle_player_auto_move_act );
    menu->addAction( M_toggle_data_auto_select_act );
    menu->addSeparator();

    menu->addAction( M_toggle_pair_mode_act );
    menu->addAction( M_toggle_snap_mode_act );
    menu->addAction( M_reverse_y_act );
    menu->addSeparator();

    {
        //QMenu * submenu = menu->addMenu( tr( "Data" ) );
        menu->addAction( M_add_data_act );
        menu->addAction( M_insert_data_act );
        menu->addAction( M_replace_data_act );
        menu->addAction( M_delete_data_act );
        menu->addAction( M_fit_model_act );
        //menu->addMenu( submenu );
    }
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::createMenuView()
{
    QMenu * menu = menuBar()->addMenu( tr( "&View" ) );

    menu->addAction( M_show_tool_bar_act );
    menu->addSeparator();

    menu->addAction( M_show_index_act );
    menu->addAction( M_show_background_data_act );
    menu->addAction( M_show_triangulation_act );
    menu->addAction( M_show_circumcircle_act );
    menu->addAction( M_show_free_kick_circle_act );
    menu->addAction( M_show_shoot_lines_act );
    menu->addAction( M_show_goalie_movable_area_act );

    //menu->addSeparator();

    //menu->addAction( M_show_data_list_act );
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::createWidgets()
{
    QFrame * top_frame = new QFrame( this );
    this->setCentralWidget( top_frame );

    QHBoxLayout * top_box = new QHBoxLayout();
    top_box->setMargin( 2 );
    top_box->setSpacing( 2 );
    //top_box->setSizeConstraint( QLayout::SetFixedSize );

    top_frame->setLayout( top_box );

    top_box->addWidget( createInputPanel(), 0, Qt::AlignTop );

    top_box->addWidget( createTreeView(), 1 );

    M_input_panel->setEnabled( false );
    M_tree_view->setEnabled( false );
}

/*-------------------------------------------------------------------*/
QWidget *
FormationEditorWindow::createInputPanel()
{
    M_input_panel = new QFrame();

    QVBoxLayout * top_vbox = new QVBoxLayout();
    top_vbox->setMargin( 2 );
    top_vbox->setSpacing( 2 );
    top_vbox->setSizeConstraint( QLayout::SetFixedSize );

    M_input_panel->setLayout( top_vbox );

    // method name
    {
        QHBoxLayout * layout = new QHBoxLayout();
        top_vbox->addLayout( layout );

        //layout->addStretch( 1 );
        {
            QLabel * label = new QLabel( tr( "Method Name " ) );
            label->setAlignment( Qt::AlignCenter );
            layout->addWidget( label, 1, Qt::AlignLeft | Qt::AlignVCenter );
        }
        //layout->addSpacing( 4 );
        {
            M_method_name = new QLineEdit( tr( "---" ) );
            M_method_name->setMinimumWidth( this->fontMetrics().width( "DelaunayTriangulationXXXX" ) + 4 );
            M_method_name->setReadOnly( true ); // no editable
            M_method_name->setEnabled( false ); // no editable
            layout->addWidget( M_method_name, 1, Qt::AlignCenter );
        }
        layout->addStretch( 1 );
    }

    // ball
    {
        QHBoxLayout * layout = new QHBoxLayout();
        top_vbox->addLayout( layout );

        //layout->addStretch( 1 );
        {
            QLabel * label = new QLabel( tr( "Ball" ) );
            label->setMaximumSize( 40, this->fontMetrics().height() + 12 );
            layout->addWidget( label, 0, Qt::AlignCenter );
        }
        //layout->addStretch( 1 );
        {
            QLabel * label = new QLabel( tr( " X:" ) );
            label->setMaximumSize( 24, this->fontMetrics().height() + 12 );
            layout->addWidget( label, 0, Qt::AlignLeft | Qt::AlignVCenter );
        }
        {
            M_ball_x = new QLineEdit( tr( "0" ) );
            M_ball_x->setMinimumSize( 48, 24 );
            M_ball_x->setMaximumSize( 64, 24 );
            M_ball_x->setValidator( new QDoubleValidator( -57.5, 57.5, 2, M_ball_x ) );
            connect( M_ball_x, SIGNAL( editingFinished() ),
                     this, SLOT( validateBallCoordinate() ) );
            layout->addWidget( M_ball_x, 0, Qt::AlignLeft | Qt::AlignVCenter );
        }
        //layout->addStretch( 1 );
        {
            QLabel * label = new QLabel( tr( " Y:" ) );
            label->setMaximumSize( 24, this->fontMetrics().height() + 12 );
            layout->addWidget( label, 0, Qt::AlignLeft | Qt::AlignVCenter );
        }
        {
            M_ball_y = new QLineEdit( tr( "0" ) );
            M_ball_y->setMinimumSize( 48, 24 );
            M_ball_y->setMaximumSize( 64, 24 );
            M_ball_y->setValidator( new QDoubleValidator( -39.0, 39.0, 2, M_ball_y ) );
            connect( M_ball_y, SIGNAL( editingFinished() ),
                     this, SLOT( validateBallCoordinate() ) );
            layout->addWidget( M_ball_y, 0, Qt::AlignLeft | Qt::AlignVCenter );
        }
        layout->addStretch( 1 );
    }

    {
        const int unum_width = this->fontMetrics().width( tr( "Unum" ) ) + 4;
        const int pair_width = this->fontMetrics().width( tr( "0000" ) ) + 4;
        const int role_width = this->fontMetrics().width( tr( "CenterForwardXXXX" ) ) + 4;
        const int coord_width = this->fontMetrics().width( tr( "-00.0000" ) ) + 4;
        // const int marker_width = this->fontMetrics().width( tr( "SPM" ) ) + 4;
        // const int smarker_width = this->fontMetrics().width( tr( "SPM" ) ) + 4;

        QGridLayout * layout = new QGridLayout();
        top_vbox->addLayout( layout );

        layout->setMargin( 1 );
        layout->setSpacing( 0 );
        layout->setColumnMinimumWidth( 0, unum_width );
        layout->setColumnMinimumWidth( 1, pair_width );
        // layout->setColumnMinimumWidth( 5, marker_width );
        // layout->setColumnMinimumWidth( 6, smarker_width );

        // header
        int row = 0;
        int col = 0;
        layout->addWidget( new QLabel( tr( "Unum" ) ), 0, col, Qt::AlignCenter ); ++col;
        {
            QLabel * l = new QLabel( tr( "Pair" ) );
            l->setToolTip( tr( "Paired Number" ) );
            layout->addWidget( l, 0, col, Qt::AlignCenter ); ++col;
        }
        layout->addWidget( new QLabel( tr( "Type" ) ), 0, col, Qt::AlignCenter ); ++col;
        layout->addWidget( new QLabel( tr( "Side" ) ), 0, col, Qt::AlignCenter ); ++col;
        layout->addWidget( new QLabel( tr( "Role Name" ) ), 0, col, Qt::AlignCenter ); ++col;
        layout->addWidget( new QLabel( tr( "X" ) ), 0, col, Qt::AlignCenter ); ++col;
        layout->addWidget( new QLabel( tr( "Y" ) ), 0, col, Qt::AlignCenter ); ++col;

        row = 1;
        for ( int i = 0; i < 11; ++i, ++row )
        {
            col = 0;
            QLabel * label = new QLabel( tr( "%1" ).arg( i + 1 ) );
            label->setAlignment( Qt::AlignCenter );
            label->setMinimumSize( unum_width, 24 );
            label->setMaximumSize( unum_width + 8, 24 );
            layout->addWidget( label, row, col, Qt::AlignCenter );
            ++col;

            M_paired_number[i] = new QLineEdit( tr( "0" ) );
            M_paired_number[i]->setMinimumSize( pair_width, 24 );
            M_paired_number[i]->setMaximumSize( pair_width, 24 );
            M_paired_number[i]->setValidator( new QIntValidator( -1, 11, M_paired_number[i] ) );
            layout->addWidget( M_paired_number[i], row, col, Qt::AlignCenter );
            ++col;

            M_role_type[i] = new QComboBox();
            M_role_type[i]->addItem( tr( "G" ) );
            M_role_type[i]->addItem( tr( "DF" ) );
            M_role_type[i]->addItem( tr( "MF" ) );
            M_role_type[i]->addItem( tr( "FW" ) );
            layout->addWidget( M_role_type[i], row, col, Qt::AlignCenter );
            ++col;

            M_role_side[i] = new QComboBox();
            M_role_side[i]->addItem( ROLE_CENTER );
            M_role_side[i]->addItem( ROLE_LEFT );
            M_role_side[i]->addItem( ROLE_RIGHT );
            layout->addWidget( M_role_side[i], row, col, Qt::AlignCenter );
            ++col;

            M_role_name[i] = new QLineEdit( tr( "Role" ) );
            M_role_name[i]->setMaximumSize( role_width, 24 );
            layout->addWidget( M_role_name[i], row, col, Qt::AlignCenter );
            ++col;

            M_player_x[i] = new QLineEdit( tr( "0.0" ) );
            M_player_x[i]->setMaximumSize( coord_width, 24 );
            M_player_x[i]->setValidator( new QDoubleValidator( -57.5, 57.5, 2, M_player_x[i] ) );
            layout->addWidget( M_player_x[i], row, col, Qt::AlignCenter );
            ++col;

            M_player_y[i] = new QLineEdit( tr( "0.0" ) );
            M_player_y[i]->setMaximumSize( coord_width, 24 );
            M_player_y[i]->setValidator( new QDoubleValidator( -39.0, 39.0, 2, M_player_y[i] ) );
            layout->addWidget( M_player_y[i], row, col, Qt::AlignCenter );
            ++col;
        }
    }

    {
        QHBoxLayout * layout = new QHBoxLayout();
        top_vbox->addLayout( layout );

        {
            QPushButton * btn = new QPushButton( tr( "Apply" ) );
            //btn->setAutoDefault( false );
            connect( btn, SIGNAL( clicked() ),
                     this, SLOT( applyToField() ) );
            layout->addWidget( btn, 0, Qt::AlignLeft );
        }
        layout->addStretch( 1 );
        {
            QPushButton * btn = new QPushButton( tr( "Reset" ) );
            btn->setAutoDefault( false );
            btn->setDefault( false );
            connect( btn, SIGNAL( clicked() ),
                     this, SLOT( resetPanelChanges() ) );
            layout->addWidget( btn, 0, Qt::AlignRight );
        }
        {
            QPushButton * btn = new QPushButton( tr( "Close" ) );
            btn->setAutoDefault( false );
            btn->setDefault( false );
            connect( btn, SIGNAL( clicked() ),
                     this, SLOT( close() ) );
            layout->addWidget( btn, 0, Qt::AlignRight );
        }
    }

    return M_input_panel;
}

/*-------------------------------------------------------------------*/
QWidget *
FormationEditorWindow::createTreeView()
{
    //M_data_list_dialog = new QDialog( this );
    //M_data_list_dialog->setModal( false );
    //M_data_list_dialog->setWindowTitle( tr( "Data List" ) );

    //M_tree_view = new FormationDataView( M_data_list_dialog );
    M_tree_view = new FormationDataView( M_main_data, this );

    connect( M_tree_view, SIGNAL( dataSelected( int ) ),
             this, SLOT( selectData( int ) ) );
    connect( M_tree_view, SIGNAL( indexChangeRequested( int, int ) ),
             this, SLOT( changeDataIndex( int, int ) ) );
    connect( M_tree_view, SIGNAL( deleteRequested( int ) ),
             this, SLOT( deleteData( int ) ) );
    connect( M_tree_view, SIGNAL( ballReplaced( int, double, double ) ),
             this, SLOT( replaceBall( int, double, double ) ) );
    connect( M_tree_view, SIGNAL( playerReplaced( int, int, double, double ) ),
             this, SLOT( replacePlayer( int, int, double, double ) ) );

    // M_data_list_dialog->setFixedWidth( M_tree_view->sizeHint().width() );
    // return M_data_list_dialog;
    return M_tree_view;
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::addToolBarActions()
{
    M_tool_bar->addAction( M_save_act );
    M_tool_bar->addSeparator();

    M_tool_bar->addAction( M_toggle_ball_sync_move_act );
    M_tool_bar->addAction( M_toggle_player_auto_move_act );
    M_tool_bar->addAction( M_toggle_data_auto_select_act );
    M_tool_bar->addSeparator();

    M_tool_bar->addAction( M_toggle_pair_mode_act );

    M_tool_bar->addAction( M_toggle_snap_mode_act );

    M_grid_size_spin_box = new QDoubleSpinBox();
    M_grid_size_spin_box->setValue( 0.5 );
    M_grid_size_spin_box->setRange( 0.1, 2.0 );
    M_grid_size_spin_box->setDecimals( 1 );
    M_grid_size_spin_box->setSingleStep( 0.1 );
    connect( M_grid_size_spin_box, SIGNAL( valueChanged( double ) ),
             this, SLOT( setGridSize( double ) ) );

    M_tool_bar->addWidget( M_grid_size_spin_box );

    M_tool_bar->addAction( M_reverse_y_act );
    M_tool_bar->addSeparator();

    //

    //M_tool_bar->addAction( M_fit_model_act );
    //M_tool_bar->addSeparator();

    M_tool_bar->addAction( M_insert_data_act );
    M_tool_bar->addAction( M_add_data_act );
    M_tool_bar->addAction( M_replace_data_act );

    //
    {
        QFrame * dummy_frame = new QFrame();
        QHBoxLayout * layout = new QHBoxLayout();
        layout->addStretch( 1 );
        dummy_frame->setLayout( layout );
        M_tool_bar->addWidget( dummy_frame );
    }

    M_tool_bar->addAction( M_delete_data_act );

    //
    M_index_spin_box = new QSpinBox();
    //M_index_spin_box->setPrefix( tr( "Index: " ) );
    M_index_spin_box->setRange( 0, 0 );
    M_index_spin_box->setWrapping( true );
    M_index_spin_box->setMaximumSize( 80, 24 );
    M_index_spin_box->setEnabled( false );
    connect( M_index_spin_box, SIGNAL( valueChanged( int ) ),
             this, SLOT( slotIndexChanged( int ) ) );
    M_tool_bar->addWidget( M_index_spin_box );
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::showWarningMessage( const std::string & err )
{
    QMessageBox::warning( this,
                          tr( "Warning" ),
                          QString::fromStdString( err ),
                          QMessageBox::Ok,
                          QMessageBox::NoButton );
}

/*-------------------------------------------------------------------*/
bool
FormationEditorWindow::checkConsistency()
{
   for ( int num = 1; num <= 11; ++num )
    {
        bool ok = false;
        const int paired_number = M_paired_number[num-1]->text().toInt( &ok );
        if ( ! ok ) return false;
        if ( paired_number == num )
        {
            std::cerr << "(FormationEditorWindow) ERROR: number(" << num << ") == paired_number(" << paired_number << ")" << std::endl;
            return false;
        }

        if ( 1 <= paired_number
             && paired_number <= 11 )
        {
            const int n = M_paired_number[paired_number-1]->text().toInt( &ok );
            if ( ! ok ) return false;
            if ( n != num )
            {
                std::cerr << "(FormationEditorWindow) ERROR: number(" << num << ") != paired(" << n << ")" << std::endl;
                return false;
            }

            if ( M_role_side[num-1]->currentText() == ROLE_CENTER )
            {
                std::cerr << "(FormationEditorWindow) ERROR: Center type has a paired number(" << paired_number << ")" << std::endl;
                return false;
            }

            if ( M_role_side[num-1]->currentText() == M_role_side[paired_number-1]->currentText() )
            {
                std::cerr << "(FormationEditorWindow) ERROR: paired players(" << num << "," << paired_number << ") have same side" << std::endl;
                return false;
            }
        }

        if ( M_role_name[num-1]->text().isEmpty() )
        {
            std::cerr << "(FormationEditorWindow) ERROR: empty role name" << std::endl;
            return false;
        }
    }

    return true;
}

/*-------------------------------------------------------------------*/
bool
FormationEditorWindow::saveChanges()
{
    std::shared_ptr< FormationEditData > ptr = M_main_data.formationEditData();
    if ( ! ptr )
    {
        return true;
    }

    if ( ! ptr->isModified() )
    {
        return true;
    }

    QMessageBox::StandardButton result
        = QMessageBox::question( this,
                                 tr( "Save Notify" ),
                                 tr( "Data is changed.\nSave?" ),
                                 QMessageBox::Yes | QMessageBox::Cancel | QMessageBox::No,
                                 QMessageBox::Yes );
    if ( result == QMessageBox::Cancel )
    {
        return false;
    }

    if ( result == QMessageBox::Yes )
    {
        if ( ptr->isModified() )
        {
            std::cerr << "(FormationEditorWindow::saveChanges)" << std::endl;
            saveConf();
        }
    }

    return true;
}

/*-------------------------------------------------------------------*/
bool
FormationEditorWindow::openConfFile( const QString & filepath )
{
    std::cerr << "(FormationEditorWindow::openConfFile) " << filepath.toStdString()
              << std::endl;

    if ( filepath.isEmpty() )
    {
        QMessageBox::warning( this,
                              tr( "Warning" ),
                              tr( "Empty file path." ),
                              QMessageBox::Ok,
                              QMessageBox::NoButton );
        return false;
    }

    QFileInfo fileinfo( filepath );

    if ( ! fileinfo.exists()
         || ! fileinfo.isReadable() )
    {
        QMessageBox::warning( this,
                              tr( "Warning" ),
                              tr( "No such a file or not readable. \n" ) + filepath,
                              QMessageBox::Ok,
                              QMessageBox::NoButton );
        return false;
    }

    std::shared_ptr< FormationEditData > new_data( new FormationEditData() );

    if ( ! new_data->openConf( filepath.toStdString() ) )
    {
        QMessageBox::warning( this,
                              tr( "Error" ),
                              tr( "Failed to open the file. \n" ) + filepath,
                              QMessageBox::Ok,
                              QMessageBox::NoButton );
        return false;
    }

    if ( ! new_data->formation()
         || ! new_data->formationData() )
    {
        QMessageBox::warning( this,
                              tr( "Error" ),
                              tr( "Failed to create a formation. \n" ) + filepath,
                              QMessageBox::Ok,
                              QMessageBox::NoButton );
        return false;
    }

    M_main_data.setFormationEditData( new_data );

    this->statusBar()->showMessage( tr( "Opened %1" ).arg( filepath ), 2000 );
    this->setWindowTitle( tr( "Formation Editor - " )
                          + fileinfo.fileName()
                          + tr( " -") );

    M_tree_view->clear();

    M_index_spin_box->setEnabled( true );
    M_input_panel->setEnabled( true );
    M_tree_view->setEnabled( true );

    const int data_count = new_data->formationData()->dataCont().size();
    M_index_spin_box->setRange( 0, data_count );
    M_index_spin_box->setValue( 0 );

    M_tree_view->unselectData();

    updatePanel();
    M_tree_view->updateData();

    emit editorUpdated();

    return true;
}

/*-------------------------------------------------------------------*/
bool
FormationEditorWindow::openBackgroundConfFile( const QString & filepath )
{
    std::cerr << "(FormationEditorWindow::openBackgroundConfFile) " << filepath.toStdString()
              << std::endl;

    std::shared_ptr< FormationEditData > ptr = M_main_data.formationEditData();
    if ( ! ptr )
    {
        QMessageBox::warning( this,
                              tr( "Warning" ),
                              tr( "Tried to open a background formation.\nBut no foreground data yet." ),
                              QMessageBox::Ok,
                              QMessageBox::NoButton );
        return false;
    }

    if ( ! ptr->formation() )
    {
        QMessageBox::warning( this,
                              tr( "Error" ),
                              tr( "Tried to open a background formation.\nNo formation for foreground data." ),
                              QMessageBox::Ok,
                              QMessageBox::NoButton );
        return false;
    }

    if ( filepath.isEmpty() )
    {
        QMessageBox::warning( this,
                              tr( "Warning" ),
                              tr( "Empty file path." ),
                              QMessageBox::Ok,
                              QMessageBox::NoButton );
        return false;
    }

    QFileInfo fileinfo( filepath );

    if ( ! fileinfo.exists()
         || ! fileinfo.isReadable() )
    {
        QMessageBox::warning( this,
                              tr( "Warning" ),
                              tr( "No such a file or not readable. \n" ) + filepath,
                              QMessageBox::Ok,
                              QMessageBox::NoButton );
        return false;
    }


    if ( ! ptr->openBackgroundConf( filepath.toStdString() ) )
    {
        QMessageBox::warning( this,
                              tr( "Error" ),
                              tr( "Failed to open a " ) + filepath,
                              QMessageBox::Ok,
                              QMessageBox::NoButton );
        return false;
    }

    emit editorUpdated();

    return true;
}

/*-------------------------------------------------------------------*/
bool
FormationEditorWindow::openDataFile( const QString & filepath )
{
   std::cerr << "(FormationEditorWindow::openDataFile) " << filepath.toStdString()
              << std::endl;

    if ( filepath.isEmpty() )
    {
        QMessageBox::warning( this,
                              tr( "Warning" ),
                              tr( "Empty file path." ),
                              QMessageBox::Ok,
                              QMessageBox::NoButton );
        return false;
    }

    std::shared_ptr< FormationEditData > ptr = M_main_data.formationEditData();

    if ( ! ptr )
    {
        QMessageBox::warning( this,
                              tr( "Warning" ),
                              tr( "No formation." ),
                              QMessageBox::Ok,
                              QMessageBox::NoButton );
        return false;
    }

    QFileInfo fileinfo( filepath );

    if ( ! fileinfo.exists()
         || ! fileinfo.isReadable() )
    {
        QMessageBox::warning( this,
                              tr( "Warning" ),
                              tr( "No such a file or not readable.\n" ) + filepath,
                              QMessageBox::Ok,
                              QMessageBox::NoButton );
        return false;
    }

    if ( ! ptr->openData( filepath.toStdString() ) )
    {
        QMessageBox::warning( this,
                              tr( "Error" ),
                              tr( "Failed to read the file.\n" ) + filepath,
                              QMessageBox::Ok,
                              QMessageBox::NoButton );
        return false;

    }

    this->statusBar()->showMessage( tr( "Opened %1" ).arg( filepath ), 2000 );

    M_tree_view->clear();

    const int data_count = ptr->formationData()->dataCont().size();
    M_index_spin_box->setRange( 0, data_count );
    M_index_spin_box->setValue( 0 );

    M_tree_view->unselectData();
    M_tree_view->updateData();

    emit editorUpdated();

    return true;
}

/*-------------------------------------------------------------------*/
bool
FormationEditorWindow::openFilesByOption()
{
    bool result = false;

    if ( ! Options::instance().feditConfFile().empty()
         && openConfFile( QString::fromStdString( Options::instance().feditConfFile() ) ) )
    {
        result = true;
    }

    if ( ! Options::instance().feditDataFile().empty() )
    {
        openDataFile( QString::fromStdString( Options::instance().feditDataFile() ) );
    }

    if ( ! Options::instance().feditBackgroundFile().empty() )
    {
        openBackgroundConfFile( QString::fromStdString( Options::instance().feditBackgroundFile() ) );
    }

    return result;
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::showEvent( QShowEvent * event )
{
    QMainWindow::showEvent( event );

    updatePanel();
    M_tree_view->updateData();

    this->centralWidget()->adjustSize();
    //this->setFixedSize( this->sizeHint() );

    Options::instance().setFeditMode( true );
    emit editorUpdated();
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::hideEvent( QHideEvent * event )
{
    QMainWindow::hideEvent( event );

    Options::instance().setFeditMode( false );
    emit editorUpdated();
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::closeEvent( QCloseEvent * event )
{
    if ( ! saveChanges() )
    {
        event->ignore();
        return;
    }

    Options::instance().setFeditMode( false );
    emit editorUpdated();

    event->accept();
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::updatePanel()
{
    if ( ! this->isVisible() )
    {
        return;
    }

    std::shared_ptr< FormationEditData > ptr = M_main_data.formationEditData();
    if ( ! ptr )
    {
        return;
    }

    rcsc::Formation::ConstPtr f = ptr->formation();
    if ( ! f )
    {
        std::cerr << "(FormationEditorWindow::updatePanel) no formation." << std::endl;
        return;
    }

    updateDataIndex();

    M_method_name->setText( QString::fromStdString( f->methodName() ) );

    // ball
    const rcsc::FormationData::Data & state = ptr->currentState();

    M_ball_x->setText( QString::number( state.ball_.x, 'f', 2 ) );
    M_ball_y->setText( QString::number( state.ball_.y, 'f', 2 ) );

    // player
    Q_ASSERT( state.players_.size() == 11 );
    for ( size_t i = 0; i < 11; ++i )
    {
        int unum = i + 1;
        M_paired_number[i]->setText( QString::number( f->pairedNumber( unum ) ) );

        const rcsc::RoleType role_type = f->roleType( unum );
        M_role_type[i]->setCurrentIndex( static_cast< int >( role_type.type() ) );
        M_role_side[i]->setCurrentIndex( role_type.side() == rcsc::RoleType::Left ? 1
                                         : role_type.side() == rcsc::RoleType::Right ? 2
                                         : 0 );
        M_role_name[i]->setText( QString::fromStdString( f->roleName( unum ) ) );

        M_player_x[i]->setText( QString::number( state.players_[i].x, 'f', 2 ) );
        M_player_y[i]->setText( QString::number( state.players_[i].y, 'f', 2 ) );
    }
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::newFile()
{
    if ( ! saveChanges() )
    {
        // data is changed, but save operation is cancelled.
        return;
    }

    QStringList names;
    // names.push_back( QString::fromStdString( FormationCDT::name() ) );
    names.push_back( QString::fromStdString( rcsc::FormationDT::NAME ) );
    // names.push_back( QString::fromStdString( FormationKNN::name() ) );
    //     names.push_back( QString::fromAscii( FormationBPN::name().c_str() ) );
    //     names.push_back( QString::fromAscii( FormationRBF::name().c_str() ) );
    //     names.push_back( QString::fromAscii( FormationNGNet::name().c_str() ) );

    bool ok = false;
    QString name = QInputDialog::getItem( this,
                                          tr( "Create New Formation" ),
                                          tr( "Choose a model name:" ),
                                          names,
                                          0,
                                          false, // no editable
                                          &ok );
    if ( ! ok )
    {
        return;
    }

    if ( M_main_data.formationEditData() )
    {
        M_main_data.clearFormationEditData();
    }

    this->setWindowTitle( tr( "FormationEditor - New Formation -" ) );

    // create new data

    std::shared_ptr< FormationEditData > new_data( new FormationEditData() );
    new_data->newFormation( name.toStdString() );

    if ( ! new_data->formation()
         || ! new_data->formationData() )
    {
        QMessageBox::critical( this,
                               tr( "Error" ),
                               tr( "Could not create a new formation." ),
                               QMessageBox::Ok,
                               QMessageBox::NoButton );
        std::cerr << "(FormationEditorWindow::newFile) ERROR Failed to initialize formation data"
                  << std::endl;
        return;
    }

    M_main_data.setFormationEditData( new_data );

    this->statusBar()->showMessage( tr( "New Formation" ), 2000 );
    this->setWindowTitle( tr( "Formation Editor - New Formation -" ) );

    M_tree_view->clear();

    M_index_spin_box->setEnabled( true );
    M_input_panel->setEnabled( true );
    M_tree_view->setEnabled( true );

    const int data_count = new_data->formationData()->dataCont().size();
    M_index_spin_box->setRange( 0, data_count );
    M_tree_view->updateData();

    M_index_spin_box->setValue( 0 );
    M_tree_view->unselectData();
    updatePanel();

    emit editorUpdated();
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::clearAll()
{
    if ( ! saveChanges() )
    {
        // data is changed, but save operation is cancelled.
        return;
    }

    M_main_data.clearFormationEditData();

    M_index_spin_box->setRange( 0, 0 );
    M_index_spin_box->setValue( 0 );
    M_tree_view->unselectData();
    M_tree_view->clear();

    M_index_spin_box->setEnabled( false );
    M_input_panel->setEnabled( false );
    M_tree_view->setEnabled( false );

    emit editorUpdated();
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::openConf()
{
    if ( ! saveChanges() )
    {
        // data changed, but save operation is canceled.
        return;
    }

    QString filter( tr( "Formation file (*.conf);;"
                        "All files (*)" ) );
    QString filepath = QFileDialog::getOpenFileName( this,
                                                     tr( "Open Formation" ),
                                                     tr( "" ),
                                                     filter );
    if ( filepath.isEmpty() )
    {
        return;
    }

    openConfFile( filepath );
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::openBackgroundConf()
{
    QString filter( tr( "Background formation file (*.conf);;"
                        "All files (*)" ) );
    QString filepath = QFileDialog::getOpenFileName( this,
                                                     tr( "Open Background Formation" ),
                                                     tr( "" ),
                                                     filter );
    if ( filepath.isEmpty() )
    {
        return;
    }

    openBackgroundConfFile( filepath );
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::saveConf()
{
    std::shared_ptr< FormationEditData > ptr = M_main_data.formationEditData();
    if ( ! ptr
         || ! ptr->formation() )
    {
        return;
    }

    if ( ptr->saveConf() )
    {
        this->statusBar()->showMessage( tr( "Saved %1" ).arg( QString::fromStdString( ptr->filePath() ) ), 2000 );
    }
    else
    {
        saveConfAs();
    }
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::saveConfAs()
{
    std::shared_ptr< FormationEditData > ptr = M_main_data.formationEditData();
    if ( ! ptr
         || ! ptr->formation() )
    {
        return;
    }

    QString filter( tr( "Formation file (*.conf);;"
                        "All files (*)" ) );
    QString filepath = QFileDialog::getSaveFileName( this,
                                                     tr( "Save Formation" ),
                                                     QString::fromStdString( ptr->filePath() ),
                                                     filter );
    if ( filepath.isEmpty() )
    {
        return;
    }

    if ( filepath.length() <= 5
         || filepath.right( 5 ) != tr( ".conf" ) )
    {
        filepath += tr( ".conf" );
    }

    if ( ptr->saveConfAs( filepath.toStdString() ) )
    {
        QFileInfo fileinfo( filepath );
        this->setWindowTitle( tr( "FormationEditor - " )
                              + fileinfo.fileName()
                              + tr( " -") );
        this->statusBar()->showMessage( tr( "Saved %1" ).arg( filepath ), 2000 );
    }
    else
    {
        QMessageBox::critical( this,
                               tr( "Error" ),
                               tr( "Failed to save the file " ) + filepath,
                               QMessageBox::Ok,
                               QMessageBox::NoButton );
    }
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::openData()
{
    QString filter( tr( "Formation data file (*.dat);;"
                        "All files (*)" ) );
    QString filepath = QFileDialog::getOpenFileName( this,
                                                     tr( "Open Training Data" ),
                                                     tr( "" ),
                                                     filter );
    if ( filepath.isEmpty() )
    {
        return;
    }

    openDataFile( filepath );
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::saveDataAs()
{
    std::shared_ptr< FormationEditData > ptr = M_main_data.formationEditData();
    if ( ! ptr
         || ! ptr->formationData() )
    {
        return;
    }

    QString filter( tr( "Formation Data file (*.dat);;"
                        "All files (*)" ) );
    QString filepath = QFileDialog::getSaveFileName( this,
                                                     tr( "Save Training Data" ),
                                                     tr( "" ),
                                                     filter );

    if ( filepath.isEmpty() )
    {
        return;
    }

    if ( filepath.length() <= 4
         || filepath.right( 4 ) != tr( ".dat" ) )
    {
        filepath += tr( ".dat" );
    }

    if ( ptr->saveDataAs( filepath.toStdString() ) )
    {
        this->statusBar()->showMessage( tr( "Saved %1" ).arg( filepath ), 2000 );
    }
    else
    {
        QMessageBox::critical( this,
                               tr( "Error" ),
                               tr( "Failed to save the file " ) + filepath,
                               QMessageBox::Ok,
                               QMessageBox::NoButton );
    }
}

/*-------------------------------------------------------------------*/
// void
// FormationEditorWindow::setPlayerAutoMove( bool onoff )
// {
//     Options::instance().setFeditPlayerAutoMove( onoff );
// }

/*-------------------------------------------------------------------*/
// void
// FormationEditorWindow::setDataAutoSelect( bool on )
// {
//     Options::instance().setDataAutoSelect( on );
// }

/*-------------------------------------------------------------------*/
// void
// FormationEditorWindow::setPairMode( bool on )
// {
//     Options::instance().setPairMode( on );
// }

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::setGridSize( double value )
{
    Options::instance().setFeditGridSize( std::min( std::max( 0.1, value ), 2.0 ) );
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::addData()
{
    std::shared_ptr< FormationEditData > ptr = M_main_data.formationEditData();
    if ( ! ptr
         || ! ptr->formationData() )
    {
        return;
    }

    std::cerr << "(FormationEditorWindow::addData)" << std::endl;

    const std::string err = ptr->addData();
    if ( ! err.empty() )
    {
        showWarningMessage( err );
        return;
    }

    const int data_count = ptr->formationData()->dataCont().size();
    M_index_spin_box->setRange( 0, data_count );

    updateDataIndex();
    M_tree_view->updateData();
    emit editorUpdated();
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::insertData()
{
    std::shared_ptr< FormationEditData > ptr = M_main_data.formationEditData();
    if ( ! ptr
         || ! ptr->formationData() )
    {
        return;
    }

    int index = M_index_spin_box->value() - 1;
    if ( index == -1 ) index = 0;

    std::cerr << "(FormationEditorWindow::insertData) at " << index << std::endl;

    const std::string err  = ptr->insertData( index );
    if ( ! err.empty() )
    {
        showWarningMessage( err );
        return;
    }

    const int data_count = ptr->formationData()->dataCont().size();
    M_index_spin_box->setRange( 0, data_count );

    updateDataIndex();
    M_tree_view->updateData();
    emit editorUpdated();
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::replaceCurrentData()
{
    std::shared_ptr< FormationEditData > ptr = M_main_data.formationEditData();
    if ( ! ptr
         || ! ptr->formationData() )
    {
        return;
    }

    const int index = M_index_spin_box->value() - 1;

    std::cerr << "(FormationEditorWindow::replaceCurrentData) index=" << index << std::endl;

    const std::string err = ptr->replaceData( index );
    if ( ! err.empty() )
    {
        showWarningMessage( err );
        return;
    }

    const int data_count = ptr->formationData()->dataCont().size();
    M_index_spin_box->setRange( 0, data_count );

    updateDataIndex();
    M_tree_view->updateData();
    emit editorUpdated();
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::deleteCurrentData()
{
    std::shared_ptr< FormationEditData > ptr = M_main_data.formationEditData();
    if ( ! ptr
         || ! ptr->formationData() )
    {
        return;
    }

    const int index = ptr->currentIndex();
    if ( index < 0 )
    {
        QMessageBox::warning( this,
                              tr( "Warning" ),
                              tr( "No selected data." ),
                              QMessageBox::Ok,
                              QMessageBox::NoButton );
        return;
    }

    std::cerr << "(FormationEditorWindow::deleteCurrentData) index=" << index << std::endl;

    const std::string err = ptr->deleteData( index );
    if ( ! err.empty() )
    {
        showWarningMessage( err );
        return;
    }

    ptr->setCurrentIndex( -1 );

    const int data_count = ptr->formationData()->dataCont().size();
    M_index_spin_box->setRange( 0, data_count );

    updateDataIndex();
    M_tree_view->updateData();
    emit editorUpdated();
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::changeDataIndex( int old_shown_index,
                                        int new_shown_index )
{
    std::shared_ptr< FormationEditData > ptr = M_main_data.formationEditData();
    if ( ! ptr
         || ! ptr->formationData() )
    {
        return;
    }

    std::cerr << "(FormaitonEditorWindow::changeDataIndex)"
              << " old=" << old_shown_index
              << " new=" << new_shown_index << std::endl;

    std::string err = ptr->changeDataIndex( old_shown_index - 1,
                                            new_shown_index - 1 );
    if ( ! err.empty() )
    {
        showWarningMessage( err );
        return;
    }

    const int data_count = ptr->formationData()->dataCont().size();
    M_index_spin_box->setRange( 0, data_count );

    selectData( ptr->currentIndex() );
    M_tree_view->updateData();
    emit editorUpdated();
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::reverseY()
{
    std::shared_ptr< FormationEditData > ptr = M_main_data.formationEditData();
    if ( ! ptr
         || ! ptr->formationData() )
    {
        return;
    }

    std::cerr << "(FormationEditorWindow::reverseY)" << std::endl;

    ptr->reverseY();
    emit editorUpdated();
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::fitModel()
{
    std::shared_ptr< FormationEditData > ptr = M_main_data.formationEditData();
    if ( ! ptr
         || ! ptr->formationData() )
    {
        return;
    }

    std::cerr << "(FormationEditorWindow::fitModel)" << std::endl;

    ptr->fitModel();

    const int data_count = ptr->formationData()->dataCont().size();
    M_index_spin_box->setRange( 0, data_count );

    M_tree_view->updateData();
    emit editorUpdated();
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::deleteData( int index )
{
    std::shared_ptr< FormationEditData > ptr = M_main_data.formationEditData();
    if ( ! ptr
         || ! ptr->formationData() )
    {
        return;
    }

    std::cerr << "(MainWindow::deleteData) index=" << index << std::endl;

    const std::string err = ptr->deleteData( index );
    if ( ! err.empty() )
    {
        showWarningMessage( err );
        return;
    }

    ptr->setCurrentIndex( -1 );

    const int data_count = ptr->formationData()->dataCont().size();
    M_index_spin_box->setRange( 0, data_count );

    updateDataIndex();
    M_tree_view->updateData();
    emit editorUpdated();
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::replaceBall( int index,
                                    double x,
                                    double y )
{
    std::shared_ptr< FormationEditData > ptr = M_main_data.formationEditData();
    if ( ! ptr
         || ! ptr->formationData() )
    {
        return;
    }

    std::cerr << "(FormationEditorWindow::replaceBall) index=" << index
              << " (" << x << " , " << y << ")" << std::endl;

    const std::string err  = ptr->replaceBall( index, x, y );
    if ( ! err.empty() )
    {
        showWarningMessage( err );
        return;
    }

    const int data_count = ptr->formationData()->dataCont().size();
    M_index_spin_box->setRange( 0, data_count );

    updateDataIndex();
    M_tree_view->updateData();
    emit editorUpdated();
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::replacePlayer( int index,
                                      int unum,
                                      double x,
                                      double y )
{
    std::shared_ptr< FormationEditData > ptr = M_main_data.formationEditData();
    if ( ! ptr
         || ! ptr->formationData() )
    {
        return;
    }

    std::cerr << "(FormationEditorWindow::replacePlayer) index=" << index
              << " unum=" << unum
              << " (" << x << " , " << y << ")" << std::endl;

    const std::string err = ptr->replacePlayer( index, unum, x, y );
    if ( ! err.empty() )
    {
        showWarningMessage( err );
        return;
    }

    const int data_count = ptr->formationData()->dataCont().size();
    M_index_spin_box->setRange( 0, data_count );

    updateDataIndex();
    M_tree_view->updateData();
    emit editorUpdated();
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::showIndex( bool onoff )
{
    Options::instance().setFeditShowIndex( onoff );
    emit editorUpdated();
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::showFreeKickCircle( bool onoff )
{
    Options::instance().setFeditShowFreeKickCircle( onoff );
    emit editorUpdated();
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::showTriangulation( bool onoff )
{
    Options::instance().setFeditShowTriangulation( onoff );
    emit editorUpdated();
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::showCircumcircle( bool onoff )
{
    Options::instance().setFeditShowCircumcircle( onoff );
    emit editorUpdated();
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::showShootLines( bool onoff )
{
    Options::instance().setFeditShowShootLines( onoff );
    emit editorUpdated();
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::showGoalieMovableArea( bool onoff )
{
    Options::instance().setFeditShowGoalieMovableArea( onoff );
    emit editorUpdated();
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::showBackgroundData( bool onoff )
{
    Options::instance().setFeditShowBackgroundData( onoff );
    emit editorUpdated();
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::validateBallCoordinate()
{
    QLineEdit * editor = qobject_cast< QLineEdit * >( sender() );
    bool ok = false;
    double value = editor->text().toDouble( &ok );
    if ( ok )
    {
        if ( value != 0.0
             && std::fabs( value ) < 0.5 )
        {
            value = rint( value * 2.0 ) * 0.5;
            editor->setText( QString::number( value, 'f', 2 ) );
        }
    }
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::applyToField()
{
    if ( ! checkConsistency() )
    {
        QMessageBox::warning( this,
                              tr( "Error" ),
                              tr( "Inconsistency detected." ),
                              QMessageBox::Ok,
                              QMessageBox::NoButton );
        return;
    }

    std::shared_ptr< FormationEditData > ptr = M_main_data.formationEditData();

    if ( ! ptr )
    {
        std::cerr << "(FormationEditorWindow::applyToField) no data" << std::endl;
        return;
    }

    const bool data_auto_select = Options::instance().feditDataAutoSelect();
    const bool player_auto_move = Options::instance().feditPlayerAutoMove();
    const bool pair_mode = Options::instance().feditPairMode();
    const bool snap_mode = Options::instance().feditSnapMode();
    Options::instance().setFeditDataAutoSelect( false );
    Options::instance().setFeditPlayerAutoMove( false );
    Options::instance().setFeditPairMode( false );
    Options::instance().setFeditSnapMode( false );

    // ball
    {
        bool ok_x = false;
        bool ok_y = false;
        double x = M_ball_x->text().toDouble( &ok_x );
        double y = M_ball_y->text().toDouble( &ok_y );
        if ( ok_x && ok_y )
        {
            ptr->moveBallTo( x, y );
        }
    }

    // players
    for ( int unum = 1; unum <= 11; ++unum )
    {
        bool ok = false;
        const int paired_number = M_paired_number[unum-1]->text().toInt( &ok );
        if ( ! ok )
        {
            std::cerr << "(FormationEditorWindow::applyToField) Invalid pair number." << std::endl;
            continue;
        }

        std::string role_name = M_role_name[unum-1]->text().toStdString();
        if ( role_name.empty() )
        {
            std::cerr << "(FormationEditorWindow::applyToField) ERROR: Empty role name." << std::endl;
        }
        else
        {
            ptr->updateRoleData( unum, paired_number, role_name );
        }

        bool ok_x = false;
        bool ok_y = false;
        double x = M_player_x[unum-1]->text().toDouble( &ok_x );
        double y = M_player_y[unum-1]->text().toDouble( &ok_y );
        if ( ok_x && ok_y )
        {
            ptr->movePlayerTo( unum, x, y );
        }

        ptr->updateRoleType( unum,
                             M_role_type[unum-1]->currentIndex(),
                             M_role_side[unum-1]->currentIndex() );
    }

    // updatePanel();

    Options::instance().setFeditDataAutoSelect( data_auto_select );
    Options::instance().setFeditPlayerAutoMove( player_auto_move );
    Options::instance().setFeditPairMode( pair_mode );
    Options::instance().setFeditSnapMode( snap_mode );

    emit editorUpdated();
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::resetPanelChanges()
{
    updatePanel();
}

/*-------------------------------------------------------------------*/
// void
// FormationEditorWindow::editMenuAboutToShow()
// {
// }

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::selectData( int index )
{
    std::shared_ptr< FormationEditData > ptr = M_main_data.formationEditData();
    if ( ! ptr
         || ! ptr->formationData() )
    {
        return;
    }

    if ( ptr->currentIndex() != index
         && ptr->setCurrentIndex( index ) )
    {
        updateDataIndex();
        updatePanel();
        emit editorUpdated();
    }
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::slotIndexChanged( int value )
{
    selectData( value - 1 );
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::updateDataIndex()
{
    std::shared_ptr< FormationEditData > ptr = M_main_data.formationEditData();

    if ( ! ptr
         || ! ptr->formationData() )
    {
        return;
    }

    const int index = ptr->currentIndex();

    if ( 0 <= index )
    {
        M_index_spin_box->setValue( index + 1 );
        M_tree_view->selectData( index );
    }
    else
    {
        M_index_spin_box->setValue( 0 );
        M_tree_view->unselectData();
    }
}
