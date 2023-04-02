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
// #include "xpm/chase.xpm"
// #include "xpm/delete.xpm"
// #include "xpm/insert.xpm"
// #include "xpm/new.xpm"
#include "xpm/open.xpm"
// #include "xpm/record.xpm"
// #include "xpm/replace.xpm"
// #include "xpm/reverse.xpm"
#include "xpm/save.xpm"
// #include "xpm/symmetry.xpm"
// #include "xpm/train.xpm"

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

    createWidgets();
    createActions();
    createMenus();
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

}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::createActionsView()
{

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
    QMenu * menu = menuBar()->addMenu( tr( "&Edit" ) );

}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::createMenuView()
{
    QMenu * menu = menuBar()->addMenu( tr( "&View" ) );

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
void
FormationEditorWindow::setPlayerAutoMove( bool onoff )
{
    Options::instance().setFeditPlayerAutoMove( onoff );
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::setDataAutoSelect( bool on )
{

}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::setPairMode( bool on )
{

}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::addData()
{

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
FormationEditorWindow::setShowIndex( bool onoff )
{

}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::setShowFreeKickCircle( bool onoff )
{

}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::setShowTriangulation( bool onoff )
{

}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::setShowCircumcircle( bool onoff )
{

}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::setShowShootLines( bool onoff )
{

}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::setShowGoalieMovableArea( bool onoff )
{
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::setShowBackgroundData( bool onoff )
{

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
