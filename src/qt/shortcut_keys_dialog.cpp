// -*-c++-*-

/*!
  \file shortcut_keys_dialog.cpp
  \brief shortcut keys setting dialog Source File.
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

#include "shortcut_keys_dialog.h"

#include <iostream>

namespace {
enum {
    TEXT_COLUMN = 0,
    KEYS_COLUMN,
    MAX_COLUMN,
};
}

/*-------------------------------------------------------------------*/
ShortcutKeysDialog::ShortcutKeysDialog( QList< QAction * > actions,
                                        QWidget * parent )
    : QDialog( parent )
{
    this->setWindowTitle( tr( "Shortcut Keys" ) );

    M_actions = actions;

    createWidgets();
}


/*-------------------------------------------------------------------*/
ShortcutKeysDialog::~ShortcutKeysDialog()
{
    std::cerr << "delete ShortcutKeysDialog" << std::endl;
}

/*-------------------------------------------------------------------*/
void
ShortcutKeysDialog::createWidgets()
{
    QVBoxLayout * main_layout = new QVBoxLayout( this );
    //main_layout->setContentsMargins( 2, 2, 2, 2 );

    main_layout->addWidget( createShortcutKeysView() );

    main_layout->addSpacing( 5 );

    main_layout->addWidget( createEditor() );

    this->setLayout( main_layout );
}

/*-------------------------------------------------------------------*/
QWidget *
ShortcutKeysDialog::createShortcutKeysView()
{
    M_view = new QTreeWidget();
    M_view->setSortingEnabled( true );
    M_view->setSelectionBehavior( QAbstractItemView::SelectRows );
    M_view->setSelectionMode( QAbstractItemView::SingleSelection );
    M_view->setSortingEnabled( true );
    M_view->setAlternatingRowColors( true );
    M_view->setAutoScroll( true );
    M_view->setEditTriggers( QAbstractItemView::NoEditTriggers );
    M_view->setIndentation( 0 );

    M_view->header()->setSectionsMovable( false );

    {
        QTreeWidgetItem * h = M_view->headerItem();
        h->setText( TEXT_COLUMN, tr( "Command" ) );
        h->setText( KEYS_COLUMN, tr( "Shortcut" ) );
    }

    const QFontMetrics metrics = this->fontMetrics();
    int text_width = 0;
    int keys_width = 0;
    for ( QAction * act : M_actions )
    {
        QTreeWidgetItem * item = new QTreeWidgetItem();
        const QString shortcut = act->shortcut().toString();

        text_width = std::max( text_width, metrics.width( act->text() ) );
        keys_width = std::max( text_width, metrics.width( shortcut ) );

        item->setText( TEXT_COLUMN, act->text() );
        item->setText( KEYS_COLUMN, shortcut );

        // The default value for flags is Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled.
        item->setFlags( item->flags() & ~Qt::ItemIsEditable & ~Qt::ItemIsDropEnabled );

        M_view->addTopLevelItem( item );
    }

    M_view->setColumnWidth( TEXT_COLUMN, text_width + 16 );
    M_view->setColumnWidth( KEYS_COLUMN, keys_width + 16 );


    connect( M_view, SIGNAL( itemSelectionChanged() ),
             this, SLOT( activateEditor() ) );

    return M_view;
}

/*-------------------------------------------------------------------*/
QWidget *
ShortcutKeysDialog::createEditor()
{
    M_editor_group = new QGroupBox( tr( "Shortcut Editor" ) );

    QGridLayout * top_layout = new QGridLayout();
    //top_layout->setContentsMargins( 1, 1, 1, 1 );

    top_layout->addWidget( new QLabel( tr( "Command:" ) ), 0, 0, Qt::AlignLeft );
    top_layout->addWidget( new QLabel( tr( "Shortcut:" ) ), 1, 0, Qt::AlignLeft );

    M_command_label = new QLabel();
    top_layout->addWidget( M_command_label, 0, 1, Qt::AlignLeft );

    M_key_sequence_edit = new QKeySequenceEdit();
    top_layout->addWidget( M_key_sequence_edit, 1, 1, Qt::AlignLeft );

    // connect( M_key_sequence_edit, SIGNAL( editingFinished() ),
    //          this, SLOT( checkConflict() ) );

    QPushButton * apply_button = new QPushButton( tr( "Apply" ) );
    top_layout->addWidget( apply_button, 1, 2 );

    connect( apply_button, SIGNAL( clicked() ),
             this, SLOT( checkAndUpdate() ) );

    QPushButton * reset_button = new QPushButton( tr( "Reset" ) );
    top_layout->addWidget( reset_button, 1, 3 );

    connect( reset_button, SIGNAL( clicked() ),
             this, SLOT( reset() ) );

    M_editor_group->setLayout( top_layout );
    M_editor_group->setEnabled( false );

    return M_editor_group;
}

/*-------------------------------------------------------------------*/
void
ShortcutKeysDialog::activateEditor()
{
    const QTreeWidgetItem * item = M_view->currentItem();
    if ( ! item )
    {
        M_command_label->clear();
        M_key_sequence_edit->clear();

        M_editor_group->setEnabled( false );
        return;
    }

    M_command_label->setText( item->text( TEXT_COLUMN ) );
    M_key_sequence_edit->setKeySequence( item->text( KEYS_COLUMN ) );

    M_editor_group->setEnabled( true );
}

/*-------------------------------------------------------------------*/
void
ShortcutKeysDialog::checkAndUpdate()
{
    const QKeySequence new_sequence = M_key_sequence_edit->keySequence();
    QAction * edited_act = nullptr;

    for ( QAction * act : M_actions )
    {
        if ( act->text() == M_command_label->text() )
        {
            edited_act = act;
            continue;
        }

        if ( act->shortcut() == new_sequence )
        {
            QMessageBox::warning( this, tr( "Shortcut Conflict" ),
                                  tr( "The new sequence conflicts with [" )
                                  + act->text()
                                  + tr( "]" ) );
            return;
        }
    }

    if ( edited_act )
    {
        QList< QTreeWidgetItem * > items = M_view->findItems( edited_act->text(),
                                                              Qt::MatchFixedString | Qt::MatchCaseSensitive,
                                                              0 );
        if ( items.empty() )
        {
            QMessageBox::warning( this, tr( "Not Found" ),
                                  tr( "Not found in the table." ) );
            return;
        }

        if ( items.size() > 1 )
        {
            QMessageBox::warning( this, tr( "Unexpected Result" ),
                                  tr( "Found several candidates." ) );
            return;
        }

        items.front()->setText( KEYS_COLUMN, new_sequence.toString() );
        edited_act->setShortcut( new_sequence );
    }
}

/*-------------------------------------------------------------------*/
void
ShortcutKeysDialog::reset()
{
    M_command_label->clear();
    M_key_sequence_edit->clear();

    M_view->clearSelection();
    M_editor_group->setEnabled( false );
}
