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


#ifndef ANA_OBJECT_SELFSLICEABLE_H
#define ANA_OBJECT_SELFSLICEABLE_H

#include <aims/resampling/quaternion.h>

namespace anatomist
{

  /** Self sliceable objects contain a plane (slice) information: slice 
      position and orientation only depend on the object itself.
  */
  class SelfSliceable
  {
  public:
    SelfSliceable();
    SelfSliceable( const Point3df & pos, const aims::Quaternion & quat );
    virtual ~SelfSliceable();

    void setOffset( const Point3df & pos );
    void setQuaternion( const aims::Quaternion & quat );
    void setSlice( const Point3df & pos, const aims::Quaternion & quat );
    aims::Quaternion quaternion() const { return _quaternion; }
    Point3df offset() const { return _offset; }
    /// alternative to setSlice()
    void setPlane( const Point4df & plane );
    /// another way to get the slice plane
    Point4df plane() const;

    /// called when the slice definition has changed
    virtual void sliceChanged() {}

    /// "silent" functions don't call sliceChanged()
    virtual void setOffsetSilent( const Point3df & pos );
    /// "silent" functions don't call sliceChanged()
    virtual void setQuaternionSilent( const aims::Quaternion & quat );
    /// "silent" functions don't call sliceChanged()
    virtual void setSliceSilent( const Point3df & pos, 
                                 const aims::Quaternion & quat );
    /// "silent" functions don't call sliceChanged()
    void setPlaneSilent( const Point4df & plane );

  protected:
    Point3df	_offset;
    aims::Quaternion	_quaternion;
  };


  // inline methods

  inline void SelfSliceable::setOffset( const Point3df & pos )
  {
    setOffsetSilent( pos );
    sliceChanged();
  }

  inline void SelfSliceable::setQuaternion( const aims::Quaternion & quat )
  {
    if( quat.vector() != Point4df( 0, 0, 0, 0 ) )
    {
      setQuaternionSilent( quat );
      sliceChanged();
    }
  }

  inline void SelfSliceable::setSlice( const Point3df & pos, 
                                       const aims::Quaternion & quat )
  {
    setSliceSilent( pos, quat );
    sliceChanged();
  }

  inline void SelfSliceable::setPlane( const Point4df & plane )
  {
    setPlaneSilent( plane );
    sliceChanged();
  }

}

#endif
