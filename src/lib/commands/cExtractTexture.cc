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

#include <anatomist/commands/cExtractTexture.h>
#include <anatomist/surface/glcomponent.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/processor/unserializer.h>
#include <anatomist/processor/Serializer.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/window/glwidgetmanager.h>
#include <anatomist/surface/texture.h>
#include <anatomist/window/viewstate.h>
#include <aims/mesh/texture.h>
#include <cartobase/object/syntax.h>
#include <graph/tree/tree.h>

using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;

ExtractTextureCommand::ExtractTextureCommand( AObject* obj, int id, 
                                              float time, 
                                              CommandContext* context )
  : WaitCommand(), SerializingCommand( context ), 
    _object( obj ), _outobject( 0 ), _id( id ), _time( time )
{
}


ExtractTextureCommand::~ExtractTextureCommand()
{
}


bool ExtractTextureCommand::initSyntax()
{
  SyntaxSet	ss;
  Syntax	& s = ss[ "ExtractTexture" ];
  
  s[ "object"   ] = Semantic( "int", true );
  s[ "time"     ] = Semantic( "float", false );
  s[ "res_pointer" ] = Semantic( "int", false );
  Registry::instance()->add( "ExtractTexture", &read, ss );
  return( true );
}


void
ExtractTextureCommand::doit()
{
  if( !theAnatomist->hasObject( _object ) )
    return;
  GLComponent	*glo = _object->glAPI();
  if( !glo || glo->glNumTextures() == 0 )
    return;

  float	tmin, tmax, intv = 1., t;

  if( _time < 0 )
    {
      tmin = _object->MinT();
      tmax = _object->MaxT();
      intv = _object->TimeStep();
    }
  else
    {
      tmin = _time;
      tmax = _time;
    }

  unsigned	i, k = 0, n;

  GLWidgetManager::sharedWidget()->makeCurrent();
  ViewState	s;

  if( glo->glDimTex(tmin, 0) == 1 )
    {
      rc_ptr<TimeTexture<float> > tx( new Texture1d );
      const GLfloat		*coords;
      float			vmin = 0, vmax = 1, vqmin = 0, vqmax = 1, 
          scl = 1.;
      const GLComponent::TexExtrema  & te = glo->glTexExtrema();
      if( !te.min.empty() )
        vmin = te.min[0];
      if( !te.max.empty() )
        vmax = te.max[0];
      if( te.scaled && !te.minquant.empty() )
        vqmin = te.minquant[0];
      if( te.scaled && !te.maxquant.empty() )
        vqmax = te.maxquant[0];
      if( te.scaled && ( vqmin != vmin || vqmax != vmax ) )
	scl = (vqmax - vqmin) / (vmax - vmin);
      for( t=tmin; t<=tmax; t+=intv, ++k )
	{
          s.time = t;
          n = glo->glNumVertex( s );
          Texture<float>	& text = (*tx)[k];
          text.reserve( n );
          coords = glo->glTexCoordArray( s, 0 );
          for( i=0; i<n; ++i )
            text.push_back( vqmin + ( coords[i] - vmin ) * scl );
	}

      ATexture	*ao = new ATexture;
      ao->setTexture( tx );
      ao->setHeaderOptions();
      ao->setName( theAnatomist->makeObjectName( "extracted_1D_texture" ) );
      ao->setPalette( *_object->getOrCreatePalette() );
      theAnatomist->registerObject( ao );
      _outobject = ao;
    }
  else if( glo->glDimTex(tmin, 0) == 2 )
    {
      rc_ptr<TimeTexture<Point2df> > tx( new Texture2d );
      const GLfloat		*coords;
      Point2df			vmin = Point2df( 0, 0 ), 
        vmax = Point2df( 1, 1 ), vqmin = Point2df( 0, 0 ), 
        vqmax = Point2df( 1, 1 ), scl = Point2df( 1, 1 );
      const GLComponent::TexExtrema  & te = glo->glTexExtrema();
      if( !te.min.empty() )
      {
        vmin[0] = te.min[0];
        if( te.min.size() >= 2 )
          vmin[1] = te.min[1];
      }
      if( !te.max.empty() )
      {
        vmin = te.max[0];
        if( te.max.size() >= 2 )
          vmax[1] = te.max[1];
      }
      if( te.scaled && !te.minquant.empty() )
      {
        vqmin = te.minquant[0];
        if( te.minquant.size() >= 2 )
          vqmin[1] = te.minquant[1];
      }
      if( te.scaled && !te.maxquant.empty() )
      {
        vqmax = te.maxquant[0];
        if( te.maxquant.size() >= 2 )
          vqmax[1] = te.maxquant[1];
      }
      
      if( te.scaled && ( vqmin[0] != vmin[0] || vqmax[0] != vmax[0] ) )
	scl[0] = (vqmax[0] - vqmin[0]) / (vmax[0] - vmin[0]);
      if( te.scaled && ( vqmin[1] != vmin[1] || vqmax[1] != vmax[1] ) )
	scl[1] = (vqmax[1] - vqmin[1]) / (vmax[1] - vmin[1]);
      for( t=tmin; t<=tmax; t+=intv, ++k )
	{
          s.time = t;
          n = glo->glNumVertex( s );
          Texture<Point2df>	& text = (*tx)[k];
          text.reserve( n );
          coords = glo->glTexCoordArray( s, 0 );
          for( i=0; i<n; ++i )
            text.push_back( Point2df( vqmin[0] 
                                      + ( coords[i*2] - vmin[0] ) * scl[0],
                                      vqmin[1] 
                                      + ( coords[i-2+1] - vmin[1] ) * scl[1] )
                          ); 
	}
      ATexture	*ao = new ATexture;
      ao->setTexture( tx );
      ao->normalize();
      ao->setHeaderOptions();
      ao->setName( theAnatomist->makeObjectName( "extracted_2D_texture" ) );
      ao->setPalette( *_object->getOrCreatePalette() );
      theAnatomist->registerObject( ao );
      _outobject = ao;
    }

  if( _outobject && context() && context()->unserial )
    context()->unserial->registerPointer( _outobject, _id, "AObject" );
}


Command* ExtractTextureCommand::read( const Tree & com, 
                                      CommandContext* context )
{
  int		obj, id = -1;
  AObject	*ao;
  void		*ptr;
  float		time = -1;

  if( !com.getProperty( "object", obj ) )
    return( 0 );
  com.getProperty( "res_pointer", id );
  com.getProperty( "time", time );

  ptr = context->unserial->pointer( obj, "AObject" );
  if( ptr )
    ao = (AObject *) ptr;
  else
    {
      cerr << "object id " << obj << " not found\n";
      return( 0 );
    }

  return new ExtractTextureCommand( ao, id, time, context );
}


void ExtractTextureCommand::write( Tree & com, Serializer* ser ) const
{
  Tree	*t = new Tree( true, name() );
  int	obj = ser->serialize( _object );
  t->setProperty( "object", obj );
  if( _outobject )
    t->setProperty( "res_pointer", ser->serialize( _outobject ) );
  if( _time >= 0 )
    t->setProperty( "time", _time );
  com.insert( t );
}

