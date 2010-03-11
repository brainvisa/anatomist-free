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

#include <anatomist/surface/glcomponent.h>
#include <anatomist/color/Material.h>
#include <anatomist/color/objectPalette.h>
#include <anatomist/window/viewstate.h>
#include <anatomist/window/glcaps.h>
#include <anatomist/window/Window.h>
#include <anatomist/reference/Transformation.h>
#include <anatomist/application/Anatomist.h>
#include <aims/data/data.h>
#include <aims/rgb/rgb.h>
#include <map>
#include <vector>

// uncomment this to allow lots of output messages about GL lists
//#define ANA_DEBUG_GLLISTS

using namespace anatomist;
using namespace carto;
using namespace std;

namespace
{

  struct TexInfo
  {
    TexInfo();

    GLComponent::glTextureMode		mode;
    GLComponent::glTextureFiltering	filter;
    GLComponent::glAutoTexturingMode	automode;
    float				rate;
    bool				changed;
    bool				envchanged;
    map<string, RefGLItem>		name;
    map<string, RefGLItem>		texenv;
    GLComponent::TexExtrema		extrema;
    bool				rgbinterp;
    float				texgenparams_object[3][4];
    float				texgenparams_eye[3][4];
  };

}


struct GLComponent::Private
{
  Private();
  ~Private();

  int id() const
  {
    if( globjectid >= 0 )
      return globjectid;
    return generateID();
  }
  int generateID() const;

  void makeRGBTex();

  mutable vector<bool>		changed;
  vector<TexInfo>		textures;

  map<string, RefGLItem>	bodyGLL;
  map<string, RefGLItem>	materialGLL;
  map<string, GLPrimitives>	mainGLL;
  unsigned			memory;
  int				renderProps;
  unsigned			ntex;

  static GLTexture		& rgtex();
  static GLTexture		& batex();
  mutable int                   globjectid;
  static set<int> & usedIDs();
};


GLComponent::Private::Private()
  : changed( GLComponent::glNOPART ), memory( 4 ), ntex( 0 ), globjectid( -1 )
{
}


GLComponent::Private::~Private()
{
  if( globjectid >= 0 )
    usedIDs().erase( globjectid );
}


set<int> & GLComponent::Private::usedIDs()
{
  static set<int> ids;
  return ids;
}


int GLComponent::Private::generateID() const
{
  set<int> & ids = usedIDs();
  if( ids.empty() )
  {
    ids.insert( 0 );
    globjectid = 0;
    return 0;
  }
  if( ids.size() == (unsigned) *ids.rbegin() + 1 )
  {
    int id = ids.size();
    ids.insert( id );
    globjectid = id;
    return id;
  }
  int id = 0;
  set<int>::iterator is, es = ids.end();
  for( is=ids.begin(); is!=es; ++is, ++id )
    if( *is != id )
      break;
  ids.insert( id );
  globjectid = id;
  return id;
}


GLTexture & GLComponent::Private::rgtex()
{
  static GLTexture		*rgt = 0;
  if( !rgt )
    {
      GLfloat	rg[ 65536*4 ];
      GLfloat	*p = rg;
      unsigned	i, j;
      for( j=0; j<256; ++j )
        for( i=0; i<256; ++i )
          {
            *p++ = GLfloat(i) / 255;
            *p++ = GLfloat(j) / 255;
            *p++ = 1.;
            *p++ = 1.;
          }

      GLCaps::glActiveTexture( GLCaps::textureID( 0 ) );
      GLenum status = glGetError();
      if( status != GL_NO_ERROR )
        cerr << "GLComponent::rgtex : OpenGL error 1: " 
             << gluErrorString(status) << endl;

      rgt = new GLTexture;
      rgt->generate();
#ifdef ANA_DEBUG_GLLISTS
      cout << "R/G texture name: " << rgt->item() << endl;
#endif
      glBindTexture( GL_TEXTURE_2D, rgt->item() );
      status = glGetError();
      if( status != GL_NO_ERROR )
        cerr << "GLComponent::rgtex : OpenGL error 2: " 
             << gluErrorString(status) << endl;
      glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
      glTexImage2D( GL_TEXTURE_2D, 0, 4, 256, 256, 0, GL_RGBA, 
		    GL_FLOAT, (GLvoid*) rg );
     }
  return *rgt;
}


GLTexture &  GLComponent::Private::batex()
{
  static GLTexture		*bt = 0;
  if( !bt )
    {
      GLfloat	b[ 65536*4 ];
      GLfloat	*p = b;
      unsigned	i, j;
      for( j=0; j<256; ++j )
        for( i=0; i<256; ++i )
          {
            *p++ = 1.;
            *p++ = 1.;
            *p++ = GLfloat(i) / 255;
            *p++ = GLfloat(j) / 255;
          }

      GLCaps::glActiveTexture( GLCaps::textureID( 1 ) );
      GLenum status = glGetError();
      if( status != GL_NO_ERROR )
        cerr << "GLComponent::batex : OpenGL error 1: " 
             << gluErrorString(status) << endl;

      bt = new GLTexture;
      bt->generate();
#ifdef ANA_DEBUG_GLLISTS
      cout << "B/A texture name: " << bt->item() << endl;
#endif
      glBindTexture( GL_TEXTURE_2D, bt->item() );
      status = glGetError();
      if( status != GL_NO_ERROR )
        cerr << "GLComponent::batex : OpenGL error 2: " 
             << gluErrorString(status) << endl;
      glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
      glTexImage2D( GL_TEXTURE_2D, 0, 4, 256, 256, 0, GL_RGBA, 
		    GL_FLOAT, (GLvoid*) b );
    }
  return *bt;
}


TexInfo::TexInfo()
  : mode( GLComponent::glGEOMETRIC ), filter( GLComponent::glFILT_NEAREST ), 
    automode( GLComponent::glTEX_MANUAL ), rate( 1. ), 
    changed( true ), envchanged( true ), rgbinterp( false )
{
  texgenparams_object[0][0] = 0.01;
  texgenparams_object[0][1] = 0;
  texgenparams_object[0][2] = 0;
  texgenparams_object[0][3] = 0;
  texgenparams_object[1][0] = 0;
  texgenparams_object[1][1] = 0.01;
  texgenparams_object[1][2] = 0;
  texgenparams_object[1][3] = 0;
  texgenparams_object[2][0] = 0;
  texgenparams_object[2][1] = 0;
  texgenparams_object[2][2] = 0.01;
  texgenparams_object[2][3] = 0;
  texgenparams_eye[0][0] = 0.01;
  texgenparams_eye[0][1] = 0;
  texgenparams_eye[0][2] = 0;
  texgenparams_eye[0][3] = 0;
  texgenparams_eye[1][0] = 0;
  texgenparams_eye[1][1] = 0.01;
  texgenparams_eye[1][2] = 0;
  texgenparams_eye[1][3] = 0;
  texgenparams_eye[2][0] = 0;
  texgenparams_eye[2][1] = 0;
  texgenparams_eye[2][2] = 0.01;
  texgenparams_eye[2][3] = 0;
}


GLComponent::TexExtrema::TexExtrema()
  : scaled( false )
{
}


// -------------------


GLComponent::GLComponent() : d( new GLComponent::Private )
{
  unsigned	i;
  for( i=0; i<glNOPART; ++i )
    d->changed[i] = true;
}


GLComponent::~GLComponent()
{
  delete d;
}


void GLComponent::glClearHasChangedFlags() const
{
  unsigned	i, n = glNOPART;
  for( i=0; i<n; ++i )
    d->changed[i] = false;
}


void GLComponent::glSetChanged( glPart p, bool x ) const
{
  // cout << "GLComponent::glSetChanged( " << p << ")\n";
  d->changed[p] = x;
  if( x && p != glGENERAL )
    {
      if( p == glGEOMETRY )
        d->changed[ glBODY ] = x;
      d->changed[ glGENERAL ] = x;
    }
}


bool GLComponent::glHasChanged( glPart p ) const
{
  return d->changed[p];
}


unsigned GLComponent::glNumTextures() const
{
  return d->textures.size();
}


unsigned GLComponent::glNumTextures( const ViewState & ) const
{
  return d->textures.size();
}


GLComponent::glTextureMode GLComponent::glTexMode( unsigned tex ) const
{
  return d->textures[tex].mode;
}


void GLComponent::glSetTexMode( GLComponent::glTextureMode mode, unsigned tex )
{
  d->textures[ tex ].mode = mode;
  if( mode == glLINEAR )
    glSetTexImageChanged( true, tex );
  else
    glSetTexEnvChanged( true, tex );
}


GLComponent::glAutoTexturingMode 
GLComponent::glAutoTexMode( unsigned tex ) const
{
  return d->textures[tex].automode;
}


void GLComponent::glSetAutoTexMode( GLComponent::glAutoTexturingMode mode, 
                                    unsigned tex )
{
  d->textures[ tex ].automode = mode;
  glSetTexEnvChanged( true, tex );
}


float GLComponent::glTexRate( unsigned tex ) const
{
  return d->textures[ tex ].rate;
}


void GLComponent::glSetTexRate( float rate, unsigned tex )
{
  d->textures[ tex ].rate = rate;
  //if( d->textures[ tex ].mode == glLINEAR )
  glSetTexImageChanged( true, tex );
}


GLComponent::glTextureFiltering 
GLComponent::glTexFiltering( unsigned tex ) const
{
  return d->textures[ tex ].filter;
}


void GLComponent::glSetTexFiltering( GLComponent::glTextureFiltering x, 
                                     unsigned tex )
{
  d->textures[ tex ].filter = x;
  glSetTexEnvChanged( true, tex );
}


GLint GLComponent::glGLTexMode( unsigned tex ) const
{
  static GLint	mode[] = 
    { 
      GL_MODULATE, 
      GL_DECAL, 
      GL_REPLACE, 
      GL_DECAL, 
      GL_BLEND, 
#ifdef GL_ADD
      GL_ADD,
#else
      GL_MODULATE,
#endif 
#ifdef GL_COMBINE
      GL_COMBINE, 
#else
      GL_MODULATE,
#endif 
    };
  return mode[ glTexMode( tex ) ];
}


GLint GLComponent::glGLTexFiltering( unsigned tex ) const
{
  static GLint	mode[] = { GL_NEAREST, GL_LINEAR };
  return mode[ glTexFiltering( tex ) ];
}


GLPrimitives GLComponent::glMainGLL( const ViewState & state )
{
  // cout << "GLComponent::glMainGLL in " << this << endl;
  string s = viewStateID( glGENERAL, state );
  GLPrimitives	p;
  bool		changed = glHasChanged( glGENERAL );
  if( !changed )
    {
      map<string, GLPrimitives>::const_iterator	i = d->mainGLL.find( s );
      if( i != d->mainGLL.end() )
        {
          p = i->second;
          // cout << "main list " << i->first << " in cache\n";
          return p;
        }
    }

  // run garbage collector
  glGarbageCollector();

  // clear lists that have changed
  if( changed )
    d->mainGLL.clear();

  // rebuild lists
  GLItemList	*noexec = new GLNoExecItemList;
  unsigned	it, nt = glNumTextures( state );
  unsigned	ntexunits = GLCaps::numTextureUnits();

  // cout << "texture units: " << ntexunits << endl;
  if( nt > ntexunits )
    nt = ntexunits;

  for( it=0; it<nt; ++it )
    {
      //cout << "texName " << it << "...\n";
      GLPrimitives	tp = glTexNameGLL( state, it );
      //cout << "TexName: " << tp.size() << " items\n";
      noexec->items.insert( noexec->items.end(), tp.begin(), tp.end() );
    }

  GLPrimitives	mp;
  mp = glMaterialGLL( state );
  noexec->items.insert( noexec->items.end(), mp.begin(), mp.end() );

  GLPrimitives	tp;
  for( it=0; it<ntexunits; ++it )
    {
      GLPrimitives	tp2 = glTexEnvGLL( state, it );
      //cout << "TexEnv: " << tp2.size() << " items\n";
      if( !tp2.empty() )
        {
          tp.push_back( tp2.front() );
          noexec->items.insert( noexec->items.end(), tp2.begin(), tp2.end() );
        }
    }
  GLPrimitives	bp;
  glBeforeBodyGLL( state, bp );
  {
    GLPrimitives bp2 = glBodyGLL( state );
    bp.insert( bp.end(), bp2.begin(), bp2.end() );
  }
  glAfterBodyGLL( state, bp );
  noexec->items.insert( noexec->items.end(), bp.begin(), bp.end() );

  GLPrimitives::iterator	ip, ep;

  // make main list
  //cout << "making main list\n";
  GLList	*ml = new GLList;
  ml->generate();
  glNewList( ml->item(), GL_COMPILE );
  if( !mp.empty() )
    {
#ifdef ANA_DEBUG_GLLISTS
      cout << "in main list " << ml->item() << ": mat: " 
           << ((GLList &) *mp.front()).item() 
           << endl;
#endif
      mp.front()->callList();
    }
  for( ip=tp.begin(), ep=tp.end(); ip!=ep; ++ip )
    {
#ifdef ANA_DEBUG_GLLISTS
      cout << "in main list " << ml->item() << ": texenv: " 
         << ((GLList &) **ip).item() << endl;
#endif
      (*ip)->callList();
    }
  for( ip=bp.begin(), ep=bp.end(); ip!=ep; ++ip )
    {
#ifdef ANA_DEBUG_GLLISTS
      cout << "in main list " << ml->item() << ": body: " 
           << ((GLList & ) **ip).item() << endl;
#endif
      (*ip)->callList();
    }
  if( !mp.empty() )
    {
      const Material	*mat = glMaterial();
      if( mat )
        {
          if( mat->renderProperty( Material::Ghost ) > 0 )
            ml->setGhost( true );
          bool	rendertwice = false;

          switch( mat->renderProperty( Material::RenderMode ) )
            {
            case Material::HiddenWireframe:
              glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
              glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
              rendertwice = true;
              break;
            case Material::Outlined:
              glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
              glDisable( GL_LIGHTING );
              {
                GLfloat *unlit = mat->unlitColor();
                glColor4f( unlit[0], unlit[1], unlit[2], unlit[3] );
              }
              rendertwice = true;
              break;
            case Material::ExtOutlined:
              glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
              glDisable( GL_LIGHTING );
              glPolygonOffset( 15, 15 );
              if( mat->lineWidth() > 0 )
                glLineWidth( mat->lineWidth() + 5 );
              else
                glLineWidth( 5. );
              glEnable( GL_POLYGON_OFFSET_LINE );
              {
                const GLfloat * c = mat->unlitColor();
                glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, c );
                glColor4f( c[0], c[1], c[2], c[3] );
              }

              rendertwice = true;
            default:
              break;
            }

          if( rendertwice )
            for( ip=bp.begin(), ep=bp.end(); ip!=ep; ++ip )
              (*ip)->callList();

          mat->popGLState();
        }
    }
  glEndList();

  RefGLItem	rl( ml );
  p.push_front( rl );
  if( !noexec->items.empty() )
    p.push_back( RefGLItem( noexec ) );
  d->mainGLL[ s ] = p;
  if( changed )
    glClearHasChangedFlags();

  return p;
}


GLPrimitives GLComponent::glBodyGLL( const ViewState & state ) const
{
  // cout << "GLComponent::glBodyGLL in " << this << endl;
  string s = viewStateID( glBODY, state );
  GLPrimitives	p;
  bool	changed = glHasChanged( glBODY );
  if( !changed )
    {
      map<string, RefGLItem>::const_iterator	i = d->bodyGLL.find( s );
      if( i != d->bodyGLL.end() )
        {
          p.push_back( i->second );
          return p;
        }
    }

  if( changed )
    d->bodyGLL.clear();

  GLList	*l = new GLList;
  l->generate();

  if( !glMakeBodyGLL( state, *l ) )
    {
      delete l;
      l = new GLList;
      l->generate();
    }

  RefGLItem	rl( l );
  d->bodyGLL[ s ] = rl;

  if( l->item() )
    p.push_back( rl );
  if( changed )
    d->changed[ glBODY ] = false;
  return p;
}


GLPrimitives GLComponent::glTexEnvGLL( const ViewState & state, 
                                       unsigned tex ) const
{
  // cout << "GLComponent::glTexEnvGLL tex " << tex << " in " << this << endl;
  GLPrimitives		p;
  unsigned		nt = glNumTextures( state );
  unsigned		texid, i, ntu = GLCaps::numTextureUnits();

  for( texid=0, i=0; i<tex && i<nt && texid<ntu; ++i, ++texid )
    if( glTexRGBInterpolation( i ) )
      ++texid;

  if( tex >= nt )
    texid += tex - nt;

  if( texid >= ntu )
    return p;

  if( tex >= nt )
    {
      GLList	*l = new GLList;
      l->generate();

      // cout << "disabling texture " << tex << endl;
      glNewList( l->item(), GL_COMPILE );
      GLCaps::glActiveTexture( GLCaps::textureID( texid ) );
      glDisable( GL_TEXTURE_1D );
      glDisable( GL_TEXTURE_2D );
      glDisable( GL_TEXTURE_3D );
      glEndList();

      RefGLItem	rl( l );
      p.push_back( rl );
      return p;
    }

  string s = viewStateID( glTEXENV, state );
  TexInfo	& t = d->textures[ tex ];
  bool	changed = glTexEnvChanged( tex );
  if( !changed )
    {
      map<string, RefGLItem>::const_iterator	i = t.texenv.find( s );
      if( i != t.texenv.end() )
        {
          p.push_back( i->second );
          // cout << "cached\n";
          return p;
        }
    }
  if( changed )
    t.texenv.clear();

  GLList	*l = new GLList;
  l->generate();

  if( !glMakeTexEnvGLL( state, *l, tex ) )
    {
      cerr << "TexEnv failed\n";
      delete l;
      l = new GLList;
    }

  RefGLItem	rl( l );
  GLItemList	*el = new GLItemList;
  el->items.push_back( rl );

  RefGLItem	rel = RefGLItem( el );

  // keep a ref to the texture itself
  GLPrimitives	tx = glTexNameGLL( state, tex );
  if( !tx.empty() )
    {
      GLNoExecItemList	*ne = new GLNoExecItemList;
      GLPrimitives::iterator	ip, ep = tx.end();
      el->items.push_back( RefGLItem( ne ) );
      for( ip=tx.begin(); ip!=ep; ++ip )
        ne->items.push_back( *ip );
    }
  t.texenv[ s ] = rel;

  if( l->item() )
    p.push_back( rel );
  if( changed )
    t.envchanged = false;
  return p;
}


GLPrimitives GLComponent::glMaterialGLL( const ViewState & state ) const
{
  // cout << "GLComponent::glMaterialGLL in " << this << endl;
  string 		s = viewStateID( glMATERIAL, state );
  GLPrimitives		p;
  bool			changed = glHasChanged( glMATERIAL );
  const Material	*mat = glMaterial();
  if( !mat )
    {
      if( changed )
        d->changed[ glMATERIAL ] = false;
      return p;
    }
  if( !changed )
    {
      map<string, RefGLItem>::const_iterator i = d->materialGLL.find( s );
      if( i != d->materialGLL.end() )
        {
          p.push_back( i->second );
          return p;
        }
    }

  if( changed )
    d->materialGLL.clear();

  GLList	*l( new GLList );
  l->generate();
  glNewList( l->item(), GL_COMPILE );
  mat->setGLMaterial();
  /*
  if( !glNormalArray( state ) )
    glColor4f( mat->Diffuse(0), mat->Diffuse(1), mat->Diffuse(2), 
               mat->Diffuse(3) );
  */
  glEndList();

  RefGLItem	rl( l );
  d->materialGLL[ s ] = rl;
  p.push_back( rl );
  if( changed )
    d->changed[ glMATERIAL ] = false;
  return p;
}


GLPrimitives GLComponent::glTexNameGLL( const ViewState & state, 
                                        unsigned tex ) const
{
  /* cout << "GLComponent::glTexNameGLL for tex " << tex << " in " << this 
     << endl; */
  GLPrimitives	p;
  bool		changed;
  string	s = viewStateID( glTEXIMAGE, state );

  TexInfo	& t = d->textures[tex];
  changed = glTexImageChanged( tex );
  if( !changed )
    {
      map<string, RefGLItem>::const_iterator 
        i = t.name.find( s );
      if( i != t.name.end() )
        {
          if( ((GLTexture &) *i->second).item() != 0 )
            p.push_back( i->second );
          // cout << "cached: " << ((GLTexture &) *i->second).item() << "\n";
          return p;
        }
    }

  if( changed )
    {
      t.name.clear();
      t.texenv.clear();
    }

  //glSetTexEnvChanged( tex );
  t.envchanged = true;

  GLTexture	*l( new GLTexture );
  l->generate();

  if( !glMakeTexImage( state, *l, tex ) )
    {
      cerr << "TexImage failed\n";
      delete l;
      l = new GLTexture;
    }

  RefGLItem	rl( l );
  t.name[ s ] = rl;
  if( l->item() )
    p.push_back( rl );
  if( changed )
    //glSetTexImageChanged( false, tex );
    t.changed = false;

  return p;
}


void GLComponent::glSetMainGLL( const string & state, GLPrimitives x )
{
  d->mainGLL[ state ] = x;
}


void GLComponent::glSetBodyGLL( const string & state, RefGLItem x )
{
  d->bodyGLL[ state ] = x;
}


void GLComponent::glSetMaterialGLL( const string & state, RefGLItem x )
{
  d->materialGLL[ state ] = x;
}


void GLComponent::glSetTexNameGLL( const string & state, RefGLItem x, 
                                   unsigned tex )
{
  d->textures[ tex ].name[ state ] = x;
}


bool GLComponent::glTexImageChanged( unsigned tex ) const
{
  return d->textures[ tex ].changed;
}


void GLComponent::glSetTexImageChanged( bool x, unsigned tex ) const
{
  /* cout << "GLComponent::glSetTexImageChanged " << x << " " << tex << endl;
     cout << "numTex: " << glNumTextures() << ", this: " << this << endl; */
  if( tex >= d->textures.size() )
    return;
  d->textures[ tex ].changed = x;
  if( x )
    {
      glSetChanged( glTEXIMAGE );
      if( tex < d->textures.size() )
        d->textures[ tex ].envchanged = x;
      if( glTexRGBInterpolation( tex ) )
        {
          glSetChanged( glTEXENV );
          glSetChanged( glBODY );
        }
      /* cout << "GLComponent::glSetTexImageChanged -> texEnv too: " << this 
         << endl; */
    }
}


bool GLComponent::glTexEnvChanged( unsigned tex ) const
{
  /* cout << "GLComponent::glTexEnvChanged( " << tex 
     << " )" << endl; */
  return d->textures[ tex ].envchanged;
}


void GLComponent::glSetTexEnvChanged( bool x, unsigned tex ) const
{
  if( tex < d->textures.size() )
    d->textures[ tex ].envchanged = x;
  if( x )
    glSetChanged( glTEXENV );
}


void GLComponent::glAddTextures( unsigned ntex )
{
  d->textures.insert( d->textures.end(), ntex, TexInfo() );
  d->ntex += ntex;
}


bool GLComponent::glMakeTexImage( const ViewState & state, 
                                  const GLTexture & gltex, unsigned tex ) const
{
  // cout << "GLComponent::glMakeTexImage tex " << tex << " for " << this << endl;
  unsigned	nt = glNumTextures( state ), texu = tex;
  if( tex >= nt )
    tex = nt - 1;

  const AObjectPalette		*objpal = glPalette( tex );
  if( !objpal )
    return false;

  const AimsData<AimsRGBA>	*cols = objpal->colors();
  float		min = objpal->min1(), max = objpal->max1();
  float		min2 = objpal->min2(), max2 = objpal->max2();
  unsigned	dimx, dimy, x, y;
  unsigned	dimpx = cols->dimX(), dimpy = cols->dimY(), utmp;
  int		xs, ys;
  unsigned	dimtex = glDimTex( state, tex );
  TexInfo	& t = d->textures[ tex ];

  dimx = dimpx;
  dimy = dimpy;
  if( objpal->glMaxSizeX() > 0 )
    dimx = objpal->glMaxSizeX();
  if( objpal->glMaxSizeY() > 0 )
    dimy = objpal->glMaxSizeY();
  // texture dims must be a power of 2
  for( x=0, utmp=1; x<32 && utmp<dimx; ++x )
    utmp = utmp << 1;
  dimx = utmp;
  for( x=0, utmp=1; x<32 && utmp<dimy; ++x )
    utmp = utmp << 1;
  dimy = utmp;
  if( dimx == 0 )
    dimx = 1;
  if( dimy == 0 )
    dimy = 1;

  //	set up GL texture map
  GLuint	texName = gltex.item();
  //cout << "texName: " << texName << endl;
  GLCaps::glActiveTexture( GLCaps::textureID( texu ) );
  GLenum status = glGetError();
  if( status != GL_NO_ERROR )
  {
    //if( status != GL_STACK_UNDERFLOW )
    cerr << "GLComponent::glMakeTexImage : OpenGL error 1: "
        << gluErrorString(status) << endl;
    return false;
  }

  if( dimtex == 1 )
  {
    glBindTexture( GL_TEXTURE_1D, texName );
    status = glGetError();
    if( status != GL_NO_ERROR )
    {
      cerr << "GLComponent::glMakeTexImage : OpenGL error 2: "
          << gluErrorString(status) << endl;
      return false;
    }
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
  }
  else if( dimtex == 2 )
  {
    glBindTexture( GL_TEXTURE_2D, texName );
    status = glGetError();
    if( status != GL_NO_ERROR )
    {
      cerr << "GLComponent::glMakeTexImage : OpenGL error 3: "
          << gluErrorString(status) << endl;
      return false;
    }
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
  }
  bool ok = true;
  do
  {
    GLint w;
    if( dimtex == 1 )
    {
      glTexImage1D( GL_PROXY_TEXTURE_1D, 0, 4, dimx, 0, GL_RGBA,
                    GL_FLOAT, 0 );
      glGetTexLevelParameteriv( GL_PROXY_TEXTURE_1D, 0, GL_TEXTURE_WIDTH, &w );
    }
    else if( dimtex == 2 )
    {
      glTexImage2D( GL_PROXY_TEXTURE_2D, 0, 4, dimx, dimy, 0, GL_RGBA,
                    GL_FLOAT, 0 );
      glGetTexLevelParameteriv( GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w );
    }
    if( w == 0 || status != GL_NO_ERROR )
    {
      cout << "texture too large. Reducing\n";
      if( dimx > dimy )
        dimx /= 2;
      else
        dimy /= 2;
      ok = false;
    }
    else
      ok = true;
  }
  while( !ok && dimx > 0 && dimy > 0 );
  if( dimx == 0 || dimy == 0 )
  {
    cerr << "texture proxy failed whatever its size\n";
    return false;
  }

  if( min == max )
    {
      min = 0;
      max = 1;
    }
  if( min2 == max2 )
    {
      min2 = 0;
      max2 = 1;
    }
  float	facx = ( (float) dimpx ) / ( ( max - min ) * dimx );
  float	facy = ( (float) dimpy ) / ( ( max2 - min2 ) * dimy );
  float	dx = min * cols->dimX() / ( max - min );
  float	dy = min2 * cols->dimY() / ( max2 - min2 );

  // allocate colormap
  GLfloat	*texImage = new GLfloat[ dimx*dimy*4 ];
  GLfloat	*ptr = texImage;
  AimsRGBA	rgb;
  bool		rgbi = glTexRGBInterpolation( tex );
  float		r = t.rate;
  if( rgbi )
    r = 1.;
  float		ir = 1. - r;

  for( y=0; y<dimy; ++y )
    {
      ys = (int) ( facy * y - dy );
      if( ys < 0 )
	ys = 0;
      else if( ys >= (int) dimpy )
	ys = dimpy - 1;
      for( x=0; x<dimx; ++x )
	{
	  xs = (int) ( facx * x - dx );
	  if( xs < 0 )
	    xs = 0;
	  else if( xs >= (int) dimpx )
	    xs = dimpx - 1;

	  rgb = (*cols)( xs, ys );
          if( t.mode == glLINEAR )
            {
              *ptr++ = (GLfloat) rgb.red()   / 255;
              *ptr++ = (GLfloat) rgb.green() / 255;
              *ptr++ = (GLfloat) rgb.blue()  / 255;
              *ptr++ = ( (GLfloat) rgb.alpha() / 255 ) * r;
            }
          else
            {
              *ptr++ = ( (GLfloat) rgb.red()   / 255 ) * r + ir;
              *ptr++ = ( (GLfloat) rgb.green() / 255 ) * r + ir;
              *ptr++ = ( (GLfloat) rgb.blue()  / 255 ) * r + ir;
              *ptr++ = (GLfloat) rgb.alpha() / 255;
            }
	}
    }
  /* cout << "texture : " << dimx << " x " << dimy << ", dim: "
     << dimtex << endl; */

  if( dimtex == 1 )
    glTexImage1D( GL_TEXTURE_1D, 0, 4, dimx, 0, GL_RGBA,
                  GL_FLOAT, (GLvoid*) texImage );
  else if( dimtex == 2 )
    glTexImage2D( GL_TEXTURE_2D, 0, 4, dimx, dimy, 0, GL_RGBA,
                  GL_FLOAT, (GLvoid*) texImage );

  status = glGetError();
  if( status != GL_NO_ERROR )
    cerr << "GLComponent::glMakeTexImage : OpenGL error 4: " 
         << gluErrorString(status) << endl;

  //	cleanup temporary texture image
  delete[] texImage;

  return true;
}


bool GLComponent::glMakeTexEnvGLL( const ViewState & state, 
                                   const GLList & gllist, unsigned tex ) const
{
  /* cout << "GLComponent::glMakeTexEnvGLL tex " << tex << ", this: " << this 
     << endl; */
  unsigned		dimtex, texname = 0, texid, i;
  bool			texok;

  glNewList( gllist.item(), GL_COMPILE );

  dimtex = glDimTex( state, tex );
  for( texid=0, i=0; i<tex; ++i, ++texid )
    if( glTexRGBInterpolation( i ) )
      ++texid;

  bool	rgb = glTexRGBInterpolation( tex );

  texok = dimtex > 0;
  if( texok )
    {
      if( rgb )
        texname = d->rgtex().item();
      else
        {
          GLPrimitives	names = glTexNameGLL( state, tex );
          if( !names.empty() )
            texname = ((GLTexture &) *names.front()).item();
          if( texname == 0 )
            texok = false;
        }
    }

  GLenum	status;
  GLuint	textype = GL_TEXTURE_1D;

  GLCaps::glActiveTexture( GLCaps::textureID( texid ) );
  glMatrixMode( GL_TEXTURE );
  glLoadIdentity();
  glMatrixMode( GL_MODELVIEW );

  if( texok )
    {
      if( rgb )
        dimtex = 2;

      /* cout << "TexEnv: OK for " << tex << ", texname: " << texname 
         << ", dim: " << dimtex << endl; */
      switch( dimtex )
        {
        case 1:
          glEnable( GL_TEXTURE_1D );
          glDisable( GL_TEXTURE_2D );
          glDisable( GL_TEXTURE_3D );
          textype = GL_TEXTURE_1D;
          glBindTexture( GL_TEXTURE_1D, texname );
          status = glGetError();
          if( status != GL_NO_ERROR )
            cerr << "GLComponent::glMakeTexEnvGLL : OpenGL error 1: " 
                 << gluErrorString(status) << endl;
          break;
        case 2:
          glEnable( GL_TEXTURE_2D );
          glDisable( GL_TEXTURE_1D );
          glDisable( GL_TEXTURE_3D );
          glBindTexture( GL_TEXTURE_2D, texname );
          status = glGetError();
          if( status != GL_NO_ERROR )
            cerr << "GLComponent::glMakeTexEnvGLL : OpenGL error 2: " 
                 << gluErrorString(status) << endl;
          textype = GL_TEXTURE_2D;
          break;

        case 3:
          glEnable( GL_TEXTURE_3D );
          glDisable( GL_TEXTURE_2D );
          glDisable( GL_TEXTURE_1D );
          glBindTexture( GL_TEXTURE_3D, texname );
          status = glGetError();
          if( status != GL_NO_ERROR )
            cerr << "GLComponent::glMakeTexEnvGLL 3 : OpenGL error 3: " 
                 << gluErrorString(status) << endl;
          textype = GL_TEXTURE_3D;
          break;

        default:
          cerr << "unsupported texture dimension: " << dimtex << endl;
          glDisable( GL_TEXTURE_1D );
          glDisable( GL_TEXTURE_2D );
          glDisable( GL_TEXTURE_3D );
        }
      if( glTexRGBInterpolation( i ) )
        glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
      else
        glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, glGLTexMode( tex ) );
      glTexParameteri( textype, GL_TEXTURE_MAG_FILTER, 
                       glGLTexFiltering( tex ) );
      glTexParameteri( textype, GL_TEXTURE_MIN_FILTER, 
                       glGLTexFiltering( tex ) );
      // cout << "texture enabled\n";
      status = glGetError();
      if( status != GL_NO_ERROR )
        cerr << "GLComponent::glMakeTexEnvGLL : OpenGL error 4: " 
             << gluErrorString(status) << endl;

      // texture generation
      glAutoTexturingMode	tm = glAutoTexMode( tex );
      if( rgb || ( !GLCaps::ext_ARB_texture_cube_map() 
                   && !GLCaps::ext_EXT_texture_cube_map() 
                   && ( tm == glTEX_REFLECTION_MAP 
                        || tm == glTEX_NORMAL_MAP ) ) )
        tm = glTEX_MANUAL;

      if( rgb )
        glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
      else
        glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, glGLTexMode( tex ) );

      switch( tm )
        {
        case glTEX_OBJECT_LINEAR:
          glEnable( GL_TEXTURE_GEN_S );
          glEnable( GL_TEXTURE_GEN_T );
          glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR );
          glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR );
          glTexGenfv( GL_S, GL_OBJECT_PLANE, glAutoTexParams( 0, tex ) );
          glTexGenfv( GL_T, GL_OBJECT_PLANE, glAutoTexParams( 1, tex ) );
          glTexParameteri( textype, GL_TEXTURE_WRAP_S, GL_REPEAT );
          glTexParameteri( textype, GL_TEXTURE_WRAP_T, GL_REPEAT );
          glTexParameteri( textype, GL_TEXTURE_WRAP_R, GL_REPEAT );
          break;
        case glTEX_EYE_LINEAR:
          glEnable( GL_TEXTURE_GEN_S );
          glEnable( GL_TEXTURE_GEN_T );
          glDisable( GL_TEXTURE_GEN_R );
          glPushAttrib( GL_TRANSFORM_BIT );
          glMatrixMode( GL_MODELVIEW );
          glPushMatrix();
          glLoadIdentity();
          glTexGenfv( GL_S, GL_EYE_PLANE, glAutoTexParams( 0, tex ) );
          glTexGenfv( GL_T, GL_EYE_PLANE, glAutoTexParams( 1, tex ) );
          glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR );
          glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR );
          glTexParameteri( textype, GL_TEXTURE_WRAP_S, GL_REPEAT );
          glTexParameteri( textype, GL_TEXTURE_WRAP_T, GL_REPEAT );
          glTexParameteri( textype, GL_TEXTURE_WRAP_R, GL_REPEAT );
          glPopMatrix();
          glPopAttrib();
          break;
        case glTEX_SPHERE_MAP:
          glEnable( GL_TEXTURE_GEN_S );
          glEnable( GL_TEXTURE_GEN_T );
          glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP );
          glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP );
          glTexParameteri( textype, GL_TEXTURE_WRAP_S, GL_REPEAT );
          glTexParameteri( textype, GL_TEXTURE_WRAP_T, GL_REPEAT );
          glTexParameteri( textype, GL_TEXTURE_WRAP_R, GL_REPEAT );
          break;
#ifdef GL_REFLECTION_MAP
        case glTEX_REFLECTION_MAP:
          glEnable( GL_TEXTURE_GEN_S );
          glEnable( GL_TEXTURE_GEN_T );
          glEnable( GL_TEXTURE_GEN_R );
          glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP );
          glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP );
          glTexGeni( GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP );
          glTexParameteri( textype, GL_TEXTURE_WRAP_S, GL_REPEAT );
          glTexParameteri( textype, GL_TEXTURE_WRAP_T, GL_REPEAT );
          glTexParameteri( textype, GL_TEXTURE_WRAP_R, GL_REPEAT );
          break;
#endif
#ifdef GL_NORMAL_MAP
        case glTEX_NORMAL_MAP:
          glEnable( GL_TEXTURE_GEN_S );
          glEnable( GL_TEXTURE_GEN_T );
          glEnable( GL_TEXTURE_GEN_R );
          glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP );
          glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP );
          glTexGeni( GL_R, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP );
          glTexParameteri( textype, GL_TEXTURE_WRAP_S, GL_REPEAT );
          glTexParameteri( textype, GL_TEXTURE_WRAP_T, GL_REPEAT );
          glTexParameteri( textype, GL_TEXTURE_WRAP_R, GL_REPEAT );
          break;
#endif
        default:
          glDisable( GL_TEXTURE_GEN_S );
          glDisable( GL_TEXTURE_GEN_T );
          glDisable( GL_TEXTURE_GEN_R );
          glTexParameteri( textype, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
          glTexParameteri( textype, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
          glTexParameteri( textype, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
          break;
        }

      if( rgb )
        {
          // blue texture

          texname = d->batex().item();
          GLCaps::glActiveTexture( GLCaps::textureID( texid + 1 ) );
          glEnable( GL_TEXTURE_2D );
          //glDisable( GL_TEXTURE_2D );
          glDisable( GL_TEXTURE_1D );
          glDisable( GL_TEXTURE_3D );
          glBindTexture( GL_TEXTURE_2D, texname );
          status = glGetError();
          if( status != GL_NO_ERROR )
            cerr << "GLComponent::glMakeTexEnvGLL : OpenGL error 5: " 
                 << gluErrorString(status) << endl;
          textype = GL_TEXTURE_2D;

          glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
          glTexParameteri( textype, GL_TEXTURE_MAG_FILTER, 
                           glGLTexFiltering( tex ) );
          glTexParameteri( textype, GL_TEXTURE_MIN_FILTER, 
                           glGLTexFiltering( tex ) );
          glTexParameteri( textype, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
          glTexParameteri( textype, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
          // cout << "texture enabled\n";
          status = glGetError();
          if( status != GL_NO_ERROR )
            cerr << "GLComponent::glMakeTexEnvGLL : OpenGL error 6: " 
                 << gluErrorString(status) << endl;
        }
    }
  else
    {
      //cout << "TexEnv: disabling " << tex << endl;
      glDisable( GL_TEXTURE_1D );
      glDisable( GL_TEXTURE_2D );
      glDisable( GL_TEXTURE_3D );
    }

  glEndList();

  return true;
}


void GLComponent::glBeforeBodyGLL( const ViewState &, 
				   GLPrimitives & ) const
{
}


void GLComponent::glAfterBodyGLL( const ViewState &, 
				  GLPrimitives & ) const
{
}


bool GLComponent::glMakeBodyGLL( const ViewState & state, 
                                 const GLList & gllist ) const
{
  // cout << "GLComponent::glMakeBodyGLL\n";
  const GLfloat	*vvertex = glVertexArray( state );
  const GLfloat	*vnormal = glNormalArray( state );
  const GLuint	*vpoly = glPolygonArray( state );
  unsigned	nvert = glNumVertex( state );
  unsigned	npoly = glNumPolygon( state );
  unsigned	ppoly = glPolygonSize( state );
  glSelectRenderMode selectmode = state.selectRenderMode;
  //cout << npoly << " polygones de " << ppoly << " pts\n";

  //selectmode = ViewState::glSELECTRENDER_OBJECT; // FIXME

  if( !vpoly || !npoly || !ppoly || !vvertex || !nvert )
    return false;
    /*{
      cerr << "Polygon elements missing: can't render it...\n";
      return( false );
      }*/

  GLenum	polytype;
  switch( ppoly )
    {
    case 1:
      polytype = GL_POINTS;
      break;
    case 2:
      polytype = GL_LINES;
      break;
    case 3:
      polytype = GL_TRIANGLES;
      break;
    case 4:
      polytype = GL_QUADS;
      break;
    default:
      cerr << "Can't use polygons with " << ppoly << " vertices\n";
      return false;
    }

  unsigned		i, n = glNumTextures( state ), m, tex = 0;
  const GLfloat		*texture;
  unsigned		ntex, dimtex;
  bool			texok;

  // disable normals and shading in selection mode
  if( selectmode != ViewState::glSELECTRENDER_NONE )
  {
    vnormal = 0;
    glDisableClientState( GL_NORMAL_ARRAY );
    glDisableClientState( GL_VERTEX_ARRAY );
    m = GLCaps::numTextureUnits();
    for( tex=0; tex<m; ++tex )
    {
      GLCaps::glClientActiveTexture( GLCaps::textureID( tex ) );
      glDisableClientState( GL_TEXTURE_COORD_ARRAY );
    }

    glNewList( gllist.item(), GL_COMPILE );

    for( tex=0; tex<m; ++tex )
    {
      GLCaps::glActiveTexture( GLCaps::textureID( tex ) );
      glDisable( GL_TEXTURE_1D );
      glDisable( GL_TEXTURE_2D );
      glDisable( GL_TEXTURE_3D );
    }
    glPushAttrib( GL_LIGHTING_BIT );
    glDisable( GL_LIGHTING );
    glEnable( GL_COLOR_MATERIAL );
    const GLuint *p = vpoly;
    unsigned j;

    switch( selectmode )
    {
    case ViewState::glSELECTRENDER_OBJECT:
      {
        int id = glObjectID();
        glColor3ub( id >> 16, (id & 0xff00) >> 8, id & 0xff );
        glBegin( polytype );
        for( i=0; i<npoly; ++i )
        {
          for( j=0; j<ppoly; ++j, ++p )
          {
            glVertex3fv( vvertex + *p * 3 );
          }
        }
        glEnd();
      }
      break;
    case ViewState::glSELECTRENDER_POLYGON:
      glBegin( polytype );
      for( i=0; i<npoly; ++i )
      {
        glColor3ub( i >> 16, (i & 0xff00) >> 8, i & 0xff );
        for( j=0; j<ppoly; ++j, ++p )
        {
          glVertex3fv( vvertex + *p * 3 );
        }
      }
      glEnd();
      break;
    default:
      break;
    }

    glPopAttrib();

    glEndList();

    return true;
  }

  /// ##########"



  glEnableClientState( GL_VERTEX_ARRAY );
  glVertexPointer( 3, GL_FLOAT, 0, vvertex );
  if( vnormal )
  {
    glEnableClientState( GL_NORMAL_ARRAY );
    glNormalPointer( GL_FLOAT, 0, vnormal );
  }
  else
  {
    glDisableClientState( GL_NORMAL_ARRAY );
  }

  m = GLCaps::numTextureUnits();
  if( n > m )
    n = m;

  vector<rc_ptr<vector<GLfloat> > >	rg(m), ba(m);

  // cout << "texturing...\n";
  for( i=0; i<n && tex < m; ++i, ++tex )
    {
      // cout << "texture " << i << endl;
      texture = glTexCoordArray( state, i );
      ntex = glTexCoordSize( state, i );
      dimtex = glDimTex( state, i );
      if( ntex > nvert )
        cout << "warning : texture is bigger than mesh\n";
      GLCaps::glClientActiveTexture( GLCaps::textureID( tex ) );
      texok = texture && ntex >= nvert;
      /* cout << "texdata: " << texture << ", size: " << ntex << ", dim: " 
         << dimtex << ", vertices: " << nvert << endl; */

      if( texok )
        {
          // cout << "enable texture " << i << endl;
          glEnableClientState( GL_TEXTURE_COORD_ARRAY );

          const AObjectPalette	*pal;
          if( glTexRGBInterpolation( i ) && tex < m-1 
              && ( pal = glPalette( i ) ) )
            {
              rg[tex].reset( new vector<GLfloat> );
              ba[tex].reset( new vector<GLfloat> );
              vector<GLfloat>	& vrg = *rg[tex];
              vector<GLfloat>	& vba = *ba[tex];
              vrg.reserve( nvert * 2 );
              vba.reserve( nvert * 2 );
              unsigned		v;
              const GLfloat	*dt = texture;
              AimsRGBA		col;
              float		r = glTexRate( i ), ir = 1. - r;

              for( v=0; v<nvert; ++v )
                {
                  if( dimtex == 1 )
                    col = pal->normColor( *dt );
                  else
                    col = pal->normColor( *dt, *(dt+1) );
                  //cout << "color: " << col << endl;
                  dt += dimtex;
                  /* if( glTexMode( i ) == glLINEAR )
                    {
                      vrg.push_back( float( col.red() ) / 255.01 );
                      vrg.push_back( float( col.green() ) / 255.01 );
                      vba.push_back( float( col.blue() ) / 255.01 );
                      vba.push_back( float( col.alpha() ) / 255.01 * r );
                    }
                  else
                  {*/
                      vrg.push_back( float( col.red() ) / 255.01 * r + ir );
                      vrg.push_back( float( col.green() ) / 255.01 * r + ir );
                      vba.push_back( float( col.blue() ) / 255.01 * r + ir );
                      vba.push_back( float( col.alpha() ) / 255.01 );
                      //}
                }
              glTexCoordPointer( 2, GL_FLOAT, 0, &vrg[0] );

              ++tex;
              GLCaps::glClientActiveTexture( GLCaps::textureID( tex ) );
              glEnableClientState( GL_TEXTURE_COORD_ARRAY );
              glTexCoordPointer( 2, GL_FLOAT, 0, &vba[0] );
            }
          else
            glTexCoordPointer( dimtex, GL_FLOAT, 0, texture );
        }
      else
        glDisableClientState( GL_TEXTURE_COORD_ARRAY );
    }
  for( ; tex<m; ++tex )
    {
      GLCaps::glClientActiveTexture( GLCaps::textureID( tex ) );
      glDisableClientState( GL_TEXTURE_COORD_ARRAY );
    }
  glDisableClientState( GL_COLOR_ARRAY );

  glNewList( gllist.item(), GL_COMPILE );

  if( !vnormal )
  {
    glPushAttrib( GL_LIGHTING_BIT );
    glDisable( GL_LIGHTING );
  }
  glDrawElements( polytype, ppoly * npoly, GL_UNSIGNED_INT, vpoly );
  if( !vnormal )
    glPopAttrib();

  glEndList();

  return true;
}


string GLComponent::viewStateID( glPart part, const ViewState & state ) const
{
  // cout << "viewStateID " << part << ", smode: " << state.selectRenderMode << endl;
  string	s;
  float		t = state.time;
  if( part == glTEXIMAGE || part == glTEXENV || part == glMATERIAL )
    return s;

  if( state.selectRenderMode != ViewState::glSELECTRENDER_NONE )
  {
    if( part == glTEXIMAGE || part == glTEXENV || part == glMATERIAL
        || part == glPALETTE )
      return s;
    s.resize( sizeof(float) + sizeof( glSelectRenderMode ) );
    (float &) s[0] = t;
    (glSelectRenderMode &) s[sizeof(float)] = state.selectRenderMode;
    return s;
  }

  s.resize( sizeof(float) );
  (float &) s[0] = t;
  return s;
}


void GLComponent::glGarbageCollector( int n )
{
#ifdef ANA_DEBUG_GLLISTS
  cout << "--> GLComponent::glGarbageCollector" << endl;
#endif

  if( n < 0 )
    n = glStateMemory();

  if( n == 0 )
    {
      d->mainGLL.clear();
      d->bodyGLL.clear();
      d->materialGLL.clear();
      unsigned	tex, nt = d->textures.size();
      for( tex=0; tex<nt; ++tex )
        {
          TexInfo	& t = d->textures[ tex ];
          t.texenv.clear();
          t.name.clear();
        }
      return;
    }

  map<string, RefGLItem>::iterator		i, e = d->bodyGLL.end();
  map<string, GLPrimitives>::iterator		im, em = d->mainGLL.end();
  list<map<string, RefGLItem>::iterator>		todel;
  list<map<string, RefGLItem>::iterator>::iterator	il, el;
  list<map<string, GLPrimitives>::iterator>		todelm;
  list<map<string, GLPrimitives>::iterator>::iterator	ilm, elm;
  int							ct;

  ct = 0;
  for( im=d->mainGLL.begin(); im!=em; ++im )
    if( im->second.empty() || im->second.front().refCount() == 1 )
      {
        ++ct;
        if( ct >= n )
          todelm.push_back( im );
      }

  for( ilm=todelm.begin(), elm=todelm.end(); ilm!=elm; ++ilm )
    {
#ifdef ANA_DEBUG_GLLISTS
      cout << "glGarbageCollector: delete main list " 
           << (*ilm)->second.front().get() << endl;
#endif
      d->mainGLL.erase( *ilm );
    }
  todelm.clear();

  ct = 0;
  for( i=d->bodyGLL.begin(); i!=e; ++i )
    if( i->second.refCount() == 1 )
      {
        ++ct;
        if( ct >= n )
          todel.push_back( i );
      }
  for( il=todel.begin(), el=todel.end(); il!=el; ++il )
    {
#ifdef ANA_DEBUG_GLLISTS
      cout << "glGarbageCollector: delete body list " 
           << (*il)->second.get() << endl;;
#endif
      d->bodyGLL.erase( *il );
    }
  todel.clear();

  ct = 0;
  for( i=d->materialGLL.begin(), e=d->materialGLL.end(); i!=e; ++i )
    if( i->second.refCount() == 1 )
      {
        ++ct;
        if( ct >= n )
          todel.push_back( i );
      }

  for( il=todel.begin(), el=todel.end(); il!=el; ++il )
    {
#ifdef ANA_DEBUG_GLLISTS
      cout << "glGarbageCollector: delete material list " 
           << (*il)->second.get() << endl;;
#endif
      d->materialGLL.erase( *il );
    }
  todel.clear();

  unsigned	tex, nt = d->textures.size();
  for( tex=0; tex<nt; ++tex )
    {
      TexInfo	& t = d->textures[ tex ];
      
      ct = 0;
      for( i=t.texenv.begin(), e=t.texenv.end(); i!=e; ++i )
        if( i->second.refCount() == 1 )
          {
            ++ct;
            if( ct >= n )
              todel.push_back( i );
          }

      for( il=todel.begin(), el=todel.end(); il!=el; ++il )
        {
#ifdef ANA_DEBUG_GLLISTS
          cout << "delete texenv list " << (*il)->second.get() << endl;;
#endif
          t.texenv.erase( *il );
        }
      todel.clear();

      ct = 0;
      for( i=t.name.begin(), e=t.name.end(); i!=e; ++i )
        if( i->second.refCount() == 1 )
          {
            ++ct;
            if( ct >= n )
              todel.push_back( i );
          }

      for( il=todel.begin(), el=todel.end(); il!=el; ++il )
        {
#ifdef ANA_DEBUG_GLLISTS
          cout << "glGarbageCollector: delete texname list " 
               << (*il)->second.get() << endl;;
#endif
          t.name.erase( *il );
        }
      todel.clear();
    }

#ifdef ANA_DEBUG_GLLISTS
  cout << "<-- GLComponent::glGarbageCollector" << endl;
#endif
}


void GLComponent::glSetStateMemory( unsigned n )
{
  d->memory = n;
}


unsigned GLComponent::glStateMemory() const
{
  return d->memory;
}


const GLComponent::TexExtrema & GLComponent::glTexExtrema( unsigned tex ) const
{
  return d->textures[ tex ].extrema;
}


GLComponent::TexExtrema & GLComponent::glTexExtrema( unsigned tex )
{
  return d->textures[ tex ].extrema;
}


set<GLComponent::glTextureMode> 
GLComponent::glAllowedTexModes( unsigned ) const
{
  set<glTextureMode>	a;
  a.insert( glGEOMETRIC );
  a.insert( glLINEAR );
  a.insert( glREPLACE );
  a.insert( glDECAL );
  a.insert( glBLEND );
  a.insert( glADD );
  // a.insert( glCOMBINE );
  return a;
}


set<GLComponent::glTextureFiltering> 
GLComponent::glAllowedTexFilterings( unsigned ) const
{
  set<glTextureFiltering>	a;
  a.insert( glFILT_NEAREST );
  a.insert( glFILT_LINEAR );
  return a;
}


set<GLComponent::glAutoTexturingMode> 
GLComponent::glAllowedAutoTexModes( unsigned ) const
{
  set<glAutoTexturingMode>	a;
  a.insert( glTEX_MANUAL );
  a.insert( glTEX_OBJECT_LINEAR );
  a.insert( glTEX_EYE_LINEAR );
  a.insert( glTEX_SPHERE_MAP );
  a.insert( glTEX_REFLECTION_MAP );
  a.insert( glTEX_NORMAL_MAP );
  return a;
}


bool GLComponent::glAllowedTexRGBInterpolation( unsigned ) const
{
  return true;
}


bool GLComponent::glAllowedTexRate( unsigned ) const
{
  return true;
}


bool GLComponent::glTexRGBInterpolation( unsigned tex ) const
{
  return d->textures[ tex ].rgbinterp;
}


void GLComponent::glSetTexRGBInterpolation( bool x, unsigned tex )
{
  TexInfo	& te = d->textures[ tex ];
  if( te.rgbinterp != x )
    {
      if( x )
        ++d->ntex;
      else
        --d->ntex;
      te.rgbinterp = x;
      glSetTexEnvChanged( true, tex );
      glSetChanged( glBODY, true );
    }
}


Object GLComponent::debugInfo() const
{
  Object	o = Object::value( Dictionary() );
  try
    {
      o->setProperty( "texture_number", Object::value( d->ntex ) );
      o->setProperty( "state_memory_size", Object::value( d->memory ) );

      Object	ch = Object::value( Dictionary() );
      o->setProperty( "changed", ch );
      map<string, RefGLItem>::const_iterator	in, en;
      map<string, GLPrimitives>::const_iterator	im, em;
      unsigned	i, j, n = d->changed.size();
      static const string	partname[] = {
        "general", "body", "material", "geometry", "palette", "referential", 
        "teximage", "texenv", 
      };

      for( i=0; i<n; ++i )
        {
          if( i < glNOPART )
            ch->setProperty( partname[i], 
                             Object::value( (int) d->changed[i] ) );
          else if( i != glNOPART )
            {
              stringstream	s;
              j = i - glTEXIMAGE_NUM;
              if( j & 1 )
                s << "teximage_";
              else
                s << "texenv_";
              s << j/2;
              ch->setProperty( s.str(), Object::value( (int) d->changed[i] ) );
            }
        }
      Object	gl = Object::value( Dictionary() );
      o->setProperty( "gllists", gl );
      Object	maingl = Object::value( Dictionary() );
      gl->setProperty( "main", maingl );

      int	r;
      for( im=d->mainGLL.begin(), em=d->mainGLL.end(); im!=em; ++im )
        {
          if( im->second.empty() )
            r = 0;
          else
            r = (int) im->second.front().refCount();
          maingl->setProperty( im->first, Object::value( r ) );
        }
      Object	matgl = Object::value( Dictionary() );
      gl->setProperty( "material", matgl );
      for( in=d->materialGLL.begin(), en=d->materialGLL.end(); in!=en; ++in )
        matgl->setProperty( in->first, 
                            Object::value( (int) in->second.refCount() ) );
      Object	bodygl = Object::value( Dictionary() );
      gl->setProperty( "body", bodygl );
      for( in=d->bodyGLL.begin(), en=d->bodyGLL.end(); in!=en; ++in )
        bodygl->setProperty( in->first, 
                             Object::value( (int) in->second.refCount() ) );

      Object	tx = Object::value( ObjectVector() );
      o->setProperty( "textures", tx );
      ObjectVector	& txl = tx->value<ObjectVector>();

      for( i=0; i<d->ntex; ++i )
        {
          //cout << "tex " << i << endl;
          txl.push_back( Object::value( Dictionary() ) );
          Dictionary	& t = txl.back()->value<Dictionary>();
          const TexInfo	& ti = d->textures[i];
          //cout << "fill tex info\n";
          t[ "mode" ] = Object::value( (int) ti.mode );
          t[ "filtering" ] = Object::value( (int) ti.filter );
          t[ "changed" ] = Object::value( ti.changed );
          t[ "rate" ] = Object::value( ti.rate );
          t[ "generation" ] = Object::value( (int) ti.automode );
          t[ "envchanged" ] = Object::value( ti.envchanged );
          t[ "interpolation" ] = Object::value( (int) ti.rgbinterp );
          Object	nn = Object::value( Dictionary() );
          t[ "texname_gllists" ] = nn;
          for( in=ti.name.begin(), en=ti.name.end(); in!=en; ++in )
            nn->setProperty( in->first, 
                             Object::value( (int) in->second.refCount() ) );
          Object	e = Object::value( Dictionary() );
          t[ "texenv_gllists" ] = e;
          for( in=ti.texenv.begin(), en=ti.texenv.end(); in!=en; ++in )
            e->setProperty( in->first, 
                            Object::value( (int) in->second.refCount() ) );
          //cout << "done tex " << i << endl;
        }
    }
  catch( exception & e )
    {
      cerr << "GLComponent::debugInfo failure: " << e.what() << endl;
    }
  return o;
}


const float* GLComponent::glAutoTexParams( unsigned coord, unsigned tex ) const
{
  if( coord >= 3 )
    coord = 0;
  switch( glAutoTexMode( tex ) )
    {
    case glTEX_OBJECT_LINEAR:
      return d->textures[ tex ].texgenparams_object[coord];
    case glTEX_EYE_LINEAR:
      return d->textures[ tex ].texgenparams_eye[coord];
    default:
      return 0;
    }
}


void GLComponent::glSetAutoTexParams( const float* params, unsigned coord, 
                                      unsigned tex )
{
  float	*buf = 0;
  if( coord < 3 )
    switch( glAutoTexMode( tex ) )
      {
      case glTEX_OBJECT_LINEAR:
        buf = d->textures[ tex ].texgenparams_object[coord];
        break;
      case glTEX_EYE_LINEAR:
        buf = d->textures[ tex ].texgenparams_eye[coord];
        break;
      default:
        break;
      }
  if( buf )
    {
      buf[0] = params[0];
      buf[1] = params[1];
      buf[2] = params[2];
      buf[3] = params[3];
      glSetTexEnvChanged( tex );
    }
}


GLPrimitives GLComponent::glHandleTransformation( const ViewState & vs,
    const Referential* objref )
{
  GLPrimitives  p;

  if( !objref )
    return p;

  const SliceViewState  *svs = vs.sliceVS();
  const Referential *r;
  if( svs )
    r = svs->winref;
  else if( vs.window )
    r = vs.window->getReferential();
  else
    return p;
  if( objref == r )
    return p;

  Transformation *trans = theAnatomist->getTransformation( objref, r );
  if( !trans )
    return p;

  // cout << "apply transf before bodyGLL\n";

  GLfloat mat[16];

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

  return p;
}


GLPrimitives GLComponent::glPopTransformation( const ViewState &,
                                               const Referential* )
{
  GLPrimitives  pl;
  GLList *p2 = new GLList;
  p2->generate();
  GLuint l2 = p2->item();
  glNewList( l2, GL_COMPILE );
  glPopMatrix();
  glEndList();
  pl.push_back( RefGLItem( p2 ) );
  return pl;
}


int GLComponent::glObjectID() const
{
  return d->id();
}

