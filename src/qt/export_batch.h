// -*-c++-*-

/*!
  \file export_batch.h
  \brief the export from the command line, without a window. Header File.
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

#ifndef SOCCERWINDOW2_QT_EXPORT_BATCH_H
#define SOCCERWINDOW2_QT_EXPORT_BATCH_H

#include "export_range.h"
#include "export_timing.h"

#include "main_data.h"

#include <QString>
#include <QStringList>

#include <ostream>
#include <vector>

class Options;
class ViewHolder;

/*!
  \class ExportBatch
  \brief the export from the command line, ported from rcssmonitor
  (docs/plan/2026-11-28-00_frame-export.md 4.5, S14): a game log read
  without a window, its ranges exported as images or a video as the export
  dialog does, the progress told on the standard error, and an exit code.
  Nothing is written in ~/.soccerwindow2.

  The options (--export-video FILE or --export-images DIR, and the others)
  are parsed with the others (Options::parseCmdLine()) and read by
  readOptions(). A RANGE is START-END (cycles or stopped frames, 1234.3;
  either end may be empty), all, a part of the game (first-half,
  second-half, half-N, extra-N, penalties, game) or goals[:BEFORE:AFTER] (a
  range for each goal, in seconds).

  It runs on the platform offscreen (main() chooses it by isAsked()): the
  field is drawn without a display.
*/
class ExportBatch {
public:

    //! the size of the images by default
    static constexpr int DEFAULT_WIDTH = 1280;
    static constexpr int DEFAULT_HEIGHT = 720;
    //! the height of the view the images are laid out as by default (the scale: the height of the images / this)
    static constexpr double LAYOUT_HEIGHT = 720.0;

    /*!
      \struct Settings
      \brief what to export and how: the options given, or the defaults
      (fixed values, not the settings of the export dialog. S14)
    */
    struct Settings {
        QString video; //!< the video file (--export-video)
        QString images; //!< the directory of the images (--export-images)
        QStringList ranges; //!< none: all
        bool join = false; //!< the ranges in one video (or one sequence of images)
        double fps = ExportTiming::DEFAULT_FPS;
        double speed = 1.0;
        int width = DEFAULT_WIDTH;
        int height = DEFAULT_HEIGHT;
        double scale = 0.0; //!< the device pixel ratio; 0: the height / LAYOUT_HEIGHT
        QString image_format = QStringLiteral( "png" ); //!< png or jpg
        QString image_prefix = QStringLiteral( "image-" );
        int crf = 18;
        QString preset = QStringLiteral( "medium" );
        QString ffmpeg; //!< empty: ffmpeg in the PATH

        bool enabled() const { return ! video.isEmpty() || ! images.isEmpty(); }
    };

    //! a range of frames and its name (for the files of a range)
    struct NamedRange {
        ExportRange range;
        QString name;
    };

    /*!
      \brief whether the command line has an option of the export from the
      command line (--export-video, --export-images, or the others but
      --export-image-prefix and --export-image-format, which are also of the
      export dialog. S21, D15): read before the application is made, to
      choose its platform and to make no window
    */
    static
    bool isAsked( const int argc,
                  const char * const * argv );

    /*!
      \brief read the options of the export parsed (Options::parseCmdLine())
      \param options the options parsed
      \param settings set to what was given (the defaults for the others)
      \param error set to the reason when an option is wrong
      \return false if an option is wrong
    */
    static
    bool readOptions( const Options & options,
                      Settings * settings,
                      QString * error );

    /*!
      \brief the ranges of the texts in the frames of a game log
      \param texts the ranges given (none: all)
      \param holder the frames and the goals (the parameters of the server: rcsc::ServerParam)
      \param ranges set to the ranges, in the order given (the goals in their order)
      \param error set to the reason when a text is wrong, or when there is no range
      \return false if a text is wrong or there is no range
    */
    static
    bool resolveRanges( const QStringList & texts,
                        const ViewHolder & holder,
                        std::vector< NamedRange > * ranges,
                        QString * error );

    /*!
      \param settings what to export
      \param log where the progress and the result are told (the standard error)
    */
    ExportBatch( const Settings & settings,
                 std::ostream & log );

    /*!
      \brief read the game log and export it: the event loop runs until the end
      \param game_log the path of the game log (.rcg or .rcg.gz)
      \return the exit code: 0 when exported, 1 otherwise
    */
    int run( const QString & game_log );

private:

    Settings M_settings;
    std::ostream & M_log;
    MainData M_main_data;

    //! the files of the videos: one, or FILE-NAME.ext for each range
    QStringList videoFiles( const std::vector< NamedRange > & ranges ) const;

    //! tell an error, and the exit code of it
    int fail( const QString & message );
};

#endif
