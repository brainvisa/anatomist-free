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


#ifndef ANA_SURFACE_MTEXTURE_H
#define ANA_SURFACE_MTEXTURE_H

#include <anatomist/mobject/globjectvector.h>
#include <anatomist/object/actions.h>
#include <graph/tree/tree.h>

namespace anatomist
{

  /** Multi-texture

  Multi-textures are rendered using multitexturing in OpenGL. To do this, the 
  OpenGL implementation must have multitexturing capabilities (extension), and 
  a sufficient number of textures units to render all textures involved in 
  the multitexture. This is hardware-dependent.
  */
  class AMTexture : public GLObjectVector
  {
  public:
    AMTexture( const std::vector<AObject *> & obj );
    virtual ~AMTexture();

    virtual int MType() const { return type(); }
    static int classType();

    virtual const GLComponent* glAPI() const { return this; }
    virtual GLComponent* glAPI() { return this; }

    virtual const AObjectPalette* glPalette( unsigned tex = 0 ) const;
    virtual unsigned glNumTextures() const;
    virtual unsigned glNumTextures( const ViewState & ) const;
    virtual unsigned glDimTex( const ViewState &, unsigned tex = 0 ) const;
    virtual unsigned glTexCoordSize( const ViewState &, 
                                     unsigned tex = 0 ) const;
    virtual const float* glTexCoordArray( const ViewState &, 
                                          unsigned tex = 0 ) const;

    virtual void glSetTexMode( glTextureMode mode, unsigned tex = 0 );
    virtual glTextureMode glTexMode( unsigned tex = 0 ) const;
    virtual glAutoTexturingMode glAutoTexMode( unsigned tex = 0 ) const;
    virtual void glSetAutoTexMode( glAutoTexturingMode mode, 
                                   unsigned tex = 0 );
    virtual const float *glAutoTexParams( unsigned coord = 0, 
                                          unsigned tex = 0 ) const;
    virtual void glSetAutoTexParams( const float* params, unsigned coord = 0, 
                                     unsigned tex = 0 );
    virtual void glSetTexRate( float rate, unsigned tex = 0 );
    virtual float glTexRate( unsigned tex = 0 ) const;
    virtual glTextureFiltering glTexFiltering( unsigned tex = 0 ) const;
    virtual void glSetTexFiltering( glTextureFiltering x, unsigned tex = 0 );
    virtual void glSetTexRGBInterpolation( bool x, unsigned tex = 0 );
    virtual bool glTexRGBInterpolation( unsigned tex = 0 ) const;

    virtual GLPrimitives glTexNameGLL( const ViewState &, 
                                       unsigned tex = 0 ) const;
    virtual void glGarbageCollector( int nkept = -1 );

    virtual bool Is2DObject() { return false; }
    virtual bool Is3DObject() { return false; }
    virtual bool CanRemove( AObject* obj );
    virtual void update( const Observable*, void* );
    virtual Tree* optionTree() const;
    virtual GLComponent* glTexture( unsigned n = 0 );
    virtual const GLComponent* glTexture( unsigned n = 0 ) const;

  private:
    static int registerClass();
  };

}

#endif

