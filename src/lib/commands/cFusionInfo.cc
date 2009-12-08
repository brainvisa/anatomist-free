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

#include <anatomist/commands/cFusionInfo.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/processor/unserializer.h>
#include <anatomist/processor/Serializer.h>
#include <anatomist/processor/context.h>
#include <anatomist/fusion/fusionFactory.h>
#include <graph/tree/tree.h>
#include <cartobase/object/syntax.h>
#include <cartobase/object/pythonwriter.h>
#include <fstream>

using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;


FusionInfoCommand::FusionInfoCommand( const vector<AObject *> & objects,
                                      const string & fname,
                                      const string & requestid,
                                      CommandContext* context )
  : RegularCommand(), SerializingCommand( context ), _obj( objects ),
    _filename( fname ), _requestid( requestid )
{
}


FusionInfoCommand::~FusionInfoCommand()
{
}


bool FusionInfoCommand::initSyntax()
{
  SyntaxSet     ss;
  Syntax        & s = ss[ "FusionInfo" ];

  s[ "filename"          ] = Semantic( "string", false );
  s[ "objects"           ] = Semantic( "int_vector", false );
  s[ "request_id"        ] = Semantic( "string", false );
  Registry::instance()->add( "FusionInfo", &read, ss );
  return true;
}


void
FusionInfoCommand::doit()
{
  ofstream      f;
  ostream       *filep = &f;
  if( !_filename.empty() )
#if defined( __GNUC__ ) && ( __GNUC__ - 0 == 3 ) && ( __GNUC_MINOR__ - 0 < 3 )
    // ios::app doesn't work on pipes on gcc 3.0 to 3.2
    f.open( _filename.c_str(), ios::out );
#else
    f.open( _filename.c_str(), ios::out | ios::app );
#endif
  else
  {
    filep = context()->ostr;
    if( !filep )
    {
      cerr << "FusionInfoCommand: no stream to write to\n";
      return;
    }
  }

  ostream       & file = *filep;
#ifndef _WIN32
  // on windows, a fdstream opened on a socket seems to return wrong state
  if( !file )
  {
    cerr << "warning: could not open output file " << _filename << endl;
    return;
  }
#endif

  Object        ex = Object::value( Dictionary() );

  if( _obj.empty() )
  {
    set<string> meths = FusionFactory::methods();
    vector<string> mvec;
    mvec.reserve( meths.size() );
    mvec.insert( mvec.begin(), meths.begin(), meths.end() );
    ex->setProperty( "all_methods", mvec );
  }
  else
  {
    set<string> meths = FusionFactory::factory()->
      allowedMethods( set<AObject *>( _obj.begin(), _obj.end() ) );
    vector<string> mvec;
    mvec.reserve( meths.size() );
    mvec.insert( mvec.begin(), meths.begin(), meths.end() );
    ex->setProperty( "allowed_methods", mvec );
  }

  if( !_requestid.empty() )
    ex->setProperty( "request_id", _requestid );

  _result = ex;

  // cout << "ObjectInfoCommand::doit writing\n";

  file << "'FusionInfo'\n";
  PythonWriter  pw;
  pw.setSingleLineMode( true );
  pw.attach( file );
  pw.write( *ex, false, false );
  file << endl << flush;
}


Command* FusionInfoCommand::read( const Tree & com, CommandContext* context )
{
  // cout << "ObjectInfoCommand::read\n";
  string        fname;
  vector<int>   ids;
  string        rid;
  vector<AObject *> obj;
  unsigned          i, n;
  void          *ptr;

  com.getProperty( "filename", fname );
  com.getProperty( "objects", ids );
  com.getProperty( "request_id", rid );

  obj.reserve( ids.size() );
  for( i=0, n=ids.size(); i<n; ++i )
  {
    ptr = context->unserial->pointer( ids[i], "AObject" );
    if( !ptr )
      cerr << "object id " << ids[i] << " not found\n";
    else
      obj.push_back( (AObject *) ptr );
  }

  return new FusionInfoCommand( obj, fname, rid, context );
}


void FusionInfoCommand::write( Tree & com, Serializer* ser ) const
{
  Tree  *t = new Tree( true, name() );
  vector<int>   ids;
  vector<AObject *>::const_iterator     io, fo = _obj.end();

  for( io=_obj.begin(); io!=fo; ++io )
    ids.push_back( ser->serialize( *io ) );

  if( !_filename.empty() )
    t->setProperty( "filename", _filename );
  if( !_obj.empty() )
    t->setProperty( "objects", ids );
  if( !_requestid.empty() )
    t->setProperty( "request_id", _requestid );
  com.insert( t );
}


Object FusionInfoCommand::result()
{
  return _result;
}


