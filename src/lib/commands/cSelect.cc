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

#include <anatomist/commands/cSelect.h>
#include <anatomist/selection/selectFactory.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/processor/unserializer.h>
#include <anatomist/processor/Serializer.h>
#include <anatomist/processor/context.h>
#include <cartobase/object/syntax.h>
#include <graph/tree/tree.h>
#include <vector>

using namespace anatomist;
using namespace carto;
using namespace std;


SelectCommand::SelectCommand( const set<AObject *> & obj, int group, 
			      int modifiers )
  : RegularCommand(), _objects( obj ), _group( group ), _modifiers( modifiers )
{
}


SelectCommand::SelectCommand( const set<AObject *> & tosel, 
			      const set<AObject *> & tounsel, int group, 
			      int modifiers )
  : RegularCommand(), _objects( tosel ), _tounsel( tounsel), _group( group ), 
    _modifiers( modifiers )
{
}


SelectCommand::~SelectCommand()
{
}


bool SelectCommand::initSyntax()
{
  SyntaxSet	ss;
  Syntax	& s = ss[ "Select" ];
  
  s[ "objects"   ].type = "int_vector";
  s[ "objects"   ].needed = false;
  s[ "unselect_objects"   ].type = "int_vector";
  s[ "unselect_objects"   ].needed = false;
  s[ "group"   ].type = "int";
  s[ "group"   ].needed = false;
  s[ "modifiers"   ].type = "string";
  s[ "modifiers"   ].needed = false;
  Registry::instance()->add( "Select", &read, ss );
  return( true );
}


void SelectCommand::doit()
{
  /*cout << "SelectCommand\n";
  cout << "select : " << _objects.size() << " objects\n";
  cout << "unselect : " << _tounsel.size() << " objects\n";
  cout << "group : " << _group << endl;
  cout << "modifiers : " << _modifiers << endl;*/
  SelectFactory	*fac = SelectFactory::factory();
  fac->unselect( _group, _tounsel );
  fac->select( (SelectFactory::SelectMode) _modifiers, _group, _objects );
  fac->refresh();
}


Command* SelectCommand::read( const Tree & com, CommandContext* context )
{
  vector<int>		ids, uids;
  set<AObject *>	obj, unsel;
  int			group = 0, mod = 0;
  void			*ptr;
  unsigned		i, n;
  string		modif;

  com.getProperty( "objects", ids );
  com.getProperty( "unselect_objects", uids );
  com.getProperty( "group", group );
  com.getProperty( "modifiers", modif );
  if( modif == "add" || ( modif.empty() && !uids.empty() ) )
    mod = Add;
  else if( modif == "toggle" )
    mod = Toggle;

  for( i=0, n=ids.size(); i<n; ++i )
    {
      ptr = context->unserial->pointer( ids[i], "AObject" );
      if( ptr )
	obj.insert( (AObject *) ptr );
      else
	cerr << "object id " << ids[i] << " not found\n";
    }

  for( i=0, n=uids.size(); i<n; ++i )
    {
      ptr = context->unserial->pointer( uids[i], "AObject" );
      if( ptr )
	unsel.insert( (AObject *) ptr );
      else
	cerr << "object id " << uids[i] << " not found\n";
    }

  return( new SelectCommand( obj, unsel, group, mod ) );
}


void SelectCommand::write( Tree & com, Serializer* ser ) const
{
  Tree					*t = new Tree( true, name() );
  vector<int>				oid, uid;
  set<AObject *>::const_iterator	io, eo=_objects.end();

  if( _group != 0 )
    t->setProperty( "group", _group );
  string	modif[3] = { "set", "add", "toggle" };
  if( _modifiers > 0 && _modifiers <= Toggle 
      && ( _modifiers != Add || _tounsel.empty() ) )
    t->setProperty( "modifiers", modif[ _modifiers ] );
  for( io=_objects.begin(); io!=eo; ++io )
    oid.push_back( ser->serialize( *io ) );
  if( !oid.empty() )
    t->setProperty( "objects", oid );
  for( io=_tounsel.begin(), eo=_tounsel.end(); io!=eo; ++io )
    uid.push_back( ser->serialize( *io ) );
  if( !uid.empty() )
    t->setProperty( "unselect_objects", uid );
  com.insert( t );
}
