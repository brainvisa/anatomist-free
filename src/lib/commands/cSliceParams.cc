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

#include <anatomist/commands/cSliceParams.h>
#include <anatomist/object/selfsliceable.h>
#include <anatomist/object/Object.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/processor/unserializer.h>
#include <anatomist/processor/Serializer.h>
#include <anatomist/processor/context.h>
#include <cartobase/object/syntax.h>
#include <graph/tree/tree.h>
#include <fstream>

using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;


SliceParamsCommand::SliceParamsCommand( const std::set<AObject *> & obj, 
                                        const Point3df *pos, 
                                        const Quaternion* q, 
                                        const Point4df* pl ) 
  : RegularCommand(), _obj( obj ), _setpos( false ), _setquat( false ), 
    _setplane( false )
{
  if( pos )
    {
      _position = *pos;
      _setpos = true;
    }

  if( q )
    {
      _setquat = true;
      _quaternion = *q;
    }

  if( pl )
    {
      _setplane = true;
      _plane = *pl;
    }
}


SliceParamsCommand::~SliceParamsCommand()
{
}


bool SliceParamsCommand::initSyntax()
{
  SyntaxSet	ss;
  Syntax	& s = ss[ "SliceParams" ];
  
  s[ "objects"    ] = Semantic( "int_vector", true );
  s[ "quaternion" ] = Semantic( "float_vector", false );
  s[ "position"   ] = Semantic( "float_vector", false );
  s[ "plane"      ] = Semantic( "float_vector", false );
  Registry::instance()->add( "SliceParams", &read, ss );
  return true;
}


void SliceParamsCommand::doit()
{
  // cout << "SliceParams::doit\n";
  if( !_setpos && !_setquat && !_setplane )
    return;
  set<AObject *>::const_iterator	io, eo = _obj.end();
  SelfSliceable				*sl;
  for( io=_obj.begin(); io!=eo; ++io )
    if( theAnatomist->hasObject( *io ) 
        && (sl = dynamic_cast<SelfSliceable *>( *io ) ) )
      {
	if( _setquat )
          sl->setQuaternion( _quaternion );
	if( _setpos )
          sl->setOffset( _position );
	if( _setplane )
          sl->setPlane( _plane );
        (*io)->notifyObservers( this );
      }
}


Command* SliceParamsCommand::read( const Tree & com, CommandContext* context )
{
  vector<int>		obj;
  vector<float>		pos, q, pl;

  if( !com.getProperty( "objects", obj ) )
    return 0;
  com.getProperty( "position", pos );
  com.getProperty( "quaternion", q );
  com.getProperty( "plane", pl );

  set<AObject *>	objs;
  unsigned		i, n = obj.size();
  void			*ptr;

  for( i=0; i<n; ++i )
    {
      ptr = context->unserial->pointer( obj[i], "AObject" );
      if( ptr )
	objs.insert( (AObject *) ptr );
      else
	cerr << "object id " << obj[i] << " not found\n";
    }

  Point3df	p;
  Point4df	pe;
  if( pos.size() >= 3 )
    p = Point3df( pos[0], pos[1], pos[2] );
  if( pl.size() >= 4 )
    pe = Point4df( pl[0], pl[1], pl[2], pl[3] );
  Quaternion	quat;
  if( q.size() >= 4 )
  {
    if( q[0] == 0 && q[1] == 0 && q[2] == 0 && q[3] == 0 )
      cerr << "SliceParamsCommand: null quaternion read\n";
    else
      quat = Quaternion( q[0], q[1], q[2], q[3] );
  }
  return new SliceParamsCommand( objs, pos.size() >= 3 ? &p : 0, 
                                 q.size() >=4  ? &quat : 0, 
                                 pl.size() >= 4 ? &pe : 0 );
}


void SliceParamsCommand::write( Tree & com, Serializer* ser ) const
{
  Tree	*t = new Tree( true, name() );

  set<AObject *>::iterator	io, eo = _obj.end();
  vector<int>			obj;
  vector<float>			pos(3), q(4), pe(4);

  obj.reserve( _obj.size() );
  for( io=_obj.begin(); io!=eo; ++io )
    obj.push_back( ser->serialize( *io ) );

  t->setProperty( "objects", obj );
  if( _setplane )
    {
      pe[0] = _plane[0];
      pe[1] = _plane[1];
      pe[2] = _plane[2];
      pe[3] = _plane[3];
    }
  else
    {
      if( _setpos )
        {
          pos[0] = _position[0];
          pos[1] = _position[1];
          pos[2] = _position[2];
          t->setProperty( "position", pos );
        }
      if( _setquat )
        {
          Point4df	v = _quaternion.vector();
          q[0] = v[0];
          q[1] = v[1];
          q[2] = v[2];
          q[3] = v[3];
          t->setProperty( "quaternion", q );
        }
    }

  com.insert( t );
}


