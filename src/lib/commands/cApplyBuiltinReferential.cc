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

#include <anatomist/commands/cApplyBuiltinReferential.h>
#include <anatomist/object/Object.h>
#include <anatomist/window/Window.h>
#include <anatomist/reference/Referential.h>
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


ApplyBuiltinReferentialCommand::ApplyBuiltinReferentialCommand(
  const set<AObject *> & o )
  : RegularCommand(), _obj( o )
{
}


ApplyBuiltinReferentialCommand::~ApplyBuiltinReferentialCommand()
{
}


bool ApplyBuiltinReferentialCommand::initSyntax()
{
  SyntaxSet     ss;
  Syntax        & s = ss[ "ApplyBuiltinReferential" ];

  s[ "objects"     ] = Semantic( "int_vector", true );
  Registry::instance()->add( "ApplyBuiltinReferential", &read, ss );

  // new name for this command
  ss[ "LoadReferentialFromHeader" ] = s;
  Registry::instance()->add( "LoadReferentialFromHeader", &read, ss );

  return true;
}


void ApplyBuiltinReferentialCommand::doit()
{
  // cout << "ApplyBuiltinReferentialCommand::doit\n";
  ObjectActions::setAutomaticReferential( _obj );
}


Command* ApplyBuiltinReferentialCommand::read( const Tree & com,
                                               CommandContext* context )
{
  set<AObject *>        obj;
  vector<int>           objId;
  unsigned              i, n;
  void                  *ptr = 0;

  com.getProperty( "objects", objId );

  for( i=0, n=objId.size(); i<n; ++i )
  {
    ptr = context->unserial->pointer( objId[i], "AObject" );
    if( ptr )
      obj.insert( (AObject *) ptr );
    else
      cerr << "object id " << objId[i] << " not found\n";
  }
  return new ApplyBuiltinReferentialCommand( obj );
}


void ApplyBuiltinReferentialCommand::write( Tree & com, Serializer* ser ) const
{
  Tree          *t = new Tree( true, name() );
  vector<int>   ids;
  set<AObject *>::const_iterator        io, fo = _obj.end();

  for( io=_obj.begin(); io!=fo; ++io )
    ids.push_back( ser->serialize( *io ) );
  if( !ids.empty() )
    t->setProperty( "objects", ids );

  com.insert( t );
}

