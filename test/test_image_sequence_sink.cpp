// -*-c++-*-

/*!
  \file test_image_sequence_sink.cpp
  \brief unit test of ImageSequenceSink and ExportJob.
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

#include <QtWidgets>
#include <QtTest>

#include "export_job.h"
#include "field_canvas.h"
#include "image_sequence_sink.h"
#include "main_data.h"
#include "options.h"
#include "view_holder.h"

#include "test_environment.h"

#include <QImageReader>
#include <QTemporaryDir>

#ifndef SOCCERWINDOW2_TEST_DATA_DIR
#error "SOCCERWINDOW2_TEST_DATA_DIR is not defined."
#endif

/*
  docs/plan/2026-11-28-00_frame-export.md 4.2, §6. Ported from rcssmonitor.
  test/data/head_v5.rcg has 3 frames (the cycles 1, 2 and 4).
*/

namespace {

QImage
colored( const QColor & color )
{
    QImage image( 32, 24, QImage::Format_RGB32 );
    image.fill( color );
    return image;
}

//! what a job told
struct Told {
    QStringList progress;
    int finished = 0;
    bool ok = false;
    QString message;

    explicit Told( ExportJob & job )
      {
          QObject::connect( &job, &ExportJob::progressed,
                            [this]( std::size_t done, std::size_t total ) {
                                progress << QString( "%1/%2" ).arg( done ).arg( total );
                            } );
          QObject::connect( &job, &ExportJob::finished,
                            [this]( bool o, const QString & m ) { ++finished; ok = o; message = m; } );
      }
};

}

/*!
  \brief unit test of the images exported
*/
class TestImageSequenceSink
    : public QObject {
    Q_OBJECT

private:

    MainData M_main_data;

private slots:

    void initTestCase()
      {
          QVERIFY( M_main_data.openRCG( SOCCERWINDOW2_TEST_DATA_DIR "/head_v5.rcg" ) );
          QCOMPARE( M_main_data.viewHolder().monitorViewCont().size(), std::size_t( 3 ) );
          QVERIFY( M_main_data.setViewDataIndex( 1 ) );
      }

    void formats();
    void writeImages();
    void notWritable();
    void tooMany();
    void job();
    void jobAborted();
    void jobFailed();
    void jobNothing();
};

/*-------------------------------------------------------------------*/
void
TestImageSequenceSink::formats()
{
    const QList< QByteArray > f = ImageSequenceSink::formats();
    QVERIFY( ! f.isEmpty() );
    QCOMPARE( f.first(), QByteArray( "png" ) );
}

/*-------------------------------------------------------------------*/
//! the images in the files of the names given; the directory made
void
TestImageSequenceSink::writeImages()
{
    QTemporaryDir tmp;
    QVERIFY( tmp.isValid() );
    const QString dir = tmp.filePath( "new/dir" );

    ImageSequenceSink sink( dir, { "a.png", "b.png" }, "png" );
    QSignalSpy finished( &sink, &FrameSink::finished );
    QVERIFY( sink.open( QSize( 32, 24 ), 10.0 ) );
    QCOMPARE( sink.write( colored( Qt::red ) ), FrameSink::ACCEPTED );
    QCOMPARE( sink.write( colored( Qt::blue ) ), FrameSink::ACCEPTED );
    QCOMPARE( sink.written(), 2 );
    sink.finish();
    QCOMPARE( finished.count(), 1 );
    QCOMPARE( finished.first().at( 0 ).toBool(), true );
    QVERIFY2( finished.first().at( 1 ).toString().startsWith( "2 images in " ), qPrintable( finished.first().at( 1 ).toString() ) );

    QCOMPARE( QImage( dir + "/a.png" ).pixelColor( 0, 0 ), QColor( Qt::red ) );
    QCOMPARE( QImage( dir + "/b.png" ).pixelColor( 0, 0 ), QColor( Qt::blue ) );
    QCOMPARE( QImageReader( dir + "/b.png" ).format(), QByteArray( "png" ) );

    // a name in a directory: made when its image is written (the ranges of Phase 3)
    ImageSequenceSink sub( dir, { "range/d.png" }, "png" );
    QVERIFY( sub.open( QSize( 32, 24 ), 10.0 ) );
    QCOMPARE( sub.write( colored( Qt::yellow ) ), FrameSink::ACCEPTED );
    QCOMPARE( QImage( dir + "/range/d.png" ).pixelColor( 0, 0 ), QColor( Qt::yellow ) );

    // JPEG, one image
    ImageSequenceSink jpeg( dir, { "c.jpg" }, "jpg" );
    QSignalSpy jpeg_finished( &jpeg, &FrameSink::finished );
    QVERIFY( jpeg.open( QSize( 32, 24 ), 10.0 ) );
    QCOMPARE( jpeg.write( colored( Qt::green ) ), FrameSink::ACCEPTED );
    jpeg.finish();
    QVERIFY( jpeg_finished.first().at( 1 ).toString().startsWith( "1 image in " ) );
    QCOMPARE( QImageReader( dir + "/c.jpg" ).format(), QByteArray( "jpeg" ) );
}

/*-------------------------------------------------------------------*/
void
TestImageSequenceSink::notWritable()
{
    QTemporaryDir tmp;
    QFile file( tmp.filePath( "file" ) );
    QVERIFY( file.open( QIODevice::WriteOnly ) );
    file.close();

    // a directory where a file is
    ImageSequenceSink sink( tmp.filePath( "file/dir" ), { "a.png" }, "png" );
    QVERIFY( ! sink.open( QSize( 32, 24 ), 10.0 ) );
    QVERIFY2( sink.errorString().contains( "Could not make the directory" ), qPrintable( sink.errorString() ) );
}

/*-------------------------------------------------------------------*/
void
TestImageSequenceSink::tooMany()
{
    QTemporaryDir tmp;
    ImageSequenceSink sink( tmp.path(), { "a.png" }, "png" );
    QVERIFY( sink.open( QSize( 32, 24 ), 10.0 ) );
    QCOMPARE( sink.write( colored( Qt::red ) ), FrameSink::ACCEPTED );
    QCOMPARE( sink.write( colored( Qt::red ) ), FrameSink::FAILED );
    QVERIFY( ! sink.errorString().isEmpty() );
}

/*-------------------------------------------------------------------*/
/*!
  A job: the frames drawn by the field canvas into the files, an image at a time
  after start() returns; the view and the frame shown unchanged.
*/
void
TestImageSequenceSink::job()
{
    FieldCanvas canvas( M_main_data );
    canvas.resize( 320, 240 );
    const Options::ViewState state = Options::instance().viewState();

    QTemporaryDir tmp;
    const std::vector< std::size_t > frames = { 0, 1, 2, 2 };
    ExportJob job( canvas,
                   new ImageSequenceSink( tmp.path(), { "0.png", "1.png", "2.png", "3.png" }, "png" ),
                   frames, QSize( 640, 480 ), 2.0, 10.0 );
    Told told( job );
    QCOMPARE( job.total(), std::size_t( 4 ) );
    job.start();
    QVERIFY( job.isRunning() );
    QCOMPARE( told.progress.size(), 0 ); // not before start() returns
    QTRY_COMPARE( told.finished, 1 );
    QVERIFY2( told.ok, qPrintable( told.message ) );
    QCOMPARE( told.progress, QStringList( { "1/4", "2/4", "3/4", "4/4" } ) );
    QVERIFY( ! job.isRunning() );
    QCOMPARE( job.done(), std::size_t( 4 ) );

    for ( int i = 0; i < 4; ++i )
    {
        const QImage image( tmp.filePath( QString( "%1.png" ).arg( i ) ) );
        QCOMPARE( image.size(), QSize( 640, 480 ) );
        const QImage expected = canvas.renderFrame( frames[i], QSize( 640, 480 ), 2.0 );
        QVERIFY( image.convertToFormat( QImage::Format_RGB32 ) == expected );
    }
    QCOMPARE( M_main_data.viewIndex(), std::size_t( 1 ) );
    QCOMPARE( Options::instance().viewState().canvas_width, state.canvas_width );
    QCOMPARE( Options::instance().viewState().field_scale, state.field_scale );
}

/*-------------------------------------------------------------------*/
//! aborted: told at once, the images written so far kept
void
TestImageSequenceSink::jobAborted()
{
    FieldCanvas canvas( M_main_data );
    QTemporaryDir tmp;
    ExportJob job( canvas,
                   new ImageSequenceSink( tmp.path(), { "0.png", "1.png", "2.png" }, "png" ),
                   { 0, 1, 2 }, QSize( 64, 48 ), 1.0, 10.0 );
    Told told( job );
    QObject::connect( &job, &ExportJob::progressed,
                      [&job]( std::size_t done, std::size_t ) { if ( done == 1 ) job.abort(); } );
    job.start();
    QTRY_COMPARE( told.finished, 1 );
    QVERIFY( ! told.ok );
    QCOMPARE( told.message, QString( "Stopped after 1 of 3 images." ) );
    QTest::qWait( 50 );
    QCOMPARE( told.finished, 1 );
    QCOMPARE( told.progress, QStringList( { "1/3" } ) );
    QVERIFY( QFile::exists( tmp.filePath( "0.png" ) ) );
    QVERIFY( ! QFile::exists( tmp.filePath( "1.png" ) ) );

    // abort() before the end is told once; after the end, nothing
    job.abort();
    QCOMPARE( told.finished, 1 );
}

/*-------------------------------------------------------------------*/
//! an error of the sink: told with its reason
void
TestImageSequenceSink::jobFailed()
{
    FieldCanvas canvas( M_main_data );
    QTemporaryDir tmp;
    QFile file( tmp.filePath( "file" ) );
    QVERIFY( file.open( QIODevice::WriteOnly ) );
    file.close();

    {
        ExportJob job( canvas, new ImageSequenceSink( tmp.filePath( "file/dir" ), { "0.png" }, "png" ),
                       { 0 }, QSize( 64, 48 ), 1.0, 10.0 );
        Told told( job );
        job.start();
        QTRY_COMPARE( told.finished, 1 );
        QVERIFY( ! told.ok );
        QVERIFY2( told.message.contains( "Could not make the directory" ), qPrintable( told.message ) );
    }
    {
        // more images than names
        ExportJob job( canvas, new ImageSequenceSink( tmp.path(), { "0.png" }, "png" ),
                       { 0, 1 }, QSize( 64, 48 ), 1.0, 10.0 );
        Told told( job );
        job.start();
        QTRY_COMPARE( told.finished, 1 );
        QVERIFY( ! told.ok );
        QCOMPARE( told.progress, QStringList( { "1/2" } ) );
    }
}

/*-------------------------------------------------------------------*/
void
TestImageSequenceSink::jobNothing()
{
    FieldCanvas canvas( M_main_data );
    QTemporaryDir tmp;
    ExportJob job( canvas, new ImageSequenceSink( tmp.path(), {}, "png" ),
                   {}, QSize( 64, 48 ), 1.0, 10.0 );
    Told told( job );
    job.start();
    QTRY_COMPARE( told.finished, 1 );
    QVERIFY( ! told.ok );
    QCOMPARE( told.message, QString( "There is no frame to export." ) );
}

SOCCERWINDOW2_GUI_TEST_MAIN( TestImageSequenceSink )
#include "test_image_sequence_sink.moc"
