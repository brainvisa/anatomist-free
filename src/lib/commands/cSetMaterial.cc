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


#include <anatomist/commands/cSetMaterial.h>
#include <anatomist/color/Material.h>
#include <anatomist/object/Object.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/processor/unserializer.h>
#include <anatomist/processor/Serializer.h>
#include <anatomist/processor/context.h>
#include <anatomist/surface/glcomponent.h>
#include <cartobase/object/syntax.h>
#include <graph/tree/tree.h>

using namespace anatomist;
using namespace carto;
using namespace std;


SetMaterialCommand::SetMaterialCommand( const set<AObject *> & obj, 
					float* ambient, float* diffuse, 
					float* emission, float* specular, 
					float shininess, bool refresh, 
                                        int lighting, int smoothshading, 
                                        int polyfiltering, int zbuffer, 
                                        int faceculling, 
                                        const std::string & polymode )
  : RegularCommand(), _obj( obj ), _shininess( shininess ), 
    _refresh( refresh ), _lighting( lighting ), 
    _smoothshading( smoothshading ), _polygonfiltering( polyfiltering ), 
    _zbuffer( zbuffer), _faceculling( faceculling ), _polygonmode( polymode )
{
  if( ambient )
    {
      _ambient[0] = ambient[0];
      _ambient[1] = ambient[1];
      _ambient[2] = ambient[2];
      _ambient[3] = ambient[3];
    }
  else
    {
      _ambient[0] = -1;
      _ambient[1] = -1;
      _ambient[2] = -1;
      _ambient[3] = -1;
    }
  if( diffuse )
    {
      _diffuse[0] = diffuse[0];
      _diffuse[1] = diffuse[1];
      _diffuse[2] = diffuse[2];
      _diffuse[3] = diffuse[3];
    }
  else
    {
      _diffuse[0] = -1;
      _diffuse[1] = -1;
      _diffuse[2] = -1;
      _diffuse[3] = -1;
    }
  if( emission )
    {
      _emission[0] = emission[0];
      _emission[1] = emission[1];
      _emission[2] = emission[2];
      _emission[3] = emission[3];
    }
  else
    {
      _emission[0] = -1;
      _emission[1] = -1;
      _emission[2] = -1;
      _emission[3] = -1;
    }
  if( specular )
    {
      _specular[0] = specular[0];
      _specular[1] = specular[1];
      _specular[2] = specular[2];
      _specular[3] = specular[3];
    }
  else
    {
      _specular[0] = -1;
      _specular[1] = -1;
      _specular[2] = -1;
      _specular[3] = -1;
    }
}


SetMaterialCommand::~SetMaterialCommand()
{
}


bool SetMaterialCommand::initSyntax()
{
  SyntaxSet	ss;
  Syntax	& s = ss[ "SetMaterial" ];
  
  s[ "objects"           ] = Semantic( "int_vector", true );
  s[ "ambient"           ] = Semantic( "float_vector" );
  s[ "diffuse"           ] = Semantic( "float_vector" );
  s[ "emission"          ] = Semantic( "float_vector" );
  s[ "specular"          ] = Semantic( "float_vector" );
  s[ "shininess"         ] = Semantic( "float" );
  s[ "refresh"           ] = Semantic( "int" );
  s[ "lighting"          ] = Semantic( "int" );
  s[ "smooth_shading"    ] = Semantic( "int" );
  s[ "polygon_filtering" ] = Semantic( "int" );
  s[ "depth_buffer"      ] = Semantic( "int" );
  s[ "face_culling"      ] = Semantic( "int" );
  s[ "polygon_mode"      ] = Semantic( "string" );

  Registry::instance()->add( "SetMaterial", &read, ss );
  return( true );
}


void SetMaterialCommand::doit()
{
  set<AObject *>::const_iterator	io = _obj.begin(), fo = _obj.end();
  unsigned				i;
  AObject				*o;

  for( io=_obj.begin(); io!=fo; ++io )
    if( theAnatomist->hasObject( *io ) )
      {
	o = *io;

	Material	&mat = o->GetMaterial();
	bool		changed = false;

	for( i=0; i<4; ++i )
	  {
	    if( _ambient[i] >= 0 && mat.Ambient()[i] != _ambient[i] )
	      {
		mat.Ambient()[i] = _ambient[i];
		changed = true;
	      }
	    if( _diffuse[i] >= 0 && mat.Diffuse()[i] != _diffuse[i] )
	      {
		mat.Diffuse()[i] = _diffuse[i];
		changed = true;
	      }
	    if( _emission[i] >= 0 && mat.Emission()[i] != _emission[i] )
	      {
		mat.Emission()[i] = _emission[i];
		changed = true;
	      }
	    if( _specular[i] >= 0 && mat.Specular()[i] != _specular[i] )
	      {
		mat.Specular()[i] = _specular[i];
		changed = true;
	      }
	  }
	if( _shininess >= 0 && mat.Shininess() != _shininess )
	  {
	    mat.SetShininess( _shininess );
	    changed = true;
	  }
        if( _lighting != -2 )
          {
            mat.setRenderProperty( Material::RenderLighting, _lighting );
            changed = true;
          }
        if( _smoothshading != -2 )
          {
            mat.setRenderProperty( Material::RenderSmoothShading, 
                                   _smoothshading );
            changed = true;
          }
        if( _polygonfiltering != -2 )
          {
            mat.setRenderProperty( Material::RenderFiltering, 
                                   _polygonfiltering );
            changed = true;
          }
        if( _zbuffer != -2 )
          {
            mat.setRenderProperty( Material::RenderZBuffer, _zbuffer );
            changed = true;
          }
        if( _faceculling != -2 )
          {
            mat.setRenderProperty( Material::RenderFaceCulling, _faceculling );
            changed = true;
          }
        if( !_polygonmode.empty() )
          {
            if( _polygonmode == "normal" )
              mat.setRenderProperty( Material::RenderMode, Material::Normal );
            else if( _polygonmode ==  "wireframe" )
              mat.setRenderProperty( Material::RenderMode, 
                                     Material::Wireframe );
            else if( _polygonmode ==  "outline"
                     || _polygonmode ==  "outlined" )
              mat.setRenderProperty( Material::RenderMode, 
                                     Material::Outlined );
            else if( _polygonmode ==  "hiddenface_wireframe" )
              mat.setRenderProperty( Material::RenderMode, 
                                     Material::HiddenWireframe );
            else // default
              mat.setRenderProperty( Material::RenderMode, -1 );
            changed = true;
          }

        if( changed )
          o->SetMaterial( mat );
        if( o->glAPI() )
	  o->glAPI()->glSetChanged( GLComponent::glMATERIAL );
	if( _refresh && changed )
	  o->notifyObservers( this );
      }
}


Command* SetMaterialCommand::read( const Tree & com, CommandContext* context )
{
  vector<int>		obj;
  set<AObject *>	objL;
  vector<float>		ambient, diffuse, emission, specular;
  float			shininess = -1;
  float			*amb = 0, *dif = 0, *emi = 0, *spe = 0;
  unsigned		i, n;
  void			*ptr;
  int			refresh = true, lighting = -2, smooth = -2, 
    filter = -2, zbuffer = -2, facecull = -2;
  string		polymode;

  if( !com.getProperty( "objects", obj ) )
    return( 0 );

  for( i=0, n=obj.size(); i<n; ++i )
    {
      ptr = context->unserial->pointer( obj[i], "AObject" );
      if( ptr )
	objL.insert( (AObject *) ptr );
      else
	{
	  cerr << "object id " << obj[i] << " not found\n";
	  return( 0 );
	}
    }

  if( com.getProperty( "ambient", ambient ) )
    amb = &ambient[0];
  if( com.getProperty( "diffuse", diffuse ) )
    dif = &diffuse[0];
  if( com.getProperty( "emission", emission ) )
    emi = &emission[0];
  if( com.getProperty( "specular", specular ) )
    spe = &specular[0];
  com.getProperty( "shininess", shininess );
  com.getProperty( "refresh", refresh );
  com.getProperty( "lighting", lighting );
  com.getProperty( "smooth_shading", smooth );
  com.getProperty( "polygon_filtering", filter );
  com.getProperty( "depth_buffer", zbuffer );
  com.getProperty( "face_culling", facecull );
  com.getProperty( "polygon_mode", polymode );

  return( new SetMaterialCommand( objL, amb, dif, emi, spe, shininess, 
				  (bool) refresh, lighting, smooth, filter, 
                                  zbuffer, facecull, polymode ) );
}


void SetMaterialCommand::write( Tree & com, Serializer* ser ) const
{
  Tree	*t = new Tree( true, name() );

  set<AObject *>::const_iterator	io;
  vector<int>				obj;
  vector<float>				amb, dif, emi, spe;
  unsigned				i;

  for( io=_obj.begin(); io!=_obj.end(); ++io ) 
    obj.push_back( ser->serialize( *io ) );

  t->setProperty( "objects", obj );
  for( i=0; i<4; ++i )
    {
      amb.push_back( _ambient[i] );
      dif.push_back( _diffuse[i] );
      emi.push_back( _emission[i] );
      spe.push_back( _specular[i] );
    }
  t->setProperty( "ambient", amb );
  t->setProperty( "diffuse", dif );
  t->setProperty( "emission", emi );
  t->setProperty( "specular", spe );
  t->setProperty( "shininess", _shininess );
  if( !_refresh )
    t->setProperty( "refresh", (int) 0 );
  if( _lighting != -2 )
    t->setProperty( "lighting", _lighting );
  if( _smoothshading !=-2 )
    t->setProperty( "smooth_shading", _smoothshading );
  if( _polygonfiltering !=-2 )
    t->setProperty( "polygon_filtering", _polygonfiltering );
  if( _zbuffer !=-2 )
    t->setProperty( "depth_buffer", _zbuffer );
  if( _faceculling !=-2 )
    t->setProperty( "face_culling", _faceculling );
  if( !_polygonmode.empty() )
    t->setProperty( "polygon_mode", _polygonmode );
  com.insert( t );
}
