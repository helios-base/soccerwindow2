// -*-c++-*-

/*!
  \file log_player_slider.h
  \brief the slider of the frames Header File.
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

#ifndef SOCCERWINDOW2_QT_LOG_PLAYER_SLIDER_H
#define SOCCERWINDOW2_QT_LOG_PLAYER_SLIDER_H

#include <QColor>
#include <QSlider>

#include <cstddef>
#include <vector>

/*!
  \class LogPlayerSlider
  \brief the slider of the frames in the tool bar of the log player (its
  value is the index of the frame), split out of LogPlayerToolBar. The
  groove shows the ranges to export as bands (the current one darker) and
  the goals as ticks, in the horizontal and the vertical orientations
  (docs/plan/2026-11-28-00_frame-export.md 4.3, S12, D11). The drawing of
  the bands and the goals is ported from rcssmonitor.
*/
class LogPlayerSlider
    : public QSlider {

    Q_OBJECT

public:

    //! a range of frames shown as a band
    struct Band {
        std::size_t first;
        std::size_t last;
        bool current;
    };

private:

    std::vector< Band > M_bands;
    std::vector< std::size_t > M_goals; //!< the frames of the goals

public:

    explicit
    LogPlayerSlider( Qt::Orientation orientation,
                     QWidget * parent = nullptr );

    void setBands( const std::vector< Band > & bands );
    const std::vector< Band > & bands() const { return M_bands; }

    void setGoals( const std::vector< std::size_t > & goals );
    const std::vector< std::size_t > & goals() const { return M_goals; }

    //! where a frame is along the groove (pixels from the left, or from the top)
    int positionOf( const std::size_t index ) const;

    //! the color of the bands and of the ticks of the goals, by the palette
    QColor bandColor( const bool current ) const;
    QColor goalColor() const;

protected:

    void paintEvent( QPaintEvent * event ) override;
};

#endif
