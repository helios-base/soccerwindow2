// -*-c++-*-

/*!
  \file test_gui_smoke.cpp
  \brief GUI smoke test of soccerwindow2 with Qt Test.

  The test creates MainWindow on the offscreen platform and checks that
  - no signal/slot connection error ("QObject::connect" warning) occurs
    when MainWindow and the other windows and dialogs are created,
  - the main shortcut keys are assigned as expected, and
  - no shortcut key is assigned to two or more actions in the same window.

  HOME, XDG_CONFIG_HOME and XDG_RUNTIME_DIR are set to a temporary directory
  before QApplication is created, so the user's ~/.soccerwindow2 is neither
  read nor modified.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <QtWidgets>
#include <QtTest>

#include "main_window.h"

#include <functional>
#include <map>
#include <memory>

namespace {

QStringList g_connect_warnings;
QtMessageHandler g_prev_handler = nullptr;

/*-------------------------------------------------------------------*/
void
message_handler( QtMsgType type,
                 const QMessageLogContext & context,
                 const QString & msg )
{
    if ( msg.contains( QLatin1String( "QObject::connect" ) ) )
    {
        g_connect_warnings << msg;
    }

    // a harmless message from the offscreen platform plugin
    if ( msg.contains( QLatin1String( "does not support propagateSizeHints" ) ) )
    {
        return;
    }

    if ( g_prev_handler )
    {
        g_prev_handler( type, context, msg );
    }
}

/*-------------------------------------------------------------------*/
QSet< QWidget * >
visible_top_level_widgets()
{
    QSet< QWidget * > widgets;
    for ( QWidget * w : QApplication::topLevelWidgets() )
    {
        if ( w->isVisible() )
        {
            widgets.insert( w );
        }
    }
    return widgets;
}

/*-------------------------------------------------------------------*/
/*!
  \brief run func and close the modal dialog opened by func.
  \return true if a modal dialog has been opened and closed.
*/
bool
run_and_close_modal_dialog( const std::function< void() > & func )
{
    bool closed = false;
    QTimer timer;
    int count = 0;
    timer.setInterval( 50 );
    QObject::connect( &timer, &QTimer::timeout,
                      [&]()
                        {
                            if ( QWidget * w = QApplication::activeModalWidget() )
                            {
                                if ( QDialog * dlg = qobject_cast< QDialog * >( w ) )
                                {
                                    dlg->reject();
                                }
                                else
                                {
                                    w->close();
                                }
                                closed = true;
                                timer.stop();
                            }
                            else if ( ++count > 100 ) // about 5 seconds
                            {
                                timer.stop();
                            }
                        } );
    timer.start();
    func();
    timer.stop();
    return closed;
}

/*-------------------------------------------------------------------*/
/*!
  \brief get the top level window that owns the action.
  Menus are skipped, so the actions in the menus of a window belong to the window.
*/
QWidget *
owner_window( const QAction * act )
{
    for ( QObject * o = act->parent(); o; o = o->parent() )
    {
        QWidget * w = qobject_cast< QWidget * >( o );
        if ( w
             && w->isWindow()
             && ! qobject_cast< QMenu * >( w ) )
        {
            return w;
        }
    }
    return nullptr;
}

}

/*-------------------------------------------------------------------*/
/*!
  \class GuiSmokeTest
*/
class GuiSmokeTest
    : public QObject {

    Q_OBJECT

private:
    std::unique_ptr< MainWindow > M_main_window;

    QAction * findAction( const QString & name )
      {
          return M_main_window->findChild< QAction * >( name );
      }

    QString connectWarnings() const
      {
          return g_connect_warnings.join( QLatin1Char( '\n' ) );
      }

private slots:

    void initTestCase();
    void cleanupTestCase();

    void createMainWindow();
    void openWindow_data();
    void openWindow();
    void openModalDialog_data();
    void openModalDialog();
    void shortcutKeysDialog();
    void shortcut_data();
    void shortcut();
    void noDuplicatedShortcut();
};

/*-------------------------------------------------------------------*/
void
GuiSmokeTest::initTestCase()
{
    g_connect_warnings.clear();
    g_prev_handler = qInstallMessageHandler( message_handler );
}

/*-------------------------------------------------------------------*/
void
GuiSmokeTest::cleanupTestCase()
{
    M_main_window.reset();
    qInstallMessageHandler( g_prev_handler );
    g_prev_handler = nullptr;
}

/*-------------------------------------------------------------------*/
void
GuiSmokeTest::createMainWindow()
{
    // the same steps as main()
    M_main_window = std::make_unique< MainWindow >();
    M_main_window->show();
    M_main_window->init();
    QCoreApplication::processEvents();

    QVERIFY( M_main_window->isVisible() );
    QVERIFY2( g_connect_warnings.isEmpty(), qPrintable( connectWarnings() ) );
}

/*-------------------------------------------------------------------*/
void
GuiSmokeTest::openWindow_data()
{
    QTest::addColumn< QString >( "action" );

    for ( const char * name : { "show_trainer_dialog",
                                "show_launcher_dialog",
                                "show_player_type_dialog",
                                "show_detail_dialog",
                                "show_view_config_dialog",
                                "show_formation_editor_window",
                                "show_label_editor_window",
                                "show_simple_label_selector",
                                "show_mark_assignment_editor",
                                "show_debug_message_window" } )
    {
        QTest::newRow( name ) << QString::fromLatin1( name );
    }
}

/*-------------------------------------------------------------------*/
void
GuiSmokeTest::openWindow()
{
    QFETCH( QString, action );
    QVERIFY( M_main_window );

    QAction * act = findAction( action );
    QVERIFY2( act, qPrintable( "action not found: " + action ) );

    g_connect_warnings.clear();
    const QSet< QWidget * > before = visible_top_level_widgets();

    act->trigger();
    QCoreApplication::processEvents();

    const QSet< QWidget * > opened = visible_top_level_widgets() - before;
    QVERIFY2( ! opened.isEmpty(), "no window was opened" );
    QVERIFY2( g_connect_warnings.isEmpty(), qPrintable( connectWarnings() ) );

    for ( QWidget * w : opened )
    {
        w->hide();
    }
}

/*-------------------------------------------------------------------*/
void
GuiSmokeTest::openModalDialog_data()
{
    QTest::addColumn< QString >( "action" );

    for ( const char * name : { "show_color_setting_dialog",
                                "show_font_setting_dialog",
                                "show_image_save_dialog" } )
    {
        QTest::newRow( name ) << QString::fromLatin1( name );
    }
}

/*-------------------------------------------------------------------*/
void
GuiSmokeTest::openModalDialog()
{
    QFETCH( QString, action );
    QVERIFY( M_main_window );

    QAction * act = findAction( action );
    QVERIFY2( act, qPrintable( "action not found: " + action ) );

    g_connect_warnings.clear();
    const bool closed = run_and_close_modal_dialog( [act]() { act->trigger(); } );

    QVERIFY2( closed, "no modal dialog was opened" );
    QVERIFY2( g_connect_warnings.isEmpty(), qPrintable( connectWarnings() ) );
}

/*-------------------------------------------------------------------*/
void
GuiSmokeTest::shortcutKeysDialog()
{
    QVERIFY( M_main_window );

    g_connect_warnings.clear();
    MainWindow * win = M_main_window.get();
    const bool closed = run_and_close_modal_dialog(
        [win]()
          {
              QMetaObject::invokeMethod( win, "showShortcutKeys" );
          } );

    QVERIFY2( closed, "no modal dialog was opened" );
    QVERIFY2( g_connect_warnings.isEmpty(), qPrintable( connectWarnings() ) );
}

/*-------------------------------------------------------------------*/
void
GuiSmokeTest::shortcut_data()
{
    QTest::addColumn< QString >( "action" );
    QTest::addColumn< QString >( "expected" );

    // key sequences in QKeySequence::PortableText
    const std::pair< const char *, const char * > table[] = {
        { "open_rcg", "Ctrl+O" },
        { "quit", "Ctrl+Q" },
        { "show_formation_editor_window", "Ctrl+Alt+F" },
        { "show_label_editor_window", "Ctrl+Alt+L" },
        { "show_simple_label_selector", "Ctrl+Alt+S" },
        { "show_mark_assignment_editor", "Ctrl+Alt+M" },
        { "show_debug_message_window", "Ctrl+D" },
        { "toggle_debug_server", "Ctrl+S" },
        { "toggle_show_team_graphic", "Shift+T" },
        { "full_screen1", "F11" },
        { "full_screen2", "Alt+Return" },
        { "select_left_player_1", "1" },
        { "select_right_player_1", "Ctrl+1" },
        { "select_right_player_9", "Ctrl+9" },
        { "select_right_player_10", "Ctrl+0" },
    };

    for ( const auto & v : table )
    {
        QTest::newRow( v.first ) << QString::fromLatin1( v.first ) << QString::fromLatin1( v.second );
    }
}

/*-------------------------------------------------------------------*/
void
GuiSmokeTest::shortcut()
{
    QFETCH( QString, action );
    QFETCH( QString, expected );
    QVERIFY( M_main_window );

    QAction * act = findAction( action );
    QVERIFY2( act, qPrintable( "action not found: " + action ) );
    QCOMPARE( act->shortcut().toString( QKeySequence::PortableText ), expected );
}

/*-------------------------------------------------------------------*/
void
GuiSmokeTest::noDuplicatedShortcut()
{
    QVERIFY( M_main_window );

    // window -> (shortcut -> action names)
    std::map< QWidget *, std::map< QString, QStringList > > table;

    for ( QWidget * top : QApplication::topLevelWidgets() )
    {
        for ( QAction * act : top->findChildren< QAction * >() )
        {
            QWidget * win = owner_window( act );
            if ( ! win )
            {
                continue;
            }

            for ( const QKeySequence & key : act->shortcuts() )
            {
                if ( key.isEmpty() )
                {
                    continue;
                }

                QStringList & names = table[win][key.toString( QKeySequence::PortableText )];
                const QString name = act->objectName().isEmpty()
                    ? act->text()
                    : act->objectName();
                if ( ! names.contains( name ) )
                {
                    names << name;
                }
            }
        }
    }

    QStringList duplicated;
    for ( const auto & w : table )
    {
        for ( const auto & k : w.second )
        {
            if ( k.second.size() > 1 )
            {
                duplicated << QString( "%1 in %2: %3" )
                    .arg( k.first,
                          QString::fromLatin1( w.first->metaObject()->className() ),
                          k.second.join( QLatin1String( ", " ) ) );
            }
        }
    }

    QVERIFY2( duplicated.isEmpty(), qPrintable( duplicated.join( QLatin1Char( '\n' ) ) ) );
}

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
    if ( ! qEnvironmentVariableIsSet( "QT_QPA_PLATFORM" ) )
    {
        qputenv( "QT_QPA_PLATFORM", "offscreen" );
    }

    QApplication app( argc, argv );
    GuiSmokeTest test;
    return QTest::qExec( &test, argc, argv );
}

#include "test_gui_smoke.moc"
