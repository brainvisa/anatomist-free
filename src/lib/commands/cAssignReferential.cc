/* Copyright (c) 1995-2005 CEA
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

#include <anatomist/commands/cAssignReferential.h>
#include <anatomist/object/Object.h>
#include <anatomist/window/Window.h>
#include <anatomist/reference/Referential.h>
#include <anatomist/reference/Geometry.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/processor/Serializer.h>
#include <anatomist/processor/unserializer.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/processor/context.h>
#include <anatomist/object/actions.h>
#include <anatomist/graph/pythonAObject.h>
#include <cartobase/object/syntax.h>
#include <graph/tree/tree.h>

using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;


AssignReferentialCommand::AssignReferentialCommand( Referential* ref, 
						    const set<AObject *> & o, 
						    const set<AWindow *> & w, 
						    int refId, 
						    CommandContext* context,
                                                    const string & filename,
                                                    const string & uuid )
  : RegularCommand(), SerializingCommand( context ), _id( refId ), _obj( o ), 
    _win( w ), _ref( ref ), _filename( filename ), _uuid( uuid )
{
}


AssignReferentialCommand::~AssignReferentialCommand()
{
}


bool AssignReferentialCommand::initSyntax()
{
  SyntaxSet	ss;
  Syntax	& s = ss[ "AssignReferential" ];
  
  s[ "ref_id"      ] = Semantic( "int", true );
  s[ "objects"     ] = Semantic( "int_vector", false );
  s[ "windows"     ] = Semantic( "int_vector", false );
  s[ "central_ref" ] = Semantic( "int", false );
  s[ "filename"    ] = Semantic( "string", false );
  s[ "ref_uuid" ] = Semantic( "string", false );
  Registry::instance()->add( "AssignReferential", &read, ss );
  return( true );
}


void AssignReferentialCommand::doit()
{
  // cout << "AssignReferentialCommand::doit\n";
  Referential	*ref = 0;

  PythonHeader  ph;
  bool          minfok = false;
  string        uuid;

  if( !_filename.empty() )
  {
    minfok = ph.readMinf( _filename );
    if( ph.getProperty( "uuid", uuid ) )
      ref = Referential::referentialOfUUID( uuid );
  }
  if( !_uuid.empty() )
  {
    if( ref )
    {
      if( uuid != _uuid )
        cerr << "warning: both UUID and filename specified - "
            "conflicting UUID\n";
    }
    else
      ref = Referential::referentialOfUUID( _uuid );
  }

  if( _ref )	// existing referential
  {
    if( ref && ref != _ref )
    {
      cerr << "mismatch between existing referential UUID and loaded one"
           << endl;
      return;
    }
    ref = _ref;
  }
  else if( _id != 0 )
  {
    if( !ref )
      ref = new Referential;
  }
  else
    return;

  if( ref )
    _ref = ref;

  if( minfok )
  {
    _ref->header() = ph;
    if( _uuid.empty() )
      _uuid = uuid;
  }
  if( !_uuid.empty() )
    _ref->header().setProperty( "uuid", _uuid );
  // TODO: update refs uuids in transformations from/to _ref

  if( _id > 0 && context() && context()->unserial )
    context()->unserial->registerPointer( ref, _id, "Referential" );
  set<AWindow *>::const_iterator	iw, fw=_win.end();
  set<AObject *>::const_iterator	io, fo=_obj.end();
  vector<string> refs;
  vector<string>::const_iterator ir, er;

  if( ref && uuid.empty() )
    uuid = ref->uuid().toString();

  if( !_obj.empty() )
  {
    // keep state of existing referentials
    set<Referential *> usedrefs = theAnatomist->getReferentials();
    if( ref )
      usedrefs.insert( ref );
    ObjectActions::setAutomaticReferential( _obj );

    for( io=_obj.begin(); io!=fo; ++io )
      if( theAnatomist->hasObject( *io ) )
      {
        if( ref )
        {
          /* determine whether we handle directly the internal AIMS
            referential, or one of the destination referentials of the
            "referentials" property of the object */
          bool dstref = false;
          if( Referential::referentialOfNameOrUUID( *io, uuid ) )
            dstref = true;
          /* in any other case, set the ref as the default AIMS ref of the
            object */
          if( !dstref )
            (*io)->setReferential( ref );
        }
        else
          (*io)->setReferentialInheritance
            ( (*io)->fallbackReferentialInheritance() );
        // cout << "AssignReferentialCommand notify\n";
        (*io)->notifyObservers( this );
      }
    // clean referentials created by setAutomaticReferential() but not needed
    set<Referential *> currentrefs = theAnatomist->getReferentials();
    set<Referential *>::iterator
        iref, eref = currentrefs.end(), unused = usedrefs.end();
    for( iref=currentrefs.begin(); iref!=eref; ++iref )
      if( usedrefs.find( *iref ) == unused )
        delete *iref;
  }

  // now set the ref on windows
  for( iw=_win.begin(); iw!=fw; ++iw )
    if( theAnatomist->hasWindow( *iw ) )
    {
      (*iw)->setReferential( ref );
      (*iw)->updateWindowGeometry();
    }
  // cout << "assignref done\n";
}


Command* AssignReferentialCommand::read( const Tree & com, 
					 CommandContext* context )
{
  int			id;
  set<AObject *>	obj;
  set<AWindow *>	win;
  vector<int>		objId;
  vector<int>		winId;
  unsigned		i, n;
  void			*ptr = 0;
  string                fname, uuid;

  com.getProperty( "ref_id", id );
  com.getProperty( "objects", objId );
  com.getProperty( "windows", winId );
  com.getProperty( "filename", fname );
  com.getProperty( "ref_uuid", uuid );

  for( i=0, n=objId.size(); i<n; ++i )
    {
      ptr = context->unserial->pointer( objId[i], "AObject" );
      if( ptr )
	obj.insert( (AObject *) ptr );
      else
	cerr << "object id " << objId[i] << " not found\n";
    }
  for( i=0, n=winId.size(); i<n; ++i )
    {
      ptr = context->unserial->pointer( winId[i], "AWindow" );
      if( ptr )
	win.insert( (AWindow *) ptr );
      else
	cerr << "window id " << winId[i] << " not found\n";
    }

  if( id != 0 )
    {
      int	central = 0;
      com.getProperty( "central_ref", central );
      if( central )
	return new AssignReferentialCommand
	  ( theAnatomist->centralReferential(), obj, win, id, context );
      ptr = context->unserial->pointer( id, "Referential" );
    }
  if( ptr )	// existing ref
    return new AssignReferentialCommand( (Referential *) ptr, obj, win, -1, 
					 context, fname );
  else	// new ref or no ref (id=0 also)
    return new AssignReferentialCommand( 0, obj, win, id, context, fname, uuid );
}


void AssignReferentialCommand::write( Tree & com, Serializer* ser ) const
{
  Tree		*t = new Tree( true, name() );
  vector<int>	ids;
  set<AObject *>::const_iterator	io, fo = _obj.end();
  set<AWindow *>::const_iterator	iw, fw = _win.end();

  for( io=_obj.begin(); io!=fo; ++io )
    ids.push_back( ser->serialize( *io ) );
  if( !ids.empty() )
    t->setProperty( "objects", ids );

  ids.clear();
  for( iw=_win.begin(); iw!=fw; ++iw )
    ids.push_back( ser->serialize( *iw ) );
  if( !ids.empty() )
    t->setProperty( "windows", ids );

  t->setProperty( "ref_id", ser->serialize( _ref ) );

  if( _ref == theAnatomist->centralReferential() )
    t->setProperty( "central_ref", (int) 1 );

  if( !_filename.empty() )
    t->setProperty( "filename", _filename );

  if( !_uuid.empty() )
    t->setProperty( "ref_uuid", _uuid );

  com.insert( t );
}
