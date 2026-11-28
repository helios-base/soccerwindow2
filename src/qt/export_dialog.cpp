// -*-c++-*-

/*!
  \file export_dialog.cpp
  \brief the dialog of the export of images and videos Source File.
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

#include "export_dialog.h"

#include "export_job.h"
#include "export_timing.h"
#include "export_settings.h"
#include "field_canvas.h"
#include "image_sequence_sink.h"
#include "main_data.h"
#include "ui_palette.h"
#include "view_holder.h"

#include <rcsc/common/server_param.h>

#include <QBuffer>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QDoubleSpinBox>
#include <QElapsedTimer>
#include <QEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontMetrics>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QImageWriter>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QProgressDialog>
#include <QPushButton>
#include <QSizePolicy>
#include <QSpinBox>
#include <QStorageInfo>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>
#include <memory>

namespace {

//! the sizes offered: the key kept in the settings, and the size (empty: as the view, or custom)
struct SizeChoice {
    const char * key;
    QSize size;
};

const SizeChoice SIZES[] = {
    { "view", QSize() },
    { "1280x720", QSize( 1280, 720 ) },
    { "1920x1080", QSize( 1920, 1080 ) },
    { "3840x2160", QSize( 3840, 2160 ) },
    { "custom", QSize() },
};

//! the digits of a number
int
digitsOf( const unsigned long long value )
{
    return static_cast< int >( QString::number( value ).size() );
}

//! a size of bytes to read: KB, MB or GB
QString
bytesText( const qint64 bytes )
{
    if ( bytes < 1024 * 1024 ) return QObject::tr( "%1 KB" ).arg( std::max< qint64 >( 1, bytes / 1024 ) );
    if ( bytes < 1024LL * 1024 * 1024 ) return QObject::tr( "%1 MB" ).arg( bytes / ( 1024.0 * 1024.0 ), 0, 'f', 1 );
    return QObject::tr( "%1 GB" ).arg( bytes / ( 1024.0 * 1024.0 * 1024.0 ), 0, 'f', 2 );
}

//! the nearest directory that is there: the free space of where the images go
QString
existingDirectory( const QString & dir )
{
    QFileInfo info( dir );
    while ( ! info.exists() && ! info.absolutePath().isEmpty() && info.absoluteFilePath() != info.absolutePath() )
    {
        info = QFileInfo( info.absolutePath() );
    }
    return info.absoluteFilePath();
}

//! the time left to read: about 40 s, about 3 min
QString
leftText( const qint64 msecs )
{
    const qint64 secs = std::max< qint64 >( 1, ( msecs + 500 ) / 1000 );
    if ( secs < 60 ) return QObject::tr( "about %1 s left" ).arg( secs );
    return QObject::tr( "about %1 min left" ).arg( ( secs + 30 ) / 60 );
}

//! the simulation step of the game log (ms)
int
step_ms()
{
    return std::max( 1, rcsc::ServerParam::i().simulatorStep() );
}

}

/*-------------------------------------------------------------------*/
ExportDialog::ExportDialog( MainData & main_data,
                            FieldCanvas & canvas,
                            QWidget * parent )
    : QDialog( parent ),
      M_main_data( main_data ),
      M_canvas( canvas ),
      M_message_error( false ),
      M_allowed( true )
{
    this->setWindowTitle( tr( "Export Images / Video" ) );
    createWidgets();
    readSettings();
    updateSummary();
    this->resize( std::max( 640, this->sizeHint().width() ), this->sizeHint().height() );
}

/*-------------------------------------------------------------------*/
ExportDialog::~ExportDialog()
{
    if ( M_job )
    {
        M_job->abort();
    }
}

/*-------------------------------------------------------------------*/
void
ExportDialog::createWidgets()
{
    QVBoxLayout * top = new QVBoxLayout( this );

    // the range (4.2)
    {
        QGroupBox * box = new QGroupBox( tr( "Range" ) );
        QFormLayout * form = new QFormLayout( box );

        const auto row = [this, form]( const QString & label,
                                       const char * name,
                                       const QString & placeholder,
                                       QLineEdit ** edit,
                                       QLabel ** info,
                                       void ( ExportDialog::*now )() ) {
            *edit = new QLineEdit();
            ( *edit )->setObjectName( name );
            ( *edit )->setPlaceholderText( placeholder );
            ( *edit )->setToolTip( tr( "A cycle (e.g. 1234), or a cycle and a stopped frame (e.g. 1234.3, or 1234,3 as the score board shows it)." ) );
            // as wide for the start and the end, however long the texts beside them
            ( *edit )->setFixedWidth( ( *edit )->fontMetrics().horizontalAdvance( QStringLiteral( "000000.000" ) ) + 16 );
            connect( *edit, &QLineEdit::textChanged,
                     this, &ExportDialog::updateSummary );
            QPushButton * button = new QPushButton( tr( "Now" ) );
            button->setObjectName( QString( name ) + "_now" );
            button->setToolTip( tr( "The position shown now" ) );
            button->setAutoDefault( false );
            connect( button, &QPushButton::clicked, this, now );
            // one line, cut by the width of the dialog: the whole text in the
            // tooltip, and the reason of a wrong one in the summary too
            *info = new QLabel();
            ( *info )->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Preferred );
            ( *info )->setMinimumWidth( 120 );
            QHBoxLayout * layout = new QHBoxLayout();
            layout->addWidget( *edit );
            layout->addWidget( button );
            layout->addWidget( *info, 1 );
            form->addRow( label, layout );
        };
        row( tr( "Start:" ), "start", tr( "the first frame" ), &M_start_edit, &M_start_info, &ExportDialog::setStartToCurrent );
        row( tr( "End:" ), "end", tr( "the last frame" ), &M_end_edit, &M_end_info, &ExportDialog::setEndToCurrent );

        // the whole log (rcssmonitor's D12: "Clear", not "All")
        QPushButton * clear = new QPushButton( tr( "Clear" ) );
        clear->setObjectName( "clear" );
        clear->setToolTip( tr( "The whole log: from the first frame to the last one" ) );
        clear->setAutoDefault( false );
        connect( clear, &QPushButton::clicked, this, &ExportDialog::clearRange );
        QHBoxLayout * buttons = new QHBoxLayout();
        buttons->addWidget( clear );
        buttons->addStretch( 1 );
        form->addRow( QString(), buttons );

        top->addWidget( box );
    }

    // the images
    {
        QGroupBox * box = new QGroupBox( tr( "Images" ) );
        QFormLayout * form = new QFormLayout( box );

        M_size_combo = new QComboBox();
        M_size_combo->setObjectName( "size" );
        M_size_combo->addItem( tr( "As the view" ), SIZES[0].key );
        for ( int i = 1; i < 4; ++i )
        {
            M_size_combo->addItem( QString( "%1 x %2" ).arg( SIZES[i].size.width() ).arg( SIZES[i].size.height() ), SIZES[i].key );
        }
        M_size_combo->addItem( tr( "Custom" ), SIZES[4].key );
        M_width_spin = new QSpinBox();
        M_width_spin->setObjectName( "width" );
        M_width_spin->setRange( 16, 7680 );
        M_height_spin = new QSpinBox();
        M_height_spin->setObjectName( "height" );
        M_height_spin->setRange( 16, 4320 );
        connect( M_size_combo, QOverload< int >::of( &QComboBox::currentIndexChanged ),
                 this, &ExportDialog::updateSummary );
        connect( M_width_spin, QOverload< int >::of( &QSpinBox::valueChanged ),
                 this, &ExportDialog::updateSummary );
        connect( M_height_spin, QOverload< int >::of( &QSpinBox::valueChanged ),
                 this, &ExportDialog::updateSummary );
        QHBoxLayout * size = new QHBoxLayout();
        size->addWidget( M_size_combo );
        size->addWidget( M_width_spin );
        size->addWidget( new QLabel( QStringLiteral( "x" ) ) );
        size->addWidget( M_height_spin );
        size->addStretch( 1 );
        form->addRow( tr( "Size:" ), size );

        // rcssmonitor's D16: the whole view is enlarged, not only the texts
        M_scale_texts_check = new QCheckBox( tr( "Enlarge the texts, logos and lines with the image" ) );
        M_scale_texts_check->setObjectName( "scale_texts" );
        M_scale_texts_check->setToolTip( tr( "The image looks as the view, enlarged: the texts, the team logos and the widths of the lines grow with the image. "
                                             "Off: they keep their sizes in pixels, smaller and thinner in a large image, and the players move more smoothly." ) );
        connect( M_scale_texts_check, &QCheckBox::toggled,
                 this, &ExportDialog::updateSummary );
        form->addRow( QString(), M_scale_texts_check );

        M_fps_spin = new QDoubleSpinBox();
        M_fps_spin->setObjectName( "fps" );
        M_fps_spin->setRange( 1.0, 120.0 );
        M_fps_spin->setDecimals( 1 );
        M_fps_spin->setSuffix( tr( " images/s" ) );
        M_speed_spin = new QDoubleSpinBox();
        M_speed_spin->setObjectName( "speed" );
        M_speed_spin->setRange( 0.1, 16.0 );
        M_speed_spin->setDecimals( 2 );
        M_speed_spin->setSingleStep( 0.5 );
        M_speed_spin->setPrefix( tr( "x " ) );
        M_speed_spin->setToolTip( tr( "How many times as fast as the game is played" ) );
        connect( M_fps_spin, QOverload< double >::of( &QDoubleSpinBox::valueChanged ),
                 this, &ExportDialog::updateSummary );
        connect( M_speed_spin, QOverload< double >::of( &QDoubleSpinBox::valueChanged ),
                 this, &ExportDialog::updateSummary );
        QHBoxLayout * rate = new QHBoxLayout();
        rate->addWidget( M_fps_spin );
        rate->addWidget( new QLabel( tr( "Speed:" ) ) );
        rate->addWidget( M_speed_spin );
        rate->addStretch( 1 );
        form->addRow( tr( "Rate:" ), rate );

        top->addWidget( box );
    }

    // where they go
    {
        QGroupBox * box = new QGroupBox( tr( "Output" ) );
        QFormLayout * form = new QFormLayout( box );

        M_dir_edit = new QLineEdit();
        M_dir_edit->setObjectName( "directory" );
        connect( M_dir_edit, &QLineEdit::textChanged,
                 this, &ExportDialog::updateSummary );
        QPushButton * browse = new QPushButton( tr( "Browse..." ) );
        browse->setAutoDefault( false );
        connect( browse, &QPushButton::clicked,
                 this, &ExportDialog::chooseDirectory );
        QHBoxLayout * dir = new QHBoxLayout();
        dir->addWidget( M_dir_edit, 1 );
        dir->addWidget( browse );
        form->addRow( tr( "Directory:" ), dir );

        M_prefix_edit = new QLineEdit();
        M_prefix_edit->setObjectName( "prefix" );
        M_prefix_edit->setMaximumWidth( 160 );
        connect( M_prefix_edit, &QLineEdit::textChanged,
                 this, &ExportDialog::updateSummary );
        M_format_combo = new QComboBox();
        M_format_combo->setObjectName( "format" );
        for ( const QByteArray & f : ImageSequenceSink::formats() )
        {
            M_format_combo->addItem( QString::fromLatin1( f ).toUpper(), QString::fromLatin1( f ) );
        }
        connect( M_format_combo, QOverload< int >::of( &QComboBox::currentIndexChanged ),
                 this, &ExportDialog::updateSummary );
        QHBoxLayout * name = new QHBoxLayout();
        name->addWidget( M_prefix_edit );
        name->addWidget( M_format_combo );
        name->addStretch( 1 );
        form->addRow( tr( "Names:" ), name );

        M_name_by_cycle_check = new QCheckBox( tr( "Name the images by the cycles" ) );
        M_name_by_cycle_check->setObjectName( "name_by_cycle" );
        M_name_by_cycle_check->setToolTip( tr( "e.g. image-c01234.00.png: when an image is a frame (10 images/s at x 1)" ) );
        connect( M_name_by_cycle_check, &QCheckBox::toggled,
                 this, &ExportDialog::updateSummary );
        form->addRow( QString(), M_name_by_cycle_check );

        top->addWidget( box );
    }

    M_summary_label = new QLabel();
    M_summary_label->setObjectName( "summary" );
    M_summary_label->setWordWrap( true );
    top->addWidget( M_summary_label );

    M_message_label = new QLabel();
    M_message_label->setObjectName( "message" );
    M_message_label->setWordWrap( true );
    top->addWidget( M_message_label );

    QDialogButtonBox * buttons = new QDialogButtonBox();
    M_export_button = buttons->addButton( tr( "Export" ), QDialogButtonBox::ActionRole );
    M_export_button->setObjectName( "export" );
    M_export_button->setAutoDefault( false );
    connect( M_export_button, &QPushButton::clicked,
             this, [this]() { startExport( true ); } );
    QPushButton * close = buttons->addButton( QDialogButtonBox::Close );
    close->setAutoDefault( false );
    connect( close, &QPushButton::clicked,
             this, &QDialog::close );
    top->addWidget( buttons );
}

/*-------------------------------------------------------------------*/
void
ExportDialog::readSettings()
{
    // the directory, the prefix and the format: the saved ones, or the
    // command line options (D7)
    const ExportSettings s = ExportSettings::read();

    const int size = M_size_combo->findData( s.size );
    M_size_combo->setCurrentIndex( size >= 0 ? size : 0 );
    M_width_spin->setValue( s.width );
    M_height_spin->setValue( s.height );
    M_scale_texts_check->setChecked( s.scale_texts );
    M_fps_spin->setValue( s.fps );
    M_speed_spin->setValue( s.speed );
    M_dir_edit->setText( s.directory.isEmpty() ? QDir::homePath() : s.directory );
    M_prefix_edit->setText( s.prefix );
    const int format = M_format_combo->findData( s.format );
    M_format_combo->setCurrentIndex( format >= 0 ? format : 0 );
    M_name_by_cycle_check->setChecked( s.name_by_cycle );
}

/*-------------------------------------------------------------------*/
void
ExportDialog::writeSettings() const
{
    ExportSettings s;
    s.directory = directory();
    s.prefix = M_prefix_edit->text();
    s.format = M_format_combo->currentData().toString();
    s.size = M_size_combo->currentData().toString();
    s.width = M_width_spin->value();
    s.height = M_height_spin->value();
    s.scale_texts = M_scale_texts_check->isChecked();
    s.fps = M_fps_spin->value();
    s.speed = M_speed_spin->value();
    s.name_by_cycle = M_name_by_cycle_check->isChecked();
    s.write();
}

/*-------------------------------------------------------------------*/
std::size_t
ExportDialog::positionIndex( const QLineEdit * edit,
                             const bool start,
                             bool * snapped,
                             QString * error ) const
{
    if ( snapped ) *snapped = false;
    const MonitorViewData::Cont & frames = M_main_data.viewHolder().monitorViewCont();
    if ( frames.empty() )
    {
        if ( error ) *error = tr( "There is no game log." );
        return ExportRange::INVALID_INDEX;
    }
    if ( edit->text().trimmed().isEmpty() )
    {
        return start ? 0 : frames.size() - 1;
    }

    ExportPosition pos;
    if ( ! ExportRange::parsePosition( edit->text(), &pos, error ) )
    {
        return ExportRange::INVALID_INDEX;
    }
    const std::size_t index = ( start
                                ? ExportRange::startIndex( frames, pos, snapped )
                                : ExportRange::endIndex( frames, pos, snapped ) );
    if ( index == ExportRange::INVALID_INDEX && error )
    {
        *error = ( start
                   ? tr( "The start %1 is after the last frame (%2)." ).arg( edit->text().trimmed(), ExportRange::positionText( frames, frames.size() - 1 ) )
                   : tr( "The end %1 is before the first frame (%2)." ).arg( edit->text().trimmed(), ExportRange::positionText( frames, 0 ) ) );
    }
    return index;
}

/*-------------------------------------------------------------------*/
ExportRange
ExportDialog::range( QString * error ) const
{
    ExportRange r;
    QString e;
    r.first = positionIndex( M_start_edit, true, nullptr, &e );
    if ( r.first != ExportRange::INVALID_INDEX )
    {
        r.last = positionIndex( M_end_edit, false, nullptr, &e );
    }
    if ( r.first != ExportRange::INVALID_INDEX
         && r.last != ExportRange::INVALID_INDEX
         && r.first > r.last )
    {
        e = tr( "The start is after the end." );
    }
    if ( ! r.isValid() )
    {
        r = ExportRange();
    }
    if ( error ) *error = e;
    return r;
}

/*-------------------------------------------------------------------*/
double
ExportDialog::framesPerImage() const
{
    return ExportTiming::framesPerImage( M_speed_spin->value(),
                                         step_ms(),
                                         M_fps_spin->value() );
}

/*-------------------------------------------------------------------*/
std::vector< std::size_t >
ExportDialog::imageFrames( const ExportRange & range ) const
{
    if ( ! range.isValid() )
    {
        return std::vector< std::size_t >();
    }
    return ExportTiming::frames( range.first, range.last, framesPerImage() );
}

/*-------------------------------------------------------------------*/
QSize
ExportDialog::imageSize() const
{
    const QString key = M_size_combo->currentData().toString();
    if ( key == QLatin1String( "view" ) )
    {
        const qreal dpr = M_canvas.devicePixelRatioF();
        return QSize( std::max( 1, static_cast< int >( std::lround( M_canvas.width() * dpr ) ) ),
                      std::max( 1, static_cast< int >( std::lround( M_canvas.height() * dpr ) ) ) );
    }
    if ( key == QLatin1String( "custom" ) )
    {
        return QSize( M_width_spin->value(), M_height_spin->value() );
    }
    for ( const SizeChoice & c : SIZES )
    {
        if ( key == QLatin1String( c.key ) ) return c.size;
    }
    return QSize( M_width_spin->value(), M_height_spin->value() );
}

/*-------------------------------------------------------------------*/
qreal
ExportDialog::imageRatio() const
{
    // the texts as large as in the view: the image is the view's layout,
    // scaled to the height of the image
    if ( ! M_scale_texts_check->isChecked()
         || M_canvas.height() <= 0 )
    {
        return 1.0;
    }
    return std::max( 0.1, static_cast< qreal >( imageSize().height() ) / M_canvas.height() );
}

/*-------------------------------------------------------------------*/
bool
ExportDialog::canNameByCycle() const
{
    // one frame an image: no frame twice, no frame skipped
    return std::fabs( framesPerImage() - 1.0 ) < ExportTiming::EPSILON;
}

/*-------------------------------------------------------------------*/
QStringList
ExportDialog::imageNames( const std::vector< std::size_t > & frames ) const
{
    QStringList names;
    const QString prefix = M_prefix_edit->text();
    const QString ext = M_format_combo->currentData().toString();

    if ( M_name_by_cycle_check->isChecked()
         && canNameByCycle() )
    {
        // PREFIXc01234.03.png: the digits of the largest cycle and stopped frame
        const MonitorViewData::Cont & cont = M_main_data.viewHolder().monitorViewCont();
        unsigned long long max_cycle = 0;
        unsigned long long max_stopped = 0;
        for ( const std::size_t i : frames )
        {
            max_cycle = std::max< unsigned long long >( max_cycle, std::max( 0, cont[i]->time().cycle() ) );
            max_stopped = std::max< unsigned long long >( max_stopped, std::max( 0, cont[i]->time().stopped() ) );
        }
        const int cycle_digits = std::max( 5, digitsOf( max_cycle ) );
        const int stopped_digits = std::max( 2, digitsOf( max_stopped ) );
        for ( const std::size_t i : frames )
        {
            names << prefix
                + QString( "c%1.%2." ).arg( cont[i]->time().cycle(), cycle_digits, 10, QLatin1Char( '0' ) )
                                      .arg( cont[i]->time().stopped(), stopped_digits, 10, QLatin1Char( '0' ) )
                + ext;
        }
        return names;
    }

    const int digits = ExportTiming::nameDigits( frames.size() );
    for ( std::size_t n = 0; n < frames.size(); ++n )
    {
        names << ExportTiming::imageName( prefix, n, digits, ext );
    }
    return names;
}

/*-------------------------------------------------------------------*/
QString
ExportDialog::directory() const
{
    const QString dir = M_dir_edit->text().trimmed();
    return dir.isEmpty() ? QDir::homePath() : QDir::cleanPath( dir );
}

/*-------------------------------------------------------------------*/
QString
ExportDialog::summaryText() const
{
    return M_summary_label->text();
}

/*-------------------------------------------------------------------*/
QString
ExportDialog::messageText() const
{
    return M_message_label->text();
}

/*-------------------------------------------------------------------*/
bool
ExportDialog::isExporting() const
{
    return M_job && M_job->isRunning();
}

/*-------------------------------------------------------------------*/
void
ExportDialog::setStart( const QString & text )
{
    M_start_edit->setText( text );
}

/*-------------------------------------------------------------------*/
void
ExportDialog::setEnd( const QString & text )
{
    M_end_edit->setText( text );
}

/*-------------------------------------------------------------------*/
void
ExportDialog::setStartToCurrent()
{
    const MonitorViewData::ConstPtr view = M_main_data.getCurrentViewData();
    if ( view ) M_start_edit->setText( ExportRange::positionText( view->time() ) );
}

/*-------------------------------------------------------------------*/
void
ExportDialog::setEndToCurrent()
{
    const MonitorViewData::ConstPtr view = M_main_data.getCurrentViewData();
    if ( view ) M_end_edit->setText( ExportRange::positionText( view->time() ) );
}

/*-------------------------------------------------------------------*/
void
ExportDialog::clearRange()
{
    M_start_edit->clear();
    M_end_edit->clear();
}

/*-------------------------------------------------------------------*/
void
ExportDialog::setDirectory( const QString & dir )
{
    M_dir_edit->setText( dir );
}

/*-------------------------------------------------------------------*/
void
ExportDialog::setExportAllowed( const bool allowed )
{
    M_allowed = allowed;
    updateSummary();
}

/*-------------------------------------------------------------------*/
void
ExportDialog::chooseDirectory()
{
    const QString dir = QFileDialog::getExistingDirectory( this,
                                                           tr( "The directory of the images" ),
                                                           directory() );
    if ( ! dir.isEmpty() )
    {
        M_dir_edit->setText( dir );
    }
}

/*-------------------------------------------------------------------*/
void
ExportDialog::showMessage( const QString & text,
                           const bool error )
{
    M_message_label->setText( text );
    M_message_error = error;
    UIPalette::markWrongText( M_message_label, error );
}

/*-------------------------------------------------------------------*/
void
ExportDialog::changeEvent( QEvent * event )
{
    QDialog::changeEvent( event );
    if ( event->type() == QEvent::PaletteChange
         || event->type() == QEvent::ApplicationPaletteChange )
    {
        // the wrong texts in the red of the new palette
        UIPalette::markWrongText( M_message_label, M_message_error );
        updateSummary();
    }
}

/*-------------------------------------------------------------------*/
void
ExportDialog::updateSummary()
{
    const MonitorViewData::Cont & frames = M_main_data.viewHolder().monitorViewCont();
    const int step = step_ms();

    // the frame of each field, or why not
    const auto info = [&]( QLineEdit * edit, QLabel * label, const bool start ) {
        bool snapped = false;
        QString error;
        const std::size_t index = positionIndex( edit, start, &snapped, &error );
        if ( index == ExportRange::INVALID_INDEX )
        {
            label->setText( frames.empty() ? QString() : error );
            label->setToolTip( label->text() );
            UIPalette::markWrongText( label, ! frames.empty() );
            return;
        }
        const rcsc::GameTime & t = frames[index]->time();
        QString text = QString( "%1 (%2)" ).arg( ExportRange::positionText( t ),
                                                 ExportRange::gameTime( t.cycle(), step ) );
        if ( snapped )
        {
            text += tr( ": no frame at %1" ).arg( edit->text().trimmed() );
        }
        label->setText( text );
        label->setToolTip( text );
        UIPalette::markWrongText( label, false );
    };
    info( M_start_edit, M_start_info, true );
    info( M_end_edit, M_end_info, false );

    const bool custom = ( M_size_combo->currentData().toString() == QLatin1String( "custom" ) );
    M_width_spin->setEnabled( custom );
    M_height_spin->setEnabled( custom );
    if ( ! custom )
    {
        const QSize size = imageSize();
        const QSignalBlocker block_width( M_width_spin );
        const QSignalBlocker block_height( M_height_spin );
        M_width_spin->setValue( size.width() );
        M_height_spin->setValue( size.height() );
    }

    M_name_by_cycle_check->setEnabled( canNameByCycle() );

    QString error;
    const ExportRange r = range( &error );
    const std::vector< std::size_t > images = imageFrames( r );
    if ( ! r.isValid() || images.empty() )
    {
        M_summary_label->setText( error.isEmpty() ? tr( "There is no frame to export." ) : error );
        UIPalette::markWrongText( M_summary_label, ! frames.empty() );
    }
    else
    {
        const QSize size = imageSize();
        const QString count = ( images.size() == 1
                                ? tr( "1 image" )
                                : tr( "%1 images" ).arg( images.size() ) );
        const QString of = ( r.size() == 1
                             ? tr( "of 1 frame" )
                             : tr( "of %1 frames" ).arg( r.size() ) );
        M_summary_label->setText( tr( "%1 %2, %3 to %4. %5 x %6 pixels; %7 s of a video at %8 images/s." )
                                  .arg( count, of,
                                        ExportRange::gameTime( frames[r.first]->time().cycle(), step ),
                                        ExportRange::gameTime( frames[r.last]->time().cycle(), step ) )
                                  .arg( size.width() ).arg( size.height() )
                                  .arg( ExportTiming::duration( images.size(), M_fps_spin->value() ), 0, 'f', 1 )
                                  .arg( M_fps_spin->value() ) );
        UIPalette::markWrongText( M_summary_label, false );
    }

    M_export_button->setEnabled( M_allowed
                                 && ! isExporting()
                                 && r.isValid()
                                 && ! images.empty() );
    if ( ! M_allowed )
    {
        M_export_button->setToolTip( tr( "Not available while connected to a server: the frames are exported after disconnecting." ) );
    }
    else
    {
        M_export_button->setToolTip( QString() );
    }
}

/*-------------------------------------------------------------------*/
bool
ExportDialog::startExport( const bool ask )
{
    if ( isExporting() )
    {
        return false;
    }
    if ( ! M_allowed )
    {
        showMessage( tr( "Not available while connected to a server." ), true );
        return false;
    }

    QString error;
    const ExportRange r = range( &error );
    const std::vector< std::size_t > frames = imageFrames( r );
    if ( ! r.isValid() || frames.empty() )
    {
        showMessage( error.isEmpty() ? tr( "There is no frame to export." ) : error, true );
        return false;
    }

    const QSize pixels = imageSize();
    const qreal dpr = imageRatio();
    const QByteArray format = M_format_combo->currentData().toString().toLatin1();
    const QStringList names = imageNames( frames );

    // the files there: asked once, not for each image
    QString dir = directory();
    for ( ; ; )
    {
        int there = 0;
        for ( const QString & name : names )
        {
            if ( QFileInfo::exists( QDir( dir ).filePath( name ) ) ) ++there;
        }
        if ( there == 0 || ! ask )
        {
            break;
        }

        QMessageBox box( QMessageBox::Question,
                         tr( "Export Images" ),
                         tr( "%1 of the %2 files are already in %3." )
                         .arg( there ).arg( names.size() ).arg( QDir::toNativeSeparators( dir ) ),
                         QMessageBox::NoButton,
                         this );
        QPushButton * overwrite = box.addButton( tr( "Overwrite" ), QMessageBox::AcceptRole );
        QPushButton * another = box.addButton( tr( "Choose Another Directory..." ), QMessageBox::ActionRole );
        box.addButton( QMessageBox::Cancel );
        box.setDefaultButton( overwrite );
        box.exec();
        if ( box.clickedButton() == overwrite )
        {
            break;
        }
        if ( box.clickedButton() != another )
        {
            return false;
        }
        const QString chosen = QFileDialog::getExistingDirectory( this,
                                                                  tr( "The directory of the images" ),
                                                                  dir );
        if ( chosen.isEmpty() )
        {
            return false;
        }
        dir = QDir::cleanPath( chosen );
        M_dir_edit->setText( dir );
    }

    // the space on the disk: the first image compressed, times the images
    qint64 estimate = 0;
    {
        QBuffer buffer;
        buffer.open( QIODevice::WriteOnly );
        QImageWriter writer( &buffer, format );
        if ( writer.write( M_canvas.renderFrame( frames.front(), pixels, dpr ) ) )
        {
            estimate = static_cast< qint64 >( buffer.size() * 1.1 * frames.size() );
        }
    }
    const QStorageInfo storage( existingDirectory( dir ) );
    if ( ask
         && estimate > 0
         && storage.isValid()
         && storage.bytesAvailable() >= 0
         && estimate > storage.bytesAvailable() )
    {
        if ( QMessageBox::warning( this,
                                   tr( "Export Images" ),
                                   tr( "The images may not fit: about %1 for them, %2 free in %3.\nExport them anyway?" )
                                   .arg( bytesText( estimate ),
                                         bytesText( storage.bytesAvailable() ),
                                         QDir::toNativeSeparators( storage.rootPath() ) ),
                                   QMessageBox::Yes | QMessageBox::No,
                                   QMessageBox::No ) != QMessageBox::Yes )
        {
            return false;
        }
    }

    writeSettings();
    emit stopRequested();

    ImageSequenceSink * sink = new ImageSequenceSink( dir, names, format );
    ExportJob * job = new ExportJob( M_canvas, sink, frames, pixels, dpr, M_fps_spin->value(), this );
    M_job = job;

    QProgressDialog * progress = new QProgressDialog( tr( "Exporting %1 images (about %2)..." ).arg( frames.size() ).arg( bytesText( estimate ) ),
                                                      tr( "Stop" ),
                                                      0, static_cast< int >( frames.size() ),
                                                      this );
    progress->setObjectName( "export_progress" );
    progress->setWindowTitle( tr( "Export Images" ) );
    progress->setWindowModality( Qt::ApplicationModal );
    progress->setMinimumDuration( 500 );
    progress->setAutoClose( false );
    progress->setAutoReset( false );
    M_progress = progress;
    connect( progress, &QProgressDialog::canceled,
             this, &ExportDialog::abortExport );
    std::shared_ptr< QElapsedTimer > clock = std::make_shared< QElapsedTimer >();
    clock->start();
    connect( job, &ExportJob::progressed,
             this, [this, clock]( std::size_t done, std::size_t total ) {
                 if ( ! M_progress ) return;
                 QString text = tr( "Exported %1 of %2 images." ).arg( done ).arg( total );
                 if ( done > 0
                      && done < total
                      && clock->elapsed() >= 2000 )
                 {
                     // the time left at the speed so far
                     text += QLatin1Char( ' ' )
                         + leftText( static_cast< qint64 >( clock->elapsed() * static_cast< double >( total - done ) / done ) );
                 }
                 // the label first: setValue() of a modal dialog processes the events
                 M_progress->setLabelText( text );
                 M_progress->setValue( static_cast< int >( done ) );
             } );
    connect( job, &ExportJob::finished,
             this, &ExportDialog::handleFinished );

    showMessage( tr( "Exporting..." ), false );
    updateSummary();
    job->start();
    return true;
}

/*-------------------------------------------------------------------*/
void
ExportDialog::abortExport()
{
    if ( M_job )
    {
        M_job->abort();
    }
}

/*-------------------------------------------------------------------*/
void
ExportDialog::handleFinished( bool ok,
                              const QString & message )
{
    if ( M_progress )
    {
        M_progress->hide();
        M_progress->deleteLater();
        M_progress = nullptr;
    }
    if ( M_job )
    {
        M_job->deleteLater();
        M_job = nullptr;
    }

    showMessage( ok ? tr( "Exported %1." ).arg( message ) : message, ! ok );
    updateSummary();
    emit exportFinished( ok, message );
}
