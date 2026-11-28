// -*-c++-*-

/*!
  \file export_dialog.h
  \brief the dialog of the export of images and videos Header File.
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

#ifndef SOCCERWINDOW2_QT_EXPORT_DIALOG_H
#define SOCCERWINDOW2_QT_EXPORT_DIALOG_H

#include "export_range.h"

#include <QDialog>
#include <QPointer>
#include <QSize>
#include <QStringList>

#include <cstddef>
#include <vector>

class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QEvent;
class QLabel;
class QLineEdit;
class QProgressDialog;
class QPushButton;
class QSpinBox;

class ExportJob;
class FieldCanvas;
class MainData;

/*!
  \class ExportDialog
  \brief File > Export Images / Video, ported from rcssmonitor
  (docs/plan/2026-11-28-00_frame-export.md 4.2, S8): the frames of a range
  of the game log as images. Modeless: the range can be taken from the
  position shown while it is open. The start and the end are positions
  (ExportRange: 1234, 1234.3 or 1234,3; empty for the first and the last
  frames of the log). The images are drawn by the field canvas
  (FieldCanvas::renderFrame()), in a size and enlarged as the view or not,
  at a rate (fps and speed: ExportTiming), and named by their numbers or by
  the cycles. Before the export, the files that are there are told once,
  and the disk space is estimated. While exporting, a progress dialog
  (application modal) can stop it.
*/
class ExportDialog
    : public QDialog {

    Q_OBJECT

private:

    MainData & M_main_data;
    FieldCanvas & M_canvas;

    QLineEdit * M_start_edit;
    QLineEdit * M_end_edit;
    QLabel * M_start_info;
    QLabel * M_end_info;

    QComboBox * M_size_combo;
    QSpinBox * M_width_spin;
    QSpinBox * M_height_spin;
    QCheckBox * M_scale_texts_check;

    QDoubleSpinBox * M_fps_spin;
    QDoubleSpinBox * M_speed_spin;

    QLineEdit * M_dir_edit;
    QLineEdit * M_prefix_edit;
    QComboBox * M_format_combo;
    QCheckBox * M_name_by_cycle_check;

    QLabel * M_summary_label;
    QLabel * M_message_label;
    bool M_message_error; //!< the message tells an error (marked again when the palette changes)
    QPushButton * M_export_button;

    bool M_allowed; //!< false while connected to a server (S3)
    QPointer< ExportJob > M_job;
    QPointer< QProgressDialog > M_progress;

public:

    ExportDialog( MainData & main_data,
                  FieldCanvas & canvas,
                  QWidget * parent = nullptr );
    ~ExportDialog() override;

    //! the range of the fields, or an invalid one with the reason
    ExportRange range( QString * error ) const;

    //! the frames one image advances: ExportTiming::framesPerImage() of the fields
    double framesPerImage() const;

    //! the frame of each image of the range
    std::vector< std::size_t > imageFrames( const ExportRange & range ) const;

    //! the size of the images (pixels)
    QSize imageSize() const;

    //! the device pixel ratio of the images: the image looks as the view, enlarged, or 1
    qreal imageRatio() const;

    //! the file names of the images of the frames (numbers, or cycles)
    QStringList imageNames( const std::vector< std::size_t > & frames ) const;

    //! naming by the cycles can be chosen: one frame an image
    bool canNameByCycle() const;

    QString directory() const;

    QString summaryText() const;
    QString messageText() const;
    bool isExporting() const;

public slots:

    void setStart( const QString & text );
    void setEnd( const QString & text );
    void setStartToCurrent();
    void setEndToCurrent();
    //! the whole log: both fields empty
    void clearRange();

    void setDirectory( const QString & dir );

    //! false while connected to a server: the export is not started (S3)
    void setExportAllowed( const bool allowed );

    //! the summary of the range and the images, and whether it can be exported
    void updateSummary();

    /*!
      \brief export with the fields
      \param ask ask before overwriting files and when the disk may not have
      enough space (false: no question, overwrite; for the tests)
      \return true if started
    */
    bool startExport( const bool ask = true );

    void abortExport();

signals:

    //! the playback should stop: the frame shown stays while exporting
    void stopRequested();

    //! the end of an export: the result
    void exportFinished( bool ok,
                         const QString & message );

protected:

    //! the colors of the wrong texts again when the palette (the color scheme) changes
    void changeEvent( QEvent * event ) override;

private:

    void createWidgets();
    void readSettings();
    void writeSettings() const;

    void chooseDirectory();
    void showMessage( const QString & text,
                      const bool error );

    //! the start or the end of a field: the index, or INVALID_INDEX with the reason
    std::size_t positionIndex( const QLineEdit * edit,
                               const bool start,
                               bool * snapped,
                               QString * error ) const;

    void handleFinished( bool ok,
                         const QString & message );
};

#endif
