/* Copyright (c) 1995-2005 CEA
 *
 *  This software and supporting documentation were developed by
 *      CEA/DSV/SHFJ
 *      4 place du General Leclerc
 *      91401 Orsay cedex
 *      France
 *
 * This software is governed by the CeCILL license version 2 under 
 * French law and abiding by the rules of distribution of free software.
 * You can  use, modify and/or redistribute the software under the 
 * terms of the CeCILL license version 2 as circulated by CEA, CNRS
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
 * knowledge of the CeCILL license version 2 and that you accept its terms.
 */


#ifndef ANA_OBJECT_SLICEABLE_H
#define ANA_OBJECT_SLICEABLE_H

#include <anatomist/surface/globject.h>
#include <aims/vector/vector.h>

namespace carto
{
  template <typename T> class Volume;
  template <typename T> class VolumeRef;
}

class AimsRGBA;

namespace anatomist
{

  struct AImage;
  class SliceViewState;


  /** Sliceable objects can draw themselves in a 2D texture. The slice 
      information is provided on demand as a ViewState information.
  */
  class Sliceable : public GLComponent
  {
  public:
    Sliceable();
    virtual ~Sliceable();

    virtual const Sliceable* sliceableAPI() const { return this; }
    virtual Sliceable* sliceableAPI() { return this; }

    virtual unsigned glNumVertex( const ViewState & ) const;
    virtual const GLfloat* glVertexArray( const ViewState & ) const;
    virtual const GLfloat* glNormalArray( const ViewState & state ) const;
    virtual unsigned glPolygonSize( const ViewState & ) const;
    virtual unsigned glNumPolygon( const ViewState & ) const;
    virtual const GLuint* glPolygonArray( const ViewState & ) const;

    virtual unsigned glDimTex( const ViewState &, unsigned tex = 0 ) const;
    virtual unsigned glTexCoordSize( const ViewState &, 
                                     unsigned tex = 0 ) const;
    virtual const GLfloat* glTexCoordArray( const ViewState &, 
                                            unsigned tex = 0 ) const;

    virtual bool glMakeTexImage( const ViewState & state, 
                                 const GLTexture & gltex, unsigned tex ) const;

    /// this function must be overloaded to actually fill the slice image
    virtual bool update2DTexture( AImage &, const Point3df & posbase, 
                                  const SliceViewState &, 
                                  unsigned tex = 0 ) const;
    /// fills a resampled RGBA volume
    virtual carto::VolumeRef<AimsRGBA> rgbaVolume( const SliceViewState* = 0,
        int tex = 0 ) const;
    /// same as the other rgbaVolume() but fills an already allocated volume
    virtual void rgbaVolume( carto::Volume<AimsRGBA> &,
                             const SliceViewState* = 0, int tex = 0 ) const;
    virtual Point3df glVoxelSize() const;
    virtual Point4df glMin2D() const = 0;
    virtual Point4df glMax2D() const = 0;
    virtual bool glAllowedTexRGBInterpolation( unsigned tex = 0 ) const;
    virtual const Referential *getReferential() const = 0;
    virtual std::string viewStateID( glPart part, const ViewState & ) const;

  private:
    struct Private;
    Private	*d;
  };


  /** AObject-inherited version of Sliceable.
  */
  class SliceableObject : public AObject, public Sliceable
  {
  public:
    SliceableObject();
    virtual ~SliceableObject();

    virtual const GLComponent* glAPI() const { return this; }
    virtual GLComponent* glAPI() { return this; }
    virtual const Sliceable* sliceableAPI() const { return this; }
    virtual Sliceable* sliceableAPI() { return this; }

    virtual Point3df glVoxelSize() const;
    virtual Point4df glMin2D() const;
    virtual Point4df glMax2D() const;

    virtual void glSetChanged( glPart, bool = true ) const;
    virtual void glSetTexImageChanged( bool x = true, unsigned tex = 0 ) const;
    virtual void glSetTexEnvChanged( bool x = true, unsigned tex = 0 ) const;

    virtual const Material *glMaterial() const;
    virtual const AObjectPalette* glPalette( unsigned tex = 0 ) const;
    const Referential *getReferential() const;
  };

}

#endif

