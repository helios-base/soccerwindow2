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
#include <string>
#include <utility>
#include <vector>

/*
  docs/plan/2026-11-28-00_frame-export.md 4.2, 4.3, S9, §6. Ported from rcssmonitor.
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


//! a frame of a game: its cycle, play mode and scores
MonitorViewData::ConstPtr
frame( const int cycle,
       const rcsc::PlayMode mode,
       const int left = 0,
       const int right = 0,
       const int stopped = 0 )
{
    rcsc::rcg::ShowInfoT show;
    show.time_ = cycle;
    const rcsc::rcg::TeamT team_l( std::string( "LEFT" ), left, 0, 0 );
    const rcsc::rcg::TeamT team_r( std::string( "RIGHT" ), right, 0, 0 );
    MonitorViewData::Ptr view = std::make_shared< MonitorViewData >( show, mode, team_l, team_r );
    view->setStoppageTime( stopped );
    return view;
}

//! the server's parameters of the halves: 300 s and 100 s of 100 ms
ExportGameParam
params( const int normal = 2,
        const int extra = 2,
        const int half_time = 300 )
{
    ExportGameParam param;
    param.step_ms = 100;
    param.half_time = half_time;
    param.nr_normal_halfs = normal;
    param.extra_half_time = 100;
    param.nr_extra_halfs = extra;
    return param;
}

//! the parts as "key first-last"
QString
textOf( const std::vector< ExportPeriod > & periods )
{
    QStringList list;
    for ( const ExportPeriod & p : periods ) list << QString( "%1 %2-%3" ).arg( p.key ).arg( p.first ).arg( p.last );
    return list.join( ", " );
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
    void periods();
    void goals();
    void around();
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

    // as an end: "0.0" and "3.0" for the frames before the stopped ones
    // (D17, ported from rcssmonitor's PR #79)
    QCOMPARE( ExportRange::endText( FRAMES, 0 ), QString( "0.0" ) );
    QCOMPARE( ExportRange::endText( FRAMES, 2 ), QString( "0.2" ) );
    QCOMPARE( ExportRange::endText( FRAMES, 3 ), QString( "1" ) );
    QCOMPARE( ExportRange::endText( FRAMES, 5 ), QString( "3.0" ) );
    QCOMPARE( ExportRange::endText( FRAMES, 7 ), QString( "3.2" ) );
    QCOMPARE( ExportRange::endText( FRAMES, 10 ), QString( "7" ) );
    QCOMPARE( ExportRange::endText( FRAMES, 100 ), QString() );
    // read back as an end: the same frame
    for ( std::size_t i = 0; i < FRAMES.size(); ++i )
    {
        ExportPosition p;
        QVERIFY( ExportRange::parsePosition( ExportRange::endText( FRAMES, i ), &p, nullptr ) );
        QCOMPARE( ExportRange::endIndex( FRAMES, p, nullptr ), i );
    }
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

/*-------------------------------------------------------------------*/
/*!
  The parts of a game by the cycles of the server's parameters (no play
  mode at the end of a half), the shootout from its first play mode.
*/
void
TestExportRange::periods()
{
    using namespace rcsc;

    // a game of two halves: 0, 3000 (the end, stopped), 3001, 6000 (time over)
    const MonitorViewData::Cont game = { frame( 0, PM_BeforeKickOff ), frame( 1, PM_KickOff_Left ), frame( 3000, PM_PlayOn ),
                            frame( 3000, PM_PlayOn, 0, 0, 1 ), frame( 3001, PM_KickOff_Right ), frame( 5999, PM_PlayOn ),
                            frame( 6000, PM_TimeOver ) };
    QCOMPARE( textOf( ExportRange::periods( game, params() ) ), QString( "first-half 0-3, second-half 4-6" ) );
    QCOMPARE( ExportRange::periods( game, params() ).front().label, QString( "First half" ) );

    // the extra time and the shootout
    const MonitorViewData::Cont extra = { frame( 0, PM_BeforeKickOff ), frame( 3000, PM_PlayOn ), frame( 3001, PM_KickOff_Right ),
                             frame( 6000, PM_PlayOn ), frame( 6001, PM_KickOff_Left ), frame( 7000, PM_PlayOn ),
                             frame( 7001, PM_KickOff_Right ), frame( 8000, PM_PlayOn ),
                             frame( 8001, PM_PenaltySetup_Left ), frame( 8002, PM_PenaltyReady_Left ),
                             frame( 8010, PM_PenaltyScore_Left ), frame( 8100, PM_TimeOver ) };
    QCOMPARE( textOf( ExportRange::periods( extra, params() ) ),
              QString( "first-half 0-1, second-half 2-3, extra-1 4-5, extra-2 6-7, penalties 8-11" ) );
    QCOMPARE( ExportRange::periods( extra, params() ).at( 2 ).label, QString( "Extra time 1" ) );
    QCOMPARE( ExportRange::periods( extra, params() ).back().label, QString( "Penalty shootout" ) );

    // a log that starts in the second half (e.g. a live game joined): only the parts there
    const MonitorViewData::Cont later = { frame( 4000, PM_PlayOn ), frame( 6000, PM_TimeOver ) };
    QCOMPARE( textOf( ExportRange::periods( later, params() ) ), QString( "second-half 0-1" ) );

    // after the last end: in the last part
    const MonitorViewData::Cont beyond = { frame( 100, PM_PlayOn ), frame( 9000, PM_PlayOn ) };
    QCOMPARE( textOf( ExportRange::periods( beyond, params( 2, 0 ) ) ), QString( "first-half 0-0, second-half 1-1" ) );

    // no time of the halves: the whole game
    QCOMPARE( textOf( ExportRange::periods( game, params( 2, 2, -1 ) ) ), QString( "game 0-6" ) );

    // no frames
    QVERIFY( ExportRange::periods( MonitorViewData::Cont(), params() ).empty() );
}

/*-------------------------------------------------------------------*/
/*!
  The goals: the changes of the score that ViewHolder saw (the index of
  the last frame before the new score, D10), the team and the score from
  the frame after (rcssmonitor's D10. ViewHolder::scoreChangeIndices() of
  soccerwindow2 is the same: checked with a game log).
*/
void
TestExportRange::goals()
{
    using namespace rcsc;
    const MonitorViewData::Cont game = { frame( 0, PM_BeforeKickOff ), frame( 100, PM_PlayOn ), frame( 101, PM_AfterGoal_Left, 1, 0 ),
                            frame( 102, PM_KickOff_Right, 1, 0 ), frame( 500, PM_AfterGoal_Right, 1, 1 ),
                            frame( 501, PM_PlayOn, 1, 1 ), frame( 800, PM_AfterGoal_Right, 1, 2 ) };
    // as ViewHolder keeps them: the frames before 101, 500 and 800
    std::vector< ExportGoal > goals = ExportRange::goals( game, { 1, 3, 5 } );
    QCOMPARE( goals.size(), std::size_t( 3 ) );
    QCOMPARE( goals[0].index, std::size_t( 1 ) );
    QCOMPARE( goals[0].frame, std::size_t( 2 ) ); // the frame of the goal
    QCOMPARE( goals[0].cycle, 101 );
    QCOMPARE( goals[0].side, 0 );
    QCOMPARE( goals[0].team, QString( "LEFT" ) );
    QCOMPARE( goals[1].index, std::size_t( 3 ) );
    QCOMPARE( goals[1].cycle, 500 );
    QCOMPARE( goals[1].side, 1 );
    QCOMPARE( goals[2].left, 1 );
    QCOMPARE( goals[2].right, 2 );
    QCOMPARE( goals[2].team, QString( "RIGHT" ) );

    // the index at the first frame with the new score (another order of
    // the records): the side by the score before, not by that frame
    goals = ExportRange::goals( game, { 2, 4, 6 } );
    QCOMPARE( goals.size(), std::size_t( 3 ) );
    QCOMPARE( goals[0].side, 0 );
    QCOMPARE( goals[1].side, 1 );
    QCOMPARE( goals[2].side, 1 );
    QCOMPARE( goals[2].cycle, 800 ); // the last frame: itself

    // a change where no score goes up, and an index out of the frames: no goals
    const MonitorViewData::Cont reset = { frame( 0, PM_PlayOn ), frame( 10, PM_PlayOn, 1, 0 ), frame( 20, PM_PlayOn, 0, 0 ),
                             frame( 30, PM_PlayOn, 0, 1 ) };
    goals = ExportRange::goals( reset, { 0, 1, 2, 100 } );
    QCOMPARE( goals.size(), std::size_t( 2 ) );
    QCOMPARE( goals[0].cycle, 10 );
    QCOMPARE( goals[1].cycle, 30 );
    QCOMPARE( goals[1].side, 1 );

    QVERIFY( ExportRange::goals( FRAMES, {} ).empty() );
    QCOMPARE( ExportRange::cyclesOf( 10.0, 100 ), 100 );
    QCOMPARE( ExportRange::cyclesOf( 2.5, 50 ), 50 );
}

/*-------------------------------------------------------------------*/
/*!
  Around a frame: the seconds before and after, of the playback (D12): a
  frame a step, a stopped one too; cut by the ends of the log.
*/
void
TestExportRange::around()
{
    // 0 to 300 a frame a cycle, with 200 stopped once (200.1)
    MonitorViewData::Cont game;
    for ( int c = 0; c <= 300; ++c )
    {
        game.push_back( frame( c, rcsc::PM_PlayOn ) );
        if ( c == 200 ) game.push_back( frame( c, rcsc::PM_PlayOn, 0, 0, 1 ) );
    }
    const auto index = [&game]( const int cycle ) {
        return ExportRange::startIndex( game, pos( cycle ), nullptr );
    };

    ExportRange r = ExportRange::around( game, index( 150 ), 10.0, 5.0, 100 );
    QCOMPARE( ExportRange::positionText( game, r.first ), QString( "50" ) );
    QCOMPARE( ExportRange::positionText( game, r.last ), QString( "200" ) ); // 200.1 is a step more
    r = ExportRange::around( game, index( 250 ), 5.0, 0.0, 100 );
    // 50 steps before 250: 200.1 takes one of them
    QCOMPARE( ExportRange::positionText( game, r.first ), QString( "200.1" ) );
    QCOMPARE( r.size(), std::size_t( 51 ) );

    // cut by the ends
    r = ExportRange::around( game, index( 20 ), 10.0, 5.0, 100 );
    QCOMPARE( r.first, std::size_t( 0 ) );
    r = ExportRange::around( game, index( 290 ), 1.0, 5.0, 100 );
    QCOMPARE( r.last, game.size() - 1 );

    // a step of 50 ms: twice the cycles
    r = ExportRange::around( game, index( 150 ), 1.0, 1.0, 50 );
    QCOMPARE( ExportRange::positionText( game, r.first ), QString( "130" ) );
    QCOMPARE( ExportRange::positionText( game, r.last ), QString( "170" ) );

    // a goal at 100 and its 50 stopped frames before the kick-off at 101: 5 s
    // after the goal are the stopped frames, not 5 s after the kick-off
    MonitorViewData::Cont goal;
    for ( int c = 0; c <= 300; ++c )
    {
        goal.push_back( frame( c, c == 100 ? rcsc::PM_AfterGoal_Left : rcsc::PM_PlayOn, c >= 100 ? 1 : 0 ) );
        for ( int s = 1; c == 100 && s <= 50; ++s )
        {
            goal.push_back( frame( c, rcsc::PM_AfterGoal_Left, 1, 0, s ) );
        }
    }
    const std::size_t goal_frame = ExportRange::startIndex( goal, pos( 100 ), nullptr );
    r = ExportRange::around( goal, goal_frame, 10.0, 5.0, 100 );
    QCOMPARE( ExportRange::positionText( goal, r.first ), QString( "0" ) );
    QCOMPARE( ExportRange::positionText( goal, r.last ), QString( "100.50" ) );
    QCOMPARE( r.size(), std::size_t( 151 ) );
    r = ExportRange::around( goal, goal_frame, 1.0, 6.0, 100 );
    QCOMPARE( ExportRange::positionText( goal, r.last ), QString( "110" ) );

    // the cycles missing in a live game: the frames there, a step each, as
    // in the images (0.2 s: 2 frames)
    const MonitorViewData::Cont missing = { frame( 0, rcsc::PM_PlayOn ), frame( 10, rcsc::PM_PlayOn ),
                                            frame( 50, rcsc::PM_PlayOn ), frame( 60, rcsc::PM_PlayOn ),
                                            frame( 90, rcsc::PM_PlayOn ) };
    r = ExportRange::around( missing, 2, 0.1, 0.2, 100 );
    QCOMPARE( r.first, std::size_t( 1 ) );
    QCOMPARE( r.last, std::size_t( 4 ) );

    // no frame there
    QVERIFY( ! ExportRange::around( MonitorViewData::Cont(), 0, 1.0, 1.0, 100 ).isValid() );
    QVERIFY( ! ExportRange::around( game, game.size(), 1.0, 1.0, 100 ).isValid() );
}

QTEST_GUILESS_MAIN( TestExportRange )
#include "test_export_range.moc"
