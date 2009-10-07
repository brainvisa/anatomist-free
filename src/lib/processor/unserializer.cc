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


#include <anatomist/processor/unserializer.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/reference/transfSet.h>
#include <iostream>

using namespace anatomist;
using namespace std;


Unserializer::Unserializer()
{
}


Unserializer::~Unserializer()
{
}


void Unserializer::registerPointer( void* ptr, int id, const string & type )
{
  //cout << "Unserializer::registerPointer : " << ptr << " -> " << id << endl;
  if( id < 0 )
    id = freeID();
  _id2ptr[id] = ptr;
  if( ptr )
    _ptr2type[ptr] = type;
}


void* Unserializer::pointer( int id ) const
{
  map<int, void*>::const_iterator	i = _id2ptr.find( id );
  if( i == _id2ptr.end() )
    return( 0 );
  return( i->second );
}


void* Unserializer::pointer( int id, const string & typecheck ) const
{
  void	*ptr = pointer( id );
  if( !ptr )
    return( 0 );
  if( type( ptr ) != typecheck )
    {
      cerr << "Id " << id << " doesn't correspond to a " << typecheck << endl;
      return( 0 );
    }
  return( ptr );
}


string Unserializer::type( void* ptr ) const
{
  map<void *, string>::const_iterator	i = _ptr2type.find( ptr );
  if( i == _ptr2type.end() )
    return( "" );
  return( i->second );
}


void Unserializer::unregister( int id )
{
  map<int, void*>::iterator	i = _id2ptr.find( id );
  if( i == _id2ptr.end() )
    return;
  _ptr2type.erase( i->second );
  _id2ptr.erase( i );
}


void Unserializer::garbageCollect()
{
  map<int, void*>::iterator	i, e = _id2ptr.end(), i2;
  map<void *, string>::iterator	j, f = _ptr2type.end(), j2;
  set<void *>			done;
  set<void *>::iterator		garbage = done.end();
  void				*ptr;
  bool				ok;
  set<Referential *>		refs = theAnatomist->getReferentials();
  set<Referential *>::iterator	er = refs.end();

  // 1st pass: cleanup _id2ptr map
  i = _id2ptr.begin();
  while( i != e )
    {
      ok = true;
      ptr = i->second;
      if( ptr ) // null pointer is used to reserve unassigned IDs
        {
          j = _ptr2type.find( ptr );
          if( j == f )	// ptr with no type
            {
              cerr << "warning: deleting ID of pointer with no type: ID: " 
                   << i->first << ", ptr: " << ptr << endl;
              i2 = i;
              ++i;
              _id2ptr.erase( i2 );
              ok = false;
            }
          else
            {
              string	& t = j->second;
              if( t == "AObject" )
                {
                  if( !theAnatomist->hasObject( (AObject *) ptr ) )
                    ok = false;
                }
              else if( t == "AWindow" )
                {
                  if( !theAnatomist->hasWindow( (AWindow *) ptr ) )
                    ok = false;
                }
              else if( t == "Referential" )
                {
                  if( refs.find( (Referential *) ptr ) == er )
                    ok = false;
                }
              else if( t == "Transformation" )
                {
                  if( !ATransformSet::instance()
                      ->hasTransformation( (Transformation *) ptr ) )
                    ok = false;
                }
              if( !ok )
                {
                  _ptr2type.erase( j );
                  i2 = i;
                  ++i;
                  _id2ptr.erase( i2 );
                }
            }
        }
      if( ok )
        {
          done.insert( ptr );
          ++i;
        }
    }

  // 2nd pass: cleanup _ptr2type map
  j = _ptr2type.begin();
  while( j != f )
    if( done.find( j->first ) == garbage )
      {
	cerr << "pointer with no ID, deleting, type: " << j->second << endl;
	j2 = j;
	++j;
	_ptr2type.erase( j2 );
      }
    else
      ++j;
}


int Unserializer::makeID( void* ptr, const std::string & type )
{
  int				id = freeID();
  registerPointer( ptr, id, type );
  return id;
}


int Unserializer::freeID() const
{
  map<int, void*>::const_iterator	ii, ei = _id2ptr.end();
  int					id = 0;
  for( ii=_id2ptr.begin(); ii!=ei && ii->first==id; ++ii, ++id ) {}
  return id;
}


int Unserializer::id( void* ptr, const string & t ) const
{
  if( !t.empty() && type( ptr ) != t )
    throw runtime_error
      ( string( "mismatching type of object: " + t 
                + " while expecting " + type( ptr ) ).c_str() );
  map<int, void *>::const_iterator	i, e = _id2ptr.end();
  for( i=_id2ptr.begin(); i!=e && i->second != ptr; ++i ) {}
  if( i != e )
    return i->first;
  throw runtime_error( "ID of object not found" );
}


