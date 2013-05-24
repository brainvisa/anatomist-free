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

#include <anatomist/object/texturewin.h>
#include <anatomist/object/texturepanel.h>
#include <anatomist/object/objectparamselect.h>
#include <anatomist/object/Object.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/surface/glcomponent.h>
#include <anatomist/application/settings.h>
#include <qlayout.h>
#include <qspinbox.h>
#include <qgroupbox.h>
#include <qpixmap.h>
#include <qicon.h>

using namespace anatomist;
using namespace std;


struct QTextureWin::Private
{
  Private( const set<AObject *> & obj );
  ObjectParamSelect	*objsel;
  QWidget		*main;
  QTexturePanel		*texpanel;
  set<AObject *>	initial;
  unsigned		currenttex;
  QSpinBox		*texbox;
};


QTextureWin::Private::Private( const set<AObject *> & obj )
  : objsel( 0 ), main( 0 ), texpanel( 0 ), initial( obj ), currenttex( 0 ), 
    texbox( 0 )
{
}


namespace
{

  bool filterTextured( const AObject* o )
  {
    const GLComponent	*c = o->glAPI();
    return c && c->glNumTextures() > 0;
  }

}


QTextureWin::QTextureWin( const set<AObject *> & obj, 
                          QWidget* parent, const char *name, Qt::WindowFlags f )
  : QWidget( parent ), Observer(), d( new Private( obj ) )
{
  setWindowTitle( tr( "Texturing properties" ) );
  setWindowFlags( f );
  setObjectName(name);
  setAttribute( Qt::WA_DeleteOnClose );
  if( windowFlags() & Qt::Window )
  {
    QPixmap	anaicon( Settings::findResourceFile(
      "icons/icon.xpm" ).c_str() );
    if( !anaicon.isNull() )
      setWindowIcon( anaicon );
  }

  QVBoxLayout	*mainlay = new QVBoxLayout( this );
  mainlay->setObjectName( "mainlayout" );
  mainlay->setMargin( 5 );
  mainlay->setSpacing( 5 );

  d->objsel = new ObjectParamSelect( obj, this );
  mainlay->addWidget( d->objsel );
  d->objsel->addFilter( filterTextured );

  d->main = new QWidget( this );
  QHBoxLayout *hlay = new QHBoxLayout( d->main );
  d->main->setLayout( hlay );
  hlay->setSpacing( 5 );
  hlay->setMargin( 0 );
  mainlay->addWidget( d->main );
  QGroupBox	*nb = new QGroupBox( tr( "Texture number" ), d->main );
  hlay->addWidget( nb );
  QVBoxLayout *gblay = new QVBoxLayout( nb );
  nb->setLayout( gblay );
  gblay->setMargin( 5 );
  gblay->setSpacing( 5 );
  d->texbox = new QSpinBox( nb );
  gblay->addWidget( d->texbox );
  d->texbox->setSingleStep( 1 );
  d->texbox->setMinimum( 0 );
  gblay->addStretch( 1 );

  d->texpanel = new QTexturePanel( obj, d->main );
  hlay->addWidget( d->texpanel );

  updateInterface();

  connect( d->texbox, SIGNAL( valueChanged( int ) ), this, 
           SLOT( selectTexture( int ) ) );
  connect( d->objsel, SIGNAL( selectionStarts() ), this, 
           SLOT( chooseObject() ) );
  connect( d->objsel, 
           SIGNAL( objectsSelected( const std::set<anatomist::AObject *> & ) ),
           this, 
           SLOT( objectsChosen( const std::set<anatomist::AObject *> & ) ) );
}


QTextureWin::~QTextureWin()
{
  //d->texpanel->setObjects( set<AObject *>() );
  //d->objsel->updateLabel( set<AObject *>() );
  cleanupObserver();

  delete d;
}


const set<AObject *> & QTextureWin::objects() const
{
  return d->texpanel->objects();
}


void QTextureWin::update( const Observable* /*observable*/, void* /*arg*/ )
{
  updateInterface();
}


void QTextureWin::unregisterObservable( Observable* obs )
{
  Observer::unregisterObservable( obs );
  AObject	*o = dynamic_cast<AObject *>( obs );
  if( !o )
    return;
  
  d->objsel->updateLabel( d->texpanel->objects() );
  updateInterface();
}


void QTextureWin::updateInterface()
{
  const set<AObject *>	& obj = d->texpanel->objects();
  if( obj.empty() )
    d->main->setEnabled( false );
  else
    {
      d->main->setEnabled( true );
      AObject	*o = *obj.begin();
      GLComponent	*c = o->glAPI();
      if( obj.size() == 1 && c && c->glNumTextures() >= 1 )
        {
          if( c->glNumTextures() >= 2 )
            d->texbox->setEnabled( true );
          else
            d->texbox->setEnabled( false );
          d->texbox->setMaximum( c->glNumTextures() - 1 );
          if( d->currenttex >= c->glNumTextures() )
            d->currenttex = 0;
          d->texbox->blockSignals( true );
          d->texbox->setValue( d->currenttex );
          d->texbox->blockSignals( false );
        }
      else
        {
          d->texbox->setEnabled( false );
        }
    }
}


void QTextureWin::chooseObject()
{
  // cout << "chooseObject\n";
  // filter out objects that don't exist anymore
  set<AObject *>::iterator	ir = d->initial.begin(), 
    er = d->initial.end(), ir2;
  while( ir!=er )
    if( theAnatomist->hasObject( *ir ) )
      ++ir;
    else
      {
        ir2 = ir;
        ++ir;
        d->initial.erase( ir2 );
      }

  d->objsel->selectObjects( d->initial, d->texpanel->objects() );
}


void QTextureWin::objectsChosen( const set<AObject *> & o )
{
  // cout << "objects chosen: " << o.size() << endl;

  const set<AObject *>			& obj = d->texpanel->objects();
  set<AObject *>::const_iterator	i, e = obj.end();
  for( i=obj.begin(); i!=e; ++i )
    (*i)->deleteObserver( this );
  d->texpanel->setObjects( o );
  for( i=obj.begin(); i!=e; ++i )
    (*i)->addObserver( this );

  d->objsel->updateLabel( o );
  d->currenttex = 0;
  if( !obj.empty() )
    {
      d->texpanel->setActiveTexture( 0 );
    }
  updateInterface();
}


void QTextureWin::selectTexture( int x )
{
  d->currenttex = (unsigned) x;
  d->texpanel->setActiveTexture( (unsigned) x );
}


