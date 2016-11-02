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

#ifndef ANA_SURFACE_VECTORFIELD_H
#define ANA_SURFACE_VECTORFIELD_H


#include <anatomist/mobject/objectVector.h>
#include <anatomist/object/sliceable.h>
#include <anatomist/surface/surface.h>


namespace anatomist
{

  class VectorField : public ObjectVector, public Sliceable
  {
  public:
    VectorField( const std::vector<AObject *> & obj );
    virtual ~VectorField();

    virtual const GLComponent* glAPI() const;
    virtual GLComponent* glAPI();
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

    virtual Point4df glMin2D() const;
    virtual Point4df glMax2D() const;
    virtual const Referential * getReferential() const;
    virtual void SetMaterial( const Material & );
    virtual Material & GetMaterial();
    virtual void setVoxelSize( const Point3df & );
    virtual Point3df VoxelSize() const;
    virtual Point3df glVoxelSize() const;
    virtual const AObjectPalette* glPalette( unsigned tex = 0 ) const;
    virtual AObjectPalette* palette();
    virtual const AObjectPalette* palette() const;
    virtual void setPalette( const AObjectPalette & palette );
    virtual void createDefaultPalette( const std::string & name = "" );

    /// Gets the current slice plane
    Point4df getPlane( const ViewState & ) const;
    virtual ObjectMenu* optionMenu() const;
    virtual Tree* optionTree() const;

    AObject *volume( int channel ) const;
    void setVolume( int channel, AObject* obj );
    Point3di spaceCoordsDimensions( int channel ) const;
    void setSpaceCoordsDimensions( int channel, const Point3di & dims );
    std::vector<int> vectorChannelPosition( int channel ) const;
    void setVectorChannelPosition( int channel, const std::vector<int> & pos );
    float scaling() const;
    void setScaling( float scaling );
    std::vector<int> volumeSize( int channel ) const;

    static int canFusion( const std::set<AObject *> & obj );
    static void editVectorFieldProperties( const std::set<AObject *> & obj );

  protected:
    void buildMesh( const ViewState & );

  private:
    struct Private;
    Private *d;
  };

}

#endif

