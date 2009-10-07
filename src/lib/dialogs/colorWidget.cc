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


#include <anatomist/dialogs/colorWidget.h>
#include <qlabel.h>
#include <aims/qtcompat/qvbox.h>
#include <aims/qtcompat/qhbox.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qslider.h>
#include <qcheckbox.h>


struct QAColorWidget_PrivateData
{
  QAColorWidget_PrivateData()
    : color( 0 ), slr( 0 ), slg( 0 ), slb( 0 ), lbr( 0 ), lbg( 0 ), lbb( 0 ) {}

  QLabel		*color;
  QSlider		*slr;
  QSlider		*slg;
  QSlider		*slb;
  QSlider		*sla;
  QLabel		*lbr;
  QLabel		*lbg;
  QLabel		*lbb;
  QLabel		*lba;
  QCheckBox		*nalpha;
  QHBox			*alphbox;
};


QAColorWidget::QAColorWidget( QColor init, QWidget * parent, 
			      const char * name, Qt::WFlags flags, 
			      bool allowAlpha, bool allowNeutralAlpha, 
			      int initalpha, bool neutral )
  : QWidget( parent, name, flags ), _pdat( new QAColorWidget_PrivateData )
{
  setCaption( name );

  QVBoxLayout	*lay1 = new QVBoxLayout( this, 10, -1, "lay1" );
  _pdat->color = new QLabel( this, "color" );
  QPixmap		pix( 80, 40 );
  pix.fill( init );
  _pdat->color->setPixmap( pix );
  _pdat->color->setFixedSize( _pdat->color->sizeHint() );

  QVBox		*fr1 = new QVBox( this );
  fr1->setSpacing( 5 );
  QHBox		*hb = new QHBox( fr1 );
  hb->setSpacing( 5 );
  new QLabel( tr( "R :" ), hb );
  _pdat->slr = new QSlider( 0, 255, 1, init.red(), Qt::Horizontal, hb, 
			    "slider_red" );
  _pdat->lbr = new QLabel( QString::number( init.red() ), hb );
  hb = new QHBox( fr1 );
  hb->setSpacing( 5 );
  new QLabel( tr( "G :" ), hb );
  _pdat->slg = new QSlider( 0, 255, 1, init.green(), Qt::Horizontal, 
			    hb, "slider_green" );
  _pdat->lbg = new QLabel( QString::number( init.green() ), hb );
  hb = new QHBox( fr1 );
  hb->setSpacing( 5 );
  new QLabel( tr( "B :" ), hb );
  _pdat->slb = new QSlider( 0, 255, 1, init.blue(), Qt::Horizontal, 
			    hb, "slider_blue" );
  _pdat->lbb = new QLabel( QString::number( init.blue() ), hb );

  _pdat->alphbox = new QVBox( fr1 );
  hb = new QHBox( _pdat->alphbox );
  if( !allowAlpha )
    hb->hide();
  hb->setSpacing( 5 );
  new QLabel( tr( "A :" ), hb );
  _pdat->sla = new QSlider( 0, 255, 1, initalpha, Qt::Horizontal, 
			    hb, "slider_alpha" );
  _pdat->lba = new QLabel( QString::number( initalpha ), hb );
  _pdat->nalpha = new QCheckBox( tr( "Neutral alpha channel" ), 
				_pdat->alphbox );
  if( !allowNeutralAlpha )
    _pdat->nalpha->hide();
  _pdat->nalpha->setChecked( neutral );

  _pdat->slr->setFixedHeight( _pdat->slr->sizeHint().height() );
  _pdat->slg->setFixedHeight( _pdat->slg->sizeHint().height() );
  _pdat->slb->setFixedHeight( _pdat->slb->sizeHint().height() );
  _pdat->sla->setFixedHeight( _pdat->slb->sizeHint().height() );
  _pdat->slr->setMinimumWidth( 50 );
  _pdat->slg->setMinimumWidth( 50 );
  _pdat->slb->setMinimumWidth( 50 );
  _pdat->sla->setMinimumWidth( 50 );
  _pdat->lbr->setMinimumWidth( 25 );
  _pdat->lbg->setMinimumWidth( 25 );
  _pdat->lbb->setMinimumWidth( 25 );
  _pdat->lba->setMinimumWidth( 25 );

  lay1->addWidget( _pdat->color, 0, Qt::AlignCenter );
  lay1->addWidget( fr1, 0, Qt::AlignCenter );

  connect( _pdat->slr, SIGNAL( valueChanged( int ) ), this, 
	   SLOT( slidersChanged( int ) ) );
  connect( _pdat->slg, SIGNAL( valueChanged( int ) ), this, 
	   SLOT( slidersChanged( int ) ) );
  connect( _pdat->slb, SIGNAL( valueChanged( int ) ), this, 
	   SLOT( slidersChanged( int ) ) );
  connect( _pdat->sla, SIGNAL( valueChanged( int ) ), this, 
	   SLOT( slidersChanged( int ) ) );
}


QAColorWidget::~QAColorWidget()
{
}


void QAColorWidget::relook( const QColor & col, int alpha, bool allowalph, 
			    bool neutralph, bool allownalph )
{
  _pdat->slr->setValue( col.red() );
  _pdat->slg->setValue( col.green() );
  _pdat->slb->setValue( col.blue() );
  _pdat->lbr->setText( QString::number( col.red() ) );
  _pdat->lbg->setText( QString::number( col.green() ) );
  _pdat->lbb->setText( QString::number( col.blue() ) );
  _pdat->sla->setValue( alpha );
  _pdat->lba->setText( QString::number( alpha ) );
  if( allowalph )
    _pdat->alphbox->show();
  else
    _pdat->alphbox->hide();
  if( allownalph )
    _pdat->nalpha->show();
  else
    _pdat->nalpha->show();
  _pdat->nalpha->setChecked( neutralph );

  QPixmap	pix = *_pdat->color->pixmap();
  pix.fill( col );
  _pdat->color->setPixmap( pix );
}


void QAColorWidget::slidersChanged( int )
{
  QPixmap	pix = *_pdat->color->pixmap();
  QColor	col = color();
  pix.fill( col );
  _pdat->lbr->setText( QString::number( col.red() ) );
  _pdat->lbg->setText( QString::number( col.green() ) );
  _pdat->lbb->setText( QString::number( col.blue() ) );
  _pdat->lba->setText( QString::number( alpha() ) );
  _pdat->color->setPixmap( pix );
  emit colorChanged();
}


QColor QAColorWidget::color() const
{
  return( QColor( _pdat->slr->value(), _pdat->slg->value(), 
		  _pdat->slb->value() ) );
}


void QAColorWidget::allowNeutralAlpha( bool x )
{
  if( x )
    _pdat->nalpha->show();
  else
    {
      _pdat->nalpha->setChecked( false );
      _pdat->nalpha->hide();
    }
}


int QAColorWidget::alpha() const
{
  return( _pdat->sla->value() );
}


bool QAColorWidget::neutralAlpha() const
{
  return( _pdat->nalpha->isChecked() );
}
