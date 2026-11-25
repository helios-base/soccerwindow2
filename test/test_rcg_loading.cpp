// -*-c++-*-

/*!
  \file test_rcg_loading.cpp
  \brief unit test of MainData::openRCG(), which reads a whole game log into
  memory by rcsc::gzread_file() and parses it there.

  The test checks that
  - the game logs of the text format (version 5 and 6) and the JSON format
    are read, both uncompressed and compressed by gzip, and
  - a file that does not exist, a directory, and a broken gzip file are
    failures, and the data that have already been read are kept.

  The game logs in test/data/ are the first lines of real game logs, copied
  from librcsc (rcsc/rcg/test_data/). Their gzip files are created in a
  temporary directory by zlib. The gzip cases are skipped if soccerwindow2 is
  built without zlib (librcsc also needs zlib or libdeflate to read them).

  HOME, XDG_CONFIG_HOME and XDG_RUNTIME_DIR are set to a temporary directory,
  so the user's ~/.soccerwindow2 is neither read nor modified.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <QtCore>
#include <QtTest>

#include "main_data.h"
#include "monitor_view_data.h"
#include "view_holder.h"

#ifdef HAVE_LIBZ
#include <zlib.h>
#endif

#include <string>

#ifndef SOCCERWINDOW2_TEST_DATA_DIR
#error "SOCCERWINDOW2_TEST_DATA_DIR is not defined."
#endif

namespace {

//! the path of the file in test/data/
QString
data_path( const QString & file_name )
{
    return QStringLiteral( SOCCERWINDOW2_TEST_DATA_DIR ) + QLatin1Char( '/' ) + file_name;
}

//! the path given to MainData::openRCG()
std::string
local_path( const QString & path )
{
    return QFile::encodeName( path ).toStdString();
}

#ifdef HAVE_LIBZ
//! write the contents of the file src to the file dst, compressed by gzip
bool
write_gzip( const QString & src,
            const QString & dst )
{
    QFile in( src );
    if ( ! in.open( QIODevice::ReadOnly ) )
    {
        return false;
    }
    const QByteArray data = in.readAll();

    gzFile out = gzopen( QFile::encodeName( dst ).constData(), "wb" );
    if ( ! out )
    {
        return false;
    }
    const int written = gzwrite( out, data.constData(), static_cast< unsigned int >( data.size() ) );
    const bool closed = ( gzclose( out ) == Z_OK );
    return written == data.size() && closed;
}
#endif

}

/*-------------------------------------------------------------------*/
class RCGLoadingTest
    : public QObject {

    Q_OBJECT

private:

    QTemporaryDir M_tmp_dir;

private slots:

    void initTestCase()
      {
          QVERIFY( M_tmp_dir.isValid() );
      }

    void openRCG_data()
      {
          QTest::addColumn< QString >( "file_name" );
          QTest::addColumn< bool >( "gzip" );
          QTest::addColumn< int >( "frames" );
          QTest::addColumn< int >( "last_cycle" );
          QTest::addColumn< QString >( "left_team" );
          QTest::addColumn< QString >( "right_team" );

          QTest::newRow( "v5" ) << QStringLiteral( "head_v5.rcg" ) << false
                                << 3 << 4 << QStringLiteral( "HELIOS2022" ) << QStringLiteral( "CYRUS" );
          QTest::newRow( "v5 gzip" ) << QStringLiteral( "head_v5.rcg" ) << true
                                     << 3 << 4 << QStringLiteral( "HELIOS2022" ) << QStringLiteral( "CYRUS" );
          QTest::newRow( "v6" ) << QStringLiteral( "head_v6.rcg" ) << false
                                << 5 << 5 << QStringLiteral( "HELIOS2026" ) << QStringLiteral( "YuShan2026" );
          QTest::newRow( "v6 gzip" ) << QStringLiteral( "head_v6.rcg" ) << true
                                     << 5 << 5 << QStringLiteral( "HELIOS2026" ) << QStringLiteral( "YuShan2026" );
          QTest::newRow( "json" ) << QStringLiteral( "head_json.rcg" ) << false
                                  << 3 << 5 << QStringLiteral( "HELIOS2023" ) << QStringLiteral( "CYRUS" );
          QTest::newRow( "json gzip" ) << QStringLiteral( "head_json.rcg" ) << true
                                       << 3 << 5 << QStringLiteral( "HELIOS2023" ) << QStringLiteral( "CYRUS" );
      }

    void openRCG()
      {
          QFETCH( QString, file_name );
          QFETCH( bool, gzip );
          QFETCH( int, frames );
          QFETCH( int, last_cycle );
          QFETCH( QString, left_team );
          QFETCH( QString, right_team );

          QString path = data_path( file_name );
          if ( gzip )
          {
#ifdef HAVE_LIBZ
              const QString gz_path = M_tmp_dir.filePath( file_name + QLatin1String( ".gz" ) );
              QVERIFY( write_gzip( path, gz_path ) );
              path = gz_path;
#else
              QSKIP( "soccerwindow2 is built without zlib." );
#endif
          }

          MainData main_data;
          QVERIFY( main_data.openRCG( local_path( path ) ) );

          const MonitorViewData::Cont & cont = main_data.viewHolder().monitorViewCont();
          QCOMPARE( static_cast< int >( cont.size() ), frames );
          QCOMPARE( static_cast< int >( cont.front()->time().cycle() ), 1 );
          QCOMPARE( static_cast< int >( cont.back()->time().cycle() ), last_cycle );
          QCOMPARE( QString::fromStdString( cont.back()->leftTeam().name() ), left_team );
          QCOMPARE( QString::fromStdString( cont.back()->rightTeam().name() ), right_team );
          QCOMPARE( static_cast< int >( main_data.viewHolder().playerTypeCont().size() ), 18 );
      }

    void openRCGFailure()
      {
          MainData main_data;
          QVERIFY( main_data.openRCG( local_path( data_path( QStringLiteral( "head_v6.rcg" ) ) ) ) );
          QCOMPARE( static_cast< int >( main_data.viewHolder().monitorViewCont().size() ), 5 );

          // a file that does not exist
          QVERIFY( ! main_data.openRCG( local_path( M_tmp_dir.filePath( QStringLiteral( "nonexistent.rcg" ) ) ) ) );

          // a directory cannot be read as a game log
          QVERIFY( ! main_data.openRCG( local_path( M_tmp_dir.path() ) ) );

          // a broken gzip file: the gzip header followed by data that are not compressed
          const QString broken_path = M_tmp_dir.filePath( QStringLiteral( "broken.rcg.gz" ) );
          {
              QFile broken( broken_path );
              QVERIFY( broken.open( QIODevice::WriteOnly ) );
              const char header[] = { '\x1f', '\x8b', '\x08', '\x00',
                                      '\x00', '\x00', '\x00', '\x00', '\x00', '\x03' };
              QCOMPARE( broken.write( header, sizeof( header ) ), static_cast< qint64 >( sizeof( header ) ) );
              QVERIFY( broken.write( "ULG5\n(show 1 ((b) 0 0 0 0))\n" ) > 0 );
          }
          QVERIFY( ! main_data.openRCG( local_path( broken_path ) ) );

          // the data that have already been read are kept
          QCOMPARE( static_cast< int >( main_data.viewHolder().monitorViewCont().size() ), 5 );
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
    RCGLoadingTest test;
    return QTest::qExec( &test, argc, argv );
}

#include "test_rcg_loading.moc"
