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
#include "formation_edit_data.h"

#include <iostream>

#include "xpm/fedit2.xpm"
#include "xpm/chase.xpm"
#include "xpm/delete.xpm"
#include "xpm/insert.xpm"
// #include "xpm/new.xpm"
#include "xpm/open.xpm"
#include "xpm/record.xpm"
#include "xpm/replace.xpm"
#include "xpm/reverse.xpm"
#include "xpm/save.xpm"
#include "xpm/symmetry.xpm"
#include "xpm/train.xpm"

// using namespace rcsc;

const QString FormationEditorWindow::ROLE_CENTER( "C" );
const QString FormationEditorWindow::ROLE_LEFT( "L" );
const QString FormationEditorWindow::ROLE_RIGHT( "R" );

/*-------------------------------------------------------------------*/
FormationEditorWindow::FormationEditorWindow( QWidget * parent )
    : QMainWindow( parent )
{
    this->setWindowIcon( QIcon( QPixmap( fedit2_xpm ) ) );
    this->setWindowTitle( tr( "Formation Editor" ) );
    this->setMinimumSize( 280, 220 );
    this->resize( 640, 480 );

    this->setWindowFlags( this->windowFlags() | Qt::WindowStaysOnTopHint );

    M_tool_bar = new QToolBar( tr( "Edit tools" ), this );
    this->addToolBar( Qt::TopToolBarArea, M_tool_bar );

    createWidgets();
    createActions();
    createMenus();

    addToolBarActions();
    this->statusBar()->showMessage( tr( "Ready" ) );
}

/*-------------------------------------------------------------------*/
FormationEditorWindow::~FormationEditorWindow()
{
    std::cerr << "delete FormationEditorWindow" << std::endl;
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::setEditData( std::shared_ptr< FormationEditData > data )
{
    M_edit_data = data;
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
    M_open_data_act->setStatusTip( tr( "Open the training data file." ) );
    connect( M_open_data_act, SIGNAL( triggered() ), this, SLOT( openData() ) );
    //
    M_save_act = new QAction( QIcon( QPixmap( save_xpm ) ),
                              tr( "Save formation" ),
                              this );
    M_save_act->setShortcut( Qt::CTRL + Qt::Key_S );
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
    M_toggle_player_auto_move_act->setChecked( true );
    this->addAction( M_toggle_player_auto_move_act );

    //
    M_toggle_data_auto_select_act = new QAction( tr( "Data Auto Select" ),
                                                 this );
    M_toggle_data_auto_select_act->setStatusTip( tr( "Toggle data is automatically select or not when ball is moved." ) );
    // connect( M_toggle_data_auto_select_act, SIGNAL( toggled( bool ) ),
    //          this, SLOT( setDataAutoSelect( bool ) ) );
    connect( M_toggle_data_auto_select_act, &QAction::toggled,
             []( bool onoff )
               {
                   Options::instance().setFeditDataAutoSelect( onoff );
               } );
    M_toggle_data_auto_select_act->setCheckable( true );
    M_toggle_data_auto_select_act->setChecked( true );
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
    M_toggle_pair_mode_act->setChecked( true );
    this->addAction( M_toggle_pair_mode_act );

    //
    M_add_data_act = new QAction( QIcon( QPixmap( record_xpm ) ),
                                  tr( "Add Data" ),
                                  this );
    M_add_data_act->setStatusTip( tr( "Add the current field state as a new data." ) );
    connect( M_add_data_act, SIGNAL( triggered() ), this, SLOT( addData() ) );
    this->addAction( M_add_data_act );

    //
    M_insert_data_act = new QAction( QIcon( QPixmap( insert_xpm ) ),
                                     tr( "Insert Data" ),
                                     this );
    M_insert_data_act->setStatusTip( tr( "Insert the field state as a new data after the current index." ) );
    connect( M_insert_data_act, SIGNAL( triggered() ), this, SLOT( insertData() ) );
    this->addAction( M_insert_data_act );

    //
    M_replace_data_act = new QAction( QIcon( QPixmap( replace_xpm ) ),
                                      tr( "Replace Data" ),
                                      this );
    M_replace_data_act->setStatusTip( tr( "Replace the current index data by the field state." ) );
    connect( M_replace_data_act, SIGNAL( triggered() ), this, SLOT( replaceData() ) );
    this->addAction( M_replace_data_act );

    //
    M_delete_data_act = new QAction( QIcon( QPixmap( delete_xpm ) ),
                                     tr( "Delete Data" ),
                                     this );
    //M_delete_data_act->setShortcut( Qt::Key_Delete );
    M_delete_data_act->setStatusTip( tr( "Delete the current index data." ) );
    connect( M_delete_data_act, SIGNAL( triggered() ), this, SLOT( deleteData() ) );
    this->addAction( M_delete_data_act );

    //
    M_reverse_y_act = new QAction( QIcon( QPixmap( reverse_xpm ) ),
                                   tr( "ReverseY" ),
                                   this );
    M_reverse_y_act->setStatusTip( tr( "Reverse Y positions." ) );
    connect( M_reverse_y_act, SIGNAL( triggered() ), this, SLOT( reverseY() ) );
    this->addAction( M_reverse_y_act );

    //
    M_fit_model_act = new QAction( QIcon( QPixmap( train_xpm ) ),
                                   tr( "Fit to Data" ),
                                   this );
    M_fit_model_act->setStatusTip( tr( "Train the formation model using the current trainig data set." ) );
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
    M_show_shoot_lines_act->setShortcut( Qt::Key_S );
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

    menu->addAction( tr( "Close" ), this, SLOT( close() ), Qt::CTRL + Qt::Key_W );

}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::createMenuEdit()
{
    //QMenu * menu = menuBar()->addMenu( tr( "&Edit" ) );

}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::createMenuView()
{
    //QMenu * menu = menuBar()->addMenu( tr( "&View" ) );

}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::createWidgets()
{
    M_splitter = new QSplitter();
    this->setCentralWidget( M_splitter );

    M_splitter->addWidget( createInputPanel() );

    //M_splitter->setStretchFactor( 0, 0 );
    //M_splitter->setStretchFactor( 1, 1 );
    M_splitter->setEnabled( false );
}

/*-------------------------------------------------------------------*/
QWidget *
FormationEditorWindow::createInputPanel()
{
    QFrame * top_frame = new QFrame();

    QVBoxLayout * top_vbox = new QVBoxLayout();
    top_vbox->setMargin( 2 );
    top_vbox->setSpacing( 2 );
    top_vbox->setSizeConstraint( QLayout::SetFixedSize );

    top_frame->setLayout( top_vbox );

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
                     this, SLOT( resetChanges() ) );
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

    return top_frame;
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::addToolBarActions()
{
    M_tool_bar->addAction( M_save_act );
    M_tool_bar->addAction( M_toggle_player_auto_move_act );
    M_tool_bar->addAction( M_toggle_pair_mode_act );
    //M_tool_bar->addAction( M_toggle_constraint_edit_mode_act );

    M_tool_bar->addSeparator();

    M_tool_bar->addAction( M_delete_data_act );
    M_tool_bar->addAction( M_reverse_y_act );

    //

    M_tool_bar->addSeparator();

    M_tool_bar->addAction( M_fit_model_act );
    M_tool_bar->addAction( M_insert_data_act );
    M_tool_bar->addAction( M_replace_data_act );
    M_tool_bar->addAction( M_add_data_act );

    //
    {
        QFrame * dummy_frame = new QFrame();
        QHBoxLayout * layout = new QHBoxLayout();
        layout->addStretch( 1 );
        dummy_frame->setLayout( layout );
        M_tool_bar->addWidget( dummy_frame );
    }

    //
    M_index_spin_box = new QSpinBox();
    //M_index_spin_box->setPrefix( tr( "Index: " ) );
    M_index_spin_box->setRange( 0, 0 );
    M_index_spin_box->setWrapping( true );
    M_index_spin_box->setMaximumSize( 80, 24 );
    connect( M_index_spin_box, SIGNAL( valueChanged( int ) ),
             this, SLOT( selectSampleVisualIndex( int ) ) );
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
void
FormationEditorWindow::showEvent( QShowEvent * event )
{
    QMainWindow::showEvent( event );

    updateView();
}

/*-------------------------------------------------------------------*/
// void
// FormationEditorWindow::closeEvent( QCloseEvent * event )
// {
//     event->accept();
//     //emit shown( false );
// }

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::updateView()
{
   if ( ! this->isVisible() )
    {
        return;
    }

    std::shared_ptr< FormationEditData > ptr = M_edit_data.lock();
    if ( ! ptr )
    {
        return;
    }

    rcsc::Formation::ConstPtr f = ptr->formation();
    if ( ! f )
    {
        std::cerr << "FormationEditorWindow: no formation." << std::endl;
        return;
    }

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

}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::openConf()
{

}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::openBackgroundConf()
{

}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::saveConf()
{

}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::saveConfAs()
{

}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::openData()
{

}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::saveDataAs()
{

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
FormationEditorWindow::addData()
{
    std::cerr << "(FormationEditorWindow::addData)" << std::endl;

    std::shared_ptr< FormationEditData > ptr = M_edit_data.lock();
    if ( ! ptr
         || ! ptr->formationData() )
    {
        return;
    }

    const std::string err = ptr->addData();
    if ( ! err.empty() )
    {
        showWarningMessage( err );
        return;
    }

    const int data_count = ptr->formationData()->dataCont().size();
    M_index_spin_box->setRange( 0, data_count );

    updateDataIndex();
    // M_edit_canvas->update(); // emit viewUpdated();
    // M_sample_view->updateData();
    // M_constraint_view->updateData();
    emit editorUpdated();
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::insertData()
{

}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::replaceData()
{

}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::deleteData()
{

}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::changeSampleIndex( int old_visual_index,
                                          int new_visual_index )
{

}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::reverseY()
{

}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::fitModel()
{

}


/*-------------------------------------------------------------------*/
void
FormationEditorWindow::deleteSample( int index )
{

}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::replaceBall( int index,
                                    double x,
                                    double y )
{

}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::replacePlayer( int index,
                                      int unum,
                                      double x,
                                      double y )
{

}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::showIndex( bool onoff )
{
    Options::instance().setFeditShowIndex( onoff );
    //M_edit_canvas->update(); // emit viewUpdated();
    emit editorUpdated();
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::showFreeKickCircle( bool onoff )
{
    Options::instance().setFeditShowFreeKickCircle( onoff );
    // M_edit_canvas->update(); // emit viewUpdated();
    emit editorUpdated();
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::showTriangulation( bool onoff )
{
    Options::instance().setFeditShowTriangulation( onoff );
    // M_edit_canvas->update(); // emit viewUpdated();
    emit editorUpdated();
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::showCircumcircle( bool onoff )
{
    Options::instance().setFeditShowCircumcircle( onoff );
    // M_edit_canvas->update(); // emit viewUpdated();
    emit editorUpdated();
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::showShootLines( bool onoff )
{
    Options::instance().setFeditShowShootLines( onoff );
    // M_edit_canvas->update(); // emit viewUpdated();
    emit editorUpdated();
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::showGoalieMovableArea( bool onoff )
{
    Options::instance().setFeditShowGoalieMovableArea( onoff );
    // M_edit_canvas->update(); // emit viewUpdated();
    emit editorUpdated();
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::showBackgroundData( bool onoff )
{
    Options::instance().setFeditShowBackgroundData( onoff );
    // M_edit_canvas->update();
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

    std::shared_ptr< FormationEditData > ptr = M_edit_data.lock();
    if ( ! ptr )
    {
        std::cerr << "(FormationEditorWindow::applyToField) no data" << std::endl;
        return;
    }

    bool data_auto_select = Options::instance().feditDataAutoSelect();
    bool player_auto_move = Options::instance().feditPlayerAutoMove();
    bool pair_mode = Options::instance().feditPairMode();
    Options::instance().setFeditDataAutoSelect( false );
    Options::instance().setFeditPlayerAutoMove( false );
    Options::instance().setFeditPairMode( false );

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

    // updateView();

    Options::instance().setFeditDataAutoSelect( data_auto_select );
    Options::instance().setFeditPlayerAutoMove( player_auto_move );
    Options::instance().setFeditPairMode( pair_mode );

    emit editorUpdated();
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::resetChanges()
{
    updateView();
}

/*-------------------------------------------------------------------*/
// void
// FormationEditorWindow::editMenuAboutToShow()
// {
// }

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::selectSample( int index )
{

}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::selectSampleVisualIndex( int value )
{

}


/*-------------------------------------------------------------------*/
void
FormationEditorWindow::updateDataIndex()
{

}
