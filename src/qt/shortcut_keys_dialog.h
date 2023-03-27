// -*-c++-*-

/*!
  \file shortcut_keys_edit_dialog.h
  \brief shortcut keys setting dialog Header File.
*/

/*
 *Copyright:

 Copyright (C) Hidehisa AKIYAMA

 This code is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2, or (at your option)
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

#ifndef SOCCERWINDOW2_SHORTCUT_KEYS_DIALOG_H
#define SOCCERWINDOW2_SHORTCUT_KEYS_DIALOG_H

#include <QDialog>

class QTreeWidget;
class QGroupBox;
class QLabel;
class QKeySequenceEdit;

/*-------------------------------------------------------------------*/
class ShortcutKeysDialog
    : public QDialog {

    Q_OBJECT

private:

    QList< QAction * > M_actions;

    QTreeWidget * M_view;

    QGroupBox * M_editor_group;
    QLabel * M_command_label;
    QKeySequenceEdit * M_key_sequence_edit;

public:

    ShortcutKeysDialog( QList< QAction * > actions,
                        QWidget * parent );

    ~ShortcutKeysDialog();

private:

    void createWidgets();
    QWidget * createShortcutKeysView();
    QWidget * createEditor();


private slots:

    void activateEditor();

    void checkAndUpdate();
    void reset();

};

#endif
