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

#include <anatomist/commands/cGenerateTexture.h>
#include <anatomist/surface/glcomponent.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/processor/unserializer.h>
#include <anatomist/processor/Serializer.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/color/objectPalette.h>
#include <anatomist/window/glwidget.h>
#include <anatomist/surface/texture.h>
#include <anatomist/window/viewstate.h>
#include <aims/mesh/texture.h>
#include <cartobase/object/syntax.h>
#include <graph/tree/tree.h>

using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;

GenerateTextureCommand::GenerateTextureCommand( AObject* obj, int id, int dim, 
                                                CommandContext* context )
  : WaitCommand(), SerializingCommand( context ), 
    _object( obj ), _outobject( 0 ), _id( id ), _dim( dim )
{
}


GenerateTextureCommand::~GenerateTextureCommand()
{
}


bool GenerateTextureCommand::initSyntax()
{
  SyntaxSet	ss;
  Syntax	& s = ss[ "GenerateTexture" ];
  
  s[ "object"      ] = Semantic( "int", false );
  s[ "dimension"   ] = Semantic( "int", false );
  s[ "res_pointer" ] = Semantic( "int", false );
  Registry::instance()->add( "GenerateTexture", &read, ss );
  return( true );
}


void
GenerateTextureCommand::doit()
{
  if( _object && !theAnatomist->hasObject( _object ) )
    _object = 0;
  GLComponent	*glo = 0;
  if( _object )
    glo = _object->glAPI();

  if( _dim == 2 )
    {
      rc_ptr<Texture2d> tex( new Texture2d );
      ATexture	*ao = new ATexture;
      unsigned	i, n = 1;
      if( glo )
        {
          ViewState	vs;
          n = glo->glNumVertex( vs );
        }
      Texture<Point2df>	& t = (*tex)[0];
      t.reserve( n );
      for( i=0; i<n; ++i )
        t.push_back( Point2df( 0, 0 ) );
      ao->setTexture( tex );
      ao->normalize();
      ao->glSetAutoTexMode( GLComponent::glTEX_NORMAL_MAP );
      ao->setHeaderOptions();
      ao->setName( theAnatomist->makeObjectName( "genrated_2D_texture" ) );
      ao->createDefaultPalette( "toto" );
      ao->palette()->set2dMode( true );
      ao->palette()->create( ao->palette()->colors()->dimX(), 256 );
      ao->palette()->fill();
      theAnatomist->registerObject( ao );
      _outobject = ao;
    }
  else
    {
      rc_ptr<Texture1d> tex( new Texture1d );
      ATexture	*ao = new ATexture;
      unsigned	i, n = 1;
      if( glo )
        {
          ViewState	vs;
          n = glo->glNumVertex( vs );
        }
      Texture<float>	& t = (*tex)[0];
      t.reserve( n );
      for( i=0; i<n; ++i )
        t.push_back( 0 );
      ao->setTexture( tex );
      ao->normalize();
      ao->glSetAutoTexMode( GLComponent::glTEX_NORMAL_MAP );
      ao->setHeaderOptions();
      ao->setName( theAnatomist->makeObjectName( "genrated_1D_texture" ) );
      theAnatomist->registerObject( ao );
      _outobject = ao;
    }

  if( _outobject && context() && context()->unserial )
    context()->unserial->registerPointer( _outobject, _id, "AObject" );
}


Command* GenerateTextureCommand::read( const Tree & com, 
                                       CommandContext* context )
{
  int		obj, id = -1;
  AObject	*ao;
  void		*ptr;
  int		dim = 1;

  if( !com.getProperty( "object", obj ) )
    return( 0 );
  com.getProperty( "res_pointer", id );
  com.getProperty( "dimension", dim );

  ptr = context->unserial->pointer( obj, "AObject" );
  if( ptr )
    ao = (AObject *) ptr;
  else
    {
      cerr << "object id " << obj << " not found\n";
      return 0;
    }

  return new GenerateTextureCommand( ao, id, dim, context );
}


void GenerateTextureCommand::write( Tree & com, Serializer* ser ) const
{
  Tree	*t = new Tree( true, name() );
  int	obj = ser->serialize( _object );
  t->setProperty( "object", obj );
  if( _outobject )
    t->setProperty( "res_pointer", ser->serialize( _outobject ) );
  if( _dim != 1 )
    t->setProperty( "dimension", _dim );
  com.insert( t );
}

