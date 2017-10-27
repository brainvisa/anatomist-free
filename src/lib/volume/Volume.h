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

#ifndef ANA_VOLUME_VOLUME_H
#define ANA_VOLUME_VOLUME_H

#include <anatomist/object/sliceable.h>
#include <anatomist/graph/pythonAObject.h>
#include <aims/data/data.h>
#include <aims/rgb/rgb.h>


namespace anatomist
{

  class Referential;
  class Transformation;
  class Geometry;

  /**   VolumeBase object :
        use for generic dynamic_cast of all AVolume<T>, thus the visitor design pattern can be used */
  class AVolumeBase
    : public SliceableObject, public PythonAObject
  {
  public:
    AVolumeBase();
    virtual ~AVolumeBase();

    virtual void setShaderParameters(const Shader &shader, const ViewState & state) const;
  };

  /**	Volume object */
  template <typename T>
  class AVolume : public AVolumeBase
  {
  public:
    AVolume( const std::string & filename = "" );
    AVolume( const AimsData<T> & );
    AVolume( carto::rc_ptr<AimsData<T> > );
    AVolume( carto::rc_ptr<carto::Volume<T> > );
    virtual ~AVolume();

    virtual AObject* clone( bool shallow = true );

    float MinX2D() const { return 0.0; }
    float MinY2D() const { return 0.0; }
    float MinZ2D() const { return 0.0; }
    float MaxX2D() const { return float(_volume->getSizeX()-1); }
    float MaxY2D() const { return float(_volume->getSizeY()-1); }
    float MaxZ2D() const { return float(_volume->getSizeZ()-1); }
    float MinT() const { return 0.0; }
    float MaxT() const { return float(_volume->getSizeT()-1); }

    virtual bool boundingBox( Point3df & bmin, Point3df & bmax ) const;

    void SetExtrema();
    void adjustPalette();

    carto::rc_ptr<carto::Volume<T> > volume() { return _volume; }
    carto::rc_ptr<AimsData<T> > aimsvolume()
    { return carto::rc_ptr<AimsData<T> >( new AimsData<T>(_volume) ); }
    const carto::rc_ptr<carto::Volume<T> > volume() const { return _volume; }
    const carto::rc_ptr<AimsData<T> > aimsvolume() const
    { return carto::rc_ptr<AimsData<T> >( new AimsData<T>( _volume ) ); }
    void setVolume( carto::rc_ptr<AimsData<T> > vol );
    virtual void setVolume( carto::rc_ptr<carto::Volume<T> > vol );
    T & operator () ( size_t x=0, size_t y=0, size_t z=0, size_t t=0 )
    { return (*_volume)( x, y , z ,t ); }
    const T & operator () ( size_t x=0, size_t y=0, size_t z=0, 
                            size_t t=0 ) const
    { return (*_volume)( x, y , z ,t ); }

    /// new API
    virtual bool update2DTexture( AImage &, const Point3df & posbase, 
                                  const SliceViewState &, 
                                  unsigned tex = 0 ) const;
    virtual Point3df VoxelSize() const;
    virtual void setVoxelSize( const Point3df & vs );
    /// Retourne la valeur d'un voxel du volume.
    float GetValue(Point3df pos,float time, Referential *winref,
		   Geometry *wingeom);
    /// Can be display in 2D windows.
    bool Is2DObject() { return(true); }
    bool textured2D() const { return( true ); }
    /// Can be display in 3D windows.
    bool Is3DObject() { return(false); }
    virtual bool isTransparent() const;
    /// Not selectable: always returns Null
    virtual AObject* ObjectAt( float x, float y, float z, float t, 
			       float tol = 0 );

    virtual bool hasTexture() const { return( true ); }
    virtual unsigned dimTexture() const { return( 1 ); }
    virtual float mixedTexValue( const Point3df & pos, float time ) const;
    virtual std::vector<float> texValues( const Point3df & pos, 
					  float time ) const;

    virtual carto::GenericObject* attributed();
    virtual const carto::GenericObject* attributed() const;

    virtual bool loadable() const { return( true ); }
    virtual bool savable() const { return( true ); }
    virtual bool reload( const std::string & filename );
    virtual bool save( const std::string & filename );
    virtual std::string objectFullTypeName(void) const;

    /// should be replaced by a real referential
    virtual bool printTalairachCoord( const Point3df & pos, 
				      const Referential* ) const;
    virtual void setInternalsChanged();

  protected:
    ///	Generic texture filling routine for any transformation
    void updateSlice( AImage & image, const Point3df & p0, float time, 
		      const Transformation* tra, const Point3df & inc, 
		      const Point3df & offset, const Geometry* wingeom ) const;
    /// Optimized texture filling routine (no transformation)
    void updateAxial( AImage *ximage, const Point3df & p0, float time ) const;
    /// Optimized texture filling routine (no transformation)
    void updateCoronal( AImage *ximage, const Point3df & p0, 
                        float time ) const;
    /// Optimized texture filling routine (no transformation)
    void updateSagittal( AImage *ximage, const Point3df & p0, 
                         float time ) const;

  private:
    struct PrivateData;
    PrivateData			*d;
    carto::rc_ptr<carto::Volume<T> >	_volume;
  };


  template<class T>
  inline AObject* 
  anatomist::AVolume<T>::ObjectAt( float, float, float, float, float ) 
  { return 0; }

}


#endif
