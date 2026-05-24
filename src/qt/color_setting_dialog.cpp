// -*-c++-*-

/*!
  \file color_setting_dialog.cpp
  \brief color setting dialog for the field canvas Source File.
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

#include "color_setting_dialog.h"
#include "draw_config.h"

#include <iostream>
#include <cassert>

/*-------------------------------------------------------------------*/
/*!

*/
ColorEntry::ColorEntry( const QColor & old_color,
                        Setter setter )
    : old_color_( old_color ),
      color_( old_color ),
      setter_( setter )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
bool
ColorEntry::setColor( const QColor & color )
{
    if ( color_ != color )
    {
        color_ = color;

        if ( setter_ )
        {
            setter_( color );
        }

        return true;
    }

    return false;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
ColorEntry::revert()
{
    return setColor( old_color_ );
}

/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/

/*-------------------------------------------------------------------*/
/*!

*/
ColorSettingDialog::ColorSettingDialog( QWidget * parent )
    : QDialog( parent )
{
    this->setWindowTitle( tr( "Color Settings" ) );

    createControls();
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ColorSettingDialog::createControls()
{
    QVBoxLayout * main_layout = new QVBoxLayout( this );

    main_layout->addLayout( createColorList() );

    main_layout->addSpacing( 5 );

    main_layout->addLayout( createAnswerButtons() );

    this->setLayout( main_layout );
}

/*-------------------------------------------------------------------*/
/*!

*/
ColorSettingDialog::~ColorSettingDialog()
{
    //std::cerr << "delete ColorSettingDialog" << std::endl;
}

/*-------------------------------------------------------------------*/
/*!

*/
QLayout *
ColorSettingDialog::createColorList()
{
    QHBoxLayout * layout = new QHBoxLayout();

    M_list_box = new QListWidget();

    createColorItems();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
    int font_width = this->fontMetrics().horizontalAdvance( tr( "Debug Unknown Opponent" ) );
#else
    int font_width = this->fontMetrics().width( tr( "Debug Unknown Opponent" ) );
#endif
    M_list_box->setMinimumWidth( font_width + 16 + 10 );
    M_list_box->setMinimumHeight( 300 );
    connect( M_list_box, SIGNAL( itemDoubleClicked( QListWidgetItem * ) ),
             this, SLOT( selectEntry( QListWidgetItem * ) ) );

    layout->addWidget( M_list_box );

    QPushButton * select_btn = new QPushButton( tr( "Select" ), this );
    connect( select_btn, SIGNAL( clicked() ),
             this, SLOT( selectEntry() ) );
    layout->addWidget( select_btn );

    return layout;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ColorSettingDialog::createColorItems()
{
    addEntry( tr( "Score Board" ),
              DrawConfig::instance().scoreBoardBrush().color(),
              []( const QColor & col ){ DrawConfig::instance().setScoreBoardColor( col ); } );
    addEntry( tr( "Score Board Font" ),
              DrawConfig::instance().scoreBoardFontPen().color(),
              []( const QColor & col ){ DrawConfig::instance().setScoreBoardFontColor( col ); } );
    addEntry( tr( "Field" ),
              DrawConfig::instance().fieldBrush().color(),
              []( const QColor & col ){ DrawConfig::instance().setFieldColor( col ); } );
    addEntry( tr( "Field Dark" ),
              DrawConfig::instance().fieldDarkBrush().color(),
              []( const QColor & col ){ DrawConfig::instance().setFieldDarkColor( col ); } );
    addEntry( tr( "Line" ),
              DrawConfig::instance().linePen().color(),
              []( const QColor & col ){ DrawConfig::instance().setLineColor( col ); } );
    addEntry( tr( "Ball" ),
              DrawConfig::instance().ballBrush().color(),
              []( const QColor & col ){ DrawConfig::instance().setBallColor( col ); } );
    addEntry( tr( "Measure" ),
              DrawConfig::instance().measurePen().color(),
              []( const QColor & col ){ DrawConfig::instance().setMeasureColor( col ); } );
    addEntry( tr( "Measure Font" ),
              DrawConfig::instance().measureFontPen().color(),
              []( const QColor & col ){ DrawConfig::instance().setMeasureFontColor( col ); } );
    //
    addEntry( tr( "Left Team" ),
              DrawConfig::instance().leftTeamPen().color(),
              []( const QColor & col ){ DrawConfig::instance().setLeftTeamColor( col ); } );
    addEntry( tr( "Left Goalie" ),
              DrawConfig::instance().leftGoaliePen().color(),
              []( const QColor & col ){ DrawConfig::instance().setLeftGoalieColor( col ); } );
    addEntry( tr( "Right Team" ),
              DrawConfig::instance().rightTeamPen().color(),
              []( const QColor & col ){ DrawConfig::instance().setRightTeamColor( col ); } );
    addEntry( tr( "Right Goalie" ),
              DrawConfig::instance().rightGoaliePen().color(),
              []( const QColor & col ){ DrawConfig::instance().setRightGoalieColor( col ); } );
    addEntry( tr( "Player Number Font" ),
              DrawConfig::instance().playerNumberFontPen().color(),
              []( const QColor & col ){ DrawConfig::instance().setPlayerNumberFontColor( col ); } );
    addEntry( tr( "Stamina Font" ),
              DrawConfig::instance().playerStaminaFontPen().color(),
              []( const QColor & col ){ DrawConfig::instance().setPlayerStaminaFontColor( col ); } );
    addEntry( tr( "Player Detail Font" ),
              DrawConfig::instance().playerDetailFontPen().color(),
              []( const QColor & col ){ DrawConfig::instance().setPlayerDetailFontColor( col ); } );
    addEntry( tr( "Selected Player Font" ),
              DrawConfig::instance().selectedPlayerFontPen().color(),
              []( const QColor & col ){ DrawConfig::instance().setSelectedPlayerFontColor( col ); } );
    addEntry( tr( "Effort Decayed" ),
              DrawConfig::instance().effortDecayedPen().color(),
              []( const QColor & col ){ DrawConfig::instance().setEffortDecayedColor( col ); } );
    addEntry( tr( "Recovery Decayed" ),
              DrawConfig::instance().recoveryDecayedPen().color(),
              []( const QColor & col ){ DrawConfig::instance().setRecoveryDecayedColor( col ); } );
    addEntry( tr( "View Cone" ),
              DrawConfig::instance().viewConePen().color(),
              []( const QColor & col ){ DrawConfig::instance().setViewConeColor( col ); } );
    addEntry( tr( "View Area" ),
              DrawConfig::instance().viewAreaPen().color(),
              []( const QColor & col ){ DrawConfig::instance().setViewAreaColor( col ); } );
    //
    addEntry( tr( "Kick Pen" ),
              DrawConfig::instance().kickPen().color(),
              []( const QColor & col ){ DrawConfig::instance().setKickColor( col ); } );
    addEntry( tr( "Kick Fault" ),
              DrawConfig::instance().kickFaultPen().color(),
              []( const QColor & col ){ DrawConfig::instance().setKickFaultColor( col ); } );
    addEntry( tr( "Catch" ),
              DrawConfig::instance().catchBrush().color(),
              []( const QColor & col ){ DrawConfig::instance().setCatchColor( col ); } );
    addEntry( tr( "Catch Fault" ),
              DrawConfig::instance().catchFaultBrush().color(),
              []( const QColor & col ){ DrawConfig::instance().setCatchFaultColor( col ); } );
    addEntry( tr( "Ball Collision" ),
              DrawConfig::instance().ballCollisionBrush().color(),
              []( const QColor & col ){ DrawConfig::instance().setBallCollisionColor( col ); } );
    addEntry( tr( "Player Collision" ),
              DrawConfig::instance().playerCollisionBrush().color(),
              []( const QColor & col ){ DrawConfig::instance().setPlayerCollisionColor( col ); } );
    addEntry( tr( "Tackle" ),
              DrawConfig::instance().tacklePen().color(),
              []( const QColor & col ){ DrawConfig::instance().setTackleColor( col ); } );
    addEntry( tr( "Tackle Fault" ),
              DrawConfig::instance().tackleFaultBrush().color(),
              []( const QColor & col ){ DrawConfig::instance().setTackleFaultColor( col ); } );
    addEntry( tr( "Foul Charged" ),
              DrawConfig::instance().foulChargedBrush().color(),
              []( const QColor & col ){ DrawConfig::instance().setFoulChargedColor( col ); } );
    addEntry( tr( "PointTo" ),
              DrawConfig::instance().pointtoPen().color(),
              []( const QColor & col ){ DrawConfig::instance().setPointtoColor( col ); } );
    addEntry( tr( "Kick Accel" ),
              DrawConfig::instance().kickAccelPen().color(),
              []( const QColor & col ){ DrawConfig::instance().setKickAccelColor( col ); } );
    addEntry( tr( "Tackle Area" ),
              DrawConfig::instance().tackleAreaPen().color(),
              []( const QColor & col ){ DrawConfig::instance().setTackleAreaColor( col ); } );
    //
    addEntry( tr( "Debug Ball" ),
              DrawConfig::instance().debugBallBrush().color(),
              []( const QColor & col ){ DrawConfig::instance().setDebugBallColor( col ); } );
    addEntry( tr( "Debug Ball Trace" ),
              DrawConfig::instance().debugBallTracePen().color(),
              []( const QColor & col ){ DrawConfig::instance().setDebugBallTraceColor( col ); } );
    addEntry( tr( "Debug Player Pen" ),
              DrawConfig::instance().debugPlayerPen().color(),
              []( const QColor & col ){ DrawConfig::instance().setDebugPlayerPenColor( col ); } );
    addEntry( tr( "Debug Self" ),
              DrawConfig::instance().debugSelfBrush().color(),
              []( const QColor & col ){ DrawConfig::instance().setDebugSelfColor( col ); } );
    addEntry( tr( "Debug Teammate" ),
              DrawConfig::instance().debugTeammateBrush().color(),
              []( const QColor & col ){ DrawConfig::instance().setDebugTeammateColor( col ); } );
    addEntry( tr( "Debug Opponent" ),
              DrawConfig::instance().debugOpponentBrush().color(),
              []( const QColor & col ){ DrawConfig::instance().setDebugOpponentColor( col ); } );
    addEntry( tr( "Debug Unknown Teammate" ),
              DrawConfig::instance().debugUnknownTeammateBrush().color(),
              []( const QColor & col ){ DrawConfig::instance().setDebugUnknownTeammateColor( col ); } );
    addEntry( tr( "Debug Unknown Opponent" ),
              DrawConfig::instance().debugUnknownOpponentBrush().color(),
              []( const QColor & col ){ DrawConfig::instance().setDebugUnknownOpponentColor( col ); } );
    addEntry( tr( "Debug Unknown Player" ),
              DrawConfig::instance().debugUnknownPlayerBrush().color(),
              []( const QColor & col ){ DrawConfig::instance().setDebugUnknownPlayerColor( col ); } );

    addEntry( tr( "Debug Pointto" ),
              DrawConfig::instance().debugPointtoPen().color(),
              []( const QColor & col ){ DrawConfig::instance().setDebugPointtoColor( col ); } );
    addEntry( tr( "Debug Target" ),
              DrawConfig::instance().debugTargetPen().color(),
              []( const QColor & col ){ DrawConfig::instance().setDebugTargetColor( col ); } );
    addEntry( tr( "Debug Comment" ),
              DrawConfig::instance().debugCommentFontPen().color(),
              []( const QColor & col ){ DrawConfig::instance().setDebugCommentColor( col ); } );
    addEntry( tr( "Debug Message" ),
              DrawConfig::instance().debugMessageFontPen().color(),
              []( const QColor & col ){ DrawConfig::instance().setDebugMessageColor( col ); } );
    addEntry( tr( "Debug Log Font" ),
              DrawConfig::instance().debugLogMessageFontPen().color(),
              []( const QColor & col ){ DrawConfig::instance().setDebugLogMessageColor( col ); } );
    addEntry( tr( "Debug Shapes" ),
              DrawConfig::instance().debugShapePen().color(),
              []( const QColor & col ){ DrawConfig::instance().setDebugShapeColor( col ); } );
    addEntry( tr( "Debug Action Sequence" ),
              DrawConfig::instance().debugActionSequencePen().color(),
              []( const QColor & col ){ DrawConfig::instance().setDebugActionSequenceColor( col ); } );
    addEntry( tr( "Debug Action Pass" ),
              DrawConfig::instance().debugActionPassPen().color(),
              []( const QColor & col ){ DrawConfig::instance().setDebugActionPassColor( col ); } );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ColorSettingDialog::addEntry( const QString & name,
                              const QColor & old_color,
                              ColorEntry::Setter setter )
{
    QPixmap pixmap( 16, 16 );
    pixmap.fill( old_color );

    M_list_box->addItem( new QListWidgetItem( QIcon( pixmap ), name ) );

    M_color_entries.push_back( ColorEntry( old_color, setter ) );
}

/*-------------------------------------------------------------------*/
/*!

*/
QLayout *
ColorSettingDialog::createAnswerButtons()
{
    QHBoxLayout * layout = new QHBoxLayout();

    QFontMetrics fm = this->fontMetrics();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
    int text_width = fm.horizontalAdvance( tr( "Cancel" ) ) + 8;
#else
    int text_width = fm.width( tr( "Cancel" ) ) + 8;
#endif

    QPushButton * def = new QPushButton( tr( "Default" ) );
    //cancel->resize( text_width, fm.height() );
    connect( def, SIGNAL( clicked() ),
             this, SLOT( setDefaultColors() ) );

    QPushButton * ok = new QPushButton( tr( "OK" ) );
    ok->resize( text_width, fm.height() );
    connect( ok, SIGNAL( clicked() ),
             this, SLOT( accept() ) );

    QPushButton * cancel = new QPushButton( tr( "Cancel" ) );
    cancel->resize( text_width, fm.height() );
    connect( cancel, SIGNAL( clicked() ),
             this, SLOT( reject() ) );


    layout->addWidget( def, 0, Qt::AlignLeft );
    layout->addStretch( 1 );
    layout->addWidget( ok, 0, Qt::AlignRight );
    layout->addWidget( cancel, 0, Qt::AlignRight );

    return layout;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ColorSettingDialog::revert()
{
    bool changed = false;

    for ( int i = 0; i < M_list_box->count(); ++i )
    {
        if ( M_color_entries[i].revert() )
        {
            QListWidgetItem * item = M_list_box->item( i );
            QPixmap pixmap( 16, 16 );
            pixmap.fill( M_color_entries[i].color_ );
            item->setIcon( QIcon( pixmap ) );
            changed = true;
        }
    }

    if ( changed )
    {
        emit colorChanged();
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ColorSettingDialog::setDefaultColors()
{
    DrawConfig::instance().setDefaultColors();

    M_list_box->clearSelection();
    M_list_box->clear();

    createColorItems();

//     bool changed = false;
//     for ( int i = 0; i < M_list_box->count(); ++i )
//     {
//         if ( M_color_entries[i].revert() )
//         {
//             QListWidgetItem * item = M_list_box->item( i );
//             QPixmap pixmap( 16, 16 );
//             pixmap.fill( M_color_entries[i].color_ );
//             item->setIcon( QIcon( pixmap ) );
//             changed = true;
//         }
//     }

//     if ( changed )
    {
        emit colorChanged();
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ColorSettingDialog::selectEntry( int index )
{
    if ( index < 0 )
    {
        return;
    }

    if ( index >= static_cast< int >( M_color_entries.size() ) )
    {
        std::cerr << __FILE__ << ": (selectEntry) Illegal index "
                  << index << std::endl;
        return;
    }

    QColor col = QColorDialog::getColor( M_color_entries[index].color_ );
    if ( ! col.isValid() )
    {
        return;
    }

    if ( M_color_entries[index].setColor( col ) )
    {
        QListWidgetItem * item = M_list_box->item( index );
        QPixmap pixmap( 16, 16 );
        pixmap.fill( col );
        item->setIcon( QIcon( pixmap ) );

        emit colorChanged();
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ColorSettingDialog::selectEntry()
{
    selectEntry( M_list_box->currentRow() );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ColorSettingDialog::selectEntry( QListWidgetItem * item )
{
    selectEntry( M_list_box->row( item ) );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ColorSettingDialog::reject()
{
    // reset all changes

    revert();

    QDialog::reject();
}
