// -*-c++-*-

/*!
  \file frame_image.h
  \brief the image of a frame saved to a file Header File.
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

#ifndef SOCCERWINDOW2_QT_FRAME_IMAGE_H
#define SOCCERWINDOW2_QT_FRAME_IMAGE_H

#include <QString>

class QImage;

namespace rcsc {
class GameTime;
}

/*!
  \class FrameImage
  \brief the name and the file of the image of a frame (File > Save Image of
  This Frame. docs/plan/2026-11-28-00_frame-export.md 4.1, 4.3, S18): pure
  functions, apart from MainWindow, which chooses the frame and the name of
  the game log and asks for the file.
*/
class FrameImage {
private:
    // not used
    FrameImage() = delete;

public:

    /*!
      \brief the default file name of an image: the name of the game log
      without .rcg(.gz) (the program's if it is empty), and the cycle of the
      frame (1234, or 1234.3 when stopped), e.g. "head_v5-1.png"
      \param game_log_name the file name of the game log, or empty (the
      frames from a server, the anonymous mode)
      \param time the time of the frame, or null if there is no frame
    */
    static
    QString defaultName( const QString & game_log_name,
                         const rcsc::GameTime * time );

    /*!
      \brief write an image to a file. The format is given by the extension
      (PNG if there is none: ".png" is added).
      \param image the image (a null image: "There is no frame to save.")
      \param path the file path
      \param error the reason of a failure, if not null
      \return true if written
    */
    static
    bool write( const QImage & image,
                const QString & path,
                QString * error );
};

#endif
