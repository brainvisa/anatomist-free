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

#include <anatomist/mobject/glmobject.h>
#include <anatomist/window/viewstate.h>
#include <anatomist/reference/Transformation.h>
#include <anatomist/application/Anatomist.h>

using namespace anatomist;
using namespace aims;
using namespace std;


struct GLMObject::Private
{
  Private() : bodyinotherref( false ) {}

  bool bodyinotherref;
};


GLMObject::GLMObject() : MObject(), GLComponent(), d( new Private )
{
}


GLMObject::~GLMObject()
{
  delete d;
}


const Material *GLMObject::glMaterial() const
{
  return &material();
}


const Material & GLMObject::material() const
{
  const GLComponent	*g = glGeometry();
  if( g && g != this )
    return *g->glMaterial();
  else
    return MObject::material();
}


Material & GLMObject::GetMaterial()
{
  const GLComponent     *g = glGeometry();
  if( g && g != this )
    return *const_cast<Material *>( g->glMaterial() );
  else
    return MObject::GetMaterial();
}


const AObjectPalette* GLMObject::glPalette( unsigned tex ) const
{
  const GLComponent	*c = glTexture();
  if( c )
    return c->glPalette( tex );
  return 0;
}


void GLMObject::glClearHasChangedFlags() const
{
  GLComponent::glClearHasChangedFlags();
  MObject::clearHasChangedFlags();
}


void GLMObject::glSetChanged( GLComponent::glPart part, bool x ) const
{
  GLComponent::glSetChanged( part, x );
  if( x )
    obsSetChanged( part );
}


bool GLMObject::render( PrimList & prim, const ViewState & vs )
{
  bool x = AObject::render( prim, vs );
  glClearHasChangedFlags();
  return x;
}


std::string GLMObject::viewStateID( glPart part, 
                                    const ViewState & state ) const
{
  // cout << "GLMObject::viewStateID " << this << ": " << name() << endl;
  switch( part )
    {
    case glGENERAL:
    case glBODY:
      {
        string	s;
        const GLComponent	*c = glGeometry();
        if( c && c != this )
          s = c->viewStateID( part, state );
        c = glTexture();
        if( c && c != this )
          return s + c->viewStateID( part, state );
        return s;
      }
    case glGEOMETRY:
    case glMATERIAL:
      {
        const GLComponent	*c = glGeometry();
        if( c && c != this )
          return c->viewStateID( part, state );
        if( part == glMATERIAL )
          return string();
        float t = state.time;
        if( t > MaxT() )
          t = MaxT();
        string	s;
        s.resize( sizeof(float) );
        (float &) s[0] = t;
        return s;
      }
    case glTEXIMAGE:
    case glTEXENV:
      {
        const GLComponent	*c = glTexture();
        if( c && c != this )
          return c->viewStateID( part, state );
        return string();
      }
    default:
      break;
    }

  return string();
}


bool GLMObject::glToRef( const Referential* objref, GLPrimitives & p ) const
{
  if( objref )
  {
    cout << "GLMObject::glToRef\n";
    const Referential *r = getReferential();
    if( objref != r )
    {
      Transformation *trans = theAnatomist->getTransformation( objref, r );
      if( trans )
      {
        // cout << "apply transf before bodyGLL\n";

        GLfloat	mat[16];

        // write 4x4 matrix in column
        mat[0] = trans->Rotation( 0, 0 );
        mat[1] = trans->Rotation( 1, 0 );
        mat[2] = trans->Rotation( 2, 0 );
        mat[3] = 0;
        mat[4] = trans->Rotation( 0, 1 );
        mat[5] = trans->Rotation( 1, 1 );
        mat[6] = trans->Rotation( 2, 1 );
        mat[7] = 0;
        mat[8] = trans->Rotation( 0, 2 );
        mat[9] = trans->Rotation( 1, 2 );
        mat[10] = trans->Rotation( 2, 2 );
        mat[11] = 0;
        mat[12] = trans->Translation( 0 );
        mat[13] = trans->Translation( 1 );
        mat[14] = trans->Translation( 2 );
        mat[15] = 1;

        GLList *p1 = new GLList;
        p1->generate();
        GLuint l1 = p1->item();

        glNewList( l1, GL_COMPILE );

        glMatrixMode( GL_MODELVIEW );
        glPushMatrix();
        glMultMatrixf( mat );

        glEndList();
        p.push_back( RefGLItem( p1 ) );

        return true;
      }
    }
  }
  return false;
}


// void GLMObject::glBeforeBodyGLL( const ViewState & state, 
// 				 GLPrimitives & pl ) const
// {
//   cout << "GLMObject::glBeforeBodyGLL\n";
//   const SliceViewState  *svs = state.sliceVS();
//   if( !svs || !svs->wantslice )
//   {
//     const AObject *o = dynamic_cast<const AObject *>( glGeometry() );
//     d->bodyinotherref = o && glToRef( o->getReferential(), pl );
//   }
//   else
//     d->bodyinotherref = 0;
// }
// 
// 
// void GLMObject::glAfterBodyGLL( const ViewState &, 
// 				GLPrimitives & pl ) const
// {
//   if( d->bodyinotherref )
//     {
//       // cout << "undo transf after bodyGLL\n";
//       // pop former transformation
//       GLList *p2 = new GLList;
//       p2->generate();
//       GLuint l2 = p2->item();
//       glNewList( l2, GL_COMPILE );
//       //if( !direct )
//       //  glFrontFace( GL_CW );
//       glPopMatrix();
//       glEndList();
//       pl.push_back( RefGLItem( p2 ) );
//     }
// }


unsigned GLMObject::glNumVertex( const ViewState & s ) const
{
  const GLComponent	*g = glGeometry();
  if( g )
    return g->glNumVertex( s );
  return 0;
}


const GLfloat* GLMObject::glVertexArray( const ViewState & s ) const
{
  const GLComponent	*g = glGeometry();
  if( g )
    return g->glVertexArray( s );
  return 0;
}


const GLfloat* GLMObject::glNormalArray( const ViewState & s ) const
{
  const GLComponent	*g = glGeometry();
  if( g )
    return g->glNormalArray( s );
  return 0;
}


unsigned GLMObject::glPolygonSize( const ViewState & s ) const
{
  const GLComponent	*g = glGeometry();
  if( g )
    return g->glPolygonSize( s );
  return 0;
}


unsigned GLMObject::glNumPolygon( const ViewState & s ) const
{
  const GLComponent	*g = glGeometry();
  if( g )
    return g->glNumPolygon( s );
  return 0;
}


const GLuint* GLMObject::glPolygonArray( const ViewState & s ) const
{
  const GLComponent	*g = glGeometry();
  if( g )
    return g->glPolygonArray( s );
  return 0;
}


unsigned GLMObject::glNumTextures() const
{
  const GLComponent	*t = glTexture();
  if( t )
    return t->glNumTextures();
  return 0;
}


unsigned GLMObject::glNumTextures( const ViewState & s ) const
{
  const GLComponent	*t = glTexture();
  if( t )
    return t->glNumTextures( s );
  return 0;
}


unsigned GLMObject::glDimTex( const ViewState & s, unsigned tex ) const
{
  const GLComponent	*t = glTexture();
  if( t )
    return t->glDimTex( s, tex );
  return 0;
}


unsigned GLMObject::glTexCoordSize( const ViewState & s, unsigned tex ) const
{
  const GLComponent	*t = glTexture();
  if( t )
    return t->glTexCoordSize( s, tex );
  return 0;
}


const GLfloat* GLMObject::glTexCoordArray( const ViewState & s, 
                                           unsigned tex ) const
{
  const GLComponent	*t = glTexture();
  if( t )
    return t->glTexCoordArray( s, tex );
  return 0;
}


GLComponent* GLMObject::glGeometry()
{
  iterator	i, e = end();
  GLComponent	*c;

  for( i=begin(); i!=e; ++i )
    {
      c = (*i)->glAPI();
      if( c && c->glNumVertex( 0 ) != 0 )
        return c;
    }
  return 0;
}


const GLComponent* GLMObject::glGeometry() const
{
  const_iterator	i, e = end();
  GLComponent		*c;

  for( i=begin(); i!=e; ++i )
    {
      c = (*i)->glAPI();
      if( c && c->glNumVertex( 0 ) != 0 )
        return c;
    }
  return 0;
}


GLComponent* GLMObject::glTexture( unsigned n )
{
  iterator	i, e = end();
  GLComponent	*c, *t = 0;
  unsigned	cnt = 0;

  for( i=begin(); i!=e && cnt<=n; ++i )
    {
      c = (*i)->glAPI();
      if( c && c->glNumTextures() != 0 )
        {
          ++cnt;
          t = c;
        }
    }
  return t;
}


const GLComponent* GLMObject::glTexture( unsigned n ) const
{
  const_iterator	i, e = end();
  const GLComponent	*c, *t = 0;
  unsigned		cnt = 0;

  for( i=begin(); i!=e && cnt<=n; ++i )
    {
      c = (*i)->glAPI();
      if( c && c->glNumTextures() != 0 )
        {
          ++cnt;
          t = c;
        }
    }
  return t;
}


GLComponent::glTextureMode GLMObject::glTexMode( unsigned tex ) const
{
  const GLComponent	*t = glTexture();
  if( t && t != glAPI() )
    return t->glTexMode( tex );
  return GLComponent::glTexMode( tex );
}


void GLMObject::glSetTexMode( glTextureMode mode, unsigned tex )
{
  GLComponent	*t = glTexture();
  if( t )
    {
      t->glSetTexMode( mode, tex );
      glSetChanged( glTEXENV );
    }
}


GLComponent::glAutoTexturingMode GLMObject::glAutoTexMode( unsigned tex ) const
{
  const GLComponent	*t = glTexture();
  if( t && t != glAPI() )
    return t->glAutoTexMode( tex );
  return GLComponent::glAutoTexMode( tex );
}


void GLMObject::glSetAutoTexMode( glAutoTexturingMode mode, unsigned tex )
{
  GLComponent	*t = glTexture();
  if( t )
    {
      t->glSetAutoTexMode( mode, tex );
      glSetChanged( glTEXENV );
    }
}


const float * GLMObject::glAutoTexParams( unsigned coord, unsigned tex ) const
{
  const GLComponent	*t = glTexture();
  if( t && t != glAPI() )
    return t->glAutoTexParams( coord, tex );
  return GLComponent::glAutoTexParams( coord, tex );
}


void GLMObject::glSetAutoTexParams( const float* params, unsigned coord, 
                                    unsigned tex )
{
  GLComponent	*t = glTexture();
  if( t )
    {
      t->glSetAutoTexParams( params, coord, tex );
      glSetChanged( glTEXENV );
    }
}


float GLMObject::glTexRate( unsigned tex ) const
{
  const GLComponent	*t = glTexture();
  if( t && t != glAPI() )
    return t->glTexRate( tex );
  return GLComponent::glTexRate( tex );
}


void GLMObject::glSetTexRate( float rate, unsigned tex )
{
  GLComponent	*t = glTexture();
  if( t )
    {
      t->glSetTexRate( rate, tex );
      glSetChanged( glTEXENV );
    }
}


GLComponent::glTextureFiltering GLMObject::glTexFiltering( unsigned tex ) const
{
  const GLComponent	*t = glTexture();
  if( t && t != glAPI() )
    return t->glTexFiltering( tex );
  return GLComponent::glTexFiltering( tex );
}


void GLMObject::glSetTexFiltering( glTextureFiltering x, unsigned tex )
{
  GLComponent	*t = glTexture();
  if( t )
    {
      t->glSetTexFiltering( x, tex );
      glSetChanged( glTEXENV );
    }
}


bool GLMObject::glTexRGBInterpolation( unsigned tex ) const
{
  const GLComponent	*t = glTexture();
  if( t && t != glAPI() )
    return t->glTexRGBInterpolation( tex );
  return GLComponent::glTexRGBInterpolation( tex );
}


void GLMObject::glSetTexRGBInterpolation( bool x, unsigned tex )
{
  GLComponent	*t = glTexture();
  if( t )
    {
      if( t != this )
        t->glSetTexRGBInterpolation( x, tex );
      else
        GLComponent::glSetTexRGBInterpolation( x, tex );
      glSetChanged( glTEXENV );
      glSetChanged( glBODY );
    }
}


bool GLMObject::glTexImageChanged( unsigned tex ) const
{
  const GLComponent	*t = glTexture();
  if( t && t != glAPI() )
    return t->glTexImageChanged( tex );
  return GLComponent::glTexImageChanged( tex );
}


void GLMObject::glSetTexImageChanged( bool x, unsigned tex ) const
{
  /* cout << "GLMObject::glSetTexImageChanged " << x << " " << tex 
     << " this: " << this << " " << endl; */
  GLComponent::glSetTexImageChanged( x, tex );
  // GLComponent::glSetChanged( glTEXIMAGE, x );
  if( x )
    obsSetChanged( glTEXIMAGE_NUM + tex * 2 );
}


bool GLMObject::glTexEnvChanged( unsigned tex ) const
{
  const GLComponent	*t = glTexture();
  if( t && t != glAPI() )
    return t->glTexEnvChanged( tex );
  return GLComponent::glTexEnvChanged( tex );
}


void GLMObject::glSetTexEnvChanged( bool x, unsigned tex ) const
{
  GLComponent::glSetTexEnvChanged( x, tex );
  // GLComponent::glSetChanged( glTEXENV, x );
  if( x )
    obsSetChanged( glTEXENV_NUM + tex * 2 );
}


bool GLMObject::glMakeTexImage( const ViewState & state, 
                                const GLTexture & gltex, unsigned tex ) const
{
  const GLComponent	*t = glTexture();
  if( t && t != glAPI() )
    {
      bool	r = t->glMakeTexImage( state, gltex, tex );
      glSetTexImageChanged( false, tex );
      return r;
    }
  return GLComponent::glMakeTexImage( state, gltex, tex );
}


bool GLMObject::glMakeTexEnvGLL( const ViewState & state, 
                                 const GLList & gllist, unsigned tex ) const
{
  const GLComponent	*t = glTexture();
  if( t && t != glAPI() )
    {
      bool	r = t->glMakeTexEnvGLL( state, gllist, tex );
      glSetTexEnvChanged( false, tex );
      return r;
    }
  return GLComponent::glMakeTexEnvGLL( state, gllist, tex );
}


void GLMObject::glSetMaterialGLL( const std::string & state, RefGLItem x )
{
  GLComponent	*g = glGeometry();
  if( g && g != glAPI() )
    g->glSetMaterialGLL( state, x );
  else
    GLComponent::glSetMaterialGLL( state, x );
}


void GLMObject::glSetTexNameGLL( const std::string & state, RefGLItem x, 
                                 unsigned tex )
{
  GLComponent	*t = glTexture();
  if( t && t != glAPI() )
    t->glSetTexNameGLL( state, x, tex );
  else
    GLComponent::glSetTexNameGLL( state, x, tex );
}


GLPrimitives GLMObject::glMaterialGLL( const ViewState & state ) const
{
  const GLComponent	*c = glGeometry();
  if( c && c != glAPI() )
    {
      GLPrimitives	r = c->glMaterialGLL( state );
      glSetChanged( glMATERIAL, false );
      return r;
    }
  return GLComponent::glMaterialGLL( state );
}


GLPrimitives GLMObject::glTexNameGLL( const ViewState & state, 
                                      unsigned tex ) const
{
  /* cout << "GLMObject::glTexNameGLL for tex " << tex << "in " 
     << (const GLComponent *) this << endl; */
  const GLComponent	*t = glTexture();
  if( t && t != glAPI() )
    {
      GLPrimitives	r = t->glTexNameGLL( state, tex );
      glSetTexImageChanged( false, tex );
      return r;
    }
  return GLComponent::glTexNameGLL( state, tex );
}


GLPrimitives GLMObject::glTexEnvGLL( const ViewState & state, 
                                     unsigned tex ) const
{
  const GLComponent	*t = glTexture();
  if( t && t != glAPI() )
    {
      GLPrimitives	r = t->glTexEnvGLL( state, tex );
      glSetTexEnvChanged( false, tex );
      return r;
    }
  return GLComponent::glTexEnvGLL( state, tex );
}


void GLMObject::glGarbageCollector( int nkept )
{
  GLComponent::glGarbageCollector( nkept );

  GLComponent	*c = glGeometry();
  if( c && c != glAPI() )
    c->glGarbageCollector( nkept );
  c = glTexture();
  if( c && c != glAPI() )
    c->glGarbageCollector( nkept );
}


void GLMObject::update( const Observable* obs, void* arg )
{
  /* cout << "GLMObject::update: " << this << ", obs: " << obs << ", " << arg 
       << endl; */
  const AObject	*o = dynamic_cast<const AObject*>( obs );
  if( o )
    {
      //cout << "AObject\n";
      const GLComponent	*c = o->glAPI();
      if( c )
        {
          //cout << "GLComponent\n";
          if( o->obsHasChanged( glGEOMETRY ) )
            glSetChanged( glGEOMETRY );
          if( o->obsHasChanged( glBODY ) )
            glSetChanged( glBODY );
          if( o->obsHasChanged( glMATERIAL ) )
            glSetChanged( glMATERIAL );
          if( o->obsHasChanged( glTEXIMAGE ) )
            glSetChanged( glTEXIMAGE );
          if( o->obsHasChanged( glTEXENV ) )
            glSetChanged( glTEXENV );
        }
    }
  MObject::update( obs, arg );
}


AObjectPalette* GLMObject::palette()
{
  GLComponent	*tx = glTexture();
  if( !tx || tx == glAPI() )
    return _palette;
  iterator	i, e = end();
  for( i=begin(); i!=e; ++i )
    if( tx == (*i)->glAPI() )
      return (*i)->palette();
  return MObject::palette();
}


const AObjectPalette* GLMObject::palette() const
{
  return glPalette();
}
