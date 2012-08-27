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


#include <anatomist/mobject/ObjectList.h>

#include <anatomist/application/Anatomist.h>
#include <anatomist/object/actions.h>
#include <graph/tree/tree.h>


using namespace anatomist;
using namespace carto;
using namespace std;


Tree*	ObjectList::_optionTree = 0;


ObjectListIterator::ObjectListIterator( const 
			ObjectListIterator::datatype::iterator & x )
  : _dataIt( x )
{
}


const_ObjectListIterator::const_ObjectListIterator( const 
			ObjectListIterator::datatype::const_iterator & x )
  : _dataIt( x )
{
}


ObjectListIterator::~ObjectListIterator()
{
}


const_ObjectListIterator::~const_ObjectListIterator()
{
}


ObjectList::ObjectList() : MObject()
{
  _type = AObject::LIST;
  //_objMenu = new ObjectMenu( "Liste", 1 );
  //_optionMenu = new list<OptionMenu>;
  //InitOptionMenu();
}


ObjectList::~ObjectList()
{
  cleanup();
  datatype::iterator	it;

  for( it=_data.begin(); it!=_data.end(); ++it )
    {
      (*it)->UnregisterParent( this );
      (*it)->deleteObserver((Observer*)this);
      if( !(*it)->Visible() ) 
	theAnatomist->mapObject( it->get() );
    }
  _data.erase( _data.begin(), _data.end() );
}


MObject::const_iterator ObjectList::find( const AObject *obj ) const
{
  shared_ptr<AObject> sob = shared_ptr<AObject>( shared_ptr<AObject>::Weak,
      const_cast<AObject *>( obj ) );
  set<shared_ptr<AObject> >::const_iterator i;

  i = _data.find( sob );
  if( i!=_data.end() ) return iterator( new const_ObjectListIterator( i ) );
  else return end();
}


Tree* ObjectList::optionTree() const
{
  if( !_optionTree )
    {
      Tree	*t, *t2;
      _optionTree = new Tree( true, "option tree" );
      t = new Tree( true, "Color" );
      _optionTree->insert( t );
      t2 = new Tree( true, "Palette" );
      t2->setProperty( "callback", &ObjectActions::colorPalette );
      t->insert( t2 );
      t2 = new Tree( true, "Material" );
      t2->setProperty( "callback", &ObjectActions::colorMaterial );
      t->insert( t2 );
      t2 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "Rendering" ) );
      t2->setProperty( "callback", &ObjectActions::colorRendering);
      t->insert( t2 );
      t = new Tree( true, "Referential" );
      _optionTree->insert( t );
      t2 = new Tree( true, "Load" );
      t2->setProperty( "callback", &ObjectActions::referentialLoad );
      t->insert( t2 );
    }
  return( _optionTree );
}


bool ObjectList::CanRemove( AObject * obj )
{
  datatype::iterator  it
      = _data.find( shared_ptr<AObject>( shared_ptr<AObject>::Weak, obj ) );
  if( it == _data.end() )
    return true;  // not mine: so I don't care if you remove it...
  return it->referenceType() != shared_ptr<AObject>::Strong;
}


