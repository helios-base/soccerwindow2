// -*-c++-*-

/*!
  \file label_editor_window.h
  \brief label editor window Header File.
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

#ifndef LABEL_EDITOR_WINDOW_H
#define LABEL_EDITOR_WINDOW_H

#include <QMainWindow>

class QAction;
//class QStandardItemModel;
//class QTableView;
class QTreeWidget;
class QTreeWidgetItem;
// class QTableWidget;
// class QTableWidgetItem;
class QSplitter;

class MainData;


class LabelEditorWindow
    : public QMainWindow {

    Q_OBJECT

private:

    MainData & M_main_data;

    QToolBar * M_tool_bar;

    QSplitter * M_splitter;

    QTreeWidget * M_time_view;
    QTreeWidget * M_label_view;
    QTreeWidget * M_values_view;

    QAction * M_open_act;
    QAction * M_save_csv_act;

    // not used
    LabelEditorWindow() = delete;
    LabelEditorWindow( const LabelEditorWindow & ) = delete;
    LabelEditorWindow & operator=( const LabelEditorWindow & ) = delete;

public:

    LabelEditorWindow( MainData & main_data,
                       QWidget * parent );

    ~LabelEditorWindow();

private:

    void createWidgets();
    void createTimeView();
    void createLabelView();
    void createValuesView();

    void createActions();
    void createMenus();
    void createMenuFile();
    void createToolBars();

public:
    void clearAll();
    bool saveChanges();

private:
    bool openFile( const QString & filepath );
    bool saveCSV( const QString & filepath );

    void initView();
    void initTimeView();
    void initValuesView();

private slots:
    void openFile();
    void saveCSV();

    void selectTimeItem();

    void updateLabelView();
    void selectLabelItem();
    void slotLabelItemDoubleClicked( QTreeWidgetItem * item,
                                     int column );
    void slotLabelItemChanged( QTreeWidgetItem * item,
                               int column );


    void showFeatureValues( const int index );

signals:

    void cycleChanged( int cycle );
    void featuresLogSelected();

};

#endif
