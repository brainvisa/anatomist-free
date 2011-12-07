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


#include <anatomist/mobject/wFusion2D.h>
#include <qlayout.h>
#include <aims/qtcompat/qhbox.h>
#include <aims/qtcompat/qhgroupbox.h>
#include <aims/qtcompat/qvgroupbox.h>
#include <aims/qtcompat/qvbuttongroup.h>
#include <qradiobutton.h>
#include <qslider.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qpen.h>
#include <qpainter.h>
#include <qbitmap.h>
#include <qlistwidget.h>
#include <aims/qtcompat/qvbox.h>
#include <qpixmap.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/processor/Processor.h>
#include <anatomist/commands/cFusion2DParams.h>
#include <anatomist/object/texturepanel.h>
#include <anatomist/object/objectparamselect.h>
#include <anatomist/application/settings.h>


using namespace anatomist;
using namespace std;


struct Fusion2DWindow::Private
{
  Private( const set<AObject *> & o ) 
    : recurs( false ), currentsub( 1 ), initial( o ) {}

  QTexturePanel		*texpanel;
  ObjectParamSelect	*objsel;
  bool			recurs;
  QVGroupBox		*objbox;
  QListWidget		*orderbx;
  int			currentsub;
  set<AObject *>	initial;
};


Fusion2DWindow::Fusion2DWindow( const set<AObject *> &objL, 
				QWidget* parent, 
				const char *name, Qt::WFlags f )
  : QWidget( parent, name, f ), Observer(), 
    pdat( new Private( objL ) )
{
  set<AObject *>::const_iterator	io, fo = objL.end();

  for( io=objL.begin(); io!=fo; ++io )
    if ( (*io)->type() == AObject::FUSION2D )
      {
	_obj.insert( *io );
	(*io)->addObserver( this );
      }

  setCaption( tr( "Fusion 2D control" ) );
  if( parent == 0 )
  {
    QPixmap	anaicon( Settings::findResourceFile(
                        "icons/icon.xpm" ).c_str() );
    if( !anaicon.isNull() )
      setIcon( anaicon );
  }

  drawContents();
}


Fusion2DWindow::~Fusion2DWindow()
{
  //cout << "~Fusion2DWindow\n";

  /** unregister observables now to avoid calling update() after the 
      destructor is called */
  set<AObject *>			obj = _obj;
  _obj.clear();
  set<AObject *>::const_iterator	io, fo = obj.end();
  cleanupObserver();

  delete pdat;
  // cout << "~Fusion2DWindow done\n";
}


void Fusion2DWindow::updateObjects()
{
  //cout << "Fusion2DWindow::updateObjects 1\n";
  if( pdat->recurs )
    return;

  //cout << "Fusion2DWindow::updateObjects 2\n";
  pdat->recurs = true;

  pdat->texpanel->updateObjects();
  pdat->recurs = false;
  //cout << "Fusion2DWindow::updateObjects done\n";
}


void Fusion2DWindow::update( const Observable* observable, void* arg )
{
  if( arg == this )
    return;

  set<AObject *>::iterator it;

  for( it=_obj.begin(); it!=_obj.end(); ++it )
    if( (*it) == (AObject*) observable )
      break;

  if( it==_obj.end() )
    {
      cerr << "Fusion2DWindow::update : unknown observable\n";
      return;
    }

  if( arg == 0 )
    {
      cout << "called obsolete Fusion2DWindow::update( obs, NULL )\n";
      const_cast<Observable *>(observable)->deleteObserver( this );
      delete this;
      return;
    }

  updateInterface();
}


void Fusion2DWindow::unregisterObservable( Observable* obs )
{
  // cout << "Fusion2DWindow::unregisterObservable\n";
  Observer::unregisterObservable( obs );
  AObject	*o = dynamic_cast<AObject *>( obs );
  if( !o )
    return;
  set<AObject *>::iterator	i = _obj.find( o );
  if( i != _obj.end() )
    {
      _obj.erase( o );
      pdat->objsel->updateLabel( _obj );
      updateInterface();
    }
}


namespace
{
  bool filterFusion2D( const AObject* o )
  {
    return ( o->type() == AObject::FUSION2D );
  }
}


void Fusion2DWindow::drawContents()
{
  QVBoxLayout	*mainlay = new QVBoxLayout( this, 5 );

  ObjectParamSelect	*hbtl = new ObjectParamSelect( _obj, this );
  pdat->objsel = hbtl;
  hbtl->addFilter( &filterFusion2D );
  mainlay->addWidget( hbtl );

  QHBox		*hb0 = new QHBox( this );
  mainlay->addWidget( hb0 );
  hb0->setSpacing( 5 );

  pdat->objbox = new QVGroupBox( tr( "Objects :" ), hb0 );
  pdat->orderbx = new QListWidget( pdat->objbox );
  QHBox	*hb1 = new QHBox( pdat->objbox );
  hb1->setSpacing( 5 );
  QPushButton	*up = new QPushButton( tr( "Up" ), hb1 );
  QPushButton	*dn = new QPushButton( tr( "Down" ), hb1 );
  if( _obj.size() != 1 )
    hb1->setEnabled( false );

  pdat->texpanel = new QTexturePanel( _obj, hb0 );
  pdat->texpanel->setVisibility( QTexturePanel::Generation, false );
  pdat->texpanel->setVisibility( QTexturePanel::Filtering, false );
  pdat->texpanel->setVisibility( QTexturePanel::Interpolation, false );

  selectSubObject( 1 );

  connect( pdat->orderbx, SIGNAL( itemSelectionChanged() ), this,
           SLOT( subObjectSelected() ) );
  connect( up, SIGNAL( clicked() ), this, SLOT( moveUpSubObject() ) );
  connect( dn, SIGNAL( clicked() ), this, SLOT( moveDownSubObject() ) );
  connect( hbtl, SIGNAL( selectionStarts() ), this, SLOT( chooseObject() ) );
  connect( hbtl, 
           SIGNAL( objectsSelected( const std::set<anatomist::AObject *> & ) ),
           this, 
           SLOT( objectsChosen( const std::set<anatomist::AObject *> & ) ) );
}


void Fusion2DWindow::updateInterface()
{
  if( pdat->recurs )
    return;

  pdat->recurs = true;
  pdat->orderbx->blockSignals( true );

  bool	hideobjbox = true;
  pdat->orderbx->clear();
  if( _obj.size() == 1 )
    {
      Fusion2D	*f = dynamic_cast<Fusion2D *>( *_obj.begin() );
      if( f )
        {
          Fusion2D::const_iterator	i, e = f->end();
          for( i=f->begin(); i!=e; ++i )
            pdat->orderbx->addItem( (*i)->name().c_str() );
          pdat->orderbx->item( pdat->currentsub )->setSelected( true );
          if( pdat->orderbx->count() > 1 )
            {
              pdat->objbox->setEnabled( true );
              hideobjbox = false;
            }
        }
    }
  if( hideobjbox )
    pdat->objbox->setEnabled( false );
  if( _obj.empty() )
    pdat->texpanel->setEnabled( false );
  else
    pdat->texpanel->setEnabled( true );

  pdat->orderbx->blockSignals( false );
  pdat->recurs = false;
}


void Fusion2DWindow::moveUpSubObject()
{
  Fusion2D	*f = currentObject();
  if( !f )
    return;
  QList<QListWidgetItem	*> items = pdat->orderbx->selectedItems();
  if( items.empty() )
    return;
  QListWidgetItem *item = items[0];
  int	i = pdat->orderbx->row( item );

  if( i == 0 )
    return;

  f->moveVolume( i, i-1 );
  selectSubObject( i-1 );

  vector<AObject *>	ord;
  ord.reserve( f->size() );
  Fusion2D::const_iterator	ii, ei = f->end();
  for( ii=f->begin(); ii!=ei; ++ii )
    ord.push_back( *ii );

  Fusion2DParamsCommand 
    *c = new Fusion2DParamsCommand( f, -1, -1, ord );
  theProcessor->execute( c );
  // updateObjects();
}


void Fusion2DWindow::moveDownSubObject()
{
  Fusion2D	*f = currentObject();
  if( !f )
    return;
  QList<QListWidgetItem *> items = pdat->orderbx->selectedItems();
  if( items.empty() )
    return;
  QListWidgetItem *item = items[0];
  int	i = pdat->orderbx->row( item );

  if( (unsigned) i + 1 >= f->size() )
    return;

  f->moveVolume( i, i+1 );
  selectSubObject( i+1 );

  vector<AObject *>	ord;
  ord.reserve( f->size() );
  Fusion2D::const_iterator	ii, ei = f->end();
  for( ii=f->begin(); ii!=ei; ++ii )
    ord.push_back( *ii );

  Fusion2DParamsCommand 
    *c = new Fusion2DParamsCommand( f, -1, -1, ord );
  theProcessor->execute( c );

  // updateObjects();
}


void Fusion2DWindow::selectSubObject( int x )
{
  pdat->currentsub = x;
  // cout << "subobject : " << x << endl;
  pdat->texpanel->setActiveTexture( x );
  if( x == 0 )
    {
      pdat->texpanel->setVisibility( QTexturePanel::Generation, true );
      pdat->texpanel->setVisibility( QTexturePanel::Filtering, true );
    }
  else
    {
      pdat->texpanel->setVisibility( QTexturePanel::Generation, false );
      pdat->texpanel->setVisibility( QTexturePanel::Filtering, false );
    }
  updateInterface();
}


void Fusion2DWindow::subObjectSelected()
{
  Fusion2D	*f = currentObject();
  if( !f )
    return;
  QList<QListWidgetItem *> items = pdat->orderbx->selectedItems();
  if( items.empty() )
    return;
  QListWidgetItem *item = items[0];
  int	x = pdat->orderbx->row( item );
  selectSubObject( x );
}


void Fusion2DWindow::chooseObject()
{
  // cout << "chooseObject\n";
  // filter out objects that don't exist anymore
  set<AObject *>::iterator	ir = pdat->initial.begin(), 
    er = pdat->initial.end(), ir2;
  while( ir!=er )
    if( theAnatomist->hasObject( *ir ) )
      ++ir;
    else
      {
        ir2 = ir;
        ++ir;
        pdat->initial.erase( ir2 );
      }

  pdat->objsel->selectObjects( pdat->initial, _obj );
}


void Fusion2DWindow::objectsChosen( const set<AObject *> & o )
{
  // cout << "objects chosen: " << o.size() << endl;

  set<AObject *>::const_iterator	i, e = _obj.end();
  while( !_obj.empty() )
    (*_obj.begin())->deleteObserver( this );
  _obj = o;
  for( i=_obj.begin(); i!=e; ++i )
    (*i)->addObserver( this );

  pdat->objsel->updateLabel( o );
  pdat->currentsub = 1;
  if( !_obj.empty() )
    {
      Fusion2D	*f = dynamic_cast<Fusion2D *>( *_obj.begin() );
      if( f )
        {
          pdat->texpanel->setObjects( _obj );
          pdat->texpanel->setActiveTexture( 1 );
        }
    }
  updateInterface();
}


Fusion2D *Fusion2DWindow::currentObject() const
{
  if( _obj.size() != 1 )
    return 0;
  return dynamic_cast<Fusion2D *>( *_obj.begin() );
}


AObject *Fusion2DWindow::currentSubOIbject() const
{
  Fusion2D	*f = currentObject();
  if( !f )
    return 0;
  Fusion2D::const_iterator	ii, e = f->end();
  int				i, n = pdat->currentsub;
  for( i=0, ii=f->begin(); i<n && ii!=e; ++i, ++ii )
    if( ii != e )
      return *ii;
  return 0;
}


