// -*-c++-*-

/*!
  \file export_settings.h
  \brief the settings of the export Header File.
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

#ifndef SOCCERWINDOW2_QT_EXPORT_SETTINGS_H
#define SOCCERWINDOW2_QT_EXPORT_SETTINGS_H

#include <QString>

/*!
  \struct ExportSettings
  \brief the settings of File > Export Images / Video, kept in the group
  Export of ~/.soccerwindow2 and written when an export starts
  (docs/plan/2026-11-28-00_frame-export.md 4.2, S10, D7). Ported from
  Options::ExportSettings of rcssmonitor.

  The directory, the prefix and the format of the images are also the
  values of the command line options --image-save-dir, --image-name-prefix
  and --image-save-format (Options): the saved ones are read into Options
  before the command line is parsed (readImageOptions()), so an option
  given there is used instead for that run (D7).
*/
struct ExportSettings {
    QString directory; //!< where the images are written (empty: the home)
    QString prefix = QStringLiteral( "image-" ); //!< the start of the names of the images
    QString format = QStringLiteral( "png" ); //!< the image format: png or jpg
    QString size = QStringLiteral( "view" ); //!< view, 1280x720, 1920x1080, 3840x2160, or custom
    int width = 1920; //!< the custom size
    int height = 1080;
    bool scale_texts = true; //!< the image looks as the view, enlarged (the device pixel ratio)
    double fps = 10.0; //!< the images a second
    double speed = 1.0; //!< how many times as fast as the game is played
    bool name_by_cycle = false; //!< the images named by the cycles (one frame an image)

    /*!
      \brief read the settings: the directory, the prefix and the format from
      Options (the saved ones, or the command line options), the others from
      the group Export
    */
    static
    ExportSettings read();

    /*!
      \brief read the saved directory, prefix and format into Options. Called
      before the command line is parsed (MainWindow::readSettings()).
    */
    static
    void readImageOptions();

    //! write the settings to the group Export, and the directory, the prefix and the format to Options
    void write() const;

    //! the image format of a name (PNG, png, jpeg, JPG, ...): "png" or "jpg"
    static
    QString formatOf( const QString & name );
};

#endif
