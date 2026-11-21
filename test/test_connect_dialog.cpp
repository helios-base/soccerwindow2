// -*-c++-*-

/*!
  \file test_connect_dialog.cpp
  \brief unit test of ConnectDialog (the "Connect to ..." dialog).

  The test checks that
  - the dialog shows the given values and returns the edited values
    (the host name and the path without the leading and trailing spaces),
  - the WebSocket path can be edited only for WebSocket,
  - the Connect button is disabled for an empty host name, and
  - WebSocket can be selected only in a build with Qt WebSockets.

  The test always uses the offscreen platform (no window is shown).
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <QtWidgets>
#include <QtTest>

#include "connect_dialog.h"

namespace {

//! the line edit whose text is the given text
QLineEdit *
find_line_edit( QWidget * parent,
                const QString & text )
{
    for ( QLineEdit * edit : parent->findChildren< QLineEdit * >() )
    {
        if ( edit->text() == text )
        {
            return edit;
        }
    }
    return nullptr;
}

QPushButton *
connect_button( QWidget * parent )
{
    QDialogButtonBox * buttons = parent->findChild< QDialogButtonBox * >();
    return buttons ? buttons->button( QDialogButtonBox::Ok ) : nullptr;
}

}

/*-------------------------------------------------------------------*/
/*!
  \class ConnectDialogTest
*/
class ConnectDialogTest
    : public QObject {

    Q_OBJECT

private slots:

    void initialValues()
      {
          ConnectDialog dialog( nullptr,
                                QStringLiteral( "192.168.0.10" ),
                                MonitorTransport::TCP,
                                6100,
                                QStringLiteral( "/player" ) );

          QCOMPARE( dialog.host(), QStringLiteral( "192.168.0.10" ) );
          QCOMPARE( dialog.transport(), MonitorTransport::TCP );
          QCOMPARE( dialog.port(), 6100 );
          QCOMPARE( dialog.wsPath(), QStringLiteral( "/player" ) );

          QComboBox * transport = dialog.findChild< QComboBox * >();
          QVERIFY( transport );
          QCOMPARE( transport->count(), 3 ); // UDP, TCP, WebSocket

          QSpinBox * port = dialog.findChild< QSpinBox * >();
          QVERIFY( port );
          QCOMPARE( port->minimum(), 1 );
          QCOMPARE( port->maximum(), 65535 );
      }

    void editedValues()
      {
          ConnectDialog dialog( nullptr,
                                QStringLiteral( "127.0.0.1" ),
                                MonitorTransport::UDP,
                                6000,
                                QStringLiteral( "/monitor" ) );

          QLineEdit * host = find_line_edit( &dialog, QStringLiteral( "127.0.0.1" ) );
          QLineEdit * ws_path = find_line_edit( &dialog, QStringLiteral( "/monitor" ) );
          QComboBox * transport = dialog.findChild< QComboBox * >();
          QSpinBox * port = dialog.findChild< QSpinBox * >();
          QVERIFY( host && ws_path && transport && port );

          host->setText( QStringLiteral( "  localhost  " ) );
          transport->setCurrentIndex( transport->findData( int( MonitorTransport::TCP ) ) );
          port->setValue( 6100 );

          QCOMPARE( dialog.host(), QStringLiteral( "localhost" ) );
          QCOMPARE( dialog.transport(), MonitorTransport::TCP );
          QCOMPARE( dialog.port(), 6100 );
      }

    void wsPathEnabledOnlyForWebSocket()
      {
          ConnectDialog dialog( nullptr,
                                QStringLiteral( "127.0.0.1" ),
                                MonitorTransport::UDP,
                                6000,
                                QStringLiteral( "/monitor" ) );

          QLineEdit * ws_path = find_line_edit( &dialog, QStringLiteral( "/monitor" ) );
          QComboBox * transport = dialog.findChild< QComboBox * >();
          QVERIFY( ws_path && transport );

          QVERIFY( ! ws_path->isEnabled() );
          transport->setCurrentIndex( transport->findData( int( MonitorTransport::TCP ) ) );
          QVERIFY( ! ws_path->isEnabled() );
          transport->setCurrentIndex( transport->findData( int( MonitorTransport::WebSocket ) ) );
          QVERIFY( ws_path->isEnabled() );

          ws_path->setText( QStringLiteral( " /player " ) );
          QCOMPARE( dialog.wsPath(), QStringLiteral( "/player" ) );
      }

    void emptyHost()
      {
          ConnectDialog dialog( nullptr,
                                QStringLiteral( "127.0.0.1" ),
                                MonitorTransport::UDP,
                                6000,
                                QStringLiteral( "/monitor" ) );

          QLineEdit * host = find_line_edit( &dialog, QStringLiteral( "127.0.0.1" ) );
          QPushButton * button = connect_button( &dialog );
          QVERIFY( host && button );

          QVERIFY( button->isEnabled() );
          host->setText( QStringLiteral( "   " ) );
          QVERIFY( ! button->isEnabled() );
          host->setText( QStringLiteral( "localhost" ) );
          QVERIFY( button->isEnabled() );
      }

    void webSocketAvailability()
      {
          ConnectDialog dialog( nullptr,
                                QStringLiteral( "127.0.0.1" ),
                                MonitorTransport::WebSocket,
                                6080,
                                QStringLiteral( "/monitor" ) );

          QComboBox * transport = dialog.findChild< QComboBox * >();
          QPushButton * button = connect_button( &dialog );
          QVERIFY( transport && button );

          const int index = transport->findData( int( MonitorTransport::WebSocket ) );
          QVERIFY( index >= 0 );
          QStandardItemModel * model = qobject_cast< QStandardItemModel * >( transport->model() );
          QVERIFY( model );

#ifdef HAVE_QT_WEBSOCKETS
          QVERIFY( model->item( index )->isEnabled() );
          QCOMPARE( dialog.transport(), MonitorTransport::WebSocket );
          QVERIFY( button->isEnabled() );
#else
          // shown to tell the user, but cannot be used
          QVERIFY( ! model->item( index )->isEnabled() );
          QVERIFY( transport->itemText( index ).contains( QLatin1String( "not available" ) ) );
          QVERIFY( ! button->isEnabled() );
#endif
      }
};

/*-------------------------------------------------------------------*/
int
main( int argc, char ** argv )
{
    // Always use the offscreen platform, so that no window is shown.
    // The -platform command line option (e.g. -platform xcb) still overrides it.
    qputenv( "QT_QPA_PLATFORM", "offscreen" );

    QApplication app( argc, argv );
    ConnectDialogTest test;
    return QTest::qExec( &test, argc, argv );
}

#include "test_connect_dialog.moc"
