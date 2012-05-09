// -*-c++-*-

/*!
  \file chain_action_log_parser.h
  \brief parser class to parse chain action log format Header File.
*/

/*
 *Copyright:

 Copyright (C) Hiroki SHIMORA

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

#ifndef SOCCERWINDOW2_CHAIN_ACTION_LOG_PARSER_H
#define SOCCERWINDOW2_CHAIN_ACTION_LOG_PARSER_H

#include <iosfwd>
#include <string>
#include <vector>
#include <map>
#include <utility>

#include <boost/shared_ptr.hpp>
#include <rcsc/geom/vector_2d.h>

#include "chain_action_data.h"

/*!
  \class ChainActionLogParser
  \brief parser class to parse chain action log format
*/
class ChainActionLogParser {

public:
    /*
      \brief construct chain action log data from stream
      \param in input strean in order to parse
      \return parse result data
     */
    boost::shared_ptr< ChainDescriptionSet > parse( std::istream & in ) const;
};

#endif
