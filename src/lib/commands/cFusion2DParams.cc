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


#include <anatomist/commands/cFusion2DParams.h>
#include <anatomist/mobject/Fusion2D.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/processor/Serializer.h>
#include <anatomist/processor/unserializer.h>
#include <anatomist/processor/context.h>
#include <anatomist/application/Anatomist.h>
#include <cartobase/object/syntax.h>
#include <graph/tree/tree.h>

using namespace anatomist;
using namespace carto;
using namespace std;


Fusion2DParamsCommand::Fusion2DParamsCommand( Fusion2D* obj, int mode, 
					      float rate )
  : RegularCommand(), _object( obj ), _mode( mode ), _rate( rate )
{
}


Fusion2DParamsCommand::Fusion2DParamsCommand( Fusion2D* obj, int mode, 
					      float rate, 
					      const vector<AObject *> & order )
  : RegularCommand(), _object( obj ), _mode( mode ), _rate( rate ), 
    _order( order )
{
}


Fusion2DParamsCommand::~Fusion2DParamsCommand()
{
}


bool Fusion2DParamsCommand::initSyntax()
{
  SyntaxSet	ss;
  Syntax	& s = ss[ "Fusion2DParams" ];
  
  s[ "object"          ].type = "int";
  s[ "object"          ].needed = true;
  s[ "mode"          ].type = "string";
  s[ "mode"          ].needed = false;
  s[ "rate"          ].type = "float";
  s[ "rate"          ].needed = false;
  s[ "reorder_objects"          ].type = "int_vector";
  s[ "reorder_objects"          ].needed = false;
  Registry::instance()->add( "Fusion2DParams", &read, ss );
  return( true );
}


void Fusion2DParamsCommand::doit()
{
  // cout << "Fusion2DParamsCommand::doit\n";
  if( !theAnatomist->hasObject( _object ) )
    return;
  GLComponent	*gc = _object->glAPI();
  if( !gc )
    return;

  if( _mode >= 0 )
    gc->glSetTexMode( (GLComponent::glTextureMode) _mode, 1 );
  if( _rate >= 0 )
    gc->glSetTexRate( _rate, 1 );
  if( !_order.empty() )
    _object->reorder( _order );
  _object->notifyObservers( this );
}


Command* Fusion2DParamsCommand::read( const Tree & com, 
				      CommandContext* context )
{
  int			id;
  void			*ptr = 0;
  AObject		*obj;
  Fusion2D		*f;
  string		smode;
  int			mode = -1;
  float			rate = -1;
  vector<int>		order;
  vector<AObject *>	oorder;

  com.getProperty( "object", id );

  ptr = context->unserial->pointer( id, "AObject" );
  if( !ptr )
    {
      cerr << "object id " << id << " not found\n";
      return( 0 );
    }

  obj = (AObject *) ptr;
  f = dynamic_cast<Fusion2D *>( obj );
  if( !f )
    {
      cerr << "Fusion2DParams : object " << obj->name() 
	   << " isn't a Fusion2D !\n";
      return( 0 );
    }

  com.getProperty( "mode", smode );
  com.getProperty( "rate", rate );
  com.getProperty( "reorder_objects", order );

  if( smode == "linear" )
    mode = GLComponent::glLINEAR;
  else if( smode == "geometric" )
    mode = GLComponent::glGEOMETRIC;
  else if( smode == "linear_on_defined" )
    mode = GLComponent::glLINEAR_ON_DEFINED;
  else if( smode == "add" )
    mode = GLComponent::glADD;

  unsigned		i, n = order.size();

  oorder.reserve( n );
  for( i=0; i<n; ++i )
    oorder.push_back( (AObject *) 
                      context->unserial->pointer( order[i], "AObject" ) );

  if( !oorder.empty() )
    return( new Fusion2DParamsCommand( f, mode, rate, oorder ) );
  else
    return( new Fusion2DParamsCommand( f, mode, rate ) );
}


void Fusion2DParamsCommand::write( Tree & com, Serializer* ser ) const
{
  Tree	*t = new Tree( true, name() );
  int	id;

  id = ser->serialize( _object );
  t->setProperty( "object", id );
  if( _mode >= 0 && _mode < 8 )
    {
      static const string smode[] = 
        { "geometric", "linear", "replace", "decal", "blend", "add", 
          "combine", "linear_on_defined"
        };
      t->setProperty( "mode", smode[_mode] );
    }
  if( _rate >= 0 && _rate <= 1 )
    t->setProperty( "rate", _rate );
  if( !_order.empty() )
    {
      vector<int>		order;
      Fusion2D::iterator	io, eo = _object->end();

      order.reserve( _object->size() );
      for( io=_object->begin(); io!=eo; ++io )
        order.push_back( ser->serialize( *io ) );
      t->setProperty( "reorder_objects", order );
    }

  com.insert( t );
  // cout << "Fusion2DParamsCommand::write done\n";
}
