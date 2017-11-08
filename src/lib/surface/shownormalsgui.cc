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

#include <anatomist/surface/shownormalsgui.h>
#include <anatomist/object/objectparamselect.h>
#include <anatomist/application/Anatomist.h>

using namespace anatomist;
using namespace std;


NormalsSettingsPanel::NormalsObserver::NormalsObserver(
  NormalsSettingsPanel* normpanel )
  : Observer(), _normpanel( normpanel )
{
  set<AObject *>::iterator io, eo = _normpanel->_objects.end();
  for( io=_normpanel->_objects.begin(); io!=eo; ++io )
    (*io)->addObserver( this );
}


NormalsSettingsPanel::NormalsObserver::~NormalsObserver()
{
}

void NormalsSettingsPanel::NormalsObserver::registerObservable(
  Observable* obs )
{
  Observer::registerObservable( obs );
  AObject *obj = dynamic_cast<AObject *>( obs );
  if( obj )
    _normpanel->_objects.insert( obj );
}


void NormalsSettingsPanel::NormalsObserver::unregisterObservable(
  Observable* obs )
{
  AObject *obj = dynamic_cast<AObject *>( obs );
  if( obj )
  {
    set<AObject *>::iterator io = _normpanel->_objects.find( obj );
    if( io != _normpanel->_objects.end() )
      _normpanel->_objects.erase( io );
  }
  Observer::unregisterObservable( obs );
}


void NormalsSettingsPanel::NormalsObserver::update(
  const Observable* observable, void *args )
{
  _normpanel->update( observable, args );
}


// ----


namespace
{

  bool filterANormalMesh( const AObject * obj )
  {
    return dynamic_cast<const ANormalsMesh *>( obj ) != 0;
  }

}

NormalsSettingsPanel::NormalsSettingsPanel(
  const std::set<AObject *> & objects, QWidget* parent )
  : QWidget( parent ), Ui::ShowNormals(), Observer()
{
  setAttribute( Qt::WA_DeleteOnClose );

  setupUi( this );

  object_selection->setAttribute( Qt::WA_DeleteOnClose );
  object_selection->close();
  delete object_selection;

  _objectsel = new ObjectParamSelect( objects, this );
  _objectsel->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
  dynamic_cast<QBoxLayout *>( layout() )->insertWidget( 0, _objectsel );
  setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
  _objectsel->addFilter( filterANormalMesh );

  _objects = objects;
  _initial = _objects;
  _observer = new NormalsObserver( this );

  if( !_objects.empty() )
    length_spin->setValue(
      dynamic_cast<ANormalsMesh *>( *_objects.begin() )->length() );
  connect( _objectsel, SIGNAL( selectionStarts() ),
           this, SLOT( chooseObject() ) );
  connect( _objectsel,
           SIGNAL( objectsSelected( const std::set<anatomist::AObject *> &) ),
           this,
           SLOT( objectsChosen( const std::set<anatomist::AObject *> & ) ) );
  connect( length_spin, SIGNAL( valueChanged( double ) ),
           this, SLOT( setLength( double ) ) );
}


NormalsSettingsPanel::~NormalsSettingsPanel()
{
}


void NormalsSettingsPanel::chooseObject()
{
  set<AObject *>::iterator io = _initial.begin(), eo = _initial.end(), io2;
  while( io!=eo )
  {
    if( !theAnatomist->hasObject( *io ) )
    {
      if( io == _initial.begin() )
      {
        _initial.erase( io );
        io = _initial.begin();
      }
      else
      {
        io2 = io;
        --io;
        _initial.erase( io2 );
        ++io;
      }
    }
    else
      ++io;
  }

  _objectsel->selectObjects( _initial, _objects );
}


void NormalsSettingsPanel::update( const Observable* observable, void* args )
{
  const ANormalsMesh *obj = dynamic_cast<const ANormalsMesh *>( observable );
  if( obj && obj == *_objects.begin() )
  {
    float val = dynamic_cast<ANormalsMesh *>( *_objects.begin() )->length();
    if( val != length_spin->value() )
      length_spin->setValue( val );
  }
}


void NormalsSettingsPanel::setLength( double value )
{
  blockSignals( true );
  set<AObject *>::iterator io, eo = _objects.end();
  for( io=_objects.begin(); io!=eo; ++io )
  {
    dynamic_cast<ANormalsMesh *>( *io )->setLength( float( value ) );
    (*io)->notifyObservers( this );
  }
  blockSignals( false );
}


void NormalsSettingsPanel::objectsChosen( const std::set<AObject *> & objects )
{
  set<AObject *>::iterator io, eo = _objects.end();
  for( io=_objects.begin(); io!=eo; ++io )
    (*io)->deleteObserver( _observer );
  _objects = objects;
  for( io=_objects.begin(); io!=eo; ++io )
    (*io)->addObserver( _observer );

  if( !_objects.empty() )
    update( *_objects.begin(), this );
}


