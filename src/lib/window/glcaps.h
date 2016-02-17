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


#ifndef ANATOMIST_WINDOW_GLCAPS_H
#define ANATOMIST_WINDOW_GLCAPS_H

// this seems needed on old linux distributions (Fedora 4)
#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif

#ifdef _WIN32 // TODO: remove this workaround (added by Denis for Windows)
#include <qgl.h>
#ifndef HAS_GLEXT
#  define HAS_GLEXT
#endif
#endif

#if defined(__APPLE__)
/* OpenGL on Mac uses non-standard include paths, it would have been 
   too simple and too much like all other systems, they definitely 
   needed to "think different"... (even Windows is more standard !)
*/
# include <OpenGL/gl.h>
# include <OpenGL/glu.h>
# ifdef HAS_GLEXT
#  include <OpenGL/glext.h>
# endif
#else
# include <GL/gl.h>
# include <GL/glu.h>
# ifdef HAS_GLEXT
#  include <GL/glext.h>
# endif
#endif

namespace anatomist
{

  /** OpenGL capabilities

  This class gives characteristics about OpenGL functions and extensions that 
  may or may not be available at run-time
   */
  class GLCaps
  {
  public:
    static bool & mustRefresh();
    static bool hasMultiTexture();
    static bool ext_ARB_shadow();
    static bool ext_SGIX_shadow();
    static bool ext_ARB_depth_texture();
    static bool ext_SGIX_depth_texture();
    static bool ext_ARB_texture_cube_map();
    static bool ext_EXT_texture_cube_map();
    static void glActiveTexture( GLenum );
    static void glClientActiveTexture( GLenum );
    static unsigned numTextureUnits();
    static GLenum textureID( unsigned );
    static bool hasGlBlendEquation();
    static void glBlendEquation( GLenum );
    static bool hasGlTexImage3D();
    static void glTexImage3D( GLenum, GLint, GLenum, GLsizei, GLsizei, 
                              GLsizei, GLint, GLenum, GLenum, const void* );
    static void glMultiTexCoord3f( GLenum target, GLfloat s, GLfloat t,
                                   GLfloat r );

    static bool hasFramebuffer();
    static void glBindFramebuffer( GLenum target, GLuint framebuffer );
    static void glBindRenderbuffer( GLenum target, GLuint renderbuffer );
    static void glFramebufferTexture2D( GLenum target, GLenum attachment,
                                        GLenum textarget, GLuint texture,
                                        GLint level );
    static void glGenFramebuffers( GLsizei n, GLuint *ids );
    static void glGenRenderbuffers( GLsizei n, GLuint *renderbuffers );
    static void glFramebufferRenderbuffer( GLenum target, GLenum attachment,
                                           GLenum renderbuffertarget,
                                           GLuint renderbuffer );
    static void glRenderbufferStorage( GLenum target, GLenum internalformat, 
                                       GLsizei width, GLsizei height );
    static void glUniform1f( GLint location, GLfloat v0 );
    static void glUniform1i( GLint location, GLint v0 );
    static void glUniform4fv( GLint location, GLsizei count,
                                  const GLfloat *value );
    static GLint glGetUniformLocation( GLuint program,
                                           const GLchar *name );
    static void glMultTransposeMatrixf( const GLfloat m[16] );
    static void glAttachShader( GLuint program, GLuint shader );
    static void glDetachShader( GLuint program, GLuint shader );
    static void glCompileShader( GLuint shader );
    static GLuint glCreateProgram();
    static GLuint glCreateShader( GLenum type );
    static void glDeleteProgram( GLuint program );
    static void glDeleteShader( GLuint shader );
    static void glGetProgramiv( GLuint program, GLenum pname, GLint *params );
    static void glGetShaderiv( GLuint shader, GLenum pname, GLint *params );
    static void glLinkProgram( GLuint program );
    static void glShaderSource( GLuint shader, GLsizei count,
                                const GLchar *const*string,
                                const GLint *length );
    static void glUseProgram( GLuint program );

    /// re-calculate number of texture units
    static void updateTextureUnits();
  };

}

#endif
