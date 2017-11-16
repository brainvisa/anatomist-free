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

#ifndef ANA_VOLUME_VOLUMEVIEW_H
#define ANA_VOLUME_VOLUMEVIEW_H

#include <anatomist/volume/Volume.h>
#include <anatomist/mobject/objectVector.h>


namespace anatomist
{

  template <typename T>
  class AVolumeView : public ObjectVector
  {
  public:
    AVolumeView( const std::list<AObject *> & );
    AVolumeView( const std::string & filename );
    AVolumeView( carto::rc_ptr<carto::Volume<T> > vol );
    AVolumeView( const std::vector<carto::rc_ptr<carto::Volume<T> > > & vols );
    virtual ~AVolumeView();

    virtual void setVolume( carto::rc_ptr<carto::Volume<T> > vol );
    // void move( const Point3d & pos, const Point3d & size );

    void setupTransformationFromView();
    void setupViewFromTransformation();
    void setTargetSize( const std::vector<int> & );
    const std::vector<int> & targetSize() const;
    int resolutionLevel() const { return _resolution_level; }
    int selectBestResolutionLevel( const Point3df & vs ) const;
    const carto::rc_ptr<AVolume<T> > view() const { return _myvolume; }
    carto::rc_ptr<AVolume<T> > view() { return _myvolume; }
    Point3df initialFOV() const { return _initial_fov; }
    void setInitialFOV( const Point3df & fov );

    // overloads
    virtual int MType() const { return type(); }
    static int classType();
    virtual bool CanRemove( AObject* obj ) { return false; }
    virtual bool render( PrimList & prim, const ViewState & vs );
    virtual void setFileName( const std::string & fname );
    virtual void SetExtrema();
    virtual void adjustPalette();
    virtual Point3df VoxelSize() const { return _myvolume->VoxelSize(); }
    bool Is2DObject() { return(true); }
    bool textured2D() const { return( true ); }
    bool Is3DObject() { return(false); }
    virtual bool isTransparent() const { return _myvolume->isTransparent(); }
    float MinT() const { return _myvolume->MinT(); }
    float MaxT() const { return _myvolume->MaxT(); }
    virtual bool boundingBox( std::vector<float> & bmin,
                              std::vector<float> & bmax ) const
    { return _myvolume->boundingBox( bmin, bmax ); }
    virtual bool boundingBox2D( std::vector<float> & bmin,
                                std::vector<float> & bmax ) const
    { return _myvolume->boundingBox2D( bmin, bmax ); }

    virtual void update( const Observable *observable, void *arg );

  private:
    void init( const std::vector<carto::rc_ptr<carto::Volume<T> > > & vols );

    carto::rc_ptr<AVolume<T> > _myvolume;
    std::vector<carto::rc_ptr<AVolume<T> > > _avolume;
    std::vector<int> _target_size;
    Point3df _initial_fov;
    int _resolution_level;
  };

}

#endif
