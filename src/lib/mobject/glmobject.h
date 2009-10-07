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

#ifndef ANA_MOBJECT_GLMOBJECT_H
#define ANA_MOBJECT_GLMOBJECT_H

#include <anatomist/surface/glcomponent.h>
#include <anatomist/mobject/MObject.h>

namespace anatomist
{

  /** A Multi-object with OpenGL rendering capabilities.
      Contrarily to other list and vector variants of MObject, this one
      is not designed as a sequence of children objects, but a
      recombination of several rendering parts: geometry, texture...
  */
  class GLMObject : public MObject, public GLComponent
  {
  public:
    GLMObject();
    virtual ~GLMObject();

    virtual const GLComponent* glAPI() const { return this; }
    virtual GLComponent* glAPI() { return this; }

    virtual const Material *glMaterial() const;
    virtual const AObjectPalette* glPalette( unsigned tex = 0 ) const;
    virtual void update( const Observable*, void* );
    virtual bool render( PrimList &, const ViewState & vs );

    virtual void glSetChanged( glPart, bool x = true ) const;
    virtual void glClearHasChangedFlags() const;
    virtual unsigned glNumVertex( const ViewState & ) const;
    virtual const GLfloat* glVertexArray( const ViewState & ) const;
    /// normals array (optional), default=0 (no normals, flat shaded faces)
    virtual const GLfloat* glNormalArray( const ViewState & ) const;
    virtual unsigned glPolygonSize( const ViewState & ) const;
    virtual unsigned glNumPolygon( const ViewState & ) const;
    /// number of vertices per polygon (default = 3: triangles)
    virtual const GLuint* glPolygonArray( const ViewState & ) const;

    virtual void glSetTexImageChanged( bool x = true, unsigned tex = 0 ) const;
    virtual void glSetTexEnvChanged( bool x = true, unsigned tex = 0 ) const;
    virtual unsigned glNumTextures() const;
    virtual unsigned glNumTextures( const ViewState & ) const;
    virtual glTextureMode glTexMode( unsigned tex = 0 ) const;
    virtual void glSetTexMode( glTextureMode mode, unsigned tex = 0 );
    virtual float glTexRate( unsigned tex = 0 ) const;
    virtual void glSetTexRate( float rate, unsigned tex = 0 );
    virtual glTextureFiltering glTexFiltering( unsigned tex = 0 ) const;
    virtual void glSetTexFiltering( glTextureFiltering x, unsigned tex = 0 );
    virtual void glSetTexRGBInterpolation( bool x, unsigned tex = 0 );
    virtual bool glTexRGBInterpolation( unsigned tex = 0 ) const;
    virtual glAutoTexturingMode glAutoTexMode( unsigned tex = 0 ) const;
    virtual void glSetAutoTexMode( glAutoTexturingMode mode, 
                                   unsigned tex = 0 );
    virtual const float *glAutoTexParams( unsigned coord = 0, 
                                          unsigned tex = 0 ) const;
    virtual void glSetAutoTexParams( const float* params, unsigned coord = 0, 
                                     unsigned tex = 0 );
    virtual bool glTexImageChanged( unsigned tex = 0 ) const;
    virtual bool glTexEnvChanged( unsigned tex = 0 ) const;
    virtual unsigned glDimTex( const ViewState &, unsigned tex = 0 ) const;
    virtual unsigned glTexCoordSize( const ViewState &, 
                                     unsigned tex = 0 ) const;
    virtual const GLfloat* glTexCoordArray( const ViewState &, 
                                            unsigned tex = 0 ) const;

    virtual bool glMakeTexImage( const ViewState & state, 
                                 const GLTexture & gltex, unsigned tex ) const;
    virtual bool glMakeTexEnvGLL( const ViewState & state, 
                                  const GLList & gllist, unsigned tex ) const;
    virtual void glSetMaterialGLL( const std::string & state, RefGLItem x );
    virtual void glSetTexNameGLL( const std::string & state, RefGLItem x, 
                                  unsigned tex = 0 );
    virtual GLPrimitives glMaterialGLL( const ViewState & state ) const;
    virtual GLPrimitives glTexNameGLL( const ViewState &, 
                                       unsigned tex = 0 ) const;
    virtual GLPrimitives glTexEnvGLL( const ViewState &, 
                                      unsigned tex = 0 ) const;
    virtual void glGarbageCollector( int nkept = -1 );
/*    virtual void glBeforeBodyGLL( const ViewState & state, 
				  GLPrimitives & pl ) const;
    virtual void glAfterBodyGLL( const ViewState & state, 
				 GLPrimitives & pl ) const;*/
    virtual AObjectPalette* palette();
    virtual const AObjectPalette* palette() const;

    // new functions
    virtual GLComponent* glGeometry();
    virtual GLComponent* glTexture( unsigned n = 0 );
    virtual const GLComponent* glGeometry() const;
    virtual const GLComponent* glTexture( unsigned n = 0 ) const;
    virtual std::string viewStateID( glPart part, const ViewState & ) const;

  protected:
    /** get to geometry object referential before drawing.
	This is an internal function not intended to be used by end users */
    virtual bool glToRef( const Referential* objref, GLPrimitives & pl ) const;

  private:
    struct Private;
    Private *d;
  };

}


#endif


