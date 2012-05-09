// -*-c++-*-

/*!
  \file chain_selector.h
  \brief action chain data selector widget Header File.
*/

/*
 *Copyright:

 Copyright (C) Hiroki SHIMORA

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

#ifndef SOCCERWINDOW2_CHAIN_SELECTOR_H
#define SOCCERWINDOW2_CHAIN_SELECTOR_H

#include <QFrame>

class MainData;

class QDialog;
class QTextEdit;
class QLineEdit;
class QPushButton;
class QListWidget;
class QShowEvent;


/*!
  \class ChainSelector
  \brief action chain data selector widget
*/
class ChainSelector
    : public QFrame {

    Q_OBJECT

private:
    const MainData & M_main_data;

private:
    QDialog * M_text_dialog;
    QTextEdit * M_text;

    QListWidget * M_list;

    std::vector< QLineEdit * > M_unum_filter;
    QPushButton * M_clear_unum_filter_button;

public:

    ChainSelector( QWidget * parent,
                   const MainData & main_data );
    ~ChainSelector();

protected:

    void showEvent( QShowEvent * event );

private slots:

    void updateChain();
    void clearUnumFilter();

    void toggleShowTextWindow();

signals:
    void configured();
};

#endif
