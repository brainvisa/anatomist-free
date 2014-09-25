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
#include <stdarg.h>
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
  typedef PFNGLMULTITEXCOORD3FARBPROC glMultiTexCoord3fFunc;
  typedef PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebufferFunc;
  typedef PFNGLBINDRENDERBUFFEREXTPROC glBindRenderbufferFunc;
  typedef PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2DFunc;
  typedef PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffersFunc;
  typedef PFNGLGENRENDERBUFFERSEXTPROC glGenRenderbuffersFunc;
  typedef PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC glFramebufferRenderbufferFunc;
  typedef PFNGLRENDERBUFFERSTORAGEEXTPROC glRenderbufferStorageFunc;
  typedef PFNGLUNIFORM1FARBPROC glUniform1fFunc;
  typedef PFNGLUNIFORM1IARBPROC glUniform1iFunc;
  typedef PFNGLUNIFORM4FVARBPROC glUniform4fvFunc;
  typedef PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocationFunc;
#else
  typedef void (*glActiveTextureFunc)( GLenum );
  typedef void (*glBlendEquationFunc)( GLenum );
  typedef void (*glTexImage3DFunc)( GLenum, GLint, GLenum, GLsizei, GLsizei,
                                    GLsizei, GLint, GLenum, GLenum,
                                    const void* );
  typedef void (*glMultiTexCoord3fFunc)( GLenum target, GLfloat s, GLfloat t,
                                         GLfloat r );
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
  typedef void (*glUniform1fFunc)( GLint location, GLfloat v0 );
  typedef void (*glUniform1iFunc)( GLint location, GLint v0 );
  typedef void (*glUniform4fvFunc)( GLint location, GLsizei count,
                                    const GLfloat *value );
  typedef GLint (*glGetUniformLocationFunc)( GLuint program,
                                             const GLchar *name );
#endif
  typedef void (APIENTRYP glMultTransposeMatrixfFunc)( const GLfloat m[16] );
  // sharders API
  typedef void (APIENTRYP glAttachShaderFunc)( GLuint program, GLuint shader );
  typedef void (APIENTRYP glDetachShaderFunc)( GLuint program, GLuint shader );
  typedef void (APIENTRYP glCompileShaderFunc)( GLuint shader );
  typedef GLuint (APIENTRYP glCreateProgramFunc)( void );
  typedef GLuint (APIENTRYP glCreateShaderFunc)( GLenum type );
  typedef void (APIENTRYP glDeleteProgramFunc)( GLuint program );
  typedef void (APIENTRYP glDeleteShaderFunc)( GLuint shader );
  typedef void (APIENTRYP glGetProgramivFunc)( GLuint program, GLenum pname,
                                               GLint *params );
  typedef void (APIENTRYP glGetShaderivFunc)( GLuint shader, GLenum pname,
                                              GLint *params );
  typedef void (APIENTRYP glLinkProgramFunc)( GLuint program );
  typedef void (APIENTRYP glShaderSourceFunc)( GLuint shader, GLsizei count,
                                               const GLchar *const*string,
                                               const GLint *length );
  typedef void (APIENTRYP glUseProgramFunc)( GLuint program );

  /* the APIENTRY macro here seems to be very important on Windows: it caused
     display bugs and crashed for one year without it. */
  void APIENTRY _void_glActiveTexture( GLenum )
  {
  }


  void APIENTRY _void_glBlendEquation( GLenum )
  {
  }

  void APIENTRY _void_glTexImage3D( GLenum, GLint, GLenum, GLsizei, GLsizei,
                                    GLsizei, GLint, GLenum, GLenum,
                                    const void* )
  {
  }

  void APIENTRY _void_glMultiTexCoord3f( GLenum target, GLfloat s, GLfloat t,
                                         GLfloat r )
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

  void APIENTRY _void_glMultTransposeMatrix( const GLfloat m[16] )
  {
  }

  void APIENTRY _void_glUniform1f( GLint location, GLfloat v0 )
  {
  }

  void APIENTRY _void_glUniform1i( GLint location, GLint v0 )
  {
  }

  void APIENTRY _void_glUniform4fvf( GLint location, GLsizei count,
                            const GLfloat *value )
  {
  }

  GLint APIENTRY _void_glGetUniformLocation( GLuint program,
                                             const GLchar *name )
  {
    return 0;
  }

  void APIENTRY _void_glAttachShader( GLuint program, GLuint shader )
  {
  }

  void APIENTRY _void_glDetachShader( GLuint program, GLuint shader )
  {
  }

  void APIENTRY _void_glCompileShader( GLuint shader )
  {
  }

  GLuint APIENTRY _void_glCreateProgram( void )
  {
  }

  GLuint APIENTRY _void_glCreateShader( GLenum type )
  {
  }

  void APIENTRY _void_glDeleteProgram( GLuint program )
  {
  }

  void APIENTRY _void_glDeleteShader( GLuint shader )
  {
  }

  void APIENTRY _void_glGetProgramiv( GLuint program, GLenum pname,
                                      GLint *params )
  {
  }

  void APIENTRY _void_glGetShaderiv( GLuint shader, GLenum pname,
                                     GLint *params )
  {
  }

  void APIENTRY _void_glLinkProgram( GLuint program )
  {
  }

  void APIENTRY _void_glShaderSource( GLuint shader, GLsizei count,
                                      const GLchar *const*string,
                                      const GLint *length )
  {
  }

  void APIENTRY _void_glUseProgram( GLuint program )
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
    glMultiTexCoord3fFunc glMultiTexCoord3f;
    glBindFramebufferFunc glBindFramebuffer;
    glBindRenderbufferFunc glBindRenderbuffer;
    glFramebufferTexture2DFunc glFramebufferTexture2D;
    glGenFramebuffersFunc glGenFramebuffers;
    glGenRenderbuffersFunc glGenRenderbuffers;
    glFramebufferRenderbufferFunc glFramebufferRenderbuffer;
    glRenderbufferStorageFunc glRenderbufferStorage;
    glUniform1fFunc glUniform1f;
    glUniform1iFunc glUniform1i;
    glUniform4fvFunc glUniform4fv;
    glGetUniformLocationFunc glGetUniformLocation;
    glMultTransposeMatrixfFunc glMultTransposeMatrixf;
    glAttachShaderFunc glAttachShader;
    glDetachShaderFunc glDetachShader;
    glCompileShaderFunc glCompileShader;
    glCreateProgramFunc glCreateProgram;
    glCreateShaderFunc glCreateShader;
    glDeleteProgramFunc glDeleteProgram;
    glDeleteShaderFunc glDeleteShader;
    glGetProgramivFunc glGetProgramiv;
    glGetShaderivFunc glGetShaderiv;
    glLinkProgramFunc glLinkProgram;
    glShaderSourceFunc glShaderSource;
    glUseProgramFunc glUseProgram;
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


#ifndef _WIN32
  template <typename funcType>
  funcType find_symbol( void* handle, funcType fallback, ... )
  {
    va_list ap;
    va_start(ap, fallback);
    const char *i = 0, *first = 0;
    funcType procAddress = 0;
    for(first=i=va_arg( ap, const char * ); !procAddress && i!=0;
        i = va_arg( ap, const char * ))
      procAddress = (funcType) dlsym( handle, i );
    va_end(ap);

    if( !procAddress )
    {
      cerr << "coud not find function " << first << ": "
            << dlerror() << endl;
      procAddress = fallback;
    }
    else
      cout << "function " << first << " found." << endl;

    return procAddress;
  }
#else
  template <typename funcType>
  funcType find_symbol( void*, funcType fallback, ... )
  {
    va_list ap;
    va_start(ap, fallback);
    const char *i = 0, *first = 0;
    funcType procAddress = 0;
    for(first=i=va_arg( ap, const char * ); !procAddress && i!=0;
        i = va_arg( ap, const char * ))
      procAddress = (funcType) wglGetProcAddress( i );

    if( !procAddress )
    {
      cerr << "coud not find function " << first << endl;
      procAddress = fallback;
    }
    else
      cout << "function " << first << " found." << endl;

    return procAddress;
  }
#endif


  GLCapsPrivate::GLCapsPrivate()
    : ext_ARB_multitexture( false ), ext_ARB_shadow( false ),
      ext_SGIX_shadow( false ), ext_ARB_depth_texture( false ),
      ext_SGIX_depth_texture( false ),
      glActiveTexture( _void_glActiveTexture ),
      glClientActiveTexture( _void_glActiveTexture ), numTextureUnits( 0 ),
      depthpeeling( false ), glBlendEquation( 0 ), glTexImage3D( 0 ),
      glMultiTexCoord3f( 0 ),
      glBindFramebuffer( 0 ), glBindRenderbuffer( 0 ),
      glFramebufferTexture2D( 0 ), glGenFramebuffers( 0 ),
      glGenRenderbuffers( 0 ), glFramebufferRenderbuffer( 0 ), glUniform1f( 0 ),
      glUniform1i( 0 ), glUniform4fv( 0 ), glGetUniformLocation( 0 ),
      glMultTransposeMatrixf( 0 ), glAttachShader( 0 ), glDetachShader( 0 ),
      glCompileShader( 0 ), glCreateProgram( 0 ), glCreateShader( 0 ),
      glDeleteProgram( 0 ), glDeleteShader( 0 ), glGetProgramiv( 0 ),
      glGetShaderiv( 0 ), glLinkProgram( 0 ), glShaderSource( 0 ),
      glUseProgram( 0 )
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

  void *handle = 0; // just to have same API as unix
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
#endif
        glActiveTexture = find_symbol(
          handle, _void_glActiveTexture,
          "glActiveTexture", "_glActiveTexture", "glActiveTextureARB",
          "_glActiveTextureARB", NULL );
        /* cout << "glActiveTexture address: " << (void *) glActiveTexture
            << endl;
        cout << (void *) &::glActiveTexture << endl;
        cout << (void *) &::glClientActiveTextureARB << endl; */

        glClientActiveTexture	= find_symbol(
          handle, _void_glActiveTexture, "glClientActiveTexture",
          "_glClientActiveTexture", "glClientActiveTextureARB",
          "_glClientActiveTextureARB", NULL );
        glBlendEquation = find_symbol(
          handle, _void_glBlendEquation,
          "glBlendEquation", "_glBlendEquation",
          "glBlendEquationEXT", "_glBlendEquationEXT", NULL );
        glTexImage3D = find_symbol(
          handle, _void_glTexImage3D,
          "glTexImage3D", "_glTexImage3D",
          "glTexImage3DEXT", "_glTexImage3DEXT", NULL );
        glMultiTexCoord3f = find_symbol(
          handle, _void_glMultiTexCoord3f,
          "glMultiTexCoord3f", "_glMultiTexCoord3f", "glMultiTexCoord3fARB",
          "_glMultiTexCoord3fARB", NULL );
        glBindFramebuffer = find_symbol(
          handle, _void_glBindFramebuffer,
          "glBindFramebuffer", "_glBindFramebuffer", "glBindFramebufferEXT",
          "_glBindFramebufferEXT", NULL );
        glBindRenderbuffer = find_symbol(
          handle, _void_glBindRenderbuffer,
          "glBindRenderbuffer", "_glBindRenderbuffer",
          "glBindRenderbufferEXT", "_glBindRenderbufferEXT", NULL );
        glFramebufferTexture2D = find_symbol(
          handle, _void_glFramebufferTexture2D,
          "glFramebufferTexture2D", "_glFramebufferTexture2D",
          "glFramebufferTexture2DEXT", "_glFramebufferTexture2DEXT", NULL );
        glGenFramebuffers = find_symbol(
          handle, _void_glGenFramebuffers,
          "glGenFramebuffers", "_glGenFramebuffers",
          "glGenFramebuffersEXT", "_glGenFramebuffersEXT", NULL );
        glGenRenderbuffers = find_symbol(
          handle, _void_glGenRenderbuffers,
          "glGenRenderbuffers", "_glGenRenderbuffers",
          "glGenRenderbuffersEXT", "_glGenRenderbuffersEXT", NULL );
        glFramebufferRenderbuffer = find_symbol(
          handle, _void_glFramebufferRenderbuffer,
          "glFramebufferRenderbuffer", "_glFramebufferRenderbuffer",
          "glFramebufferRenderbufferEXT", "_glFramebufferRenderbufferEXT",
          NULL );
        glRenderbufferStorage = find_symbol(
          handle, _void_glRenderbufferStorage,
          "glRenderbufferStorage", "_glRenderbufferStorage",
          "glRenderbufferStorageEXT", "_glRenderbufferStorageEXT", NULL );

#ifndef _WIN32
      }
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

#ifndef _WIN32
  if( handle )
  {
#endif
    glUniform1f = find_symbol(
      handle, _void_glUniform1f, "glUniform1f", "_glUniform1f",
      "glUniform1fARB", "_glUniform1fARB", NULL );
    glUniform1i = find_symbol(
      handle, _void_glUniform1i, "glUniform1i", "_glUniform1i",
      "glUniform1iARB", "_glUniform1iARB", NULL );
    glUniform4fv = find_symbol(
      handle, _void_glUniform4fvf, "glUniform4fv", "_glUniform4fv",
      "glUniform4fvARB", "_glUniform4fvARB", NULL );
    glGetUniformLocation = find_symbol(
      handle, _void_glGetUniformLocation,
      "glGetUniformLocation", "_glGetUniformLocation",
      "glGetUniformLocationARB", "_glGetUniformLocationARB", NULL );
    glMultTransposeMatrixf = find_symbol(
      handle, _void_glMultTransposeMatrix,
      "glMultTransposeMatrixf", "_glMultTransposeMatrixf",
      "glMultTransposeMatrixfARB", "_glMultTransposeMatrixfARB", NULL );
    glAttachShader = find_symbol(
      handle, _void_glAttachShader,
      "glAttachShader", "_glAttachShader", NULL );
    glDetachShader = find_symbol(
      handle, _void_glDetachShader,
      "glDetachShader", "_glDetachShader", NULL );
    glCompileShader = find_symbol(
      handle, _void_glCompileShader,
      "glCompileShader", "_glCompileShader", NULL );
    glCreateProgram = find_symbol(
      handle, _void_glCreateProgram,
      "glCreateProgram", "_glCreateProgram", NULL );
    glCreateShader = find_symbol(
      handle, _void_glCreateShader,
      "glCreateShader", "_glCreateShader", NULL );
    glDeleteProgram = find_symbol(
      handle, _void_glDeleteProgram,
      "glDeleteProgram", "_glDeleteProgram", NULL );
    glDeleteShader = find_symbol(
      handle, _void_glDeleteShader,
      "glDeleteShader", "_glDeleteShader", NULL );
    glGetProgramiv = find_symbol(
      handle, _void_glGetProgramiv,
      "glGetProgramiv", "_glGetProgramiv", NULL );
    glGetShaderiv = find_symbol(
      handle, _void_glGetShaderiv,
      "glGetShaderiv", "_glGetShaderiv", NULL );
    glLinkProgram = find_symbol(
      handle, _void_glLinkProgram,
      "glLinkProgram", "_glLinkProgram", NULL );
    glShaderSource = find_symbol(
      handle, _void_glShaderSource,
      "glShaderSource", "_glShaderSource", NULL );
    glUseProgram = find_symbol(
      handle, _void_glUseProgram,
      "glUseProgram", "_glUseProgram", NULL );
#ifndef _WIN32
  }
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


void GLCaps::glMultiTexCoord3f( GLenum target, GLfloat s, GLfloat t,
                                   GLfloat r )
{
  _glcapsPrivate().glMultiTexCoord3f( target, s, t, r );
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

void GLCaps::glUniform1f( GLint location, GLfloat v0 )
{
  _glcapsPrivate().glUniform1f( location, v0 );
}


void GLCaps::glUniform1i( GLint location, GLint v0 )
{
  _glcapsPrivate().glUniform1i( location, v0 );
}


void GLCaps::glUniform4fv( GLint location, GLsizei count, const GLfloat *value )
{
  _glcapsPrivate().glUniform4fv( location, count, value );
}


GLint GLCaps::glGetUniformLocation( GLuint program, const GLchar *name )
{
  return _glcapsPrivate().glGetUniformLocation( program, name );
}


void GLCaps::glMultTransposeMatrixf( const GLfloat m[16] )
{
  _glcapsPrivate().glMultTransposeMatrixf( m );
}


void GLCaps::glAttachShader( GLuint program, GLuint shader )
{
  _glcapsPrivate().glAttachShader( program, shader );
}


void GLCaps::glDetachShader( GLuint program, GLuint shader )
{
  _glcapsPrivate().glDetachShader( program, shader );
}


void GLCaps::glCompileShader( GLuint shader )
{
  _glcapsPrivate().glCompileShader( shader );
}


GLuint GLCaps::glCreateProgram()
{
  return _glcapsPrivate().glCreateProgram();
}


GLuint GLCaps::glCreateShader( GLenum type )
{
  return _glcapsPrivate().glCreateShader( type );
}


void GLCaps::glDeleteProgram( GLuint program )
{
  _glcapsPrivate().glDeleteProgram( program );
}


void GLCaps::glDeleteShader( GLuint shader )
{
  _glcapsPrivate().glDeleteShader( shader );
}


void GLCaps::glGetProgramiv( GLuint program, GLenum pname, GLint *params )
{
  _glcapsPrivate().glGetProgramiv( program, pname, params );
}


void GLCaps::glGetShaderiv( GLuint shader, GLenum pname, GLint *params )
{
  _glcapsPrivate().glGetShaderiv( shader, pname, params );
}


void GLCaps::glLinkProgram( GLuint program )
{
  _glcapsPrivate().glLinkProgram( program );
}


void GLCaps::glShaderSource( GLuint shader, GLsizei count,
                             const GLchar *const*string,
                             const GLint *length )
{
  _glcapsPrivate().glShaderSource( shader, count, string, length );
}


void GLCaps::glUseProgram( GLuint program )
{
  _glcapsPrivate().glUseProgram( program );
}


