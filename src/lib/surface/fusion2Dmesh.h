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

#ifndef ANA_SURFACE_FUSION2DMESH_H
#define ANA_SURFACE_FUSION2DMESH_H

#include <anatomist/mobject/objectVector.h>
#include <anatomist/object/sliceable.h>
#include <anatomist/surface/triangulated.h>

namespace anatomist
{
  /**	Anatomist object which contains several meshes: it displays
    *  the mesh polygons that intersect the current slice plane
    */
  class Fusion2DMesh : public ObjectVector, public Sliceable
  {
  public:
    Fusion2DMesh( const std::vector<AObject *> & obj );
    virtual ~Fusion2DMesh();

    virtual const GLComponent* glAPI() const
    {
      return this;
    }
    virtual GLComponent* glAPI()
    {
      return this;
    }
    virtual const Sliceable* sliceableAPI() const
    {
      return this;
    }
    virtual Sliceable* sliceableAPI()
    {
      return this;
    }

    virtual bool Is2DObject()
    {
      return true;
    }

    virtual bool render( PrimList &, const ViewState & );
    virtual void update( const Observable *, void * );

    virtual const Material * glMaterial() const;
    virtual unsigned glNumVertex( const ViewState & ) const;
    virtual const GLfloat* glVertexArray( const ViewState & ) const;
    virtual const GLfloat* glNormalArray( const ViewState & ) const;
    virtual unsigned glPolygonSize( const ViewState & ) const;
    virtual unsigned glNumPolygon( const ViewState & ) const;
    virtual const GLuint* glPolygonArray( const ViewState & ) const;
    virtual void glBeforeBodyGLL( const ViewState &, GLPrimitives & ) const;
    virtual void glAfterBodyGLL( const ViewState &, GLPrimitives & ) const;
    virtual std::vector<float> glMin2D() const;
    virtual std::vector<float> glMax2D() const;
    virtual const Referential * getReferential() const;
    virtual void SetMaterial( const Material & );
    virtual Material & GetMaterial();
    virtual void setVoxelSize( const std::vector<float> & );
    virtual std::vector<float> voxelSize() const;
    virtual bool boundingBox2D( std::vector<float> & bmin,
                                std::vector<float> & bmax ) const
    { return boundingBox( bmin, bmax ); }

    //! Gets the current slice plane
    Point4df getPlane( const ViewState & ) const;

    //! Updates the merged surface according to the surface object list and the current plane
    void updateMergedSurface( const ViewState & );

    //! Returns true if the merged surface needs to be updated and false otherwise
    bool needMergedSurfaceUpdate() const;

  private:
    ASurface<2>	* _mergedSurface; //! The merged surface
    std::vector<float> _voxelSize; //! The voxel size
  };
}

#endif
