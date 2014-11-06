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

#include <anatomist/color/Material.h>
#include <cartobase/object/object.h>

using namespace anatomist;
using namespace carto;
using namespace std;

namespace
{
  const size_t NrenderProps = Material::NormalIsDirection + 1;
}

struct Material::Private
{
  Private();
  Private( const Private & );
  Private & operator = ( const Private & );
  int	renderProps[ NrenderProps ];
  GLfloat unlitColor[4];
  float lineWidth;
};


Material::Private::Private(): lineWidth( 0. )
{
  for( unsigned i = 0; i<NrenderProps; ++i )
    renderProps[ i ] = -1;
  renderProps[ SelectableMode ] = SelectableWhenOpaque;
  unlitColor[0] = 0.;
  unlitColor[1] = 0.;
  unlitColor[2] = 0.;
  unlitColor[3] = 1.;
}


Material::Private::Private( const Private & p ) : lineWidth(p.lineWidth)
{
  for( unsigned i = 0; i<NrenderProps; ++i )
    renderProps[ i ] = p.renderProps[ i ];
  unlitColor[0] = p.unlitColor[0];
  unlitColor[1] = p.unlitColor[1];
  unlitColor[2] = p.unlitColor[2];
  unlitColor[3] = p.unlitColor[3];
}


Material::Private & Material::Private::operator = ( const Private & p )
{
  if( this != & p )
  {
    for( unsigned i = 0; i<NrenderProps; ++i )
      renderProps[ i ] = p.renderProps[ i ];
    unlitColor[0] = p.unlitColor[0];
    unlitColor[1] = p.unlitColor[1];
    unlitColor[2] = p.unlitColor[2];
    unlitColor[3] = p.unlitColor[3];
    lineWidth = p.lineWidth;
  }
  return *this;
}


Material::Material()
  : d( new Private )
{
  _ambient[0] = _ambient[1] = _ambient[2] = 0.1;
  _ambient[3] = 1.0;

  _diffuse[0] = _diffuse[1] = _diffuse[2] = 0.8;
  _diffuse[3] = 1.0;

  _specular[0] = _specular[1] = _specular[2] = 0.2;
  _specular[3] = 1.0;

  _shininess = 20.0;

  _emission[0] = _emission[1] = _emission[2] = 0.0;
  _emission[3] = 1.0;
}

Material::Material(const Material &material)
  : d( new Private( *material.d ) )
{
  _shininess = material.Shininess();

  int	i;
  for (i=0; i<4; i++) 
    {
      _ambient[i] = material.Ambient(i);
      _diffuse[i] = material.Diffuse(i);
      _specular[i] = material.Specular(i);
      _emission[i] = material.Emission(i);
    }
}

Material::~Material()
{
  delete d;
}

void Material::SetAmbient(float r, float g, float b, float a)
{
  _ambient[0] = r;
  _ambient[1] = g;
  _ambient[2] = b;
  _ambient[3] = a;
}

void Material::SetDiffuse(float r, float g, float b, float a)
{
  _diffuse[0] = r;
  _diffuse[1] = g;
  _diffuse[2] = b;
  _diffuse[3] = a;
}

void Material::SetSpecular(float r, float g, float b, float a)
{
  _specular[0] = r;
  _specular[1] = g;
  _specular[2] = b;
  _specular[3] = a;
}

void Material::SetEmission(float r, float g, float b, float a)
{
  _emission[0] = r;
  _emission[1] = g;
  _emission[2] = b;
  _emission[3] = a;
}

void Material::SetShininess(float val) 
{
  _shininess = val;
  //glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &_shininess);
}

void Material::SetAmbientR(float val) 
{ 
  _ambient[0] = val;
  //glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, _ambient); 
}

void Material::SetAmbientG(float val) 
{
  _ambient[1] = val; 
  //glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, _ambient); 
}

void Material::SetAmbientB(float val) 
{
  _ambient[2] = val; 
  //glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, _ambient); 
}

void Material::SetAmbientA(float val) 
{
  _ambient[3] = val; 
  //glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, _ambient); 
}

void Material::SetDiffuseR(float val) 
{
  _diffuse[0] = val; 
  //glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, _diffuse);
}

void Material::SetDiffuseG(float val) 
{
  _diffuse[1] = val; 
  //glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, _diffuse);
}

void Material::SetDiffuseB(float val) 
{
  _diffuse[2] = val; 
  //glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, _diffuse);
}

void Material::SetDiffuseA(float val) 
{
  _diffuse[3] = val; 
  //glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, _diffuse);
}

void Material::SetSpecularR(float val) 
{
  _specular[0] = val; 
  //glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, _specular);
}

void Material::SetSpecularG(float val) 
{
  _specular[1] = val; 
  //glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, _specular);
}

void Material::SetSpecularB(float val) 
{
  _specular[2] = val; 
  //glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, _specular);
}

void Material::SetSpecularA(float val) 
{
  _specular[3] = val; 
  //glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, _specular);
}

void Material::SetEmissionR(float val) 
{
  _emission[0] = val; 
  //glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, _emission);
}

void Material::SetEmissionG(float val) 
{
  _emission[1] = val; 
  //glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, _emission);
}

void Material::SetEmissionB(float val) 
{
  _emission[2] = val; 
  //glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, _emission);
}

void Material::SetEmissionA(float val) 
{
  _emission[3] = val; 
  //glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, _emission);
}


GLfloat *Material::unlitColor() const
{
  return d->unlitColor;
}


GLfloat Material::unlitColor( int i ) const
{
  return d->unlitColor[i];
}


void Material::setUnlitColor( float r, float g, float b, float a )
{
  d->unlitColor[0] = r;
  d->unlitColor[1] = g;
  d->unlitColor[2] = b;
  d->unlitColor[3] = a;
}


float Material::lineWidth() const
{
  return d->lineWidth;
}


void Material::setLineWidth( float w )
{
  d->lineWidth = w;
}


bool Material::IsBlended() const
{
  if ((_ambient[3]*_diffuse[3]*_specular[3]*_emission[3]) != 1.0) return(true);
  else return(false);
}

void Material::setGLMaterial() const
{
  int	topush 
    = GL_LIGHTING_BIT 
    | ( d->renderProps[ RenderSmoothShading ] != -1 ) * GL_LIGHTING_BIT
    | ( d->renderProps[ RenderFiltering ] != -1 ) * GL_LINE_BIT
    | ( d->renderProps[ RenderZBuffer ] != -1 ) * GL_DEPTH_BUFFER_BIT
    | ( d->renderProps[ RenderFaceCulling ] != -1
        || d->renderProps[ FrontFace ] != -1 ) * GL_POLYGON_BIT
    | ( d->renderProps[ RenderMode ] != -1 )
    * ( GL_POLYGON_BIT | GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT );

  //if( topush != 0 )
  glPushAttrib( topush );

  glLightModeli( GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE );
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, _ambient);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, _specular);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &_shininess);
  glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, _emission);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, _diffuse);
  glColor4f( _diffuse[0], _diffuse[1], _diffuse[2], _diffuse[3] );
  if( d->lineWidth > 0 )
    glLineWidth( d->lineWidth );
  else
    glLineWidth( 1. );

  // always use blending: here we don't know if a texture will be mixed to 
  // the material

  /* if (IsBlended())
     {*/
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  /*  }
      else glBlendFunc(GL_ONE, GL_ZERO); */

  switch( d->renderProps[ RenderLighting ] )
    {
    case 1:
      glEnable( GL_LIGHTING );
      break;
    case 0:
      glDisable( GL_LIGHTING );
      break;
    default:
      break;
    }
  switch( d->renderProps[ RenderSmoothShading ] )
    {
    case 1:
      glShadeModel( GL_SMOOTH );
      break;
    case 0:
      glShadeModel( GL_FLAT );
      break;
    default:
      break;
    }
  switch( d->renderProps[ RenderFiltering ] )
    {
    case 1:
      glEnable( GL_LINE_SMOOTH );
      glEnable( GL_POLYGON_SMOOTH );
      break;
    case 0:
      glDisable( GL_LINE_SMOOTH );
      glDisable( GL_POLYGON_SMOOTH );
      break;
    default:
      break;
    }
  switch( d->renderProps[ RenderZBuffer ] )
    {
    case 1:
      glDepthMask( GL_TRUE );
      break;
    case 0:
      glDepthMask( GL_FALSE );
      break;
    default:
      break;
    }
  switch( d->renderProps[ RenderFaceCulling ] )
    {
    case 1:
      glEnable( GL_CULL_FACE );
      break;
    case 0:
      glDisable( GL_CULL_FACE );
      break;
    default:
      break;
    }
  switch( d->renderProps[ FrontFace ] )
  {
    case 0:
      glFrontFace( GL_CW );
      break;
    case 1:
      glFrontFace( GL_CCW );
      break;
    default:
      break;
  }
  switch( d->renderProps[ RenderMode ] )
    {
    case Normal:
      glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
      glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
      break;
    case Wireframe:
      glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
      glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
      break;
    case HiddenWireframe:
      glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
      glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
      glPolygonOffset( 1.05, 1 );
      glEnable( GL_POLYGON_OFFSET_FILL );
      break;
    case Outlined:
      glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
      glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
      glPolygonOffset( 1.05, 1 );
      glEnable( GL_POLYGON_OFFSET_FILL );
      break;
    case ExtOutlined:
      glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
      glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
      glEnable( GL_LIGHTING );
      glDisable( GL_POLYGON_OFFSET_LINE );
      glDisable( GL_POLYGON_OFFSET_FILL );
      glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE, _diffuse );
      break;
    default:
      break;
    }
}


void Material::popGLState() const
{
  glPopAttrib();
}


Material &Material::operator = (const Material &theMaterial)
{
  if (this != &theMaterial)
  {
    _shininess = theMaterial.Shininess();
    int	i;
    for (i=0; i<4; i++)
    {
      _ambient[i] = theMaterial.Ambient(i);
      _diffuse[i] = theMaterial.Diffuse(i);
      _specular[i] = theMaterial.Specular(i);
      _emission[i] = theMaterial.Emission(i);
    }
    *d = *theMaterial.d;
  }

  return(*this);
}

std::istream & anatomist::operator >> ( std::istream &in,
                                        anatomist::Material &material )
{
  in >> material._shininess;

  int	i;
  for (i=0; i<4; i++) in >> material._ambient[i];
  for (i=0; i<4; i++) in >> material._diffuse[i];
  for (i=0; i<4; i++) in >> material._specular[i];
  for (i=0; i<4; i++) in >> material._emission[i];

  return(in);
}

std::ostream & anatomist::operator << (std::ostream &out, 
			   const anatomist::Material &material)
{
  out << material._shininess << endl;

  int	i;
  for (i=0; i<4; i++) out << material._ambient[i] << " ";
  out << endl;
  for (i=0; i<4; i++) out << material._diffuse[i] << " ";
  out << endl;
  for (i=0; i<4; i++) out << material._specular[i] << " ";
  out << endl;
  for (i=0; i<4; i++) out << material._emission[i] << " ";
  out << endl;

  return(out);
}


bool Material::operator != ( const Material & mat ) const
{
  return( _ambient[0] != mat._ambient[0] || _ambient[1] != mat._ambient[1] 
          || _ambient[2] != mat._ambient[2] || _ambient[3] != mat._ambient[3]
          || _diffuse[0] != mat._diffuse[0] || _diffuse[1] != mat._diffuse[1]
          || _diffuse[2] != mat._diffuse[2] || _diffuse[3] != mat._diffuse[3]
          || _specular[0] != mat._specular[0]
          || _specular[1] != mat._specular[1]
          || _specular[2] != mat._specular[2]
          || _specular[3] != mat._specular[3]
          || _shininess != mat._shininess || _emission[0] != mat._emission[0]
          || _emission[1] != mat._emission[1]
          || _emission[2] != mat._emission[2]
          || _emission[3] != mat._emission[3]
          || d->renderProps[ RenderLighting ]
          != mat.d->renderProps[ RenderLighting ]
          || d->renderProps[ RenderSmoothShading ] 
          != mat.d->renderProps[ RenderSmoothShading ]
          || d->renderProps[ RenderFiltering ] 
          != mat.d->renderProps[ RenderFiltering ]
          || d->renderProps[ RenderZBuffer ] 
          != mat.d->renderProps[ RenderZBuffer ]
          || d->renderProps[ RenderFaceCulling ] 
          != mat.d->renderProps[ RenderFaceCulling ]
          || d->renderProps[ RenderMode ] 
          != mat.d->renderProps[ RenderMode ]
          || d->renderProps[ SelectableMode ]
          != mat.d->renderProps[ SelectableMode ]
          || d->renderProps[ FrontFace ] != mat.d->renderProps[ FrontFace ]
          || d->unlitColor[0] != mat.d->unlitColor[0]
          || d->unlitColor[1] != mat.d->unlitColor[1]
          || d->unlitColor[2] != mat.d->unlitColor[2]
          || d->unlitColor[3] != mat.d->unlitColor[3]
          || d->lineWidth != mat.d->lineWidth
          );
}


void Material::set( const GenericObject & obj )
{
  /*
    PythonWriter	pw;
    pw.attach( cout );
    pw.write( obj, false, true );
  */

  Object	vec;
  unsigned	n;
  try
    {
      vec = obj.getProperty( "ambient" );
      n = vec->size();
      try
        {
          if( n >= 1 )
            _ambient[0] = vec->getArrayItem(0)->getScalar();
          if( n >= 2 )
            _ambient[1] = vec->getArrayItem(1)->getScalar();
          if( n >= 3 )
            _ambient[2] = vec->getArrayItem(2)->getScalar();
          if( n >= 4 )
            _ambient[3] = vec->getArrayItem(3)->getScalar();
        }
      catch( ... )
        {
        }
    }
  catch( ... )
    {
    }
  try
    {
      vec = obj.getProperty( "diffuse" );
      try
        {
          n = vec->size();
          //std::cout << "diffuse: " << n << "\n";
          if( n >= 1 )
            _diffuse[0] = vec->getArrayItem(0)->getScalar();
          if( n >= 2 )
            _diffuse[1] = vec->getArrayItem(1)->getScalar();
          if( n >= 3 )
            _diffuse[2] = vec->getArrayItem(2)->getScalar();
          if( n >= 4 )
            _diffuse[3] = vec->getArrayItem(3)->getScalar();
        }
      catch( ... )
        {
        }
    }
  catch( ... )
    {
    }
  try
    {
      vec = obj.getProperty( "emission" );
      try
        {
          n = vec->size();
          if( n >= 1 )
            _emission[0] = vec->getArrayItem(0)->getScalar();
          if( n >= 2 )
            _emission[1] = vec->getArrayItem(1)->getScalar();
          if( n >= 3 )
            _emission[2] = vec->getArrayItem(2)->getScalar();
          if( n >= 4 )
            _emission[3] = vec->getArrayItem(3)->getScalar();
        }
      catch( ... )
        {
        }
    }
  catch( ... )
    {
    }
  try
    {
      vec = obj.getProperty( "specular" );
      try
        {
          n = vec->size();
          if( n >= 1 )
            _specular[0] = vec->getArrayItem(0)->getScalar();
          if( n >= 2 )
            _specular[1] = vec->getArrayItem(1)->getScalar();
          if( n >= 3 )
            _specular[2] = vec->getArrayItem(2)->getScalar();
          if( n >= 4 )
            _specular[3] = vec->getArrayItem(3)->getScalar();
        }
      catch( ... )
        {
        }
    }
  catch( ... )
    {
    }
  try
  {
    vec = obj.getProperty( "unlit_color" );
    try
    {
      n = vec->size();
      if( n >= 1 )
        d->unlitColor[0] = vec->getArrayItem(0)->getScalar();
      if( n >= 2 )
        d->unlitColor[1] = vec->getArrayItem(1)->getScalar();
      if( n >= 3 )
        d->unlitColor[2] = vec->getArrayItem(2)->getScalar();
      if( n >= 4 )
        d->unlitColor[3] = vec->getArrayItem(3)->getScalar();
    }
    catch( ... )
    {
    }
  }
  catch( ... )
  {
  }
  try
    {
      vec = obj.getProperty( "shininess" );
      try
        {
          _shininess = vec->getScalar();
        }
      catch( ... )
        {
        }
    }
  catch( ... )
    {
    }
  try
    {
      vec = obj.getProperty( "lighting" );
      try
        {
          d->renderProps[ RenderLighting ] = (int) vec->getScalar();
        }
      catch( ... )
        {
        }
    }
  catch( ... )
    {
    }
  try
    {
      vec = obj.getProperty( "smooth_shading" );
      try
        {
          d->renderProps[ RenderSmoothShading ] = (int) vec->getScalar();
        }
      catch( ... )
        {
        }
    }
  catch( ... )
    {
    }
  try
    {
      vec = obj.getProperty( "polygon_filtering" );
      try
        {
          d->renderProps[ RenderFiltering ] = (int) vec->getScalar();
        }
      catch( ... )
        {
        }
    }
  catch( ... )
    {
    }
  try
    {
      vec = obj.getProperty( "depth_buffer" );
      try
        {
          d->renderProps[ RenderZBuffer ] = (int) vec->getScalar();
        }
      catch( ... )
        {
        }
    }
  catch( ... )
    {
    }
  try
    {
      vec = obj.getProperty( "polygon_mode" );
      try
        {
          string	m = vec->getString();
          if( m == "normal" )
            d->renderProps[ RenderMode ] = Normal;
          else if( m ==  "wireframe" )
            d->renderProps[ RenderMode ] = Wireframe;
          else if( m ==  "outlined" )
            d->renderProps[ RenderMode ] = Outlined;
          else if( m ==  "hiddenface_wireframe" )
            d->renderProps[ RenderMode ] = HiddenWireframe;
          else if( m ==  "ext_outlined" )
            d->renderProps[ RenderMode ] = ExtOutlined;
          else // default
            d->renderProps[ RenderMode ] = -1;
        }
      catch( ... )
        {
        }
    }
  catch( ... )
    {
    }
  try
    {
      vec = obj.getProperty( "face_culling" );
      try
        {
          d->renderProps[ RenderFaceCulling ] = (int) vec->getScalar();
        }
      catch( ... )
        {
        }
    }
  catch( ... )
    {
    }
  try
  {
    vec = obj.getProperty( "ghost" );
    d->renderProps[ SelectableMode ] = (int) vec->getScalar() == 0 ?
      SelectableWhenOpaque : GhostSelection;
  }
  catch( ... )
  {
  }
  try
  {
    vec = obj.getProperty( "selectable_mode" );
    try
    {
      string        m = vec->getString();
      if( m == "always_selectable" )
        d->renderProps[ SelectableMode ] = AlwaysSelectable;
      else if( m ==  "ghost" )
        d->renderProps[ SelectableMode ] = GhostSelection;
      else if( m ==  "selectable_when_opaque" )
        d->renderProps[ SelectableMode ] = SelectableWhenOpaque;
      else if( m ==  "selectable_when_not_totally_transparent" )
        d->renderProps[ SelectableMode ] = SelectableWhenNotTotallyTransparent;
      else // default
        d->renderProps[ SelectableMode ] = -1;
    }
    catch( ... )
    {
    }
  }
  catch( ... )
    {
    }
  try
  {
    vec = obj.getProperty( "front_face" );
    try
    {
      string        m = vec->getString();
      if( m ==  "cw" || m =="clockwise" )
        d->renderProps[ FrontFace ] = 0;
      else if( m ==  "ccw" || m == "counterclockwise" )
        d->renderProps[ FrontFace ] = 1;
      else // default
        d->renderProps[ RenderMode ] = -1;
    }
    catch( ... )
    {
    }
  }
  catch( ... )
  {
  }
  try
  {
    vec = obj.getProperty( "line_width" );
    float w = vec->getScalar();
    if( w > 0 )
      d->lineWidth = w;
    else
      d->lineWidth = 0;
  }
  catch( ... )
  {
  }
}


Object Material::genericDescription() const
{
  Object	o = Object::value( Dictionary() );

  Object	amb = Object::value( vector<Object>() );
  amb->insertArrayItem( -1, Object::value( _ambient[0] ) );
  amb->insertArrayItem( -1, Object::value( _ambient[1] ) );
  amb->insertArrayItem( -1, Object::value( _ambient[2] ) );
  amb->insertArrayItem( -1, Object::value( _ambient[3] ) );
  o->setProperty( "ambient", amb );

  Object	dif = Object::value( vector<Object>() );
  dif->insertArrayItem( -1, Object::value( _diffuse[0] ) );
  dif->insertArrayItem( -1, Object::value( _diffuse[1] ) );
  dif->insertArrayItem( -1, Object::value( _diffuse[2] ) );
  dif->insertArrayItem( -1, Object::value( _diffuse[3] ) );
  o->setProperty( "diffuse", dif );

  Object	emi = Object::value( vector<Object>() );
  emi->insertArrayItem( -1, Object::value( _emission[0] ) );
  emi->insertArrayItem( -1, Object::value( _emission[1] ) );
  emi->insertArrayItem( -1, Object::value( _emission[2] ) );
  emi->insertArrayItem( -1, Object::value( _emission[3] ) );
  o->setProperty( "emission", emi );

  Object	spe = Object::value( vector<Object>() );
  spe->insertArrayItem( -1, Object::value( _specular[0] ) );
  spe->insertArrayItem( -1, Object::value( _specular[1] ) );
  spe->insertArrayItem( -1, Object::value( _specular[2] ) );
  spe->insertArrayItem( -1, Object::value( _specular[3] ) );
  o->setProperty( "specular", spe );

  Object        unlit = Object::value( vector<Object>() );
  unlit->insertArrayItem( -1, Object::value( d->unlitColor[0] ) );
  unlit->insertArrayItem( -1, Object::value( d->unlitColor[1] ) );
  unlit->insertArrayItem( -1, Object::value( d->unlitColor[2] ) );
  unlit->insertArrayItem( -1, Object::value( d->unlitColor[3] ) );
  o->setProperty( "unlit_color", unlit );

  o->setProperty( "shininess", _shininess );
  if( d->renderProps[ RenderLighting ] >= 0 )
    o->setProperty( "lighting", d->renderProps[ RenderLighting ] );
  if( d->renderProps[ RenderSmoothShading ] >= 0 )
    o->setProperty( "smooth_shading", d->renderProps[ RenderSmoothShading ] );
  if( d->renderProps[ RenderFiltering ] >= 0 )
    o->setProperty( "polygon_filtering", d->renderProps[ RenderFiltering ] );
  if( d->renderProps[ RenderZBuffer ] >= 0 )
    o->setProperty( "depth_buffer", d->renderProps[ RenderZBuffer ] );
  if( d->renderProps[ RenderFaceCulling ] >= 0 )
    o->setProperty( "face_culling", d->renderProps[ RenderFaceCulling ] );
  if( d->renderProps[ RenderMode ] >= 0 )
    {
      static string mode[] = { "normal", "wireframe", "outlined", 
                               "hiddenface_wireframe", "fast", "ext_outlined"
      };
      o->setProperty( "polygon_mode", mode[ d->renderProps[ RenderMode ] ] );
    }
  if( d->renderProps[ SelectableMode ] >= 0 )
  {
    static string mode[] = { "always_selectable", "ghost",
                             "selectable_when_opaque",
                              "selectable_when_not_totally_transparent" };
    o->setProperty( "selectable_mode",
                    mode[ d->renderProps[ SelectableMode ] ] );
  }
  if( d->renderProps[ FrontFace ] >= 0 )
  {
    if( d->renderProps[ FrontFace ] == 0 )
      o->setProperty( "front_face", "clockwise" );
    else
      o->setProperty( "front_face", "counterclockwise" );
  }
  if( d->lineWidth > 0 )
    o->setProperty( "line_width", d->lineWidth );

  return o;
}


int Material::renderProperty( RenderProperty p ) const
{
  return d->renderProps[ p ];
}


void Material::setRenderProperty( RenderProperty p, int x )
{
  if( p != RenderMode && p != SelectableMode )
  {
    if( x < 0 )
      x = -1;
    else if( x > 1 )
      x = 1;
  }
  d->renderProps[ p ] = x;
}


