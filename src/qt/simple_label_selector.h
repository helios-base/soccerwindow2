// -*-c++-*-

/*!
  \file simple_label_editor_window.h
  \brief cycle label editor window Header File.
*/

/*
 *Copyright:

 Copyright (C) Hidehisa Akiyama

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

#ifndef SIMPLE_LABEL_SELECTOR_H
#define SIMPLE_LABEL_SELECTOR_H

#include <QMainWindow>

#include <rcsc/game_time.h>

class QAction;
class QStandardItemModel;
class QTableView;
class QPushButton;

class MainData;

class SimpleLabelSelector
    : public QMainWindow {

    Q_OBJECT

private:

    const MainData & M_main_data;
    bool M_init;

    QToolBar * M_tool_bar;

    QTableView * M_time_label_view;
    QStandardItemModel * M_model;

    QAction * M_save_csv_act;

    // not used
    SimpleLabelSelector() = delete;
    SimpleLabelSelector( const SimpleLabelSelector & ) = delete;
    SimpleLabelSelector & operator=( const SimpleLabelSelector & ) = delete;

public:

    SimpleLabelSelector( MainData & main_data,
                         QWidget * parent );

    ~SimpleLabelSelector();

protected:
    void showEvent( QShowEvent * event );

private:

    void createView();

    void createActions();
    void createMenus();
    void createToolBars();

    void updateData();

    bool saveCSV( const QString & filepath );
    bool openCSV( const QString & filepath );

public:
    void clearAll();


private slots:
    void saveCSV();
    void openCSV();
    void selectLabel( const int label );
    void onTableSelectionChanged( const QModelIndex & index,
                                  const QModelIndex & previous );

signals:

    void cycleSelected( const rcsc::GameTime & t );

};

#endif
