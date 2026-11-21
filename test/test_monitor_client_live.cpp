// -*-c++-*-

/*!
  \file test_monitor_client_live.cpp
  \brief test of MonitorClient with a real rcssserver (run locally).

  The test is run only when the environment variable RCSSSERVER is set to
  the path of the rcssserver executable (the develop branch or later).
  Otherwise, it exits with the status 77, which CTest reports as skipped.

    RCSSSERVER=/path/to/rcssserver ctest --test-dir build -R monitor_client_live --output-on-failure

  rcssserver is started with a temporary RCSS_CONF_DIR (~/.rcssserver is
  not used) and the port numbers different from the usual ones
  (UDP 16000, TCP 16100, WebSocket 16180), so that the test does not
  interfere with a running rcssserver. server::max_monitors is set to 1:
  if (dispbye) is not delivered, the next connection is refused.
  server::coach_w_referee is set to true, because the server accepts the
  trainer commands from the monitors only in the coach mode.

  For each transport and protocol version, the test checks that
  - the client connects and receives the frames,
  - a command (change_mode) reaches the server, and
  - the server refuses the second monitor with (error no_more_monitor).

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
#include "view_holder.h"

#include <rcsc/types.h>

#include <cstdio>
#include <memory>

namespace {

const quint16 UDP_PORT = 16000;
const quint16 TCP_PORT = 16100;
const quint16 WS_PORT = 16180;

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

}

/*-------------------------------------------------------------------*/
/*!
  \class MonitorClientLiveTest
*/
class MonitorClientLiveTest
    : public QObject {

    Q_OBJECT

private:

    QString M_server_path;
    QTemporaryDir M_server_dir;
    QProcess M_server;

    std::unique_ptr< MainData > M_main_data;
    QObject M_parent;

public:

    explicit
    MonitorClientLiveTest( const QString & server_path )
        : M_server_path( server_path )
      { }

private:

    //! create a client. the parent of the client is M_parent.
    MonitorClient * createClient( const MonitorTransport::Type type,
                                  const int version,
                                  MainData & main_data )
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

          return new MonitorClient( &M_parent, main_data, transport, version );
      }

    static
    quint16 portOf( const MonitorTransport::Type type )
      {
          switch ( type ) {
          case MonitorTransport::TCP:
              return TCP_PORT;
          case MonitorTransport::WebSocket:
              return WS_PORT;
          default:
              break;
          }
          return UDP_PORT;
      }

    //! wait until rcssserver accepts a TCP connection
    bool waitForServer()
      {
          QElapsedTimer timer;
          timer.start();
          while ( timer.elapsed() < 10 * 1000 )
          {
              if ( M_server.state() != QProcess::Running )
              {
                  return false;
              }

              QTcpSocket probe;
              probe.connectToHost( QHostAddress::LocalHost, TCP_PORT );
              if ( probe.waitForConnected( 200 ) )
              {
                  probe.disconnectFromHost();
                  return true;
              }
              QTest::qWait( 100 );
          }
          return false;
      }

private slots:

    void initTestCase()
      {
          g_prev_handler = qInstallMessageHandler( message_handler );

          QVERIFY( M_server_dir.isValid() );
          const QString conf_dir = M_server_dir.path() + QLatin1String( "/conf" );
          QVERIFY( QDir().mkpath( conf_dir ) );

          QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
          env.insert( QStringLiteral( "RCSS_CONF_DIR" ), conf_dir );
          M_server.setProcessEnvironment( env );
          M_server.setWorkingDirectory( M_server_dir.path() );
          M_server.setProcessChannelMode( QProcess::MergedChannels );
          M_server.setStandardOutputFile( M_server_dir.path() + QLatin1String( "/rcssserver.log" ) );

          M_server.start( M_server_path,
                          QStringList()
                          << QStringLiteral( "server::port=%1" ).arg( UDP_PORT )
                          << QStringLiteral( "server::coach_port=%1" ).arg( UDP_PORT + 1 )
                          << QStringLiteral( "server::olcoach_port=%1" ).arg( UDP_PORT + 2 )
                          << QStringLiteral( "server::tcp_port=%1" ).arg( TCP_PORT )
                          << QStringLiteral( "server::ws_port=%1" ).arg( WS_PORT )
                          << QStringLiteral( "server::max_monitors=1" )
                          // accept the trainer commands (e.g. change_mode) from the monitor
                          << QStringLiteral( "server::coach_w_referee=true" )
                          << QStringLiteral( "server::game_logging=false" )
                          << QStringLiteral( "server::text_logging=false" ) );
          QVERIFY2( M_server.waitForStarted( 5000 ),
                    qPrintable( "failed to start " + M_server_path ) );
          QVERIFY2( waitForServer(), "rcssserver is not ready" );
      }

    void cleanupTestCase()
      {
          if ( M_server.state() != QProcess::NotRunning )
          {
              M_server.terminate();
              if ( ! M_server.waitForFinished( 5000 ) )
              {
                  M_server.kill();
                  M_server.waitForFinished( 5000 );
              }
          }

          QFile log( M_server_dir.path() + QLatin1String( "/rcssserver.log" ) );
          if ( log.open( QIODevice::ReadOnly ) )
          {
              qDebug( "rcssserver output:\n%s", log.readAll().constData() );
          }

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
          // delete the clients. (dispbye) is sent to the server.
          qDeleteAll( M_parent.children() );
          M_main_data.reset();

          // let the server remove the monitor before the next test
          QTest::qWait( 300 );

          QVERIFY2( g_connect_warnings.isEmpty(),
                    qPrintable( g_connect_warnings.join( QLatin1Char( '\n' ) ) ) );
      }

    void connectAndCommand_data()
      {
          QTest::addColumn< int >( "type" );
          QTest::addColumn< int >( "version" );

          for ( int version : { -1, 5, 4, 3, 2, 1 } )
          {
              QTest::newRow( qPrintable( QStringLiteral( "udp v%1" ).arg( version ) ) )
                  << int( MonitorTransport::UDP ) << version;
          }
      }

    void connectAndCommand()
      {
          QFETCH( int, type );
          QFETCH( int, version );

          MonitorClient * client = createClient( static_cast< MonitorTransport::Type >( type ),
                                                 version,
                                                 *M_main_data );
          QVERIFY( client );

          QSignalSpy received( client, &MonitorClient::received );
          QSignalSpy disconnect_requested( client, &MonitorClient::disconnectRequested );
          QSignalSpy error_message( client, &MonitorClient::errorMessage );

          QString error;
          QVERIFY2( client->start( QStringLiteral( "127.0.0.1" ),
                                   portOf( static_cast< MonitorTransport::Type >( type ) ),
                                   &error ),
                    qPrintable( error ) );

          // the frames arrive (a cycle is 100 ms).
          // ViewHolder keeps only the last frame of before_kick_off,
          // so the number of the received messages is checked.
          QTRY_VERIFY_WITH_TIMEOUT( received.count() >= 5, 5000 );
          QVERIFY( ! M_main_data->viewHolder().monitorViewCont().empty() );
          QVERIFY( client->isConnected() );

          // a command reaches the server.
          // change the play mode to the one different from the current one.
          const rcsc::PlayMode target = ( M_main_data->viewHolder().lastPlayMode() == rcsc::PM_FreeKick_Left
                                          ? rcsc::PM_FreeKick_Right
                                          : rcsc::PM_FreeKick_Left );
          client->sendChangeMode( target );
          QTRY_COMPARE_WITH_TIMEOUT( M_main_data->viewHolder().lastPlayMode(), target, 3000 );

          QCOMPARE( disconnect_requested.count(), 0 );
          QVERIFY2( error_message.isEmpty(),
                    error_message.isEmpty() ? "" : qPrintable( error_message.first().first().toString() ) );
      }

    void noMoreMonitor_data()
      {
          QTest::addColumn< int >( "type" );

          QTest::newRow( "udp" ) << int( MonitorTransport::UDP );
      }

    void noMoreMonitor()
      {
          QFETCH( int, type );

          // the first monitor
          MonitorClient * first = createClient( MonitorTransport::UDP, 5, *M_main_data );
          QVERIFY( first );
          QString error;
          QVERIFY2( first->start( QStringLiteral( "127.0.0.1" ), UDP_PORT, &error ),
                    qPrintable( error ) );
          QTRY_VERIFY_WITH_TIMEOUT( M_main_data->viewHolder().monitorViewCont().size() >= 1, 5000 );

          // the second monitor is refused by server::max_monitors=1
          MainData second_data;
          MonitorClient * second = createClient( static_cast< MonitorTransport::Type >( type ),
                                                 5,
                                                 second_data );
          QVERIFY( second );
          QSignalSpy disconnect_requested( second, &MonitorClient::disconnectRequested );
          QSignalSpy error_message( second, &MonitorClient::errorMessage );
          QVERIFY2( second->start( QStringLiteral( "127.0.0.1" ),
                                   portOf( static_cast< MonitorTransport::Type >( type ) ),
                                   &error ),
                    qPrintable( error ) );

          QTRY_COMPARE_WITH_TIMEOUT( disconnect_requested.count(), 1, 3000 );
          QCOMPARE( error_message.count(), 1 );
          QVERIFY2( error_message.first().first().toString().contains( QLatin1String( "no_more_monitor" ) ),
                    qPrintable( error_message.first().first().toString() ) );

          delete second;
      }
};

/*-------------------------------------------------------------------*/
int
main( int argc, char ** argv )
{
    const QByteArray server_path = qgetenv( "RCSSSERVER" );
    if ( server_path.isEmpty() )
    {
        std::printf( "SKIP: set RCSSSERVER to the path of rcssserver to run this test.\n" );
        return 77; // skipped
    }

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
    MonitorClientLiveTest test( QFile::decodeName( server_path ) );
    return QTest::qExec( &test, argc, argv );
}

#include "test_monitor_client_live.moc"
