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

#include <anatomist/surface/wvectorfield.h>

#include <qlayout.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qtabbar.h>
#include <qradiobutton.h>
#include <qpixmap.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/surface/vectorfield.h>
#include <anatomist/object/objectparamselect.h>
#include <anatomist/application/settings.h>
#include <anatomist/application/globalConfig.h>

using namespace anatomist;
using namespace std;


struct VectorFieldEditionWindow::Private
{
  Private( const set<AObject *> & o ) : operating( false ), initial( o ),
    modified( false ), coords( 3 )
  {}
  bool			operating;
  set<AObject *>	initial;
  ObjectParamSelect	*objsel;
  bool			modified;
  vector<vector<QSpinBox *> > coords;
};


anatomist::internal::VectorFieldCoordSpinBox::~VectorFieldCoordSpinBox()
{
}


// VectorFieldEditionWindow class


VectorFieldEditionWindow::VectorFieldEditionWindow( const set<AObject *> &objL,
                                                    QWidget* parent,
                                                    const char *name,
                                                    Qt::WindowFlags f )
  : QWidget( parent, f ), _parents( objL ),
    d( new Private( objL ) )
{

  setObjectName(name);
  setAttribute(Qt::WA_DeleteOnClose);

  QWidget *uiwid = new QWidget( this );
  setupUi( uiwid );

  for (set<AObject *>::iterator it=_parents.begin();
       it!=_parents.end();++it)
    (*it)->addObserver( (Observer*)this );

  setWindowTitle( name );
  if( windowFlags() & Qt::Window )
  {
    QPixmap anaicon( Settings::findResourceFile( "icons/icon.xpm"
      ).c_str() );
    if( !anaicon.isNull() )
      setWindowIcon( anaicon );
  }

  ObjectParamSelect *sel
    = new ObjectParamSelect( _parents, this );
  d->objsel = sel;

  QVBoxLayout	*vboxlayout = new QVBoxLayout( this );
  vboxlayout->setSpacing( 0 );
  vboxlayout->setMargin( 0 );
  vboxlayout->addWidget( sel );
  vboxlayout->addWidget( uiwid );

  delete x_coord0;
  delete y_coord0;
  delete z_coord0;
  x_coord0 = 0;
  y_coord0 = 0;
  z_coord0 = 0;

  scale_lineedit->setValidator( new QDoubleValidator );

  updateInterface();

  // connections
  connect( sel, SIGNAL( selectionStarts() ), this, SLOT( chooseObject() ) );
  connect( sel,
           SIGNAL( objectsSelected( const std::set<anatomist::AObject *> & ) ),
           this,
           SLOT( objectsChosen( const std::set<anatomist::AObject *> & ) ) );

  connect( scale_lineedit, SIGNAL( editingFinished() ),
           this, SLOT( scalingChanged() ) );
  connect( x_vol, SIGNAL( activated( int ) ),
           this, SLOT( xVolumeChanged( int ) ) );
  connect( x_map0, SIGNAL( activated( int ) ),
           this, SLOT( xSpace0Changed( int ) ) );
  connect( x_map1, SIGNAL( activated( int ) ),
           this, SLOT( xSpace1Changed( int ) ) );
  connect( x_map2, SIGNAL( activated( int ) ),
           this, SLOT( xSpace2Changed( int ) ) );

  connect( y_vol, SIGNAL( activated( int ) ),
           this, SLOT( yVolumeChanged( int ) ) );
  connect( y_map0, SIGNAL( activated( int ) ),
           this, SLOT( ySpace0Changed( int ) ) );
  connect( y_map1, SIGNAL( activated( int ) ),
           this, SLOT( ySpace1Changed( int ) ) );
  connect( y_map2, SIGNAL( activated( int ) ),
           this, SLOT( ySpace2Changed( int ) ) );

  connect( z_vol, SIGNAL( activated( int ) ),
           this, SLOT( zVolumeChanged( int ) ) );
  connect( z_map0, SIGNAL( activated( int ) ),
           this, SLOT( zSpace0Changed( int ) ) );
  connect( z_map1, SIGNAL( activated( int ) ),
           this, SLOT( zSpace1Changed( int ) ) );
  connect( z_map2, SIGNAL( activated( int ) ),
           this, SLOT( zSpace2Changed( int ) ) );
}


VectorFieldEditionWindow::~VectorFieldEditionWindow()
{
  d->operating = true;

  cleanupObserver();

  delete d;
}


void VectorFieldEditionWindow::update( const Observable*, void* arg )
{
  if( arg != this && !d->operating )
    updateInterface();
}


void VectorFieldEditionWindow::unregisterObservable( Observable* obs )
{
  Observer::unregisterObservable( obs );
  AObject	*o = dynamic_cast<AObject *>( obs );
  if( !o )
    return;
  _parents.erase( o );
  d->objsel->updateLabel( _parents );
  updateInterface();
}


void VectorFieldEditionWindow::updateInterface()
{
  if( d->operating )
    return;

  d->operating = true;

  blockSignals( true );

  if( !_parents.empty() )
  {
    anatomist::VectorField* vf
      = static_cast<anatomist::VectorField *>( *_parents.begin() );

    scale_lineedit->setText( QString::number( vf->scaling() ) );
    x_vol->clear();
    y_vol->clear();
    z_vol->clear();
    x_vol->addItem( "<none>" );
    y_vol->addItem( "<none>" );
    z_vol->addItem( "<none>" );

    MObject::iterator io, eo = vf->end();
    --eo; // skip last one
    for( io=vf->begin(); io!=eo; ++io )
    {
      x_vol->addItem( (*io)->name().c_str() );
      y_vol->addItem( (*io)->name().c_str() );
      z_vol->addItem( (*io)->name().c_str() );
    }

    vector<QComboBox *> i_vol( 3 );
    i_vol[0] = x_vol;
    i_vol[1] = y_vol;
    i_vol[2] = z_vol;
    vector<vector<QComboBox *> > i_map( 3, vector<QComboBox *>( 3 ) );
    i_map[0][0] = x_map0;
    i_map[0][1] = x_map1;
    i_map[0][2] = x_map2;
    i_map[1][0] = y_map0;
    i_map[1][1] = y_map1;
    i_map[1][2] = y_map2;
    i_map[2][0] = z_map0;
    i_map[2][1] = z_map1;
    i_map[2][2] = z_map2;
    vector<QHBoxLayout *> coord_lay( 3 );
    coord_lay[0] = x_coord_layout;
    coord_lay[1] = y_coord_layout;
    coord_lay[2] = z_coord_layout;

    int chan, i, n, m;
    QSpinBox* sb;

    for( chan=0; chan<3; ++chan )
    {
      AObject *vol = vf->volume( chan );
      int cind = 0;
      if( vol )
        cind = x_vol->findText( vol->name().c_str() );
      i_vol[chan]->setCurrentIndex( cind );
      i_map[chan][0]->clear();
      i_map[chan][1]->clear();
      i_map[chan][2]->clear();
      bool enabled = (bool) vol;
      i_map[chan][0]->setEnabled( enabled );
      i_map[chan][1]->setEnabled( enabled );
      i_map[chan][2]->setEnabled( enabled );

      if( vol )
      {
        vector<int> dims = vf->volumeSize( chan );
        for( i=0, n=dims.size(); i<n; ++i )
        {
          i_map[chan][0]->addItem( QString::number( i ) );
          i_map[chan][1]->addItem( QString::number( i ) );
          i_map[chan][2]->addItem( QString::number( i ) );
        }
        Point3di mapcoord = vf->spaceCoordsDimensions( chan );
        i_map[chan][0]->setCurrentIndex( mapcoord[0] );
        i_map[chan][1]->setCurrentIndex( mapcoord[1] );
        i_map[chan][2]->setCurrentIndex( mapcoord[2] );

        for( i=n, m=d->coords[chan].size(); i<m; ++i )
          delete d->coords[chan][i];
        d->coords[chan].resize( min( n, m ) );
        for( i=m; i<n; ++i )
        {
          sb = new anatomist::internal::VectorFieldCoordSpinBox( chan, i );
          d->coords[chan].push_back( sb );
          coord_lay[chan]->addWidget( sb );
          connect( sb, SIGNAL( valueChanged( int, int, int ) ),
                   this, SLOT( setFixedCoord( int, int, int ) ) );
        }

        vector<int> pos = vf->vectorChannelPosition( chan );
        for( i=0; i<n; ++i )
        {
          sb = d->coords[chan][i];
          if( mapcoord[0] == i || mapcoord[1] == i || mapcoord[2] == i )
          {
            sb->setValue( 0 );
            sb->setEnabled( false );
          }
          else
          {
            sb->setRange( 0, dims[i] - 1 );
            if( i < pos.size() )
              sb->setValue( pos[i] );
            else
              sb->setValue( 0 );
            sb->setEnabled( true );
          }
        }
      }
      else
      {
        for( i=0, n=d->coords[chan].size(); i<n; ++i )
          delete d->coords[chan][i];
        d->coords[chan].clear();
      }
    }
  }

  blockSignals( false );
  d->operating = false;
}


void VectorFieldEditionWindow::chooseObject()
{
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
  set<AObject *> possible;
  set<AObject *> obj = theAnatomist->getObjects();
  set<AObject *>::const_iterator io, eo=obj.end();
  for( io=obj.begin(); io!=eo; ++io )
    if( dynamic_cast<VectorField *>( *io ) )
      possible.insert( *io );

  d->objsel->selectObjects( d->initial, possible );
}


anatomist::VectorField* VectorFieldEditionWindow::vectorField() const
{
  return static_cast<anatomist::VectorField *>( *_parents.begin() );
}


void VectorFieldEditionWindow::objectsChosen( const set<AObject *> & o )
{
  d->operating = true;

  set<AObject *>::const_iterator	i, e = o.end();
  while( !_parents.empty() )
    (*_parents.begin())->deleteObserver( this );
  _parents = o;
  for( i=o.begin(); i!=e; ++i )
  {
    (*i)->addObserver( this );
    break; // only one
  }

  d->objsel->updateLabel( _parents );
  d->operating = false;
  updateInterface();
}


void VectorFieldEditionWindow::scalingChanged()
{
  anatomist::VectorField* vf = vectorField();
  float value = scale_lineedit->text().toFloat();
  vf->setScaling( value );
  vf->notifyObservers( this );
}


void VectorFieldEditionWindow::xVolumeChanged( int index )
{
  setVolume( 0, index );
}


void VectorFieldEditionWindow::yVolumeChanged( int index )
{
  setVolume( 1, index );
}


void VectorFieldEditionWindow::zVolumeChanged( int index )
{
  setVolume( 2, index );
}


void VectorFieldEditionWindow::setVolume( int chan, int index )
{
  anatomist::VectorField* vf = vectorField();
  if( index == 0 )
    vf->setVolume( chan, 0 );
  else
  {
    MObject::iterator io, eo = vf->end();
    int i;
    for( i=1, io=vf->begin(); i<index; ++i, ++io )
    {}

    vf->setVolume( chan, *io );
  }
  vf->notifyObservers( this );
}


void VectorFieldEditionWindow::xSpace0Changed( int index )
{
  setSpaceDim( 0, 0, index );
}


void VectorFieldEditionWindow::xSpace1Changed( int index )
{
  setSpaceDim( 0, 1, index );
}


void VectorFieldEditionWindow::xSpace2Changed( int index )
{
  setSpaceDim( 0, 2, index );
}


void VectorFieldEditionWindow::ySpace0Changed( int index )
{
  setSpaceDim( 1, 0, index );
}


void VectorFieldEditionWindow::ySpace1Changed( int index )
{
  setSpaceDim( 1, 1, index );
}


void VectorFieldEditionWindow::ySpace2Changed( int index )
{
  setSpaceDim( 1, 2, index );
}


void VectorFieldEditionWindow::zSpace0Changed( int index )
{
  setSpaceDim( 2, 0, index );
}


void VectorFieldEditionWindow::zSpace1Changed( int index )
{
  setSpaceDim( 2, 1, index );
}


void VectorFieldEditionWindow::zSpace2Changed( int index )
{
  setSpaceDim( 2, 2, index );
}


void VectorFieldEditionWindow::setSpaceDim( int chan, int dim, int index )
{
  anatomist::VectorField* vf = vectorField();
  if( !vf->volume( chan ) )
    return;
  Point3di p;
  switch( chan )
  {
  case 0:
    p[0] = x_map0->currentIndex();
    p[1] = x_map1->currentIndex();
    p[2] = x_map2->currentIndex();
    break;
  case 1:
    p[0] = y_map0->currentIndex();
    p[1] = y_map1->currentIndex();
    p[2] = y_map2->currentIndex();
    break;
  case 2:
    p[0] = z_map0->currentIndex();
    p[1] = z_map1->currentIndex();
    p[2] = z_map2->currentIndex();
    break;
  }
  vf->setSpaceCoordsDimensions( chan, p );
  vf->notifyObservers( this );
}


void VectorFieldEditionWindow::setFixedCoord( int chan, int coord, int value )
{
  anatomist::VectorField* vf = vectorField();
  if( !vf->volume( chan ) )
    return;
  size_t i, n = d->coords[chan].size();
  vector<int> pos( n );
  for( i=0; i<n; ++i )
    pos[i] = d->coords[chan][i]->value();
  vf->setVectorChannelPosition( chan, pos );
  vf->notifyObservers();
}


