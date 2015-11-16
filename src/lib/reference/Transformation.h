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


#ifndef ANA_REFERENCE_TRANSFORMATION_H
#define ANA_REFERENCE_TRANSFORMATION_H

//--- header files ------------------------------------------------------------

#include <aims/resampling/motion.h>


//--- class declarations ------------------------------------------------------

namespace aims
{
  class Quaternion;
}

namespace anatomist
{

  class Referential;

  /**	Transformation between two referentials. 
	Actually linear transformations up to now, but could be extended.
	All transformations are registered and stored into the TransformSet 
	singleton object
  */

  class Transformation
  { 
  public:
    Transformation( Referential *,Referential *, bool regist = false, 
		    bool generated = false );
    Transformation( Referential *,Referential *, 
		    const Transformation & trans );
    Transformation();
    /// Unregister from the TransformSet and other objects
    virtual ~Transformation();

    /// Operator = doesn't copy source / dest references
    Transformation & operator = ( const Transformation & trans );
    Transformation & operator *= ( const Transformation & trans );
    // Transformation & operator += ( const Transformation & trans );
    Transformation operator - () const;
    Motion & motion() { return _motion; }
    const Motion & motion() const { return _motion; }

    void setRotation( float** r );
    void setTranslation( float* t );
    /// 4x4 matrix
    void setMatrix( float** m );
    ///	4x3 matrix in Vip/Aims file form (1st line:translation)
    void setMatrixT( float m[4][3] );
    /// Set the rotation.
    void SetRotation(int , int , float );
    /// Set the translation.
    void SetTranslation(int , float );
    /// Get the rotation.
    float Rotation(int ,int );
    aims::Quaternion quaternion() const;
    void setQuaternion( const aims::Quaternion & q );
    /// Get the translation.
    float Translation(int );
    Point3df translation() const;
    Referential* source() const { return( _source ); }
    Referential* destination() const { return( _dest ); }
    /// Inverts the transformation matrix
    void invert();
    /// Inverts source and destination referentials
    void invertReferentials();
    /// true if the transformation is direct, false if it changes orientation
    bool isDirect() const;

    /// Transforms a point
    Point3df transform( const Point3df & pos ) const;
    /* transforms cube (pmin1, pmin2) and builds the new bounding box 
       in the new ref */
    void transformBoundingBox( const Point3df & pmin1, const Point3df & pmax1, 
			       Point3df & pmin2, Point3df & pmax2 );

    ///	Registers the transformation to the TransformSet
    void registerTrans();
    void unregisterTrans();
    bool isGenerated() const { return( _generated ); }
    void setGenerated( bool x ) { _generated = x; }

    /// Adds a new motion to motion history
    void addMotionToHistory(const Motion &);
    /// Undoes last motion
    void undo();
    /// Redoes last motion
    void redo();
    /// Returns the motion history size
    std::size_t motionHistorySize() const
    {
    	return _motionHistory.size();
    }
    /// Returns the current motion history index
    int motionHistoryIndex() const
    {
    	return _motionHistoryIndex;
    }

    /** Static transform function: with transformation, org and dest 
	geometries.
	These functions are designed to help various situations of 
	transformations and geometry changes. They are as fast as possible 
	(inline, direct, with the fewest required data copying and temporary 
	variables) */
    static Point3df transform( const Point3df & pos, const Transformation* tra, 
			       const Point3df & voxSizeOrg, 
			       const Point3df & voxSizeDst );
    /// slower than above: must find the transformation (not inline)
    static Point3df transform( const Point3df & pos, const Referential* orgRef, 
			       const Referential* dstRef, 
			       const Point3df & voxSizeOrg, 
			       const Point3df & voxSizeDst );
    /// no transformation, only geometries
    static Point3df transform( const Point3df & pos, 
			       const Point3df & voxSizeOrg, 
			       const Point3df & voxSizeDst );
    /// transformation, dest geometry but no org geometry
    static Point3df transform( const Point3df & pos, const Transformation* tra, 
			       const Point3df & voxSizeDst );
    /// transformation, org geometry but no dest geometry
    static Point3df transform( const Point3df & pos, 
			       const Point3df & voxSizeOrg, 
			       const Transformation* tra );
    /// no transformation, no dest geometry but org geometry
    static Point3df transform( const Point3df & pos, 
			       const Point3df & voxSizeOrg );
    /** no transformation, no org geometry but dest geometry. 
	The function name changes here to differ from the one above */
    static Point3df transformDG( const Point3df & pos, 
				 const Point3df & voxSizeDst );

  protected:
    Motion      _motion;
    Referential *_source;
    Referential	*_dest;
    bool	_generated;
    std::vector <Motion> _motionHistory;
    int _motionHistoryIndex;
  };


  //	inline functions


  inline Point3df 
  Transformation::transform( const Point3df & pos, const Transformation* tra, 
			     const Point3df & voxSizeOrg, 
			     const Point3df & voxSizeDst )
  {
    Point3df	pt = tra->transform( Point3df( pos[0] * voxSizeOrg[0], 
					       pos[1] * voxSizeOrg[1], 
					       pos[2] * voxSizeOrg[2] ) );
    return( Point3df( pt[0] / voxSizeDst[0], pt[1] / voxSizeDst[1], 
		      pt[2] / voxSizeDst[2] ) );
  }


  inline Point3df 
  Transformation::transform( const Point3df & pos, 
			     const Point3df & voxSizeOrg, 
			     const Point3df & voxSizeDst )
  {
    return( Point3df( pos[0] * voxSizeOrg[0] / voxSizeDst[0], 
		      pos[1] * voxSizeOrg[1] / voxSizeDst[1], 
		      pos[2] * voxSizeOrg[2] / voxSizeDst[2] ) );
  }


  inline Point3df 
  Transformation::transform( const Point3df & pos, const Point3df & voxSizeOrg, 
			     const Transformation* tra )
  {
    return( tra->transform( Point3df( pos[0] * voxSizeOrg[0], 
				      pos[1] * voxSizeOrg[1], 
				      pos[2] * voxSizeOrg[2] ) ) );
  }


  inline Point3df 
  Transformation::transform( const Point3df & pos, const Transformation* tra, 
			   const Point3df & voxSizeDst )
  {
    Point3df	pt = tra->transform( pos );
    return( Point3df( pt[0] / voxSizeDst[0], pt[1] / voxSizeDst[1], 
		      pt[2] / voxSizeDst[2] ) );
  }


  inline Point3df 
  Transformation::transform( const Point3df & pos, 
			   const Point3df & voxSizeOrg )
  {
    return( Point3df( pos[0] * voxSizeOrg[0], pos[1] * voxSizeOrg[1], 
		      pos[2] * voxSizeOrg[2] ) );
  }


  inline Point3df 
  Transformation::transformDG( const Point3df & pos, 
			     const Point3df & voxSizeDst )
  {
    return( Point3df( pos[0] / voxSizeDst[0], pos[1] / voxSizeDst[1], 
		      pos[2] / voxSizeDst[2] ) );
  }

  inline Point3df
  Transformation::translation() const
  {
    return _motion.translation();
  }

}


namespace carto
{
  DECLARE_GENERIC_OBJECT_TYPE( anatomist::Transformation * )
  DECLARE_GENERIC_OBJECT_TYPE( std::set<anatomist::Transformation *> )
  DECLARE_GENERIC_OBJECT_TYPE( std::vector<anatomist::Transformation *> )
  DECLARE_GENERIC_OBJECT_TYPE( std::list<anatomist::Transformation *> )
}

#endif
