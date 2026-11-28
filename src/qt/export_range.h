// -*-c++-*-

/*!
  \file export_range.h
  \brief the ranges of the frames to export Header File.
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

#ifndef SOCCERWINDOW2_QT_EXPORT_RANGE_H
#define SOCCERWINDOW2_QT_EXPORT_RANGE_H

#include "monitor_view_data.h"

#include <QString>

#include <cstddef>
#include <limits>

/*!
  \struct ExportPosition
  \brief a position in a game log as the user writes it: a cycle (1234), or
  a cycle and a frame stopped in it (1234.3, or 1234,3 as the score board
  shows it)
*/
struct ExportPosition {
    int cycle = -1;
    int stopped = -1; //!< the stopped frame, or -1 for the cycle as a whole

    bool isValid() const { return cycle >= 0; }
};

/*!
  \struct ExportRange
  \brief a range of frames to export, and the functions of the positions,
  ported from rcssmonitor (docs/plan/2026-11-28-00_frame-export.md 4.2, S9):
  pure functions on the frames (MonitorViewData::Cont, in the order of the
  cycles and the stopped frames).

  A start at a cycle is its first frame, and an end at a cycle is its last
  frame, the frames stopped in it too; a position with a stopped frame is
  that frame. Both ends are included. A position not in the log (out of it,
  or missing in a live game) is moved to the next frame for a start, and to
  the previous frame for an end ("snapped").
*/
struct ExportRange {

    //! no frame
    static constexpr std::size_t INVALID_INDEX = std::numeric_limits< std::size_t >::max();

    std::size_t first = INVALID_INDEX;
    std::size_t last = INVALID_INDEX;

    bool isValid() const
      {
          return first != INVALID_INDEX
              && last != INVALID_INDEX
              && first <= last;
      }

    //! the number of the frames
    std::size_t size() const { return isValid() ? last - first + 1 : 0; }

    /*!
      \brief read a position: "1234", "1234.3" or "1234,3" (spaces around are ignored)
      \param error set to the reason if it is not one (may be null)
      \return false if it is not one
    */
    static
    bool parsePosition( const QString & text,
                        ExportPosition * pos,
                        QString * error );

    //! a position as written: "1234", or "1234.3" for a stopped frame
    static
    QString positionText( const rcsc::GameTime & time );

    //! the position of a frame, or an empty text if it is not one
    static
    QString positionText( const MonitorViewData::Cont & frames,
                          const std::size_t index );

    /*!
      \brief the first frame of a range starting at a position
      \param snapped set to true if there is no frame at the position and
      the next one is taken (may be null)
      \return the index, or INVALID_INDEX if it is after the last frame
    */
    static
    std::size_t startIndex( const MonitorViewData::Cont & frames,
                            const ExportPosition & pos,
                            bool * snapped );

    /*!
      \brief the last frame of a range ending at a position: the last
      frame of the cycle, the frames stopped in it too
      \param snapped set to true if there is no frame at the position and
      the previous one is taken (may be null)
      \return the index, or INVALID_INDEX if it is before the first frame
    */
    static
    std::size_t endIndex( const MonitorViewData::Cont & frames,
                          const ExportPosition & pos,
                          bool * snapped );

    /*!
      \brief the time of the game at a cycle: "m:ss"
      \param step_ms the simulation step (ms)
    */
    static
    QString gameTime( const int cycle,
                      const int step_ms );
};

#endif
