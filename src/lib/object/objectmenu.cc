/* Copyright (c) 2006 CEA
 *
 *  This software and supporting documentation were developed by
 *      CEA/DSV/SHFJ
 *      4 place du General Leclerc
 *      91401 Orsay cedex
 *      France
 *
 * This software is governed by the CeCILL license version 2 under
 * French law and abiding by the rules of distribution of free software.
 * You can  use, modify and/or redistribute the software under the
 * terms of the CeCILL license version 2 as circulated by CEA, CNRS
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
 * knowledge of the CeCILL license version 2 and that you accept its terms.
 */

#include <anatomist/object/objectmenu.h>
#include <graph/tree/tree.h>



using namespace anatomist;
using namespace carto;
using namespace std;

ObjectMenuCallback::ObjectMenuCallback()
  : RCObject()
{
}


ObjectMenuCallback::~ObjectMenuCallback()
{
}


bool ObjectMenuCallback::operator == ( const ObjectMenuCallback & x ) const
{
  return typeid( x ) == typeid( *this );
}

// ----------------------------------------

ObjectMenuCallbackFunc::ObjectMenuCallbackFunc( CallbackFunc cbk )
: ObjectMenuCallback(), _func( cbk )
{
}


ObjectMenuCallbackFunc::~ObjectMenuCallbackFunc()
{
}


bool ObjectMenuCallbackFunc::operator == ( const ObjectMenuCallback & x ) const
{
  const ObjectMenuCallbackFunc
    *y = dynamic_cast<const ObjectMenuCallbackFunc *>( &x );
  if( y )
    return y->_func == _func;
  return ObjectMenuCallback::operator == ( x );
}


void ObjectMenuCallbackFunc::doit ( const set<AObject *> & obj )
{
  _func( obj );
}

// ----------------------------------------

struct ObjectMenu::Private
{
  Private();
  ~Private();

  Private & operator = ( const Private & x );

  Tree  *menu;
};


ObjectMenu::Private::Private()
  : menu( 0 )
{
}


ObjectMenu::Private::~Private()
{
  delete menu;
}


ObjectMenu::Private &
ObjectMenu::Private::operator = ( const ObjectMenu::Private & x )
{
  if( this != &x )
  {
    // TODO
  }
  return *this;
}

// -----------------------------------------

ObjectMenu::ObjectMenu()
  : d( new Private )
{
}


ObjectMenu::ObjectMenu( const ObjectMenu & x )
  : d( new Private )
{
  *this = x;
}


ObjectMenu::ObjectMenu( Tree & t )
  : d( new Private )
{
	d->menu = &t;
}

ObjectMenu::~ObjectMenu()
{
  delete d;
}


ObjectMenu & ObjectMenu::operator = ( const ObjectMenu & x )
{
  if( this != &x )
  {
    *d = *x.d;
  }
  return *this;
}


void ObjectMenu::insertItem(const vector<string> & inside,
                            const string & text )
{
  if( !d->menu )
    d->menu = new Tree( true, "option tree" );
  Tree  *t = item( inside );
  if( !t )
    return;

  t->insert( new Tree( true, text ) );
}


void ObjectMenu::insertItem( const vector<string> & inside,
                             const string & text, ObjectMenuCallback* cbk )
{
  if( !d->menu )
    d->menu = new Tree( true, "option tree" );
  Tree  *t = item( inside );
  if( !t )
  {
    delete cbk;
    return;
  }

  Tree  *ti = new Tree( true, text );
  ti->setProperty( "objectmenucallback", rc_ptr<ObjectMenuCallback>( cbk ) );
  t->insert( ti );
}


void ObjectMenu::insertItem( const vector<string> & inside,
                             const string & text,
                             ObjectMenuCallbackFunc::CallbackFunc cbk )
{
  insertItem( inside, text, new ObjectMenuCallbackFunc( cbk ) );
}


Tree* ObjectMenu::tree()
{
  return d->menu;
}

const Tree* ObjectMenu::tree() const
{
  return d->menu;
}

Tree* ObjectMenu::releaseTree()
{
  Tree  *t = d->menu;
  d->menu = 0;
  return t;
}


Tree* ObjectMenu::item( const vector<string> & pos, bool create )
{
  if( !d->menu )
    return 0;

  vector<string>::const_iterator  ip, ep = pos.end();
  Tree::const_iterator            i, e;
  Tree                            *cur = d->menu, *st;

  for( ip=pos.begin(); ip!=ep; ++ip )
  {
    for( i=cur->begin(), e=cur->end(); i!=e; ++i )
    {
      st = static_cast<Tree *>( *i );
      if( st->getSyntax() == *ip )
      {
        cur = st;
        break;
      }
    }
    if( i == e )
    {
      if( !create )
        return 0; // not found
      st = new Tree( true, *ip );
      cur->insert( st );
      cur = st;
    }
  }
  if( ip == ep )
    return cur; // found

  // not found
  return 0;
}

// ---

ObjectMenuRegistrerClass::ObjectMenuRegistrerClass()
{
}


ObjectMenuRegistrerClass::~ObjectMenuRegistrerClass()
{
}


ObjectMenuRegistrerFuncClass::ObjectMenuRegistrerFuncClass
( ObjectMenuRegistrerFuncClass::RegistrerFunc f )
  : ObjectMenuRegistrerClass(), _f( f )
{
}


ObjectMenuRegistrerFuncClass::~ObjectMenuRegistrerFuncClass()
{
}


ObjectMenu* ObjectMenuRegistrerFuncClass::doit( const AObject* otype,
                                                ObjectMenu* om )
{
  return _f( otype, om );
}


#include <cartobase/object/object_d.h>
INSTANTIATE_GENERIC_OBJECT_TYPE(  rc_ptr<ObjectMenuCallback> )

