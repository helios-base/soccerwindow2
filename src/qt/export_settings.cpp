// -*-c++-*-

/*!
  \file export_settings.cpp
  \brief the settings of the export Source File.
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

#include "export_settings.h"

#include "options.h"

#include <QDir>
#include <QSettings>
#include <QVariant>

namespace {

//! the settings file of soccerwindow2
QString
settings_file()
{
    return QDir::homePath() + QStringLiteral( "/.soccerwindow2" );
}

}

/*-------------------------------------------------------------------*/
QString
ExportSettings::formatOf( const QString & name )
{
    const QString f = name.trimmed().toLower();
    return ( f == QLatin1String( "jpg" ) || f == QLatin1String( "jpeg" ) )
        ? QStringLiteral( "jpg" )
        : QStringLiteral( "png" );
}

/*-------------------------------------------------------------------*/
void
ExportSettings::readImageOptions()
{
    QSettings settings( settings_file(), QSettings::IniFormat );
    settings.beginGroup( "Export" );

    Options & opt = Options::instance();
    QVariant val = settings.value( "directory" );
    if ( val.isValid() ) opt.setImageSaveDir( val.toString().toStdString() );

    val = settings.value( "prefix" );
    if ( val.isValid() ) opt.setImageNamePrefix( val.toString().toStdString() );

    // upper case, as the default of the option ("PNG"), which ImageSaveDialog
    // of --auto-image-save compares with the names of the formats
    val = settings.value( "format" );
    if ( val.isValid() && ! val.toString().isEmpty() ) opt.setImageSaveFormat( formatOf( val.toString() ).toUpper().toStdString() );

    settings.endGroup();
}

/*-------------------------------------------------------------------*/
ExportSettings
ExportSettings::read()
{
    ExportSettings s;

    // the saved ones, or the command line options (D7)
    const Options & opt = Options::instance();
    s.directory = QString::fromStdString( opt.imageSaveDir() );
    s.prefix = QString::fromStdString( opt.imageNamePrefix() );
    s.format = formatOf( QString::fromStdString( opt.imageSaveFormat() ) );

    QSettings settings( settings_file(), QSettings::IniFormat );
    settings.beginGroup( "Export" );

    QVariant val = settings.value( "size" );
    if ( val.isValid() && ! val.toString().isEmpty() ) s.size = val.toString();

    val = settings.value( "width" );
    if ( val.isValid() && val.toInt() > 0 ) s.width = val.toInt();

    val = settings.value( "height" );
    if ( val.isValid() && val.toInt() > 0 ) s.height = val.toInt();

    val = settings.value( "scale_texts" );
    if ( val.isValid() ) s.scale_texts = val.toBool();

    val = settings.value( "fps" );
    if ( val.isValid() && val.toDouble() > 0.0 ) s.fps = val.toDouble();

    val = settings.value( "speed" );
    if ( val.isValid() && val.toDouble() > 0.0 ) s.speed = val.toDouble();

    val = settings.value( "name_by_cycle" );
    if ( val.isValid() ) s.name_by_cycle = val.toBool();

    settings.endGroup();
    return s;
}

/*-------------------------------------------------------------------*/
void
ExportSettings::write() const
{
    Options & opt = Options::instance();
    opt.setImageSaveDir( directory.toStdString() );
    opt.setImageNamePrefix( prefix.toStdString() );
    opt.setImageSaveFormat( formatOf( format ).toUpper().toStdString() );

    QSettings settings( settings_file(), QSettings::IniFormat );
    settings.beginGroup( "Export" );
    settings.setValue( "directory", directory );
    settings.setValue( "prefix", prefix );
    settings.setValue( "format", format );
    settings.setValue( "size", size );
    settings.setValue( "width", width );
    settings.setValue( "height", height );
    settings.setValue( "scale_texts", scale_texts );
    settings.setValue( "fps", fps );
    settings.setValue( "speed", speed );
    settings.setValue( "name_by_cycle", name_by_cycle );
    settings.endGroup();
}
