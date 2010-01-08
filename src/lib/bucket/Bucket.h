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


#ifndef ANA_BUCKET_BUCKET_H
#define ANA_BUCKET_BUCKET_H


#include <anatomist/surface/globject.h>
#include <anatomist/graph/pythonAObject.h>
#include <aims/mesh/surface.h>
#include <aims/bucket/bucket.h>


namespace anatomist
{

  class Referential;
  class Geometry;
  class Transformation;
  class SliceViewState;

  /**	Bucket class
   */
  class Bucket : public AGLObject, public PythonAObject
  {
  public:
    Bucket(const char *filename=NULL);
    ~Bucket();

    float MinT() const { return( _minT ); }
    float MaxT() const { return( _maxT ); }
    float MinX2D() const { return( _minX ); }
    float MinY2D() const { return( _minY ); }
    float MinZ2D() const { return( _minZ ); }
    float MaxX2D() const { return( _maxX ); }
    float MaxY2D() const { return( _maxY ); }
    float MaxZ2D() const { return( _maxZ ); }
    virtual bool boundingBox( Point3df & bmin, Point3df & bmax ) const;

    virtual void 
    setSubBucketGeomExtrema( const Point3df& pmin = Point3df(0., 0., 0.), 
                             const Point3df& pmax = Point3df(0., 0., 0.) ) ;
    virtual void setGeomExtrema();
    virtual Point3df VoxelSize() const;
    virtual void setVoxelSize( const Point3df & vs );
    virtual void setBucketChanged();
    virtual bool hasBucketChanged() const;
    void setBucket( const aims::BucketMap<Void>& theBuck );
    void setBucket( carto::rc_ptr<aims::BucketMap<Void> > theBuck );
    aims::BucketMap<Void> & bucket() { return( *_bucket ); }
    const aims::BucketMap<Void> & bucket() const { return( *_bucket ); }
    carto::rc_ptr<aims::BucketMap<Void> > rcBucket() { return _bucket; }

    size_t createFacet( size_t t = 0 ) const;
    virtual bool Is2DObject();
    virtual bool Is3DObject() { return true; }
    const AimsSurface<4, Void>* surface( const ViewState & ) const;
    void setSurface( AimsSurfaceFacet* surf );
    void freeSurface();
    virtual unsigned glNumVertex( const ViewState & ) const;
    virtual const GLfloat* glVertexArray( const ViewState & ) const;
    virtual const GLfloat* glNormalArray( const ViewState & ) const;
    virtual unsigned glNumPolygon( const ViewState & ) const;
    virtual const GLuint* glPolygonArray( const ViewState & ) const;
    virtual unsigned glPolygonSize( const ViewState & ) const { return 4; }
    bool allow2DRendering() const;
    /// when false, a bucket will always appear 3D
    void setAllow2DRendering( bool x );

    virtual bool empty() const;
    void insert( const aims::BucketMap<Void> & region );
    void erase( const aims::BucketMap<Void> & region );
    void meshSubBucket( aims::BucketMap<Void>::Bucket::const_iterator ibegin, 
			aims::BucketMap<Void>::Bucket::const_iterator iend, 
			AimsSurface<4,Void> *surf, bool glonfly=false ) const;
    void meshSubBucket( const std::vector<std::pair<
			aims::BucketMap<Void>::Bucket::const_iterator, 
			aims::BucketMap<Void>::Bucket::const_iterator> > & iv, 
			AimsSurface<4,Void> *surf, bool glonfly=false ) const;

    virtual AObject* ObjectAt( float x, float y, float z, float t, 
			       float tol = 0 );

    virtual bool loadable() const { return( true ); }
    virtual bool savable() const { return( true ); }
    virtual bool save( const std::string & filename );
    virtual bool reload( const std::string & filename );

    virtual void setInternalsChanged();
    virtual carto::GenericObject* attributed();
    virtual const carto::GenericObject* attributed() const;

    virtual Tree* optionTree() const;
    static Tree*	_optionTree;

  protected:
    carto::rc_ptr<aims::BucketMap<Void> > _bucket;
    mutable int  _minX,_minY,_minZ,_minT;
    mutable int  _maxX,_maxY,_maxZ,_maxT;

    void freeSurface() const;
    virtual void setBucketChanged() const;
    virtual std::string viewStateID( glPart part, const ViewState & ) const;

  private:
    struct Private;
    Private	*d;

    const AimsSurface<4,Void>* meshPlane( const SliceViewState & ) const;
  };

  inline void 
  Bucket::meshSubBucket( aims::BucketMap<Void>::Bucket::const_iterator ibegin, 
			 aims::BucketMap<Void>::Bucket::const_iterator iend, 
			 AimsSurface<4,Void> *surf, bool glonfly ) const
  {
    typedef aims::BucketMap<Void>::Bucket::const_iterator	biter;
    typedef std::pair<biter, biter>				piter;
    std::vector<piter>	ivec;
    ivec.push_back( piter( ibegin, iend ) );
    meshSubBucket( ivec, surf, glonfly );
  }

}


#endif
