// -*-c++-*-

/*!
  \file test_export_range.cpp
  \brief unit test of ExportRange.
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

#include <QtTest>

#include "export_range.h"

#include <memory>
#include <utility>
#include <vector>

/*
  docs/plan/2026-11-28-00_frame-export.md 4.2, S9, §6. Ported from rcssmonitor.
*/

namespace {

//! frames of these (cycle, stopped frame) pairs
MonitorViewData::Cont
framesOf( const std::vector< std::pair< int, int > > & times )
{
    MonitorViewData::Cont cont;
    for ( const auto & t : times )
    {
        rcsc::rcg::ShowInfoT show;
        show.time_ = t.first;
        MonitorViewData::Ptr view = std::make_shared< MonitorViewData >( show, rcsc::PM_PlayOn,
                                                                          rcsc::rcg::TeamT(), rcsc::rcg::TeamT() );
        view->setStoppageTime( t.second );
        cont.push_back( view );
    }
    return cont;
}

/*
  0 (0.0, 0.1, 0.2: the kick-off waited), 1, 2, 3 (3.0 3.1 3.2: a foul),
  4, 6 (5 missing in a live game), 7
*/
const MonitorViewData::Cont FRAMES = framesOf( { { 0, 0 }, { 0, 1 }, { 0, 2 }, { 1, 0 }, { 2, 0 },
                                     { 3, 0 }, { 3, 1 }, { 3, 2 }, { 4, 0 }, { 6, 0 }, { 7, 0 } } );

ExportPosition
pos( const int cycle,
     const int stopped = -1 )
{
    ExportPosition p;
    p.cycle = cycle;
    p.stopped = stopped;
    return p;
}

}

/*!
  \brief unit test of ExportRange
*/
class TestExportRange
    : public QObject {
    Q_OBJECT

private slots:

    void parse_data();
    void parse();
    void text();
    void startIndex_data();
    void startIndex();
    void endIndex_data();
    void endIndex();
    void range();
    void gameTime();
};

/*-------------------------------------------------------------------*/
void
TestExportRange::parse_data()
{
    QTest::addColumn< QString >( "text" );
    QTest::addColumn< int >( "cycle" ); // -1: not one
    QTest::addColumn< int >( "stopped" );

    QTest::newRow( "a cycle" ) << "1234" << 1234 << -1;
    QTest::newRow( "a stopped frame" ) << "1234.3" << 1234 << 3;
    QTest::newRow( "a stopped frame as the score board" ) << "1234,3" << 1234 << 3;
    QTest::newRow( "the first stopped frame" ) << "1234.0" << 1234 << 0;
    QTest::newRow( "spaces" ) << "  12 " << 12 << -1;
    QTest::newRow( "zero" ) << "0" << 0 << -1;
    QTest::newRow( "negative" ) << "-1" << -1 << -1;
    QTest::newRow( "a dot only" ) << "12." << -1 << -1;
    QTest::newRow( "two dots" ) << "1.2.3" << -1 << -1;
    QTest::newRow( "a comma only" ) << "12," << -1 << -1;
    QTest::newRow( "a time" ) << "1:30" << -1 << -1;
    QTest::newRow( "empty" ) << "" << -1 << -1;
}

/*-------------------------------------------------------------------*/
void
TestExportRange::parse()
{
    QFETCH( QString, text );
    QFETCH( int, cycle );
    QFETCH( int, stopped );

    ExportPosition p;
    QString error = "x";
    const bool ok = ExportRange::parsePosition( text, &p, &error );
    QCOMPARE( ok, cycle >= 0 );
    if ( ok )
    {
        QCOMPARE( p.cycle, cycle );
        QCOMPARE( p.stopped, stopped );
        QVERIFY( error.isEmpty() );
    }
    else
    {
        QVERIFY2( error.contains( "Not a position" ), qPrintable( error ) );
    }
}

/*-------------------------------------------------------------------*/
void
TestExportRange::text()
{
    QCOMPARE( ExportRange::positionText( FRAMES, 0 ), QString( "0" ) );
    QCOMPARE( ExportRange::positionText( FRAMES, 2 ), QString( "0.2" ) );
    QCOMPARE( ExportRange::positionText( FRAMES, 3 ), QString( "1" ) );
    QCOMPARE( ExportRange::positionText( FRAMES, 100 ), QString() );
    QCOMPARE( ExportRange::positionText( rcsc::GameTime( 1234, 3 ) ), QString( "1234.3" ) );
}

/*-------------------------------------------------------------------*/
void
TestExportRange::startIndex_data()
{
    QTest::addColumn< int >( "cycle" );
    QTest::addColumn< int >( "stopped" );
    QTest::addColumn< int >( "index" ); // -1: none
    QTest::addColumn< bool >( "snapped" );

    QTest::newRow( "a cycle: its first frame" ) << 3 << -1 << 5 << false;
    QTest::newRow( "the kick-off" ) << 0 << -1 << 0 << false;
    QTest::newRow( "a stopped frame" ) << 3 << 2 << 7 << false;
    QTest::newRow( "the first stopped frame" ) << 3 << 0 << 5 << false;
    QTest::newRow( "a stopped frame not there: the next frame" ) << 3 << 5 << 8 << true;
    QTest::newRow( "a missing cycle: the next frame" ) << 5 << -1 << 9 << true;
    QTest::newRow( "after the log" ) << 8 << -1 << -1 << false;
}

/*-------------------------------------------------------------------*/
void
TestExportRange::startIndex()
{
    QFETCH( int, cycle );
    QFETCH( int, stopped );
    QFETCH( int, index );
    QFETCH( bool, snapped );

    bool s = ! snapped;
    const std::size_t i = ExportRange::startIndex( FRAMES, pos( cycle, stopped ), &s );
    QCOMPARE( i, index < 0 ? ExportRange::INVALID_INDEX : std::size_t( index ) );
    QCOMPARE( s, snapped );
}

/*-------------------------------------------------------------------*/
void
TestExportRange::endIndex_data()
{
    QTest::addColumn< int >( "cycle" );
    QTest::addColumn< int >( "stopped" );
    QTest::addColumn< int >( "index" ); // -1: none
    QTest::addColumn< bool >( "snapped" );

    QTest::newRow( "a cycle: its last frame, the stopped ones too" ) << 3 << -1 << 7 << false;
    QTest::newRow( "the kick-off waited" ) << 0 << -1 << 2 << false;
    QTest::newRow( "a cycle without stopped frames" ) << 4 << -1 << 8 << false;
    QTest::newRow( "a stopped frame" ) << 3 << 1 << 6 << false;
    QTest::newRow( "a stopped frame not there: the previous frame" ) << 3 << 5 << 7 << true;
    QTest::newRow( "a missing cycle: the previous frame" ) << 5 << -1 << 8 << true;
    QTest::newRow( "after the log: its last frame" ) << 100 << -1 << 10 << true;
}

/*-------------------------------------------------------------------*/
void
TestExportRange::endIndex()
{
    QFETCH( int, cycle );
    QFETCH( int, stopped );
    QFETCH( int, index );
    QFETCH( bool, snapped );

    bool s = ! snapped;
    const std::size_t i = ExportRange::endIndex( FRAMES, pos( cycle, stopped ), &s );
    QCOMPARE( i, index < 0 ? ExportRange::INVALID_INDEX : std::size_t( index ) );
    QCOMPARE( s, snapped );

    // before the log
    const MonitorViewData::Cont later = framesOf( { { 10, 0 }, { 11, 0 } } );
    QCOMPARE( ExportRange::endIndex( later, pos( 9 ), nullptr ), ExportRange::INVALID_INDEX );
    QCOMPARE( ExportRange::startIndex( later, pos( 9 ), &s ), std::size_t( 0 ) );
    QVERIFY( s );
    // no frames, no position
    QCOMPARE( ExportRange::startIndex( MonitorViewData::Cont(), pos( 0 ), nullptr ), ExportRange::INVALID_INDEX );
    QCOMPARE( ExportRange::endIndex( FRAMES, ExportPosition(), nullptr ), ExportRange::INVALID_INDEX );
}

/*-------------------------------------------------------------------*/
//! a range: both ends included
void
TestExportRange::range()
{
    ExportRange r;
    QVERIFY( ! r.isValid() );
    QCOMPARE( r.size(), std::size_t( 0 ) );

    // 3 to 3: the frames of the foul
    r.first = ExportRange::startIndex( FRAMES, pos( 3 ), nullptr );
    r.last = ExportRange::endIndex( FRAMES, pos( 3 ), nullptr );
    QVERIFY( r.isValid() );
    QCOMPARE( r.size(), std::size_t( 3 ) );

    r.first = 8;
    r.last = 7;
    QVERIFY( ! r.isValid() );
    QCOMPARE( r.size(), std::size_t( 0 ) );
}

/*-------------------------------------------------------------------*/
void
TestExportRange::gameTime()
{
    QCOMPARE( ExportRange::gameTime( 0, 100 ), QString( "0:00" ) );
    QCOMPARE( ExportRange::gameTime( 1234, 100 ), QString( "2:03" ) );
    QCOMPARE( ExportRange::gameTime( 6000, 100 ), QString( "10:00" ) );
    QCOMPARE( ExportRange::gameTime( 6000, 50 ), QString( "5:00" ) );
    QCOMPARE( ExportRange::gameTime( -5, 100 ), QString( "0:00" ) );
}

QTEST_GUILESS_MAIN( TestExportRange )
#include "test_export_range.moc"
