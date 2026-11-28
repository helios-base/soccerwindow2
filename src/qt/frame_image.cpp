// -*-c++-*-

/*!
  \file frame_image.cpp
  \brief the image of a frame saved to a file Source File.
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "frame_image.h"

#include <rcsc/game_time.h>

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QImageWriter>

#ifndef PACKAGE_NAME
#define PACKAGE_NAME "soccerwindow2"
#endif

/*-------------------------------------------------------------------*/
QString
FrameImage::defaultName( const QString & game_log_name,
                         const rcsc::GameTime * time )
{
    QString name = game_log_name;
    if ( name.endsWith( QLatin1String( ".gz" ) ) ) name.chop( 3 );
    if ( name.endsWith( QLatin1String( ".rcg" ) ) ) name.chop( 4 );
    if ( name.isEmpty() ) name = QStringLiteral( PACKAGE_NAME );

    if ( time )
    {
        // the cycle: 1234, or 1234.3 when stopped
        name += QLatin1Char( '-' ) + ( time->stopped() > 0
                                       ? QString( "%1.%2" ).arg( time->cycle() ).arg( time->stopped() )
                                       : QString::number( time->cycle() ) );
    }
    return name + QStringLiteral( ".png" );
}

/*-------------------------------------------------------------------*/
bool
FrameImage::write( const QImage & image,
                   const QString & path,
                   QString * error )
{
    if ( image.isNull() )
    {
        if ( error ) *error = QCoreApplication::translate( "FrameImage", "There is no frame to save." );
        return false;
    }

    const QString file = ( QFileInfo( path ).suffix().isEmpty()
                           ? path + QStringLiteral( ".png" )
                           : path );
    QImageWriter writer( file );
    if ( ! writer.write( image ) )
    {
        if ( error ) *error = QCoreApplication::translate( "FrameImage", "Could not write the image to %1:\n%2" )
                         .arg( QDir::toNativeSeparators( file ), writer.errorString() );
        return false;
    }

    if ( error ) error->clear();
    return true;
}
