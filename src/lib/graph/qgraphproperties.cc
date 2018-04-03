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

#include <anatomist/graph/qgraphproperties.h>
#include <anatomist/graph/Graph.h>
#include <anatomist/object/objectparamselect.h>
#include <anatomist/application/settings.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/commands/cGraphDisplayProperties.h>
#include <anatomist/processor/Processor.h>
#include <anatomist/color/objectPalette.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qcombobox.h>
#include <qgroupbox.h>

using namespace anatomist;
using namespace std;

namespace
{

  bool filterGraphs( const AObject* o )
  {
    return dynamic_cast<const AGraph *>( o );
  }

}


struct QGraphProperties::Private
{
  Private( const set<AObject *> & obj )
    : objects( obj ), initial( obj ), modified( false ), updating( false ) {}

  set<AObject *>	objects;
  set<AObject *>	initial;
  ObjectParamSelect	*objsel;
  QComboBox		*modecombo;
  QComboBox		*propcombo;
  QWidget		*main;
  bool			modified;
  bool			updating;
  QCheckBox             *vertexmask;
  QCheckBox             *edgemask;
  QComboBox		*labelpropcombo;
  QLabel                *proplabel;
  QGroupBox             *maskgroup;
  QGroupBox             *labelpropgroup;
  QCheckBox             *keeppalette;
};


QGraphProperties::QGraphProperties( const set<AObject *> & obj,
                                    QWidget* parent )
  : QWidget( parent ),
    d( new Private( obj ) )
{
  setWindowTitle( "Graph display properties" );
  setObjectName("graphProperties");
  setAttribute(Qt::WA_DeleteOnClose);
  QPixmap	anaicon( Settings::findResourceFile(
                         "icons/icon.xpm" ).c_str() );
  if( !anaicon.isNull() )
    setWindowIcon( anaicon );

  QVBoxLayout	*mainLay = new QVBoxLayout( this );
  mainLay->setMargin( 10 );
  mainLay->setSpacing( 10 );
  d->objsel = new ObjectParamSelect( obj, this );
  mainLay->addWidget( d->objsel );
  d->objsel->addFilter( filterGraphs );

  d->main = new QWidget( this );
  mainLay->addWidget( d->main );
  QVBoxLayout *vlay = new QVBoxLayout( d->main );
  vlay->setSpacing( 10 );
  vlay->setMargin( 0 );

  QWidget *modeb = new QWidget( d->main );
  vlay->addWidget( modeb );
  QGridLayout *glay = new QGridLayout( modeb );
  glay->setSpacing( 10 );
  glay->setMargin( 0 );

  glay->addWidget( new QLabel( tr( "Display mode : " ), modeb ), 0, 0 );
  d->modecombo = new QComboBox( modeb );
  glay->addWidget( d->modecombo, 0, 1 );
  d->modecombo->addItem( tr( "Normal" ) );
  d->modecombo->addItem( tr( "Property map" ) );

  d->proplabel = new QLabel( tr( "Property to be mapped : " ), modeb );
  glay->addWidget( d->proplabel, 1, 0 );
  d->propcombo = new QComboBox( modeb );
  glay->addWidget( d->propcombo, 1, 1 );
  d->propcombo->setEditable( true );

  d->keeppalette = new QCheckBox( tr( "Keep palette absolute values" ),
    modeb );
  glay->addWidget( d->keeppalette, 2, 0 );
  glay->addWidget( new QWidget( modeb ), 2, 1 );

  d->maskgroup = new QGroupBox( tr( "Map property on :" ), modeb );
  glay->addWidget( d->maskgroup, 3, 0 );
  vlay = new QVBoxLayout( d->maskgroup );
  d->vertexmask = new QCheckBox( tr( "nodes" ), d->maskgroup );
  vlay->addWidget( d->vertexmask );
  d->edgemask = new QCheckBox( tr( "relations" ), d->maskgroup );
  vlay->addWidget( d->edgemask );
  vlay->addStretch( 1 );

  d->labelpropgroup
      = new QGroupBox( tr( "label / nomenclature property :" ), modeb );
  glay->addWidget( d->labelpropgroup, 3, 1 );
  vlay = new QVBoxLayout( d->labelpropgroup );
  d->labelpropcombo = new QComboBox( d->labelpropgroup );
  vlay->addWidget( d->labelpropcombo );
  d->labelpropcombo->addItem( tr( "default (as in settings)" ) );
  d->labelpropcombo->addItem( tr( "name (manual)" ) );
  d->labelpropcombo->addItem( tr( "label (automatic)" ) );
  vlay->addStretch( 1 );

  for( set<AObject *>::iterator it=obj.begin(); it!=obj.end(); ++it )
    (*it)->addObserver( this );

  updateInterface();

  connect( d->objsel, SIGNAL( selectionStarts() ), this,
           SLOT( chooseObject() ) );
  connect( d->objsel,
           SIGNAL( objectsSelected( const std::set<anatomist::AObject *> & ) ),
           this,
           SLOT( objectsChosen( const std::set<anatomist::AObject *> & ) ) );
  connect( d->modecombo, SIGNAL( textChanged( const QString & ) ), this,
           SLOT( modeChanged( const QString & ) ) );
  connect( d->modecombo, SIGNAL( activated( int ) ), this,
           SLOT( modeChanged( int ) ) );
  connect( d->propcombo, SIGNAL( textChanged( const QString & ) ), this,
           SLOT( propertyChanged( const QString & ) ) );
  connect( d->propcombo, SIGNAL( activated( int ) ), this,
           SLOT( propertyChanged( int ) ) );
  connect( d->vertexmask, SIGNAL( toggled( bool ) ), this,
           SLOT( vertexMaskChanged( bool ) ) );
  connect( d->edgemask, SIGNAL( toggled( bool ) ), this,
           SLOT( edgeMaskChanged( bool ) ) );
  connect( d->labelpropcombo, SIGNAL( activated( int ) ), this,
           SLOT( labelPropertyChanged( int ) ) );
}


QGraphProperties::~QGraphProperties()
{
  runCommand();
  cleanupObserver();
  delete d;
}


void QGraphProperties::update( const Observable*, void* arg )
{
  if( arg == 0 )
    {
      close();
      return;
    }
  if( arg != this && !d->updating )
    updateInterface();
}


void QGraphProperties::modeChanged( const QString & )
{
  d->modified = true;
  updateObjects();
}


void QGraphProperties::modeChanged( int )
{
  d->modified = true;
  updateObjects();
  updateInterface();
}


void QGraphProperties::propertyChanged( const QString & )
{
  d->modified = true;
  updateObjects();
}


void QGraphProperties::propertyChanged( int )
{
  d->modified = true;
  updateObjects();
}


void QGraphProperties::vertexMaskChanged( bool )
{
  d->modified = true;
  updateObjects();
}


void QGraphProperties::edgeMaskChanged( bool )
{
  d->modified = true;
  updateObjects();
}


void QGraphProperties::labelPropertyChanged( int )
{
  d->modified = true;
  updateObjects();
}


void QGraphProperties::updateInterface()
{
  if( d->updating )
    return;

  d->updating = true;

  set<AObject *>::const_iterator	io, eo = d->objects.end();
  AGraph				*g = 0;

  for( io=d->objects.begin(); io!=eo; ++io )
    {
      g = dynamic_cast<AGraph *>( *io );
      if( g )
        break;
    }
  if( !g )
    {
      d->main->setEnabled( false );
      d->updating = false;
      return;
    }
  d->main->setEnabled( true );

  d->modecombo->blockSignals( true );
  d->propcombo->blockSignals( true );
  d->labelpropcombo->blockSignals( true );

  d->modecombo->setCurrentIndex( (int) g->colorMode() );
  string prop;
  try
  {
    prop = g->graph()->getProperty( "label_property" )->getString();
  }
  catch( ... )
  {
  }
  if( prop.empty() )
    d->labelpropcombo->setCurrentIndex( 0 );
  else if( prop == "name" )
    d->labelpropcombo->setCurrentIndex( 1 );
  else if( prop == "label" )
    d->labelpropcombo->setCurrentIndex( 2 );

  // update properties list
  set<string> props;
  AGraph::iterator		ig, eg;
  const AttributedAObject	*aao;
  carto::Object			ob;

  for( io=d->objects.begin(); io!=eo; ++io )
    {
      g = (AGraph *) *io;
      for( ig=g->begin(), eg=g->end(); ig!=eg; ++ig )
        {
          aao = dynamic_cast<const AttributedAObject *>( *ig );
          if( aao )
            for( ob=aao->attributed()->objectIterator(); ob->isValid();
                 ob->next() )
              try
                {
                  ob->currentValue()->getScalar();
                  // if it hasn't failed, it's OK
                  props.insert( ob->key() );
                }
              catch( ... )
                {
                }
        }
    }
  d->propcombo->clear();
  set<string>::iterator	si, es = props.end();
  for( si=props.begin(); si!=es; ++si )
    d->propcombo->addItem( si->c_str() );

  d->propcombo->setItemText( d->propcombo->currentIndex(), 
                             g->colorProperty().c_str() );
  d->vertexmask->setChecked( g->colorPropertyMask() & 1 );
  d->edgemask->setChecked( g->colorPropertyMask() & 2 );

  if( g->colorMode() == AGraph::Normal )
  {
    d->proplabel->setEnabled( false );
    d->propcombo->setEnabled( false );
    d->maskgroup->setEnabled( false );
    d->labelpropgroup->setEnabled( true );
  }
  else
  {
    d->proplabel->setEnabled( true );
    d->propcombo->setEnabled( true );
    d->maskgroup->setEnabled( true );
    d->labelpropgroup->setEnabled( false );
  }

  d->objsel->updateLabel( d->objects );

  d->modecombo->blockSignals( false );
  d->propcombo->blockSignals( false );
  d->labelpropcombo->blockSignals( false );

  d->updating = false;
}


void QGraphProperties::unregisterObservable( Observable* obs )
{
  // cout << "unregisterObservable " << obs << endl;
  Observer::unregisterObservable( obs );
  AObject	*o = dynamic_cast<AObject *>( obs );
  if( !o )
    return;
  d->objects.erase( o );
  d->objsel->updateLabel( d->objects );
  updateInterface();
}


void QGraphProperties::updateObjects()
{
  set<AObject *>::const_iterator	io, fo = d->objects.end();
  AGraph				*g;
  AGraph::ColorMode			cm = AGraph::Normal;

  if( d->modecombo->currentText() == tr( "Property map" ) )
    cm = AGraph::PropertyMap;

  bool keeppalette = d->keeppalette->isChecked();
  float minp = 0, maxp = 0;

  for( io=d->objects.begin(); io!=fo; ++io )
    {
      g = static_cast<AGraph *>( *io );
      if( keeppalette )
      {
        AObjectPalette *pal = g->palette();
        const GLComponent::TexExtrema & te = g->glTexExtrema( 0 );
        minp = te.minquant[0];
        maxp = te.maxquant[0];
        float tspan = maxp - minp;
        if( tspan == 0. )
          tspan = 1.;
        maxp = pal->max1() * tspan + minp;
        minp = pal->min1() * tspan + minp;
      }
      g->setColorMode( cm, false );
      g->setColorProperty( d->propcombo->currentText().toStdString(), false );
      g->setColorPropertyMask( ( d->vertexmask->isChecked() & 1 )
          + 2 * ( d->edgemask->isChecked() & 1 ), false );
      Graph *gg = g->graph();
      string  prop;
      try
      {
        prop = gg->getProperty( "label_property" )->getString();
      }
      catch( ... )
      {
      }
      switch( d->labelpropcombo->currentIndex() )
      {
      case 1:
        if( prop != "name" )
        {
          gg->setProperty( "label_property", "name" );
          g->setChanged();
        }
        break;
      case 2:
        if( prop != "label" )
        {
          gg->setProperty( "label_property", "label" );
          g->setChanged();
        }
        break;
      default:
        if( !prop.empty() )
        {
          if( gg->hasProperty( "label_property" ) )
          gg->removeProperty( "label_property" );
          g->setChanged();
        }
        break;
      }
      g->updateExtrema();
      if( keeppalette )
      {
        AObjectPalette *pal = g->palette();
        const GLComponent::TexExtrema & te = g->glTexExtrema( 0 );
        float span = te.maxquant[0] - te.minquant[0];
        if( span == 0 )
          span = 1.;
        pal->setMin1( ( minp - te.minquant[0] ) / span );
        pal->setMax1( ( maxp - te.minquant[0] ) / span );
      }
      g->recolor();
      g->notifyObservers( this );
    }
}


void QGraphProperties::updateObjPal()
{
}


void QGraphProperties::chooseObject()
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

  d->objsel->selectObjects( d->initial, d->objects );
}


void QGraphProperties::objectsChosen( const set<AObject *> & o )
{
  // cout << "objects chosen: " << o.size() << endl;
  runCommand();

  while( !d->objects.empty() )
    (*d->objects.begin())->deleteObserver( this );

  set<AObject *>::const_iterator	i, e = d->objects.end();
  d->objects = o;
  for( i=d->objects.begin(); i!=e; ++i )
    (*i)->addObserver( this );

  updateInterface();
}


void QGraphProperties::runCommand()
{
  if( d->modified && !d->objects.empty() )
    {
      AGraph	*g = static_cast<AGraph *>( *d->objects.begin() );
      string	modes[] = { "Normal", "PropertyMap" };
      GraphDisplayPropertiesCommand	*com
        = new GraphDisplayPropertiesCommand( d->objects,
                                             modes[ g->colorMode() ],
                                             g->colorProperty() );

      // pb: unnecessary command execution: should be only writen, not executed
      // because it has already been done before.
      theProcessor->execute( com );
    }
}


void QGraphProperties::openProperties( const set<AObject *> & obj )
{
  QGraphProperties	*x = new QGraphProperties( obj, theAnatomist->getQWidgetAncestor() );
  x->setWindowFlags(Qt::Window);
  x->show();
}


