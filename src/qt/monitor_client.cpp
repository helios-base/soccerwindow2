// -*-c++-*-

/*!
  \file monitor_client.cpp
  \brief Monitor Client class Source File.
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

#include <QByteArray>
#include <QCoreApplication>
#include <QTimer>

#include "monitor_client.h"

#include "monitor_transport.h"
#include "options.h"
#include "main_data.h"

#include <rcsc/trainer/trainer_command.h>
#include <rcsc/monitor/monitor_command.h>
#include <rcsc/rcg/types.h>

#include <algorithm>
#include <sstream>
#include <iostream>
#include <cassert>
#include <cstring>


namespace {
const int POLL_INTERVAL_MS = 1000;
}

/*-------------------------------------------------------------------*/
/*!

*/
MonitorClient::MonitorClient( QObject * parent,
                              MainData & main_data,
                              MonitorTransport * transport,
                              const int version )

    : QObject( parent ),
      M_main_data( main_data ),
      M_transport( transport ),
      M_timer( new QTimer( this ) ),
      M_version( version ),
      M_waited_msec( 0 ),
      M_transport_connected( false ),
      M_first_message_received( false )
{
    assert( transport );

    M_transport->setParent( this );

    connect( M_transport, &MonitorTransport::connected,
             this, &MonitorClient::handleConnected );
    connect( M_transport, &MonitorTransport::messagesAvailable,
             this, &MonitorClient::handleMessages );
    connect( M_transport, &MonitorTransport::disconnected,
             this, &MonitorClient::handleDisconnected );

    connect( M_timer, &QTimer::timeout,
             this, &MonitorClient::handleTimer );
}

/*-------------------------------------------------------------------*/
/*!

*/
MonitorClient::~MonitorClient()
{
    disconnect();

    //std::cerr << "delete MonitorClient" << std::endl;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
MonitorClient::start( const QString & host,
                      quint16 port,
                      QString * error )
{
    // check protocol version range
    if ( M_version != -1 // json
         && ( M_version < 1 || 5 < M_version ) )
    {
        if ( error )
        {
            *error = tr( "Unsupported protocol version %1." ).arg( M_version );
        }
        return false;
    }

    // the server replies text messages to v1/v2 over TCP/WebSocket.
    // they must not be read as the binary structures.
    if ( ( M_version == 1 || M_version == 2 )
         && ! M_transport->isBinarySafe() )
    {
        if ( error )
        {
            *error = tr( "The protocol version %1 is not available over %2."
                         " Use version 3 or later, or -1 (JSON)." )
                .arg( M_version )
                .arg( monitor_transport_type_name( M_transport->type() ) );
        }
        return false;
    }

    if ( ! M_transport->open( host, port, error ) )
    {
        return false;
    }

    // the timer also limits the wait time for the connection setup.
    M_waited_msec = 0;
    M_timer->start( POLL_INTERVAL_MS );

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
MonitorClient::disconnect()
{
    M_timer->stop();

    // no more notification from the transport
    M_transport->disconnect( this );

    if ( isConnected() )
    {
        sendDispBye();
    }

    M_transport->close();
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
MonitorClient::isConnected() const
{
    return M_transport->isOpen();
}

/*-------------------------------------------------------------------*/
/*!

*/
QString
MonitorClient::peerName() const
{
    return M_transport->peerName();
}

/*-------------------------------------------------------------------*/
/*!

*/
void
MonitorClient::handleConnected()
{
    M_transport_connected = true;
    sendDispInit();
}

/*-------------------------------------------------------------------*/
/*!

*/
void
MonitorClient::handleMessages()
{
    int receive_count = 0;

    QByteArray msg;
    while ( M_transport->popMessage( msg ) )
    {
        ++receive_count;

        if ( ! handleMessage( msg ) )
        {
            // disconnection has been requested.
            return;
        }
    }

    if ( receive_count > 0 )
    {
        M_waited_msec = 0;
        M_timer->start( POLL_INTERVAL_MS );

        emit received();
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
MonitorClient::handleMessage( const QByteArray & msg )
{
    if ( ! M_first_message_received )
    {
        M_first_message_received = true;

        // the server replies an error message instead of the initial data.
        // e.g. (error no_more_monitor)
        if ( msg.startsWith( "(error" ) )
        {
            requestDisconnect( tr( "Connection refused by the server: %1" )
                               .arg( QString::fromLatin1( msg.constData() ) ) );
            return false;
        }
    }

    if ( M_version == -1
         || M_version >= 3 )
    {
        // JSON or S-Expression.
        // QByteArray::constData() is NUL terminated.
        if ( ! M_main_data.receiveMonitorPacket( msg.constData(), M_version ) )
        {
            std::cerr << PACKAGE_NAME << " recv: " << msg.constData() << std::endl;
        }
    }
    else if ( M_version == 2 )
    {
        rcsc::rcg::dispinfo_t2 disp2;
        std::memset( &disp2, 0, sizeof( disp2 ) );
        std::memcpy( &disp2, msg.constData(),
                     std::min( static_cast< size_t >( msg.size() ), sizeof( disp2 ) ) );

        if ( ! M_main_data.receiveMonitorPacket( disp2 ) )
        {
            std::cerr << PACKAGE_NAME << " recv: " << msg.constData() << std::endl;
        }
    }
    else if ( M_version == 1 )
    {
        rcsc::rcg::dispinfo_t disp1;
        std::memset( &disp1, 0, sizeof( disp1 ) );
        std::memcpy( &disp1, msg.constData(),
                     std::min( static_cast< size_t >( msg.size() ), sizeof( disp1 ) ) );

        if ( ! M_main_data.receiveMonitorPacket( disp1 ) )
        {
            std::cerr << PACKAGE_NAME << " recv: " << msg.constData() << std::endl;
        }
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
MonitorClient::handleDisconnected( const QString & reason )
{
    if ( M_transport_connected )
    {
        requestDisconnect( tr( "Disconnected from the server: %1" ).arg( reason ) );
    }
    else
    {
        requestDisconnect( tr( "Failed to connect to the server: %1" ).arg( reason ) );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
MonitorClient::handleTimer()
{
    M_waited_msec += POLL_INTERVAL_MS;

    //std::cerr << "handleTimer waited = " << M_waited_msec << std::endl;

    const int wait_seconds = Options::instance().waitSeconds();
    if ( wait_seconds > 0
         && M_waited_msec >= wait_seconds * 1000 )
    {
        requestDisconnect( tr( "No message from the server for %1 seconds." )
                           .arg( M_waited_msec / 1000 ) );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
MonitorClient::requestDisconnect( const QString & reason )
{
    M_timer->stop();

    std::cerr << PACKAGE_NAME << ": " << reason.toStdString() << std::endl;

    if ( Options::instance().autoQuitMode() )
    {
        std::cerr << PACKAGE_NAME << ": auto quit mode. exit..." << std::endl;
        QCoreApplication::exit();
    }

    emit errorMessage( reason );
    emit disconnectRequested();
}

/*-------------------------------------------------------------------*/
/*!

*/
void
MonitorClient::sendCommand( const std::string & com )
{
    if ( ! isConnected() )
    {
        return;
    }

    if ( ! M_transport->sendMessage( QByteArray::fromStdString( com ) ) )
    {
        std::cerr << PACKAGE_NAME << " failed to send: " << com << std::endl;
        return;
    }

    std::cerr << PACKAGE_NAME << " send: " << com << std::endl;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
MonitorClient::sendCommand( const rcsc::MonitorCommand & com )
{
    std::ostringstream ostr;
    com.toCommandString( ostr );

    sendCommand( ostr.str() );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
MonitorClient::sendCommand( const rcsc::TrainerCommand & com )
{
    std::ostringstream ostr;
    com.toCommandString( ostr );

    sendCommand( ostr.str() );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
MonitorClient::sendDispInit()
{
    rcsc::MonitorInitCommand com( M_version );

    sendCommand( com );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
MonitorClient::sendDispBye()
{
    rcsc::MonitorByeCommand com;

    sendCommand( com );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
MonitorClient::sendKickOff()
{
    rcsc::MonitorKickOffCommand com;

    sendCommand( com );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
MonitorClient::sendFreeKick( const double & x,
                             const double & y,
                             const rcsc::SideID side )
{
    rcsc::MonitorFreeKickCommand com( x, y, side );

    sendCommand( com );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
MonitorClient::sendMove( const rcsc::SideID side,
                         const int unum,
                         const double & x, const double & y,
                         const double & angle )
{
    if ( side == rcsc::NEUTRAL )
    {
        std::cerr << __FILE__ << ": (sendMove)"
                  << " dispplayer invalie side" << std::endl;
        return;
    }

    if ( unum < 1 || 11 < unum )
    {
        std::cerr << __FILE__ << ": (sendMove)"
                  << " dispplayer invalie unum " << unum << std::endl;
        return;
    }

    rcsc::MonitorMovePlayerCommand com( side, unum, x, y, angle );

    sendCommand( com );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
MonitorClient::sendDiscard( const rcsc::SideID side,
                            const int unum )
{
    if ( side == rcsc::NEUTRAL )
    {
        std::cerr << __FILE__ << ": (sendDiscard)"
                  << " dispdiscard Invalie side" << std::endl;
        return;
    }

    if ( unum < 1 || 11 < unum )
    {
        std::cerr << __FILE__ << ": (sendDiscard)"
                  << " dispdiscard invalie unum " << unum << std::endl;
        return;
    }

    rcsc::MonitorDiscardPlayerCommand com( side, unum );

    sendCommand( com );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
MonitorClient::sendCard( const rcsc::SideID side,
                         const int unum,
                         const rcsc::Card card )
{
    if ( side == rcsc::NEUTRAL )
    {
        std::cerr << __FILE__ << ": (sendCard)"
                  << " dispcard invalid side" << std::endl;
        return;
    }

    if ( unum < 1 || 11 < unum )
    {
        std::cerr << __FILE__ << ": (sendCard)"
                  << " dispcard invalid unum " << unum << std::endl;
        return;
    }

    if ( card != rcsc::YELLOW
         && card != rcsc::RED )
    {
        std::cerr << __FILE__ << ": (sendCard)"
                  << " dispcard invalid card " << card << std::endl;
        return;
    }

    rcsc::MonitorCardCommand com( side, unum, card );

    sendCommand( com );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
MonitorClient::sendChangeMode( const rcsc::PlayMode pmode )
{
    rcsc::TrainerChangeModeCommand com( pmode );

    sendCommand( com );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
MonitorClient::sendTrainerMoveBall( const double & x,
                                    const double & y )
{
    rcsc::TrainerMoveBallCommand com( x, y );

    sendCommand( com );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
MonitorClient::sendTrainerMoveBall( const double & x,
                                    const double & y,
                                    const double & vx,
                                    const double & vy )
{
    rcsc::TrainerMoveBallCommand com( x, y, vx, vy );

    sendCommand( com );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
MonitorClient::sendTrainerMovePlayer( const std::string & team_name,
                                      const int unum,
                                      const double & x,
                                      const double & y )
{
    rcsc::TrainerMovePlayerCommand com( team_name, unum,
                                        x, y );

    sendCommand( com );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
MonitorClient::sendTrainerMovePlayer( const std::string & team_name,
                                      const int unum,
                                      const double & x,
                                      const double & y,
                                      const double & angle )
{
    rcsc::TrainerMovePlayerCommand com( team_name, unum,
                                        x, y,
                                        angle );

    sendCommand( com );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
MonitorClient::sendTrainerMovePlayer( const std::string & team_name,
                                      const int unum,
                                      const double & x,
                                      const double & y,
                                      const double & angle,
                                      const double & vx,
                                      const double & vy )
{
    rcsc::TrainerMovePlayerCommand com( team_name, unum,
                                        x, y,
                                        angle,
                                        vx, vy );

    sendCommand( com );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
MonitorClient::sendRecover()
{
    rcsc::TrainerRecoverCommand com;

    sendCommand( com );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
MonitorClient::sencChangePlayerType( const std::string & team_name,
                                     const int unum,
                                     const int type )
{
    rcsc::TrainerChangePlayerTypeCommand com( team_name, unum, type );

    sendCommand( com );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
MonitorClient::sendCheckBal()
{
    rcsc::TrainerCheckBallCommand com;

    sendCommand( com );
}
