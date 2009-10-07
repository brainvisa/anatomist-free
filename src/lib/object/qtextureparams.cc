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

#include <anatomist/object/qtextureparams.h>
#include <qlayout.h>
#include <aims/qtcompat/qvgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qslider.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qvalidator.h>
#include <math.h>

using namespace std;

struct QVectorCompEditor::Private
{
  Private();

  QLineEdit	*lineedit;
  QSlider	*slider;
  float		value;
};


QVectorCompEditor::Private::Private()
  : lineedit( 0 ), slider( 0 ), value( 0 )
{
}


QVectorCompEditor::QVectorCompEditor( const QString & label, QWidget* parent, 
                                      const char* name )
  : QHBox( parent, name ), d( new Private )
{
  setSpacing( 5 );
  new QLabel( label, this );
  d->lineedit = new QLineEdit( this );
  d->slider = new QSlider( -100, 100, 1, 0, Qt::Horizontal, this );
  QValidator	*v = new QDoubleValidator( -1, 1, 6, d->lineedit );
  d->lineedit->setValidator( v );

  connect( d->lineedit, SIGNAL( textChanged( const QString & ) ), 
           this, SLOT( editChanged( const QString & ) ) );
  connect( d->slider, SIGNAL( valueChanged( int ) ), this, 
           SLOT( sliderChanged( int ) ) );
}


QVectorCompEditor::~QVectorCompEditor()
{
  delete d;
}

void QVectorCompEditor::setValue( float x )
{
  d->value = 0;
  d->lineedit->setText( QString::number( x ) );
  d->slider->setValue( (int) rint( x*100 ) );
}


float QVectorCompEditor::value() const
{
  return d->value;
}


void QVectorCompEditor::sliderChanged( int x )
{
  d->lineedit->setText( QString::number( float( x ) / 100 ) );
  d->value = float( x ) / 100;
  emit valueChanged( d->value );
}


void QVectorCompEditor::editChanged( const QString & s )
{
  bool	ok;
  float	x = s.toFloat( &ok );
  if( ok )
    {
      d->slider->setValue( (int) rint( x*100 ) );
      d->value = x;
      emit valueChanged( d->value );
      emit valueChanged( this, d->value );
    }
}

// ----------

struct QTextureVectorEditor::Private
{
  Private();

  float			values[4];
  float			scale;
  QVectorCompEditor	*edits[4];
  QLineEdit		*scaleed;
  bool			recursing;
};


QTextureVectorEditor::Private::Private() 
  : scale( 1 ), scaleed( 0 ), recursing( false )
{
  values[0] = 1;
  values[1] = 0;
  values[2] = 0;
  values[3] = 0;
}


QTextureVectorEditor::QTextureVectorEditor( QWidget* parent, const char* name )
  : QVBox( parent, name ), d( new Private )
{
  setSpacing( 5 );
  setMargin( 5 );
  QVGroupBox	*dirbox = new QVGroupBox( tr( "Direction:" ), this );
  QVGroupBox	*scale = new QVGroupBox( tr( "Scale:" ), this );
  d->edits[0] = new QVectorCompEditor( "x:", dirbox );
  d->edits[1] = new QVectorCompEditor( "y:", dirbox );
  d->edits[2] = new QVectorCompEditor( "z:", dirbox );
  d->edits[3] = new QVectorCompEditor( "p:", dirbox );
  d->edits[3]->hide();
  d->scaleed = new QLineEdit( "1", scale );
  QDoubleValidator	*v = new QDoubleValidator( d->scaleed );
  v->setBottom( 0 );
  d->scaleed->setValidator( v );

  connect( d->edits[0], 
           SIGNAL( valueChanged( QVectorCompEditor *, float ) ), 
           this, 
           SLOT( componentValueChanged( QVectorCompEditor *, float ) ) );
  connect( d->edits[1], 
           SIGNAL( valueChanged( QVectorCompEditor *, float ) ), 
           this, 
           SLOT( componentValueChanged( QVectorCompEditor *, float ) ) );
  connect( d->edits[2], 
           SIGNAL( valueChanged( QVectorCompEditor *, float ) ), 
           this, 
           SLOT( componentValueChanged( QVectorCompEditor *, float ) ) );
  connect( d->edits[3], 
           SIGNAL( valueChanged( QVectorCompEditor *, float ) ), 
           this, 
           SLOT( componentValueChanged( QVectorCompEditor *, float ) ) );
  connect( d->scaleed, SIGNAL( textChanged( const QString & ) ), this, 
           SLOT( scaleChanged( const QString & ) ) );

  d->edits[0]->setValue( d->values[0] );
  d->edits[3]->setValue( d->values[3] );
}


QTextureVectorEditor::~QTextureVectorEditor()
{
  delete d;
}


void QTextureVectorEditor::setValues( const std::vector<float> & values )
{
  float	inorm = 1. / sqrt( values[0] * values[0] + values[1] * values[1] 
                           + values[2] * values[2] );
  d->scale = 1. / inorm;
  d->values[0] = values[0] * inorm;
  d->values[1] = values[1] * inorm;
  d->values[2] = values[2] * inorm;
  d->values[3] = values[3] * inorm;
  d->recursing = true;
  d->edits[0]->setValue( d->values[0] );
  d->edits[1]->setValue( d->values[1] );
  d->edits[2]->setValue( d->values[2] );
  d->edits[3]->setValue( d->values[3] );
  d->scaleed->setText( QString::number( d->scale ) );
  d->recursing = false;
}


vector<float> QTextureVectorEditor::values() const
{
  vector<float>	val;
  val.reserve( 4 );
  val.push_back( d->values[0] * d->scale );
  val.push_back( d->values[1] * d->scale );
  val.push_back( d->values[2] * d->scale );
  val.push_back( d->values[3] );
  return val;
}


void QTextureVectorEditor::componentValueChanged( QVectorCompEditor* ed, 
                                                  float x )
{
  if( d->recursing )
    return;

  d->recursing = true;

  unsigned	comp = 0;
  if( ed != d->edits[0] )
  {
    if( ed == d->edits[1] )
      comp = 1;
    else if( ed == d->edits[2] )
      comp = 2;
    else
      comp = 3;
  }
  d->values[comp] = x;
  if( comp < 3 )
    {
      unsigned	i = 1, j = 2;
      switch( comp )
        {
        case 1:
          i = 0;
          break;
        case 2:
          i = 0;
          j = 1;
          break;
        default:
          break;
        }
      if( x >= 1 )
        {
          d->values[comp] = 1;
          d->values[i] = 0;
          d->values[j] = 0;
        }
      else if( d->values[i] != 0 || d->values[j] != 0 )
        {
          float	alpha 
            = sqrt( ( 1. - x * x ) / 
                    ( d->values[i] * d->values[i] 
                      + d->values[j] * d->values[j] ) );
          d->values[i] *= alpha;
          d->values[j] *= alpha;
        }
      else
        {
          d->values[i] = sqrt( ( 1. - x * x ) / 2 );
          d->values[j] = d->values[i];
        }
      d->edits[i]->setValue( d->values[i] );
      d->edits[j]->setValue( d->values[j] );
    }
  d->recursing = false;
}


void QTextureVectorEditor::scaleChanged( const QString & s )
{
  bool	ok = false;
  float	x = s.toFloat( &ok );
  if( ok )
    d->scale = x;
}

// ----------

struct QTextureParams::Private
{
  Private();

  QTextureVectorEditor	*edits[3];
};


QTextureParams::Private::Private()
{
}


QTextureParams::QTextureParams( QWidget *parent, const char *name, 
                                bool modal, Qt::WFlags f )
  : QDialog( parent, name, modal, f ), d( new Private )
{
  setCaption( tr( "Texture generation parameters" ) );
  QVBoxLayout	*layout = new QVBoxLayout( this );
  layout->setMargin( 5 );
  layout->setSpacing( 5 );
  QTabWidget	*comps = new QTabWidget( this );
  QHBox	*buts = new QHBox( this );
  layout->addWidget( comps );
  layout->addWidget( buts );
  d->edits[0] = new QTextureVectorEditor( comps );
  comps->addTab( d->edits[0], tr( "1st comp." ) );
  d->edits[1] = new QTextureVectorEditor( comps );
  comps->addTab( d->edits[1], tr( "2nd comp." ) );
  d->edits[2] = new QTextureVectorEditor( comps );
  comps->addTab( d->edits[2], tr( "3rd comp." ) );
  buts->setSpacing( 10 );
  QPushButton	*bok = new QPushButton( tr( "OK" ), buts, "ok_button" );
  QPushButton	*bcc = new QPushButton( tr( "Cancel" ), buts, 
                                        "cancel_button" );
  bok->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
  bcc->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
  bok->setDefault( false );
  connect( bok, SIGNAL( clicked() ), this, SLOT( accept() ) );
  connect( bcc, SIGNAL( clicked() ), this, SLOT( reject() ) );
}


QTextureParams::~QTextureParams()
{
  delete d;
}


void QTextureParams::setParams( unsigned comp, 
                                const std::vector<float> & values )
{
  d->edits[comp]->setValues( values );
}


std::vector<float> QTextureParams::params( unsigned comp ) const
{
  return d->edits[comp]->values();
}



