// -*-c++-*-

/*!
  \file test_monitor_client.cpp
  \brief unit test of MonitorClient with fake servers (QTcpServer, QUdpSocket).

  The test checks that
  - dispinit is sent after the connection is established, and the commands
    are sent with the delimiter of the transport,
  - the received messages are passed to MainData, and received() is emitted,
  - an "(error ...)" reply to dispinit is a connection failure, but a later
    one is not,
  - the disconnection by the server, a refused connection, and no message
    for --wait-seconds are told by errorMessage() and disconnectRequested()
    once,
  - the protocol versions 1 and 2 are refused over TCP before connecting,
  - disconnect() sends (dispbye) and emits no signal, and
  - the UDP transport sends the commands to the port that the server
    replied from.

  HOME, XDG_CONFIG_HOME and XDG_RUNTIME_DIR are set to a temporary directory,
  so the user's ~/.soccerwindow2 is neither read nor modified.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <QtCore>
#include <QtNetwork>
#include <QtTest>

#include "monitor_client.h"
#include "monitor_transport.h"
#include "main_data.h"
#include "options.h"
#include "view_holder.h"

#include <rcsc/types.h>

#include <cstring>
#include <memory>
#include <vector>

namespace {

QStringList g_connect_warnings;
QtMessageHandler g_prev_handler = nullptr;

void
message_handler( QtMsgType type,
                 const QMessageLogContext & context,
                 const QString & msg )
{
    // a signal or a slot connected by its name is not found
    if ( msg.contains( QLatin1String( "QObject::connect" ) ) )
    {
        g_connect_warnings << msg;
    }

    if ( g_prev_handler )
    {
        g_prev_handler( type, context, msg );
    }
}

//! a port of the local host where nothing listens
quint16
closed_port()
{
    QTcpServer server;
    server.listen( QHostAddress::LocalHost, 0 );
    return server.serverPort(); // closed when the server is gone
}

/*!
  \brief create a show message of the monitor protocol version 4 or later,
  in the same format as rcssserver (players at the kick off positions).
*/
QByteArray
show_message( const int time,
              const int playmode )
{
    QByteArray msg = QByteArray( "(show " ) + QByteArray::number( time )
        + " (pm " + QByteArray::number( playmode ) + ")"
        + " (tm null null 0 0)"
        + " ((b) 0 0 0 0)";
    for ( const char side : { 'l', 'r' } )
    {
        for ( int unum = 1; unum <= 11; ++unum )
        {
            const int x = ( side == 'l' ? -3 : 3 ) * unum;
            msg += " ((" + QByteArray( 1, side ) + " " + QByteArray::number( unum ) + ")"
                + " 0 0 " + QByteArray::number( x ) + " -37 0 0 0 0"
                + " (v h 90) (fp 0 0) (s 8000 1 1 130600)"
                + " (c 0 0 0 0 0 0 0 0 0 0 0 0))";
        }
    }
    msg += ")";
    return msg;
}

//! a message with the terminating NUL, as sent over TCP
QByteArray
with_nul( const char * msg )
{
    return QByteArray( msg, static_cast< int >( std::strlen( msg ) ) + 1 );
}

//! set the value of --wait-seconds through the command line parser
bool
set_wait_seconds( const int sec )
{
    QByteArray value = QByteArray::number( sec );
    std::vector< char * > argv = { const_cast< char * >( "test_monitor_client" ),
                                   const_cast< char * >( "--wait-seconds" ),
                                   value.data() };
    return Options::instance().parseCmdLine( static_cast< int >( argv.size() ), argv.data() );
}

}

/*-------------------------------------------------------------------*/
/*!
  \class MonitorClientTest
*/
class MonitorClientTest
    : public QObject {

    Q_OBJECT

private:

    std::unique_ptr< MainData > M_main_data;
    QObject M_parent;

    //! create a client. the parent of the client is M_parent.
    MonitorClient * createClient( const MonitorTransport::Type type,
                                  const int version )
      {
          QString error;
          MonitorTransport * transport = create_monitor_transport( type,
                                                                   QStringLiteral( "/monitor" ),
                                                                   &M_parent,
                                                                   &error );
          if ( ! transport )
          {
              qWarning( "failed to create the transport: %s", qPrintable( error ) );
              return nullptr;
          }

          return new MonitorClient( &M_parent, *M_main_data, transport, version );
      }

    //! read from the socket until the size of the data reaches the size
    static
    bool readUntil( QIODevice * socket,
                    QByteArray & data,
                    const int size )
      {
          QElapsedTimer timer;
          timer.start();
          while ( data.size() < size
                  && timer.elapsed() < 5000 )
          {
              data += socket->readAll();
              if ( data.size() < size )
              {
                  QTest::qWait( 10 );
              }
          }
          return data.size() >= size;
      }

    //! read one message terminated by NUL from the socket
    static
    QByteArray readMessage( QIODevice * socket,
                            QByteArray & buffer )
      {
          QElapsedTimer timer;
          timer.start();
          while ( timer.elapsed() < 5000 )
          {
              const int pos = buffer.indexOf( '\0' );
              if ( pos >= 0 )
              {
                  const QByteArray msg = buffer.left( pos );
                  buffer.remove( 0, pos + 1 );
                  return msg;
              }
              buffer += socket->readAll();
              if ( buffer.indexOf( '\0' ) < 0 )
              {
                  QTest::qWait( 10 );
              }
          }
          return QByteArray();
      }

private slots:

    void initTestCase()
      {
          g_prev_handler = qInstallMessageHandler( message_handler );
          QCOMPARE( Options::instance().waitSeconds(), 5 );
          QVERIFY( ! Options::instance().autoQuitMode() );
      }

    void cleanupTestCase()
      {
          qInstallMessageHandler( g_prev_handler );
          g_prev_handler = nullptr;
      }

    void init()
      {
          g_connect_warnings.clear();
          M_main_data.reset( new MainData() );
      }

    void cleanup()
      {
          qDeleteAll( M_parent.children() );
          M_main_data.reset();

          const QStringList warnings = g_connect_warnings;
          g_connect_warnings.clear();
          QVERIFY2( warnings.isEmpty(),
                    qPrintable( warnings.join( QLatin1Char( '\n' ) ) ) );
      }

    /*!
      TCP: dispinit after the connection, the received messages, the
      commands, and disconnect().
     */
    void tcpSession()
      {
          QTcpServer server;
          QVERIFY( server.listen( QHostAddress::LocalHost, 0 ) );

          MonitorClient * client = createClient( MonitorTransport::TCP, 5 );
          QVERIFY( client );
          QSignalSpy received( client, &MonitorClient::received );
          QSignalSpy disconnect_requested( client, &MonitorClient::disconnectRequested );
          QSignalSpy error_message( client, &MonitorClient::errorMessage );

          QString error;
          QVERIFY2( client->start( QStringLiteral( "127.0.0.1" ), server.serverPort(), &error ),
                    qPrintable( error ) );
          QCOMPARE( client->peerName(),
                    QStringLiteral( "tcp://127.0.0.1:%1" ).arg( server.serverPort() ) );

          QTRY_VERIFY( server.hasPendingConnections() );
          QTcpSocket * peer = server.nextPendingConnection();
          QByteArray buffer;

          // dispinit after the connection
          QCOMPARE( readMessage( peer, buffer ), QByteArray( "(dispinit version 5)" ) );
          QVERIFY( client->isConnected() );

          // the received messages are passed to MainData. the second one comes in pieces.
          const QByteArray show0 = show_message( 0, rcsc::PM_BeforeKickOff );
          const QByteArray show1 = show_message( 1, rcsc::PM_PlayOn );
          peer->write( show0 + '\0' + show1.left( 100 ) );
          QTRY_VERIFY( received.count() >= 1 );
          QCOMPARE( M_main_data->viewHolder().lastPlayMode(), rcsc::PM_BeforeKickOff );
          peer->write( show1.mid( 100 ) + '\0' );
          QTRY_COMPARE( M_main_data->viewHolder().lastPlayMode(), rcsc::PM_PlayOn );
          QVERIFY( ! M_main_data->viewHolder().monitorViewCont().empty() );

          // an error reply to a command does not close the connection
          peer->write( with_nul( "(error illegal_command_form)" ) );
          QTest::qWait( 100 );
          QVERIFY( client->isConnected() );

          // the commands
          client->sendKickOff();
          QCOMPARE( readMessage( peer, buffer ), QByteArray( "(dispstart)" ) );
          client->sendChangeMode( rcsc::PM_FreeKick_Left );
          QCOMPARE( readMessage( peer, buffer ), QByteArray( "(change_mode free_kick_l)" ) );

          // disconnect(): dispbye, then the connection is closed. no signal.
          QSignalSpy peer_closed( peer, &QTcpSocket::disconnected );
          client->disconnect();
          QVERIFY( ! client->isConnected() );
          QTRY_COMPARE( peer_closed.count(), 1 );
          QCOMPARE( readMessage( peer, buffer ), QByteArray( "(dispbye)" ) );

          QTest::qWait( 100 );
          QCOMPARE( disconnect_requested.count(), 0 );
          QCOMPARE( error_message.count(), 0 );
      }

    /*!
      TCP: an error reply to dispinit (e.g. server::max_monitors) is a
      connection failure.
     */
    void tcpErrorReply()
      {
          QTcpServer server;
          QVERIFY( server.listen( QHostAddress::LocalHost, 0 ) );

          MonitorClient * client = createClient( MonitorTransport::TCP, 5 );
          QVERIFY( client );
          QSignalSpy received( client, &MonitorClient::received );
          QSignalSpy disconnect_requested( client, &MonitorClient::disconnectRequested );
          QSignalSpy error_message( client, &MonitorClient::errorMessage );

          QString error;
          QVERIFY2( client->start( QStringLiteral( "127.0.0.1" ), server.serverPort(), &error ),
                    qPrintable( error ) );
          QTRY_VERIFY( server.hasPendingConnections() );
          QTcpSocket * peer = server.nextPendingConnection();
          QByteArray buffer;
          QCOMPARE( readMessage( peer, buffer ), QByteArray( "(dispinit version 5)" ) );

          peer->write( with_nul( "(error no_more_monitor)" ) );
          QTRY_COMPARE( disconnect_requested.count(), 1 );
          QCOMPARE( error_message.count(), 1 );
          QVERIFY2( error_message.at( 0 ).at( 0 ).toString().contains( QLatin1String( "no_more_monitor" ) ),
                    qPrintable( error_message.at( 0 ).at( 0 ).toString() ) );
          QCOMPARE( received.count(), 0 );
      }

    /*!
      TCP: the server closes the connection. told once.
     */
    void tcpServerClosed()
      {
          QTcpServer server;
          QVERIFY( server.listen( QHostAddress::LocalHost, 0 ) );

          MonitorClient * client = createClient( MonitorTransport::TCP, 5 );
          QVERIFY( client );
          QSignalSpy disconnect_requested( client, &MonitorClient::disconnectRequested );
          QSignalSpy error_message( client, &MonitorClient::errorMessage );

          QString error;
          QVERIFY2( client->start( QStringLiteral( "127.0.0.1" ), server.serverPort(), &error ),
                    qPrintable( error ) );
          QTRY_VERIFY( server.hasPendingConnections() );
          QTcpSocket * peer = server.nextPendingConnection();
          QByteArray buffer;
          QCOMPARE( readMessage( peer, buffer ), QByteArray( "(dispinit version 5)" ) );

          peer->disconnectFromHost();
          QTRY_COMPARE( disconnect_requested.count(), 1 );
          QCOMPARE( error_message.count(), 1 );
          QVERIFY2( error_message.at( 0 ).at( 0 ).toString().startsWith( QLatin1String( "Disconnected from the server" ) ),
                    qPrintable( error_message.at( 0 ).at( 0 ).toString() ) );
          QVERIFY( ! client->isConnected() );

          QTest::qWait( 100 );
          QCOMPARE( disconnect_requested.count(), 1 );
      }

    /*!
      TCP: no server. told once with the reason.
     */
    void tcpRefused()
      {
          MonitorClient * client = createClient( MonitorTransport::TCP, 5 );
          QVERIFY( client );
          QSignalSpy disconnect_requested( client, &MonitorClient::disconnectRequested );
          QSignalSpy error_message( client, &MonitorClient::errorMessage );

          QString error;
          QVERIFY2( client->start( QStringLiteral( "127.0.0.1" ), closed_port(), &error ),
                    qPrintable( error ) );
          QTRY_COMPARE_WITH_TIMEOUT( disconnect_requested.count(), 1, 4000 );
          QCOMPARE( error_message.count(), 1 );
          QVERIFY2( error_message.at( 0 ).at( 0 ).toString().startsWith( QLatin1String( "Failed to connect to the server" ) ),
                    qPrintable( error_message.at( 0 ).at( 0 ).toString() ) );

          QTest::qWait( 100 );
          QCOMPARE( disconnect_requested.count(), 1 );
      }

    /*!
      TCP: no message for --wait-seconds.
     */
    void tcpNoMessage()
      {
          QVERIFY( set_wait_seconds( 1 ) );
          QCOMPARE( Options::instance().waitSeconds(), 1 );

          QTcpServer server;
          QVERIFY( server.listen( QHostAddress::LocalHost, 0 ) );

          MonitorClient * client = createClient( MonitorTransport::TCP, 5 );
          QVERIFY( client );
          QSignalSpy disconnect_requested( client, &MonitorClient::disconnectRequested );
          QSignalSpy error_message( client, &MonitorClient::errorMessage );

          QString error;
          const bool started = client->start( QStringLiteral( "127.0.0.1" ), server.serverPort(), &error );
          if ( started )
          {
              QTRY_COMPARE_WITH_TIMEOUT( disconnect_requested.count(), 1, 3000 );
          }

          QVERIFY( set_wait_seconds( 5 ) );

          QVERIFY2( started, qPrintable( error ) );
          QCOMPARE( error_message.count(), 1 );
          QCOMPARE( error_message.at( 0 ).at( 0 ).toString(),
                    QStringLiteral( "No message from the server for 1 seconds." ) );
      }

    /*!
      the protocol versions that cannot be used are refused before connecting.
     */
    void versionCheck_data()
      {
          QTest::addColumn< int >( "type" );
          QTest::addColumn< int >( "version" );
          QTest::addColumn< bool >( "ok" );

          QTest::newRow( "udp v1" ) << int( MonitorTransport::UDP ) << 1 << true;
          QTest::newRow( "udp v2" ) << int( MonitorTransport::UDP ) << 2 << true;
          QTest::newRow( "udp v0" ) << int( MonitorTransport::UDP ) << 0 << false;
          QTest::newRow( "udp v6" ) << int( MonitorTransport::UDP ) << 6 << false;
          QTest::newRow( "tcp v1" ) << int( MonitorTransport::TCP ) << 1 << false;
          QTest::newRow( "tcp v2" ) << int( MonitorTransport::TCP ) << 2 << false;
          QTest::newRow( "tcp v3" ) << int( MonitorTransport::TCP ) << 3 << true;
          QTest::newRow( "tcp v-1" ) << int( MonitorTransport::TCP ) << -1 << true;
          QTest::newRow( "tcp v6" ) << int( MonitorTransport::TCP ) << 6 << false;
      }

    void versionCheck()
      {
          QFETCH( int, type );
          QFETCH( int, version );
          QFETCH( bool, ok );

          // a server so that the TCP connections succeed
          QTcpServer server;
          QVERIFY( server.listen( QHostAddress::LocalHost, 0 ) );

          MonitorClient * client = createClient( static_cast< MonitorTransport::Type >( type ), version );
          QVERIFY( client );

          QString error;
          QCOMPARE( client->start( QStringLiteral( "127.0.0.1" ), server.serverPort(), &error ), ok );
          QCOMPARE( error.isEmpty(), ok );
          if ( ! ok )
          {
              QVERIFY2( error.contains( QString::number( version ) ), qPrintable( error ) );
          }
      }

    /*!
      UDP: dispinit is sent to the given port, and the commands are sent to
      the port that the server replied from.
     */
    void udpSession()
      {
          QUdpSocket server;
          QVERIFY( server.bind( QHostAddress( QHostAddress::LocalHost ), 0 ) );
          // the port dedicated to the monitor (as rcssserver does)
          QUdpSocket dedicated;
          QVERIFY( dedicated.bind( QHostAddress( QHostAddress::LocalHost ), 0 ) );

          MonitorClient * client = createClient( MonitorTransport::UDP, 5 );
          QVERIFY( client );
          QSignalSpy received( client, &MonitorClient::received );
          QSignalSpy disconnect_requested( client, &MonitorClient::disconnectRequested );

          QString error;
          QVERIFY2( client->start( QStringLiteral( "127.0.0.1" ), server.localPort(), &error ),
                    qPrintable( error ) );
          QCOMPARE( client->peerName(),
                    QStringLiteral( "udp://127.0.0.1:%1" ).arg( server.localPort() ) );

          // dispinit with the terminating NUL
          QTRY_VERIFY( server.hasPendingDatagrams() );
          QNetworkDatagram dispinit = server.receiveDatagram();
          QCOMPARE( dispinit.data(), with_nul( "(dispinit version 5)" ) );

          // reply from the dedicated port
          const QByteArray show = show_message( 0, rcsc::PM_BeforeKickOff ) + '\0';
          dedicated.writeDatagram( show, dispinit.senderAddress(), static_cast< quint16 >( dispinit.senderPort() ) );
          QTRY_VERIFY( received.count() >= 1 );
          QCOMPARE( M_main_data->viewHolder().lastPlayMode(), rcsc::PM_BeforeKickOff );

          // the command goes to the dedicated port
          client->sendKickOff();
          QTRY_VERIFY( dedicated.hasPendingDatagrams() );
          QCOMPARE( dedicated.receiveDatagram().data(), with_nul( "(dispstart)" ) );
          QVERIFY( ! server.hasPendingDatagrams() );

          // dispbye also goes to the dedicated port
          client->disconnect();
          QTRY_VERIFY( dedicated.hasPendingDatagrams() );
          QCOMPARE( dedicated.receiveDatagram().data(), with_nul( "(dispbye)" ) );
          QCOMPARE( disconnect_requested.count(), 0 );
      }
};

/*-------------------------------------------------------------------*/
int
main( int argc, char ** argv )
{
    // Use a temporary home directory so that ~/.soccerwindow2 is not touched.
    QTemporaryDir tmp_dir;
    if ( ! tmp_dir.isValid() )
    {
        qCritical( "failed to create a temporary directory" );
        return 1;
    }

    const QString home = tmp_dir.path() + QLatin1String( "/home" );
    const QString runtime = tmp_dir.path() + QLatin1String( "/runtime" );
    if ( ! QDir().mkpath( home )
         || ! QDir().mkpath( runtime )
         || ! QFile::setPermissions( runtime, QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner ) )
    {
        qCritical( "failed to set up the temporary directories" );
        return 1;
    }

    qputenv( "HOME", QFile::encodeName( home ) );
    qputenv( "XDG_CONFIG_HOME", QFile::encodeName( home + QLatin1String( "/.config" ) ) );
    qputenv( "XDG_RUNTIME_DIR", QFile::encodeName( runtime ) );

    QCoreApplication app( argc, argv );
    MonitorClientTest test;
    return QTest::qExec( &test, argc, argv );
}

#include "test_monitor_client.moc"
