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


//#define GL_GLEXT_PROTOTYPES
#include <anatomist/window/glcaps.h>
#include <anatomist/application/globalConfig.h>
#include <anatomist/application/Anatomist.h>
#include <iostream>
#ifdef _WIN32
#  include <wingdi.h>
#else
#  include <dlfcn.h>
#  ifndef APIENTRY
#    define APIENTRY
#  endif
#endif

using namespace anatomist;
using namespace carto;
using namespace std;

namespace
{
#ifdef _WIN32
  /* would be OK for linux also, but MacOS, of course, does not have
     these types defined */
  typedef PFNGLACTIVETEXTUREARBPROC glActiveTextureFunc;
  typedef PFNGLBLENDEQUATIONEXTPROC glBlendEquationFunc;
  typedef PFNGLTEXIMAGE3DEXTPROC glTexImage3DFunc;
  typedef PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebufferFunc;
  typedef PFNGLBINDRENDERBUFFEREXTPROC glBindRenderbufferFunc;
  typedef PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2D;
  typedef PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffersFunc;
  typedef PFNGLGENRENDERBUFFERSEXTPROC glGenRenderbuffersFunc;
  typedef PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC glFramebufferRenderbufferFunc;
  typedef PFNGLRENDERBUFFERSTORAGEEXTPROC glRenderbufferStorageFunc;
#else
  typedef void (*glActiveTextureFunc)( GLenum );
  typedef void (*glBlendEquationFunc)( GLenum );
  typedef void (*glTexImage3DFunc)( GLenum, GLint, GLenum, GLsizei, GLsizei,
                                    GLsizei, GLint, GLenum, GLenum,
                                    const void* );
  typedef void (*glBindFramebufferFunc)( GLenum target, GLuint framebuffer );
  typedef void (*glBindRenderbufferFunc)( GLenum target, GLuint renderbuffer );
  typedef void (*glFramebufferTexture2DFunc)( GLenum target, GLenum attachment,
                                              GLenum textarget, GLuint texture,
                                              GLint level );
  typedef void (*glGenFramebuffersFunc)( GLsizei n, GLuint *ids );
  typedef void (*glGenRenderbuffersFunc)( GLsizei n, GLuint *renderbuffers );
  typedef void (*glFramebufferRenderbufferFunc)( GLenum target, 
                                                 GLenum attachment,
                                                 GLenum renderbuffertarget,
                                                 GLuint renderbuffer );
  typedef void (*glRenderbufferStorageFunc)( GLenum target, 
                                             GLenum internalformat,
                                             GLsizei width, GLsizei height );
#endif

  /* the APIENTRY macro here seems to be very important on Windows: it caused
     display bugs and crashed for one year without it. */
  void APIENTRY _void_glActiveTexture( GLenum )
  {
  }


  void APIENTRY _void_glBlendEquation( GLenum )
  {
  }

  void APIENTRY _void_glTexImage3D( GLenum, GLint, GLenum, GLsizei, GLsizei,
                           GLsizei, GLint, GLenum, GLenum, const void* )
  {
  }

  void APIENTRY _void_glBindFramebuffer( GLenum target, GLuint framebuffer )
  {
  }

  void APIENTRY _void_glBindRenderbuffer( GLenum target, GLuint renderbuffer )
  {
  }

  void APIENTRY _void_glFramebufferTexture2D( GLenum target, GLenum attachment,
                                              GLenum textarget, GLuint texture,
                                              GLint level )
  {
  }

  void APIENTRY _void_glGenFramebuffers( GLsizei n, GLuint *ids )
  {
  }

  void APIENTRY _void_glGenRenderbuffers( GLsizei n, GLuint *renderbuffers )
  {
  }

  void APIENTRY _void_glFramebufferRenderbuffer(
    GLenum target, GLenum attachment, GLenum renderbuffertarget,
    GLuint renderbuffer )
  {
  }


  void APIENTRY _void_glRenderbufferStorage(
    GLenum target, GLenum internalformat, GLsizei width, GLsizei height )
  {
  }


  struct GLCapsPrivate
  {
    GLCapsPrivate();
    void updateTextureUnits();

    bool	ext_ARB_multitexture;
    bool	ext_ARB_shadow;
    bool	ext_SGIX_shadow;
    bool	ext_ARB_depth_texture;
    bool	ext_SGIX_depth_texture;
    bool	ext_ARB_texture_cube_map;
    bool	ext_EXT_texture_cube_map;
    glActiveTextureFunc	glActiveTexture;
    glActiveTextureFunc	glClientActiveTexture;
    unsigned	numTextureUnits;
    bool	depthpeeling;
    glBlendEquationFunc glBlendEquation;
    glTexImage3DFunc glTexImage3D;
    glBindFramebufferFunc glBindFramebuffer;
    glBindRenderbufferFunc glBindRenderbuffer;
    glFramebufferTexture2DFunc glFramebufferTexture2D;
    glGenFramebuffersFunc glGenFramebuffers;
    glGenRenderbuffersFunc glGenRenderbuffers;
    glFramebufferRenderbufferFunc glFramebufferRenderbuffer;
    glRenderbufferStorageFunc glRenderbufferStorage;
  };


  GLCapsPrivate & _glcapsPrivate()
  {
    static GLCapsPrivate	p;
    return p;
  }


#if !defined( GLU_VERSION_1_3 ) || defined( _WIN32 )
  // no gluCheckExtension() function before GLU 1.3
  /* gluCheckExtension() on win32/MinGW: glu.h says it is
     1.3 compliant but the function is not in the library
  */

  bool gluCheckExtension( const GLubyte* ext, const GLubyte* extlist )
  {
    string	elist = (const char *) extlist;
    string	e = (const char *) ext;
    return elist.find( e ) != string::npos;
  }
#endif


  GLCapsPrivate::GLCapsPrivate()
    : ext_ARB_multitexture( false ), ext_ARB_shadow( false ),
      ext_SGIX_shadow( false ), ext_ARB_depth_texture( false ),
      ext_SGIX_depth_texture( false ),
      glActiveTexture( _void_glActiveTexture ),
      glClientActiveTexture( _void_glActiveTexture ), numTextureUnits( 0 ),
      depthpeeling( false ), glBlendEquation( 0 ), glTexImage3D( 0 ),
      glBindFramebuffer( 0 ), glBindRenderbuffer( 0 ),
      glFramebufferTexture2D( 0 ), glGenFramebuffers( 0 ),
      glGenRenderbuffers( 0 ), glFramebufferRenderbuffer( 0 )
  {
  const GLubyte	*p = glGetString( GL_EXTENSIONS );
  if( !p )
    {
      cout << "No OpenGL extensions found\n";
      return;
    }

#ifndef _WIN32

#if defined( Q_WS_MAC )
/* MacOS has "different" behaviours with dlopen:
   dlopen( 0 ) doesn't seem to work (maybe because of the
   dylib / bundle distinction)
   and we must provide an absolute path for openGL (probably 
   because libGL is not in the DYLD_LIBRARY_PATH paths)
*/
#if !defined( Q_WS_X11 )
  void	*handle = dlopen( "/System/Library/Frameworks/OpenGL.framework/" 
                          "Libraries/libGL.dylib", RTLD_LAZY );
#else
  // Mac X11 uses another libGL
  void  *handle = dlopen( "/usr/X11R6/lib/libGL.dylib", RTDL_LAZY );
#endif // Q_WS_X11
#else
  void	*handle = dlopen( 0, RTLD_LAZY );
#endif // Q_WS_MAC
  if( !handle )
    cerr << "could not dlopen myself: " << dlerror() << endl;

#else // !_WIN32

/* Windows uses a different mechanism for OpenGL functions / extensions
   We don't need this anymore (it didn't work for extensions)

  HMODULE	handle = GetModuleHandle( 0 );
  if( !handle )
    cerr << "could not dlopen myself" << endl;
*/

#endif

#ifndef GL_ARB_multitexture
  cout << "GL_ARB_multitexture not compiled\n";
#else
  if( gluCheckExtension( (const GLubyte *) "GL_ARB_multitexture", p ) )
    {
      cout << "Multitexturing present\n";
      ext_ARB_multitexture = true;

#ifndef _WIN32

      if( handle )
        {
          glActiveTexture 
            = (glActiveTextureFunc) dlsym( handle, "glActiveTexture" );
          if( !glActiveTexture 
            && !(
              glActiveTexture 
                = (glActiveTextureFunc) dlsym( handle, "_glActiveTexture" ) ) )
          {
            glActiveTexture 
              = (glActiveTextureFunc) dlsym( handle, "glActiveTextureARB" );
            if( !glActiveTexture 
          && !(
            glActiveTexture 
              = (glActiveTextureFunc) dlsym( handle, 
                                             "_glActiveTextureARB" ) ) )
            {
              cerr << "coud not find function glActiveTexture: " 
                    << dlerror() << endl;
              glActiveTexture = _void_glActiveTexture;
            }
          }
          /* cout << "glActiveTexture address: " << (int) glActiveTexture 
             << endl;
          cout << (int) &::glActiveTexture << endl;
          cout << (int) &::glClientActiveTextureARB << endl; */

          glClientActiveTexture	= (glActiveTextureFunc) 
            dlsym( handle, "glClientActiveTexture" );
          if( !glClientActiveTexture )
            {
              glClientActiveTexture	= (glActiveTextureFunc) 
                dlsym( handle, "glClientActiveTextureARB" );
              if( !glClientActiveTexture )
                {
                  cerr << "coud not find function glClientActiveTexture: " 
                       << dlerror() << endl;
                  glClientActiveTexture = _void_glActiveTexture;
                }
            }

          glBlendEquation = (glBlendEquationFunc) dlsym( handle, 
            "glBlendEquation" );
          if( !glBlendEquation )
          {
            cerr << "coud not find function glBlendEquation: " 
                 << dlerror() << endl;
            glBlendEquation = _void_glBlendEquation;
          }

          glTexImage3D = (glTexImage3DFunc) dlsym( handle, 
            "glTexImage3D" );
          if( !glTexImage3D )
          {
            cerr << "coud not find function glTexImage3D: " 
                 << dlerror() << endl;
            glTexImage3D = _void_glTexImage3D;
          }

          glBindFramebuffer = (glBindFramebufferFunc) dlsym( handle, 
            "glBindFramebuffer" );
          if( !glBindFramebuffer )
          {
            glBindFramebuffer = (glBindFramebufferFunc) dlsym( handle, 
              "glBindFramebufferEXT" );
            if( !glBindFramebuffer )
            {
              cerr << "coud not find function glBindFramebuffer: " 
                  << dlerror() << endl;
              glBindFramebuffer = _void_glBindFramebuffer;
            }
          }

          glBindRenderbuffer = (glBindRenderbufferFunc) dlsym( handle, 
            "glBindRenderbuffer" );
          if( !glBindRenderbuffer )
          {
            glBindRenderbuffer = (glBindRenderbufferFunc) dlsym( handle, 
              "glBindRenderbufferEXT" );
            if( !glBindRenderbuffer )
            {
              cerr << "coud not find function glBindRenderbuffer: " 
                  << dlerror() << endl;
              glBindRenderbuffer = _void_glBindRenderbuffer;
            }
          }

          glFramebufferTexture2D = (glFramebufferTexture2DFunc) dlsym( handle, 
            "glFramebufferTexture2D" );
          if( !glFramebufferTexture2D )
          {
            glFramebufferTexture2D = 
              (glFramebufferTexture2DFunc) dlsym( handle, 
                "glFramebufferTexture2DEXT" );
            if( !glFramebufferTexture2D )
            {
              cerr << "coud not find function glFramebufferTexture2D: " 
                  << dlerror() << endl;
              glFramebufferTexture2D = _void_glFramebufferTexture2D;
            }
          }

          glGenFramebuffers = (glGenFramebuffersFunc) dlsym( handle, 
            "glGenFramebuffers" );
          if( !glGenFramebuffers )
          {
            glGenFramebuffers = (glGenFramebuffersFunc) dlsym( handle, 
              "glGenFramebuffersEXT" );
            if( !glGenFramebuffers )
            {
              cerr << "coud not find function glGenFramebuffers: " 
                  << dlerror() << endl;
              glGenFramebuffers = _void_glGenFramebuffers;
            }
          }

          glGenRenderbuffers = (glGenRenderbuffersFunc) dlsym( handle, 
            "glGenRenderbuffers" );
          if( !glGenRenderbuffers )
          {
            glGenRenderbuffers = (glGenRenderbuffersFunc) dlsym( handle, 
              "glGenRenderbuffersEXT" );
            if( !glGenRenderbuffers )
            {
              cerr << "coud not find function glGenRenderbuffers: " 
                  << dlerror() << endl;
              glGenRenderbuffers = _void_glGenRenderbuffers;
            }
          }

          glFramebufferRenderbuffer = (glFramebufferRenderbufferFunc)
            dlsym( handle, "glFramebufferRenderbuffer" );
          if( !glFramebufferRenderbuffer )
          {
            glFramebufferRenderbuffer = (glFramebufferRenderbufferFunc)
              dlsym( handle, "glFramebufferRenderbufferEXT" );
            if( !glFramebufferRenderbuffer )
            {
              cerr << "coud not find function glFramebufferRenderbuffer: " 
                  << dlerror() << endl;
              glFramebufferRenderbuffer = _void_glFramebufferRenderbuffer;
            }
          }

          glRenderbufferStorage = (glRenderbufferStorageFunc)
            dlsym( handle, "glRenderbufferStorage" );
          if( !glRenderbufferStorage )
          {
            glRenderbufferStorage = (glRenderbufferStorageFunc)
              dlsym( handle, "glRenderbufferStorageEXT" );
            if( !glRenderbufferStorage )
            {
              cerr << "coud not find function glRenderbufferStorage: " 
                  << dlerror() << endl;
              glRenderbufferStorage = _void_glRenderbufferStorage;
            }
          }

        }

#else // WIN32

          glActiveTexture 
            = (glActiveTextureFunc) 
            wglGetProcAddress( "glActiveTexture" );
          if( !glActiveTexture )
            {
              glActiveTexture 
                = (glActiveTextureFunc) 
                wglGetProcAddress( "glActiveTextureARB" );
              if( !glActiveTexture )
                {
                  cerr << "coud not find function glActiveTexture: " 
                       << GetLastError() << endl;
                  glActiveTexture = _void_glActiveTexture;
                }
              else cout << "glActiveTextureARB found\n";
            }
          else cout << "standard glActiveTexture found\n";
          cout << "glActiveTexture address: " << (int) glActiveTexture << endl;
          // cout << (int) &::glActiveTexture << endl;
          // cout << (int) &::glClientActiveTextureARB << endl;

          glClientActiveTexture	= (glActiveTextureFunc) 
            wglGetProcAddress( "glClientActiveTexture" );
          if( !glClientActiveTexture )
            {
              glClientActiveTexture	= (glActiveTextureFunc) 
                wglGetProcAddress( "glClientActiveTextureARB" );
              if( !glClientActiveTexture )
                {
                  cerr << "coud not find function glClientActiveTexture" 
                       << endl;
                  glClientActiveTexture = _void_glActiveTexture;
                }
              else cout << "glClientActiveTextureARB found\n";
            }
          else cout << "standard glClientActiveTexture found\n";

          glBlendEquation = (glBlendEquationFunc)
            wglGetProcAddress( "glBlendEquation" );
          if( !glBlendEquation )
          {
            cerr << "coud not find function glBlendEquation: " << endl;
            glBlendEquation = _void_glBlendEquation;
          }
          else cout << "glBlendEquation found \n";

          glTexImage3D = (glTexImage3DFunc)
            wglGetProcAddress( "glTexImage3D" );
          if( !glBlendEquation )
          {
            cerr << "coud not find function glTexImage3D: " << endl;
            glTexImage3D = _void_glTexImage3D;
          }
          else cout << "glTexImage3D found \n";

          glBindFramebuffer = (glBindFramebufferFunc)
            wglGetProcAddress( "glBindFramebuffer" );
          if( !glBindFramebuffer )
          {
            cerr << "coud not find function glBindFramebuffer: " << endl;
            glBindFramebuffer = _void_glBindFramebuffer;
          }
          else cout << "glBindFramebuffer found \n";

          glBindRenderbuffer = (glBindRenderbufferFunc)
            wglGetProcAddress( "glBindRenderbuffer" );
          if( !glBindRenderbuffer )
          {
            cerr << "coud not find function glBindRenderbuffer: " << endl;
            glBindRenderbuffer = _void_glBindRenderbuffer;
          }
          else cout << "glBindRenderbuffer found \n";

          glFramebufferTexture2D = (glFramebufferTexture2DFunc)
            wglGetProcAddress( "glFramebufferTexture2D" );
          if( !glFramebufferTexture2D )
          {
            cerr << "coud not find function glFramebufferTexture2D: " << endl;
            glFramebufferTexture2D = _void_glFramebufferTexture2D;
          }
          else cout << "glFramebufferTexture2D found \n";

          glGenFramebuffers = (glGenFramebuffersFunc)
            wglGetProcAddress( "glGenFramebuffers" );
          if( !glGenFramebuffers )
          {
            cerr << "coud not find function glGenFramebuffers: " << endl;
            glGenFramebuffers = _void_glGenFramebuffers;
          }
          else cout << "glGenFramebuffers found \n";

          glGenRenderbuffers = (glGenRenderbuffersFunc)
            wglGetProcAddress( "glGenRenderbuffers" );
          if( !glGenRenderbuffers )
          {
            cerr << "coud not find function glGenRenderbuffers: " << endl;
            glGenRenderbuffers = _void_glGenRenderbuffers;
          }
          else cout << "glGenRenderbuffers found \n";

          glFramebufferRenderbuffer = (glFramebufferRenderbufferFunc)
            wglGetProcAddress( "glFramebufferRenderbuffer" );
          if( !glFramebufferRenderbuffer )
          {
            cerr << "coud not find function glFramebufferRenderbuffer: "
              << endl;
            glFramebufferRenderbuffer = _void_glFramebufferRenderbuffer;
          }
          else cout << "glFramebufferRenderbuffer found \n";

          glRenderbufferStorage = (glRenderbufferStorageFunc)
            wglGetProcAddress( "glRenderbufferStorage" );
          if( !glRenderbufferStorage )
          {
            cerr << "coud not find function glRenderbufferStorage: "
              << endl;
            glRenderbufferStorage = _void_glRenderbufferStorage;
          }
          else cout << "glRenderbufferStorage found \n";

#endif

      updateTextureUnits();

      if( glActiveTexture == _void_glActiveTexture 
          || glClientActiveTexture == _void_glActiveTexture )
        {
          cout << "multitexturing is not complete, disabling it\n";
          glActiveTexture = _void_glActiveTexture;
          glClientActiveTexture = _void_glActiveTexture;
          numTextureUnits = 1;
        }
    }
  else
    cout << "No multitexturing available\n";
#endif

#ifndef GL_ARB_shadow
    cout << "GL_ARB_shadow extension not compiled\n";
#else
  if( gluCheckExtension( (const GLubyte *) "GL_ARB_shadow", p ) )
    {
      cout << "GL_ARB_shadow present\n";
      ext_ARB_shadow = true;
    }
  else
    cout << "GL_ARB_shadow extension not available\n";
#endif

#ifndef GL_SGIX_shadow
    cout << "GL_SGIX_shadow extension not compiled\n";
#else
  if( gluCheckExtension( (const GLubyte *) "GL_SGIX_shadow", p ) )
    {
      cout << "GL_SGIX_shadow present\n";
      ext_SGIX_shadow = true;
    }
  else
    cout << "GL_SGIX_shadow extension not available\n";
#endif

#ifndef GL_SGIX_depth_texture
  cout << "GL_SGIX_depth_texture extension not compiled\n";
#else
  if( gluCheckExtension( (const GLubyte *) "GL_SGIX_depth_texture", p ) )
    {
      cout << "GL_SGIX_depth_texture extension present\n";
      ext_SGIX_depth_texture = true;
    }
  else
    cout << "GL_SGIX_depth_texture extension not available\n";
#endif

#ifndef GL_ARB_depth_texture
  cout << "GL_ARB_depth_texture extension not compiled\n";
#else
  if( gluCheckExtension( (const GLubyte *) "GL_ARB_depth_texture", p ) )
    {
      cout << "GL_ARB_depth_texture extension present\n";
      ext_ARB_depth_texture = true;
    }
  else
    cout << "GL_ARB_depth_texture extension not available\n";
#endif

  if( gluCheckExtension( (const GLubyte *) "GL_ARB_texture_cube_map", p ) )
    {
      cout << "GL_ARB_texture_cube_map extension present\n";
      ext_ARB_texture_cube_map = true;
    }
  else
    cout << "GL_ARB_texture_cube_map extension not available\n";

  if( gluCheckExtension( (const GLubyte *) "GL_EXT_texture_cube_map", p ) )
    {
      cout << "GL_EXT_texture_cube_map extension present\n";
      ext_EXT_texture_cube_map = true;
    }
  else
    cout << "GL_EXT_texture_cube_map extension not available\n";

#ifndef _WIN32
  if( handle )
    dlclose( handle );
#else
  //CloseHandle( handle );
#endif

  }
}


void GLCapsPrivate::updateTextureUnits()
{
//   cout << "updateTextureUnits\n";
  glGetError();
  GLint     ntex = 1;
  glGetIntegerv( GL_MAX_TEXTURE_UNITS_ARB, &ntex );
  int status = glGetError();
  if( status != GL_NO_ERROR )
    cerr << "OpenGL error: "
         << gluErrorString(status) << endl;
  numTextureUnits = (unsigned) ntex;
  cout << "Number of texture units: " << numTextureUnits << endl;
  GlobalConfiguration   *cfg = theAnatomist->config();
  int imt = -1;
  try
  {
    Object  x = cfg->getProperty( "maxTextureUnitsUsed" );
    if( !x.isNull() )
      imt = (int) x->getScalar();
  }
  catch( ... )
  {
  }
  if( imt >= 0 && imt < ntex )
  {
    cout << "Texture units limited in configuration: " << imt << endl;
    numTextureUnits = (unsigned) imt;
  }
}


bool GLCaps::hasMultiTexture()
{
  return _glcapsPrivate().ext_ARB_multitexture;
}


bool GLCaps::ext_ARB_shadow()
{
  return _glcapsPrivate().ext_ARB_shadow;
}


bool GLCaps::ext_SGIX_shadow()
{
  return _glcapsPrivate().ext_SGIX_shadow;
}


bool GLCaps::ext_ARB_depth_texture()
{
  return _glcapsPrivate().ext_ARB_depth_texture;
}


bool GLCaps::ext_SGIX_depth_texture()
{
  return _glcapsPrivate().ext_SGIX_depth_texture;
}


bool GLCaps::ext_ARB_texture_cube_map()
{
  return _glcapsPrivate().ext_ARB_texture_cube_map;
}


bool GLCaps::ext_EXT_texture_cube_map()
{
  return _glcapsPrivate().ext_EXT_texture_cube_map;
}


void GLCaps::glActiveTexture( GLenum x )
{
  _glcapsPrivate().glActiveTexture( x );
}


void GLCaps::glClientActiveTexture( GLenum x )
{
  _glcapsPrivate().glClientActiveTexture( x );
}

bool & GLCaps::mustRefresh()
{
  static bool r = true;
  return r;
}

unsigned GLCaps::numTextureUnits()
{
  if (mustRefresh())
  {
    // Number of texture units may have been processed in an unavailable context
    // try to refresh it
    updateTextureUnits();
    mustRefresh() = false;
  }

  return _glcapsPrivate().numTextureUnits;
}


GLenum GLCaps::textureID( unsigned x )
{
#ifdef GL_VERSION_1_3

  static unsigned	texids[] = { GL_TEXTURE0, GL_TEXTURE1, GL_TEXTURE2, 
                                     GL_TEXTURE3, GL_TEXTURE4, GL_TEXTURE5, 
                                     GL_TEXTURE6, GL_TEXTURE7, GL_TEXTURE8, };
  if( x > 8 )
    return texids[0] + x;
  return texids[ x ];

#else
#ifdef GL_ARB_multitexture

  static unsigned	texids[] = { GL_TEXTURE0_ARB, GL_TEXTURE1_ARB, 
                                     GL_TEXTURE2_ARB, GL_TEXTURE3_ARB, 
				     GL_TEXTURE4_ARB, GL_TEXTURE5_ARB, 
                                     GL_TEXTURE6_ARB, GL_TEXTURE7_ARB, 
				     GL_TEXTURE8_ARB, };
  if( x > 8 )
    return texids[0] + x;
  return texids[ x ];

#else

  return 0;

#endif
#endif
}


bool GLCaps::hasGlBlendEquation()
{
  return _glcapsPrivate().glBlendEquation;
}


void GLCaps::glBlendEquation( GLenum x )
{
  _glcapsPrivate().glBlendEquation( x );
}


bool GLCaps::hasGlTexImage3D()
{
  return _glcapsPrivate().glTexImage3D;
}


void GLCaps::glTexImage3D( GLenum target, GLint level, GLenum internalformat, 
    GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, 
    GLenum type, const void* data )
{
  _glcapsPrivate().glTexImage3D( target, level, internalformat, width, height, 
                                 depth, border, format, type, data );
}


void GLCaps::updateTextureUnits()
{
  _glcapsPrivate().updateTextureUnits();
}


bool GLCaps::hasFramebuffer()
{
  return _glcapsPrivate().glBindFramebuffer;
}


void GLCaps::glBindFramebuffer( GLenum target, GLuint framebuffer )
{
  _glcapsPrivate().glBindFramebuffer( target, framebuffer );
}


void GLCaps::glBindRenderbuffer( GLenum target, GLuint renderbuffer )
{
  _glcapsPrivate().glBindRenderbuffer( target, renderbuffer );
}


void GLCaps::glFramebufferTexture2D( GLenum target, GLenum attachment,
                                     GLenum textarget, GLuint texture,
                                     GLint level )
{
  _glcapsPrivate().glFramebufferTexture2D( target, attachment, textarget,
                                           texture, level );
}


void GLCaps::glGenFramebuffers( GLsizei n, GLuint *ids )
{
  _glcapsPrivate().glGenFramebuffers( n, ids );
}


void GLCaps::glGenRenderbuffers( GLsizei n, GLuint *renderbuffers )
{
  _glcapsPrivate().glGenRenderbuffers( n, renderbuffers );
}


void GLCaps::glFramebufferRenderbuffer( GLenum target, GLenum attachment,
                                        GLenum renderbuffertarget,
                                        GLuint renderbuffer )
{
  _glcapsPrivate().glFramebufferRenderbuffer( target, attachment,
                                              renderbuffertarget,
                                              renderbuffer );
}


void GLCaps::glRenderbufferStorage( GLenum target, GLenum internalformat,
                                    GLsizei width, GLsizei height )
{
  _glcapsPrivate().glRenderbufferStorage( target, internalformat, width,
                                          height );
}

