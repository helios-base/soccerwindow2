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
#include <vector>

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
  \struct ExportPeriod
  \brief a part of a game: a half, a half of the extra time, or the penalty
  shootout, as frames
*/
struct ExportPeriod {
    std::size_t first = 0;
    std::size_t last = 0;
    QString key; //!< first-half, second-half, half-3, extra-1, penalties, game
    QString label; //!< for the user: "First half", ...
};

/*!
  \struct ExportGoal
  \brief a goal: where ViewHolder saw the score change, and the score after it
*/
struct ExportGoal {
    std::size_t index = 0; //!< the frame of ViewHolder::scoreChangeIndices() (the last one before the new score)
    std::size_t frame = 0; //!< the frame of the goal: the first one with the new score (the frame after the index)
    int cycle = 0; //!< the cycle of the goal: of the frame after the index
    int side = 0; //!< 0: the left team scored, 1: the right team
    int left = 0; //!< the score after the goal
    int right = 0;
    QString team; //!< the name of the team that scored
};

/*!
  \struct ExportGameParam
  \brief the parameters of the server that divide a game into its parts
  (ExportRange::periods()): the times in seconds, as in the game log
*/
struct ExportGameParam {
    double half_time = 300.0; //!< half_time (s)
    int nr_normal_halfs = 2;
    double extra_half_time = 100.0; //!< extra_half_time (s)
    int nr_extra_halfs = 2;
    int step_ms = 100; //!< simulator_step (ms)

    //! the parameters of the game log open (rcsc::ServerParam)
    static
    ExportGameParam current();
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
      \brief a frame as the end of a range: its position, but "1234.0" for
      the first frame of a cycle with frames stopped in it after it, as
      "1234" as an end is the last of them (ported from rcssmonitor's PR #79.
      docs/plan/2026-11-28-00_frame-export.md 4.6, D17)
      \return the text, or an empty text if it is not a frame
    */
    static
    QString endText( const MonitorViewData::Cont & frames,
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

    //! the cycles of some seconds of the game (rounded)
    static
    int cyclesOf( const double seconds,
                  const int step_ms );

    /*!
      \brief the halves, the halves of the extra time and the penalty
      shootout in the frames (docs/plan/2026-11-28-00_frame-export.md 4.3):
      by the cycles of the server's parameters (half_time, nr_normal_halfs,
      extra_half_time, nr_extra_halfs; rcssserver records no play mode at
      the end of a half), the shootout from its first play mode on. Only
      the parts with frames; the whole game if the halves have no time.
    */
    static
    std::vector< ExportPeriod > periods( const MonitorViewData::Cont & frames,
                                         const ExportGameParam & param );

    /*!
      \brief the goals: the changes of the score that ViewHolder saw, as "go
      to the next score" of the log player uses them (rcssmonitor's D10). The
      team and the score are read from the frame after each (the first one
      with the new score); a change where no score goes up is not a goal.
      \param score_changed ViewHolder::scoreChangeIndices()
    */
    static
    std::vector< ExportGoal > goals( const MonitorViewData::Cont & frames,
                                     const std::vector< std::size_t > & score_changed );

    /*!
      \brief the frames around a frame: from some seconds before it to some
      seconds after it, cut by the ends of the log. The seconds are of the
      playback (D12): a frame is a step, a stopped one too, as in the log
      player and in the exported images (ExportTiming). E.g. 5 s after a goal
      are the 50 stopped frames before the kick-off (rcssmonitor counted the
      cycles: 5 s after the kick-off).
      \param center the index of the frame (a goal: ExportGoal::frame)
      \return the range (invalid if the index is not of a frame)
    */
    static
    ExportRange around( const MonitorViewData::Cont & frames,
                        const std::size_t center,
                        const double before_s,
                        const double after_s,
                        const int step_ms );
};

#endif
