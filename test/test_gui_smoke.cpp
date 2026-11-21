// -*-c++-*-

/*!
  \file test_gui_smoke.cpp
  \brief GUI smoke test of soccerwindow2 with Qt Test.

  The test creates MainWindow on the offscreen platform and checks that
  - no signal/slot connection error ("QObject::connect" warning) occurs
    when MainWindow and the other windows and dialogs are created,
  - the main shortcut keys are assigned as expected,
  - no shortcut key is assigned to two or more actions in the same window,
  - the light palette is used by default even if the desktop uses a dark
    color scheme, and the color scheme can be changed from the menu and
    the command line option,
  - the icons are adapted to the palette (the dark lines become light with
    a dark palette),
  - the icons embedded with the Qt resource system (UIIcon::fromName()) have
    the images for the light and dark color schemes in 16x16 and 32x32
    pixels, and the images are switched according to the palette, and
  - the checked tool buttons are emphasized with the highlight color without
    changing their sizes.

  HOME, XDG_CONFIG_HOME and XDG_RUNTIME_DIR are set to a temporary directory
  before QApplication is created, so the user's ~/.soccerwindow2 is neither
  read nor modified.

  The test always uses the offscreen platform (no window is shown) even if
  QT_QPA_PLATFORM is set. To see the windows, run it with "-platform xcb".
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <QtWidgets>
#include <QtTest>

#include "main_window.h"
#include "options.h"
#include "ui_palette.h"
#include "ui_icon.h"
#include "ui_style.h"

#include <cstdlib>
#include <functional>
#include <map>
#include <memory>

namespace {

QStringList g_connect_warnings;
QtMessageHandler g_prev_handler = nullptr;

//! a dark palette to simulate the dark color scheme of the desktop
const QPalette g_dark_palette( QColor( 50, 50, 50 ) );

//! the application palette after setting g_dark_palette
//! (the roles that g_dark_palette does not set are resolved by Qt)
QPalette g_system_palette;

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
  \brief compare the colors of the main roles.
  \return an empty string if the colors are the same, otherwise the differences.
*/
QString
palette_difference( const QPalette & actual,
                    const QPalette & expected )
{
    const QPalette::ColorRole roles[] = {
        QPalette::Window, QPalette::WindowText, QPalette::Base, QPalette::Text,
        QPalette::Button, QPalette::ButtonText, QPalette::Light, QPalette::Midlight,
        QPalette::Mid, QPalette::Dark, QPalette::Shadow,
        QPalette::Highlight, QPalette::HighlightedText,
    };
    const QPalette::ColorGroup groups[] = { QPalette::Active, QPalette::Disabled };

    QStringList diffs;
    for ( QPalette::ColorGroup g : groups )
    {
        for ( QPalette::ColorRole r : roles )
        {
            const QColor a = actual.color( g, r );
            const QColor e = expected.color( g, r );
            if ( a != e )
            {
                diffs << QString( "group=%1 role=%2 actual=%3 expected=%4" )
                    .arg( int( g ) ).arg( int( r ) )
                    .arg( a.name( QColor::HexArgb ), e.name( QColor::HexArgb ) );
            }
        }
    }
    return diffs.join( QLatin1Char( '\n' ) );
}

/*-------------------------------------------------------------------*/
//! count the opaque dark gray pixels (the pixels to be replaced with a dark palette)
int
count_dark_gray_pixels( const QImage & image )
{
    int count = 0;
    for ( int y = 0; y < image.height(); ++y )
    {
        for ( int x = 0; x < image.width(); ++x )
        {
            const QColor c = image.pixelColor( x, y );
            if ( c.alpha() > 0 && c.hslSaturation() < 60 && c.lightness() < 110 )
            {
                ++count;
            }
        }
    }
    return count;
}

/*-------------------------------------------------------------------*/
/*!
  \brief count the pixels on the outermost border of the image whose color
  is close to the given color.
  The inside of the image (e.g. the antialiased text) is not checked.
*/
int
count_border_color_pixels( const QImage & image,
                           const QColor & color )
{
    int count = 0;
    for ( int y = 0; y < image.height(); ++y )
    {
        for ( int x = 0; x < image.width(); ++x )
        {
            if ( x != 0 && y != 0
                 && x != image.width() - 1 && y != image.height() - 1 )
            {
                continue;
            }
            const QColor c = image.pixelColor( x, y );
            if ( std::abs( c.red() - color.red() ) <= 8
                 && std::abs( c.green() - color.green() ) <= 8
                 && std::abs( c.blue() - color.blue() ) <= 8 )
            {
                ++count;
            }
        }
    }
    return count;
}

/*-------------------------------------------------------------------*/
/*!
  \brief load an embedded PNG file of the icons.
  ("light/16/open" -> ":/icons/light/16/open.png")
  The image is converted to the premultiplied format through QPixmap in the
  same way as the icons, so that it can be compared with the images of the
  icons (to_image()) without the rounding errors of the translucent pixels.
*/
QImage
resource_image( const QString & path )
{
    return QPixmap( QStringLiteral( ":/icons/%1.png" ).arg( path ) ).toImage()
        .convertToFormat( QImage::Format_ARGB32_Premultiplied );
}

/*-------------------------------------------------------------------*/
//! convert the pixmap to an image (in device pixels) to compare it with resource_image()
QImage
to_image( const QPixmap & pix )
{
    QImage image = pix.toImage().convertToFormat( QImage::Format_ARGB32_Premultiplied );
    image.setDevicePixelRatio( 1.0 );
    return image;
}

/*-------------------------------------------------------------------*/
QString
saved_ui_color_scheme()
{
    QSettings settings( QDir::homePath() + QLatin1String( "/.soccerwindow2" ),
                        QSettings::IniFormat );
    return settings.value( QLatin1String( "Options/ui_color_scheme" ) ).toString();
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
    void uiColorSchemeDefault();
    void uiColorSchemeMenu();
    void uiColorSchemeStyleChange();
    void uiColorSchemeCommandLine();
    void uiIconAdaptToPalette();
    void uiIconFollowsPalette();
    void uiIconFromName();
    void uiIconFromNameHighDpi();
    void uiIconFromNameNotFound();
    void uiIconResources();
    void uiIconFromNameInToolBar();
    void toolButtonCheckedEmphasis();
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

    // simulate the dark color scheme of the desktop
    QApplication::setPalette( g_dark_palette );
    g_system_palette = QApplication::palette();
    QVERIFY( g_system_palette.color( QPalette::Window ).lightness() < 128 );
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
GuiSmokeTest::uiColorSchemeDefault()
{
    QVERIFY( M_main_window );

    // no saved setting and no command line option: light
    QCOMPARE( Options::instance().uiColorScheme(), Options::UI_COLOR_SCHEME_LIGHT );

    const QString diff = palette_difference( QApplication::palette(), UIPalette::light() );
    QVERIFY2( diff.isEmpty(), qPrintable( diff ) );

    QAction * light = findAction( QStringLiteral( "ui_color_scheme_light" ) );
    QVERIFY( light );
    QVERIFY( light->isChecked() );
}

/*-------------------------------------------------------------------*/
void
GuiSmokeTest::uiColorSchemeMenu()
{
    QVERIFY( M_main_window );

    QAction * system = findAction( QStringLiteral( "ui_color_scheme_system" ) );
    QAction * light = findAction( QStringLiteral( "ui_color_scheme_light" ) );
    QVERIFY( system );
    QVERIFY( light );

    // System: back to the palette of the desktop (the simulated dark palette)
    system->trigger();
    QVERIFY( system->isChecked() );
    QCOMPARE( Options::instance().uiColorScheme(), Options::UI_COLOR_SCHEME_SYSTEM );
    {
        const QString diff = palette_difference( QApplication::palette(), g_system_palette );
        QVERIFY2( diff.isEmpty(), qPrintable( diff ) );
    }
    QCOMPARE( saved_ui_color_scheme(), QStringLiteral( "system" ) );

    // Light
    light->trigger();
    QVERIFY( light->isChecked() );
    QCOMPARE( Options::instance().uiColorScheme(), Options::UI_COLOR_SCHEME_LIGHT );
    {
        const QString diff = palette_difference( QApplication::palette(), UIPalette::light() );
        QVERIFY2( diff.isEmpty(), qPrintable( diff ) );
    }
    QCOMPARE( saved_ui_color_scheme(), QStringLiteral( "light" ) );
}

/*-------------------------------------------------------------------*/
void
GuiSmokeTest::uiColorSchemeStyleChange()
{
    QVERIFY( M_main_window );

    // find the actions of the "Qt Style" menu
    QAction * windows_style = nullptr;
    QAction * original_style = nullptr;
    const QString original_name = QApplication::style()->objectName();
    for ( QAction * act : M_main_window->findChildren< QAction * >() )
    {
        const QString name = act->data().toString();
        if ( name.isEmpty()
             || ! QStyleFactory::keys().contains( name ) )
        {
            continue;
        }
        if ( name.compare( QLatin1String( "Windows" ), Qt::CaseInsensitive ) == 0 )
        {
            windows_style = act;
        }
        if ( name.compare( original_name, Qt::CaseInsensitive ) == 0 )
        {
            original_style = act;
        }
    }
    if ( ! windows_style || ! original_style )
    {
        QSKIP( "the style actions are not found" );
    }

    // the light palette is kept after changing the style
    windows_style->trigger();
    QCOMPARE( QApplication::style()->objectName().toLower(), QStringLiteral( "windows" ) );
    {
        const QString diff = palette_difference( QApplication::palette(), UIPalette::light() );
        QVERIFY2( diff.isEmpty(), qPrintable( diff ) );
    }

    original_style->trigger();
    {
        const QString diff = palette_difference( QApplication::palette(), UIPalette::light() );
        QVERIFY2( diff.isEmpty(), qPrintable( diff ) );
    }
}

/*-------------------------------------------------------------------*/
void
GuiSmokeTest::uiColorSchemeCommandLine()
{
    char arg0[] = "soccerwindow2";
    char arg_system[] = "--ui-color-scheme=system";
    char arg_light[] = "--ui-color-scheme=light";
    char arg_invalid[] = "--ui-color-scheme=dark";

    {
        char * argv[] = { arg0, arg_system, nullptr };
        QVERIFY( Options::instance().parseCmdLine( 2, argv ) );
        QCOMPARE( Options::instance().uiColorScheme(), Options::UI_COLOR_SCHEME_SYSTEM );
    }
    {
        // an unsupported value does not change the current value
        char * argv[] = { arg0, arg_invalid, nullptr };
        Options::instance().parseCmdLine( 2, argv );
        QCOMPARE( Options::instance().uiColorScheme(), Options::UI_COLOR_SCHEME_SYSTEM );
    }
    {
        char * argv[] = { arg0, arg_light, nullptr };
        QVERIFY( Options::instance().parseCmdLine( 2, argv ) );
        QCOMPARE( Options::instance().uiColorScheme(), Options::UI_COLOR_SCHEME_LIGHT );
    }
}

/*-------------------------------------------------------------------*/
void
GuiSmokeTest::uiIconAdaptToPalette()
{
    // black, half transparent black, gray, red, transparent
    QImage image( 5, 1, QImage::Format_ARGB32 );
    image.setPixelColor( 0, 0, QColor( 0, 0, 0 ) );
    image.setPixelColor( 1, 0, QColor( 0, 0, 0, 128 ) );
    image.setPixelColor( 2, 0, QColor( 128, 128, 128 ) );
    image.setPixelColor( 3, 0, QColor( 255, 0, 0 ) );
    image.setPixelColor( 4, 0, QColor( 0, 0, 0, 0 ) );

    // light palette: not changed
    QVERIFY( ! UIIcon::isDark( UIPalette::light() ) );
    QCOMPARE( UIIcon::adaptToPalette( image, UIPalette::light() ), image );

    // dark palette: the dark gray pixels become the text color
    QVERIFY( UIIcon::isDark( g_system_palette ) );
    const QColor text = g_system_palette.color( QPalette::WindowText );
    const QImage adapted = UIIcon::adaptToPalette( image, g_system_palette );
    QCOMPARE( adapted.pixelColor( 0, 0 ), text );
    QColor half_text = text;
    half_text.setAlpha( 128 );
    QCOMPARE( adapted.pixelColor( 1, 0 ), half_text );
    QCOMPARE( adapted.pixelColor( 2, 0 ), image.pixelColor( 2, 0 ) );
    QCOMPARE( adapted.pixelColor( 3, 0 ), image.pixelColor( 3, 0 ) );
    QCOMPARE( adapted.pixelColor( 4, 0 ).alpha(), 0 );
}

/*-------------------------------------------------------------------*/
void
GuiSmokeTest::uiIconFollowsPalette()
{
    QVERIFY( M_main_window );

    QAction * light = findAction( QStringLiteral( "ui_color_scheme_light" ) );
    QAction * system = findAction( QStringLiteral( "ui_color_scheme_system" ) );
    QAction * act = findAction( QStringLiteral( "log_player_go_first" ) );
    QVERIFY( light );
    QVERIFY( system );
    QVERIFY( act );
    QVERIFY( ! act->icon().isNull() );

    // light: the original dark lines
    light->trigger();
    const QImage light_image = act->icon().pixmap( 16, 16 ).toImage();
    QVERIFY( count_dark_gray_pixels( light_image ) > 0 );

    // system (the simulated dark palette): no dark lines
    system->trigger();
    const QImage dark_image = act->icon().pixmap( 16, 16 ).toImage();
    QCOMPARE( count_dark_gray_pixels( dark_image ), 0 );
    QCOMPARE( dark_image.size(), light_image.size() );

    // back to light: the same as the first image
    light->trigger();
    QCOMPARE( act->icon().pixmap( 16, 16 ).toImage(), light_image );
}

/*-------------------------------------------------------------------*/
void
GuiSmokeTest::uiIconFromName()
{
    QVERIFY( M_main_window );

    QAction * light = findAction( QStringLiteral( "ui_color_scheme_light" ) );
    QAction * system = findAction( QStringLiteral( "ui_color_scheme_system" ) );
    QVERIFY( light );
    QVERIFY( system );

    const QIcon icon = UIIcon::fromName( QStringLiteral( "open" ) );
    QVERIFY( ! icon.isNull() );

    // light: the images for the light color scheme
    light->trigger();
    QCOMPARE( to_image( icon.pixmap( 16, 16 ) ), resource_image( "light/16/open" ) );
    QCOMPARE( to_image( icon.pixmap( 32, 32 ) ), resource_image( "light/32/open" ) );
    QCOMPARE( icon.availableSizes(), QList< QSize >() << QSize( 16, 16 ) << QSize( 32, 32 ) );

    // the icon is not drawn larger than the requested size, and the images
    // are used without scaling if possible
    QCOMPARE( icon.actualSize( QSize( 16, 16 ) ), QSize( 16, 16 ) );
    QCOMPARE( icon.actualSize( QSize( 24, 24 ) ), QSize( 16, 16 ) );
    QCOMPARE( icon.actualSize( QSize( 48, 48 ) ), QSize( 32, 32 ) );
    QCOMPARE( icon.actualSize( QSize( 8, 8 ) ), QSize( 8, 8 ) );
    QCOMPARE( to_image( icon.pixmap( 24, 24 ) ), resource_image( "light/16/open" ) );
    QCOMPARE( icon.pixmap( 8, 8 ).size(), QSize( 8, 8 ) );

    // the disabled icon is generated by the style
    QCOMPARE( icon.pixmap( 16, 16, QIcon::Disabled ).size(), QSize( 16, 16 ) );
    QVERIFY( to_image( icon.pixmap( 16, 16, QIcon::Disabled ) ) != resource_image( "light/16/open" ) );

    // system (the simulated dark palette): the images for the dark color scheme
    system->trigger();
    QCOMPARE( to_image( icon.pixmap( 16, 16 ) ), resource_image( "dark/16/open" ) );
    QCOMPARE( to_image( icon.pixmap( 32, 32 ) ), resource_image( "dark/32/open" ) );
    QVERIFY( resource_image( "dark/16/open" ) != resource_image( "light/16/open" ) );

    light->trigger();
    QCOMPARE( to_image( icon.pixmap( 16, 16 ) ), resource_image( "light/16/open" ) );
}

/*-------------------------------------------------------------------*/
void
GuiSmokeTest::uiIconFromNameHighDpi()
{
    QVERIFY( ! UIIcon::isDark( QApplication::palette() ) );

    const QIcon icon = UIIcon::fromName( QStringLiteral( "open" ) );
    QVERIFY( ! icon.isNull() );

    // painting the 16x16 icon on a device whose device pixel ratio is 2 uses the 32x32 image
    QImage image( 32, 32, QImage::Format_ARGB32_Premultiplied );
    image.fill( Qt::transparent );
    image.setDevicePixelRatio( 2.0 );
    {
        QPainter painter( &image );
        icon.paint( &painter, QRect( 0, 0, 16, 16 ) );
    }
    image.setDevicePixelRatio( 1.0 );
    QCOMPARE( image, resource_image( "light/32/open" ) );

#if QT_VERSION >= QT_VERSION_CHECK( 6, 0, 0 )
    // the pixmap for the device pixel ratio 2
    const QPixmap pix2 = icon.pixmap( QSize( 16, 16 ), 2.0 );
    QCOMPARE( pix2.size(), QSize( 32, 32 ) );
    QCOMPARE( pix2.devicePixelRatio(), 2.0 );
    QCOMPARE( to_image( pix2 ), resource_image( "light/32/open" ) );

    // a fractional device pixel ratio: the 32x32 image is scaled down
    const QPixmap pix15 = icon.pixmap( QSize( 16, 16 ), 1.5 );
    QCOMPARE( pix15.size(), QSize( 24, 24 ) );
#endif
}

/*-------------------------------------------------------------------*/
void
GuiSmokeTest::uiIconFromNameNotFound()
{
    QTest::ignoreMessage( QtWarningMsg,
                          QRegularExpression( QStringLiteral( "UIIcon: the icon \"no_such_icon\" is not found" ) ) );
    QVERIFY( UIIcon::fromName( QStringLiteral( "no_such_icon" ) ).isNull() );
}

/*-------------------------------------------------------------------*/
void
GuiSmokeTest::uiIconResources()
{
    UIIcon::initResources();

    const QStringList files = QDir( QStringLiteral( ":/icons/light/16" ) )
        .entryList( QStringList() << QStringLiteral( "*.png" ), QDir::Files, QDir::Name );
    QVERIFY2( ! files.isEmpty(), "no icon is embedded" );

    // all the icons have the images of the correct sizes for both color schemes
    QStringList errors;
    for ( const QString & file : files )
    {
        const QString name = QFileInfo( file ).completeBaseName();
        if ( UIIcon::fromName( name ).isNull() )
        {
            errors << name + ": UIIcon::fromName() failed";
            continue;
        }
        for ( const int size : { 16, 32 } )
        {
            const QImage light = resource_image( QStringLiteral( "light/%1/%2" ).arg( size ).arg( name ) );
            const QImage dark = resource_image( QStringLiteral( "dark/%1/%2" ).arg( size ).arg( name ) );
            if ( light.size() != QSize( size, size )
                 || dark.size() != QSize( size, size ) )
            {
                errors << QStringLiteral( "%1: the size of the %2px images is wrong" ).arg( name ).arg( size );
            }
            else if ( light == dark )
            {
                errors << QStringLiteral( "%1: the %2px images for the light and dark color schemes are the same" ).arg( name ).arg( size );
            }
        }
    }
    QVERIFY2( errors.isEmpty(), qPrintable( errors.join( QLatin1Char( '\n' ) ) ) );
}

/*-------------------------------------------------------------------*/
void
GuiSmokeTest::uiIconFromNameInToolBar()
{
    QVERIFY( M_main_window );

    QAction * light = findAction( QStringLiteral( "ui_color_scheme_light" ) );
    QAction * system = findAction( QStringLiteral( "ui_color_scheme_system" ) );
    QVERIFY( light );
    QVERIFY( system );
    light->trigger();

    QToolBar bar;
    bar.setIconSize( QSize( 16, 16 ) );
    QAction * act = bar.addAction( UIIcon::fromName( QStringLiteral( "open" ) ), QStringLiteral( "Open" ) );
    bar.show();
    QCoreApplication::processEvents();

    QWidget * button = bar.widgetForAction( act );
    QVERIFY( button );
    const QImage light_image = button->grab().toImage();

    // the tool button is painted with the images for the current palette
    system->trigger();
    QCoreApplication::processEvents();
    const QImage dark_image = button->grab().toImage();
    QCOMPARE( dark_image.size(), light_image.size() );
    QVERIFY( dark_image != light_image );

    light->trigger();
    QCoreApplication::processEvents();
    QCOMPARE( button->grab().toImage(), light_image );
}

/*-------------------------------------------------------------------*/
void
GuiSmokeTest::toolButtonCheckedEmphasis()
{
    QVERIFY( M_main_window );

    // MainWindow::init() and the style menu set UIStyle.
    QVERIFY( dynamic_cast< UIStyle * >( QApplication::style() ) );
    QVERIFY( ! QApplication::style()->objectName().isEmpty() );

    QToolBar bar;
    QAction * act = bar.addAction( QStringLiteral( "Checked" ) );
    act->setCheckable( true );
    bar.show();
    QCoreApplication::processEvents();

    QWidget * button = bar.widgetForAction( act );
    QVERIFY( button );
    const QColor highlight = QApplication::palette().color( QPalette::Highlight );

    act->setChecked( false );
    QCoreApplication::processEvents();
    const QSize unchecked_size = button->size();
    QCOMPARE( count_border_color_pixels( button->grab().toImage(), highlight ), 0 );

    // the checked button has the border of the highlight color, and the size is not changed
    act->setChecked( true );
    QCoreApplication::processEvents();
    QCOMPARE( button->size(), unchecked_size );
    const QImage checked = button->grab().toImage();
    // most of the outermost pixels (except the rounded corners) have the highlight color
    const int border_length = 2 * ( checked.width() + checked.height() ) - 4;
    QVERIFY2( count_border_color_pixels( checked, highlight ) > border_length / 2,
              qPrintable( QString( "%1 / %2" ).arg( count_border_color_pixels( checked, highlight ) ).arg( border_length ) ) );
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
    // Always use the offscreen platform, even if QT_QPA_PLATFORM is set by
    // the desktop session (e.g. "xcb"), so that no window is shown.
    // The -platform command line option (e.g. -platform xcb) still overrides it.
    qputenv( "QT_QPA_PLATFORM", "offscreen" );

    QApplication app( argc, argv );
    GuiSmokeTest test;
    return QTest::qExec( &test, argc, argv );
}

#include "test_gui_smoke.moc"
