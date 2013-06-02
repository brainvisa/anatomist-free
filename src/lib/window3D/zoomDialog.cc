/* This software and supporting documentation are distributed by
 *     Institut Federatif de Recherche 49
 *     CEA/NeuroSpin, Batiment 145,
 *     91191 Gif-sur-Yvette cedex
 *     France
 *
 * This software is governed by the CeCILL-B license under
 * French law and abiding by the rules of distribution of free software.
 * You can  use, modify and/or redistribute the software under the
 * terms of the CeCILL-B license as circulated by CEA, CNRS
 * and INRIA at the following URL "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and  rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty  and the software's author,  the holder of the
 * economic rights,  and the successive licensors  have only  limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading,  using,  modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean  that it is complicated to manipulate,  and  that  also
 * therefore means  that it is reserved for developers  and  experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and,  more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL-B license and that you accept its terms.
 */


#include <anatomist/window3D/zoomDialog.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>


using namespace anatomist;


namespace anatomist
{
  struct ZoomDialog_Private
  {
    ZoomDialog_Private() : zoom( 0 ), fixzoom( 0 ), resize( 0 ) {}

    QLineEdit	*zoom;
    QComboBox	*fixzoom;
    QCheckBox	*resize;
  };
}


ZoomDialog::ZoomDialog( float zoom, bool forceResize, QWidget* parent, 
			const char* name, bool modal, Qt::WFlags f )
  : QDialog( parent, f ), d( new ZoomDialog_Private )
{
  setWindowTitle( tr( "Zoom selection" ) );
  setObjectName(name);
  setModal(modal);

  QVBoxLayout	*l = new QVBoxLayout( this );
  setLayout( l );
  l->setMargin( 5 );
  l->setSpacing( 5 );
  l->addWidget( new QLabel( tr( "Zoom :" ), this ) );
  QWidget *hb = new QWidget( this );
  l->addWidget( hb );
  QHBoxLayout *hlay = new QHBoxLayout( hb );
  hb->setLayout( hlay );
  hlay->setSpacing( 10 );
  hlay->setMargin( 0 );

  float		z = zoom;
  d->zoom = new QLineEdit( QString::number( z ), hb );
  hlay->addWidget( d->zoom );
  QComboBox	*fz = d->fixzoom = new QComboBox( hb );
  hlay->addWidget( fz );

  unsigned	i;
  for( i=10; i>1; --i )
    fz->addItem( QString( "1:" ) + QString::number( i ) );
  fz->addItem( "2:3" );
  fz->addItem( "1" );
  fz->addItem( "3:2" );
  for( i=2; i<=10; ++i )
    fz->addItem( QString::number( i ) );
  fz->setMaxVisibleItems( fz->count() );
  fz->setCurrentIndex( 10 );

  d->resize = new QCheckBox( tr( "Resize window to fit objects" ), this );
  l->addWidget( d->resize );
  d->resize->setChecked( forceResize );

  QWidget *hb2 = new QWidget( this );
  l->addWidget( hb2 );
  hlay = new QHBoxLayout( hb2 );
  hb2->setLayout( hlay );
  hlay->setSpacing( 10 );
  hlay->setMargin( 0 );
  QPushButton	*ok = new QPushButton( tr( "OK" ), hb2 );
  hlay->addWidget( ok );
  ok->setDefault( true );
  QPushButton *cc = new QPushButton( tr( "Cancel" ), hb2 );
  hlay->addWidget( cc );
  connect( ok, SIGNAL( clicked() ), this, SLOT( accept() ) );
  connect( cc, SIGNAL( clicked() ), this, SLOT( reject() ) );
  connect( d->fixzoom, SIGNAL( activated( int ) ), this, 
	   SLOT( fixedZoomChanged( int ) ) );
}


ZoomDialog::~ZoomDialog()
{
}


QString ZoomDialog::zoomText() const
{
  return( d->zoom->text() );
}


void ZoomDialog::setZoomText( const QString & txt )
{
  d->zoom->setText( txt );
}


bool ZoomDialog::mustResize() const
{
  return( d->resize->isChecked() );
}


void ZoomDialog::setMustResize( bool x )
{
  d->resize->setChecked( x );
}


void ZoomDialog::fixedZoomChanged( int n )
{
  if( n < 9 )
    d->zoom->setText( QString::number( 1. / ( 10 - n ) ) );
  else if( n == 9 )
    d->zoom->setText( QString::number( 2./3 ) );
  else if( n == 10 )
    d->zoom->setText( "1" );
  else if( n == 11 )
    d->zoom->setText( "1.5" );
  else
    d->zoom->setText( QString::number( n - 10 ) );
}
