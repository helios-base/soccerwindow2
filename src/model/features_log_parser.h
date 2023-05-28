// -*-c++-*-

/*!
  \file features_log.h
  \brief features log parser class Header File.
*/

/*
 *Copyright:

 Copyright (C) Hidehisa Akiyama

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

#ifndef SOCCERWINDOW2_MODEL_FEATURES_LOG_PARSER_H
#define SOCCERWINDOW2_MODEL_FEATURES_LOG_PARSER_H

#include "features_log.h"

#include <iosfwd>
#include <memory>


class FeaturesLogParser {
public:

    WholeFeaturesLog::Ptr parse( std::istream & is ) const;

private:

    bool parseHeaderLine( std::istream & is,
                          WholeFeaturesLog::Ptr holder ) const;
    bool parseFeatureNamesLine( std::istream & is,
                                WholeFeaturesLog::Ptr holder ) const;
    bool parseValueLines( std::istream & is,
                          WholeFeaturesLog::Ptr holder ) const;

    FeaturesLog::Ptr parseValueLine( const std::string & line,
                                     const size_t float_features_size,
                                     const size_t cat_features_size ) const;

};


#endif
