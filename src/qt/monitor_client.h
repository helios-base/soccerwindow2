// -*-c++-*-

/*!
  \file monitor_client.h
  \brief Monitor Client class Header File.
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

#ifndef SOCCERWINDOW2_QT_MONITOR_CLIENT_H
#define SOCCERWINDOW2_QT_MONITOR_CLIENT_H

#include <QObject>
#include <QString>

#include <rcsc/types.h>

#include <string>

class QByteArray;
class QTimer;

namespace rcsc {
class MonitorCommand;
class TrainerCommand;
}

class MainData;
class MonitorTransport;

/*!
  \class MonitorClient
  \brief monitor client that connects to the rcssserver.

  The messages are sent and received by MonitorTransport (UDP, TCP or WebSocket).
*/
class MonitorClient
    : public QObject {

    Q_OBJECT

private:

    MainData & M_main_data;

    MonitorTransport * M_transport;
    QTimer * M_timer;

    int M_version; //!< protocol version

    int M_waited_msec;

    bool M_transport_connected; //!< true after the transport is connected
    bool M_first_message_received; //!< true after the first message from the server


    //! not used
    MonitorClient() = delete;
    MonitorClient( const MonitorClient & ) = delete;
    MonitorClient & operator=( const MonitorClient & ) = delete;
public:

    /*!
      \brief constructor
      \param parent parent object
      \param main_data the received data are stored in this object
      \param transport transport instance. its ownership is moved to this object.
      \param version monitor protocol version
     */
    MonitorClient( QObject * parent,
                   MainData & main_data,
                   MonitorTransport * transport,
                   const int version );

    ~MonitorClient();

    /*!
      \brief start the connection.
      \param host host name or IP address of the server
      \param port port number of the server
      \param error the reason of the failure is stored, if not null
      \return false only if the failure is detected immediately

      dispinit is sent when the transport is connected.
     */
    bool start( const QString & host,
                quint16 port,
                QString * error );

    /*!
      \brief send dispbye if connected, and close the connection.
      No signal is emitted after this method.
     */
    void disconnect();

    bool isConnected() const;

    //! get the peer description, e.g. "udp://127.0.0.1:6000"
    QString peerName() const;

private:

    bool handleMessage( const QByteArray & msg );
    void requestDisconnect( const QString & reason );

    void sendCommand( const std::string & com );
    void sendCommand( const rcsc::MonitorCommand & com );
    void sendCommand( const rcsc::TrainerCommand & com );

public:

    // monitor command

    void sendDispInit();
    void sendDispBye();
    void sendKickOff();
    void sendFreeKick( const double & x,
                       const double & y,
                       const rcsc::SideID side );
    void sendDropBall( const double & x,
                       const double & y )
      {
          sendFreeKick( x, y, rcsc::NEUTRAL );
      }
    void sendFreeKickLeft( const double & x,
                           const double & y )
      {
          sendFreeKick( x, y, rcsc::LEFT );
      }
    void sendFreeKickRight( const double & x,
                            const double & y )
      {
          sendFreeKick( x, y, rcsc::RIGHT );
      }

    void sendMove( const rcsc::SideID side,
                   const int unum,
                   const double & x,
                   const double & y,
                   const double & angle );

    void sendDiscard( const rcsc::SideID side,
                      const int unum );

    void sendCard( const rcsc::SideID side,
                   const int unum,
                   const rcsc::Card card );


    // trainer command

    void sendChangeMode( const rcsc::PlayMode pmode );

    void sendTrainerMoveBall( const double & x,
                              const double & y );

    void sendTrainerMoveBall( const double & x,
                              const double & y,
                              const double & vx,
                              const double & vy );

    void sendTrainerMovePlayer( const std::string & team_name,
                                const int unum,
                                const double & x,
                                const double & y );
    void sendTrainerMovePlayer( const std::string & team_name,
                                const int unum,
                                const double & x,
                                const double & y,
                                const double & angle );
    void sendTrainerMovePlayer( const std::string & team_name,
                                const int unum,
                                const double & x,
                                const double & y,
                                const double & angle,
                                const double & vx,
                                const double & vy );


    void sendRecover();

    void sencChangePlayerType( const std::string & team_name,
                               const int unum,
                               const int type );

    void sendCheckBal();

private slots:

    void handleConnected();
    void handleMessages();
    void handleDisconnected( const QString & reason );
    void handleTimer();

signals:

    //! emitted when some messages have been received.
    void received();

    /*!
      \brief emitted when the connection is lost, or no message is received
      for Options::waitSeconds(). The receiver should delete this object
      with deleteLater().
     */
    void disconnectRequested();

    //! error message to be shown in the status bar
    void errorMessage( const QString & msg );

};

#endif
