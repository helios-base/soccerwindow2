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


#include <functional>
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

    int font_width = this->fontMetrics().width( tr( "Debug Unknown Opponent" ) );
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
    using std::placeholders::_1;

    DrawConfig * d = &DrawConfig::instance();

    addEntry( tr( "Score Board" ),
              d->scoreBoardBrush().color(),
              std::bind( &DrawConfig::setScoreBoardColor, d, _1 ) );
    addEntry( tr( "Score Board Font" ),
              d->scoreBoardFontPen().color(),
              std::bind( &DrawConfig::setScoreBoardFontColor, d, _1 ) );
    addEntry( tr( "Field" ),
              d->fieldBrush().color(),
              std::bind( &DrawConfig::setFieldColor, d, _1 ) );
    addEntry( tr( "Field Dark" ),
              d->fieldDarkBrush().color(),
              std::bind( &DrawConfig::setFieldDarkColor, d, _1 ) );
    addEntry( tr( "Line" ),
              d->linePen().color(),
              std::bind( &DrawConfig::setLineColor, d, _1 ) );
    addEntry( tr( "Ball" ),
              d->ballBrush().color(),
              std::bind( &DrawConfig::setBallColor, d, _1 ) );
    addEntry( tr( "Measure" ),
              d->measurePen().color(),
              std::bind( &DrawConfig::setMeasureColor, d, _1 ) );
    addEntry( tr( "Measure Font" ),
              d->measureFontPen().color(),
              std::bind( &DrawConfig::setMeasureFontColor, d, _1 ) );
    //
    addEntry( tr( "Left Team" ),
              d->leftTeamPen().color(),
              std::bind( &DrawConfig::setLeftTeamColor, d, _1 ) );
    addEntry( tr( "Left Goalie" ),
              d->leftGoaliePen().color(),
              std::bind( &DrawConfig::setLeftGoalieColor, d, _1 ) );
    addEntry( tr( "Right Team" ),
              d->rightTeamPen().color(),
              std::bind( &DrawConfig::setRightTeamColor, d, _1 ) );
    addEntry( tr( "Right Goalie" ),
              d->rightGoaliePen().color(),
              std::bind( &DrawConfig::setRightGoalieColor, d, _1 ) );
    addEntry( tr( "Player Number Font" ),
              d->playerNumberFontPen().color(),
              std::bind( &DrawConfig::setPlayerNumberFontColor, d, _1 ) );
    addEntry( tr( "Stamina Font" ),
              d->playerStaminaFontPen().color(),
              std::bind( &DrawConfig::setPlayerStaminaFontColor, d, _1 ) );
    addEntry( tr( "Player Detail Font" ),
              d->playerDetailFontPen().color(),
              std::bind( &DrawConfig::setPlayerDetailFontColor, d, _1 ) );
    addEntry( tr( "Selected Player Font" ),
              d->selectedPlayerFontPen().color(),
              std::bind( &DrawConfig::setSelectedPlayerFontColor, d, _1 ) );
    addEntry( tr( "Effort Decayed" ),
              d->effortDecayedPen().color(),
              std::bind( &DrawConfig::setEffortDecayedColor, d, _1 ) );
    addEntry( tr( "Recovery Decayed" ),
              d->recoveryDecayedPen().color(),
              std::bind( &DrawConfig::setRecoveryDecayedColor, d, _1 ) );
    addEntry( tr( "View Cone" ),
              d->viewConePen().color(),
              std::bind( &DrawConfig::setViewConeColor, d, _1 ) );
    addEntry( tr( "View Area" ),
              d->viewAreaPen().color(),
              std::bind( &DrawConfig::setViewAreaColor, d, _1 ) );
    //
    addEntry( tr( "Kick Pen" ),
              d->kickPen().color(),
              std::bind( &DrawConfig::setKickColor, d, _1 ) );
    addEntry( tr( "Kick Fault" ),
              d->kickFaultPen().color(),
              std::bind( &DrawConfig::setKickFaultColor, d, _1 ) );
    addEntry( tr( "Catch" ),
              d->catchBrush().color(),
              std::bind( &DrawConfig::setCatchColor, d, _1 ) );
    addEntry( tr( "Catch Fault" ),
              d->catchFaultBrush().color(),
              std::bind( &DrawConfig::setCatchFaultColor, d, _1 ) );
    addEntry( tr( "Ball Collision" ),
              d->ballCollisionBrush().color(),
              std::bind( &DrawConfig::setBallCollisionColor, d, _1 ) );
    addEntry( tr( "Player Collision" ),
              d->playerCollisionBrush().color(),
              std::bind( &DrawConfig::setPlayerCollisionColor, d, _1 ) );
    addEntry( tr( "Tackle" ),
              d->tacklePen().color(),
              std::bind( &DrawConfig::setTackleColor, d, _1 ) );
    addEntry( tr( "Tackle Fault" ),
              d->tackleFaultBrush().color(),
              std::bind( &DrawConfig::setTackleFaultColor, d, _1 ) );
    addEntry( tr( "Foul Charged" ),
              d->foulChargedBrush().color(),
              std::bind( &DrawConfig::setFoulChargedColor, d, _1 ) );
    addEntry( tr( "PointTo" ),
              d->pointtoPen().color(),
              std::bind( &DrawConfig::setPointtoColor, d, _1 ) );
    addEntry( tr( "Kick Accel" ),
              d->kickAccelPen().color(),
              std::bind( &DrawConfig::setKickAccelColor, d, _1 ) );
    addEntry( tr( "Tackle Area" ),
              d->tackleAreaPen().color(),
              std::bind( &DrawConfig::setTackleAreaColor, d, _1 ) );
    //
    addEntry( tr( "Debug Ball" ),
              d->debugBallBrush().color(),
              std::bind( &DrawConfig::setDebugBallColor, d, _1 ) );
    addEntry( tr( "Debug Ball Trace" ),
              d->debugBallTracePen().color(),
              std::bind( &DrawConfig::setDebugBallTraceColor, d, _1 ) );
    addEntry( tr( "Debug Player Pen" ),
              d->debugPlayerPen().color(),
              std::bind( &DrawConfig::setDebugPlayerPenColor, d, _1 ) );
    addEntry( tr( "Debug Self" ),
              d->debugSelfBrush().color(),
              std::bind( &DrawConfig::setDebugSelfColor, d, _1 ) );
    addEntry( tr( "Debug Teammate" ),
              d->debugTeammateBrush().color(),
              std::bind( &DrawConfig::setDebugTeammateColor, d, _1 ) );
    addEntry( tr( "Debug Opponent" ),
              d->debugOpponentBrush().color(),
              std::bind( &DrawConfig::setDebugOpponentColor, d, _1 ) );
    addEntry( tr( "Debug Unknown Teammate" ),
              d->debugUnknownTeammateBrush().color(),
              std::bind( &DrawConfig::setDebugUnknownTeammateColor, d, _1 ) );
    addEntry( tr( "Debug Unknown Opponent" ),
              d->debugUnknownOpponentBrush().color(),
              std::bind( &DrawConfig::setDebugUnknownOpponentColor, d, _1 ) );
    addEntry( tr( "Debug Unknown Player" ),
              d->debugUnknownPlayerBrush().color(),
              std::bind( &DrawConfig::setDebugUnknownPlayerColor, d, _1 ) );

    addEntry( tr( "Debug Pointto" ),
              d->debugPointtoPen().color(),
              std::bind( &DrawConfig::setDebugPointtoColor, d, _1 ) );
    addEntry( tr( "Debug Target" ),
              d->debugTargetPen().color(),
              std::bind( &DrawConfig::setDebugTargetColor, d, _1 ) );
    addEntry( tr( "Debug Comment" ),
              d->debugCommentFontPen().color(),
              std::bind( &DrawConfig::setDebugCommentColor, d, _1 ) );
    addEntry( tr( "Debug Message" ),
              d->debugMessageFontPen().color(),
              std::bind( &DrawConfig::setDebugMessageColor, d, _1 ) );
    addEntry( tr( "Debug Log Font" ),
              d->debugLogMessageFontPen().color(),
              std::bind( &DrawConfig::setDebugLogMessageColor, d, _1 ) );
    addEntry( tr( "Debug Shapes" ),
              d->debugShapePen().color(),
              std::bind( &DrawConfig::setDebugShapeColor, d, _1 ) );
    addEntry( tr( "Debug Action Sequence" ),
              d->debugActionSequencePen().color(),
              std::bind( &DrawConfig::setDebugActionSequenceColor, d, _1 ) );
    addEntry( tr( "Debug Action Pass" ),
              d->debugActionPassPen().color(),
              std::bind( &DrawConfig::setDebugActionPassColor, d, _1 ) );
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
    int text_width = fm.width( tr( "Cancel" ) ) + 8;

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
