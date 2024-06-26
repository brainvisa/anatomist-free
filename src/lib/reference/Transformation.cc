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


//--- header files ------------------------------------------------------------

#include <anatomist/reference/Transformation.h>
#include <anatomist/reference/transfSet.h>
#include <aims/resampling/quaternion.h>
#include <iostream>


using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;
using aims::Quaternion;


namespace anatomist
{

//--- methods -----------------------------------------------------------------

Transformation::Transformation() 
  : _motion(), _source( 0 ), _dest( 0 ), _generated( false ),
    _motionHistoryIndex(-1)
{
}


Transformation::Transformation( Referential* src, Referential* dst, 
				bool regist, bool generated )
  : _motion(), _source( src ), _dest( dst ), _generated( generated ),
    _motionHistoryIndex(-1)
{
  if( regist )
    registerTrans();
}


Transformation::Transformation( Referential* src, Referential* dst, 
				const Transformation& trans )
  : _motion( trans._motion ), _source( src ), _dest( dst ), _generated( false ),
    _motionHistoryIndex(trans._motionHistoryIndex)
{
  *this = trans;
  //ATransformSet::instance()->registerTransformation( this );
}

Transformation::~Transformation()
{
  unregisterTrans();
}

Transformation & Transformation::operator = ( const Transformation& trans )
{
  if( this == &trans )
    return( *this );

  this->operator = ( trans._motion );

  return( *this );
}


Transformation & Transformation::operator = (
  const AffineTransformation3d & trans )
{
  bool reg = ATransformSet::instance()->hasTransformation( this );
  if( reg )
    reg = ( _motion != trans );

  _motion = trans;

  if( reg )
    ATransformSet::instance()->updateTransformation( this );

  return *this;
}


void Transformation::SetRotation(int i,int j,float val)
{
  if( (i >= 0) && (i <= 2) && (j >= 0) && (j <= 2) )
    _motion.rotation()(i,j) = val;
}

float Transformation::Rotation(int i,int j)
{
  if( (i >= 0) && (i <= 2) && (j >= 0) && (j <= 2) )
    return _motion.rotation()(i,j);
  else return 0;
}

void Transformation::SetTranslation(int i,float val)
{
  if( (i >= 0) && (i <= 2) )
    _motion.matrix()(i, 3) = val;
}

float Transformation::Translation(int i)
{
  if( (i >= 0) && (i <= 2) )
    return _motion.translation()[i];
  else return 0;
}

void Transformation::invert()
{
  bool	reg = ATransformSet::instance()->hasTransformation( this );

  unique_ptr<AffineTransformation3d> inv = _motion.inverse();
  if( reg )
    reg = ( _motion !=  *inv );

  AffineTransformationBase::epsilon = 1e-5;

  _motion = *inv;

  if( reg )
    ATransformSet::instance()->updateTransformation( this );
}

void Transformation::invertReferentials()
{
  bool	reg = ATransformSet::instance()->hasTransformation( this );
  if( reg )
    unregisterTrans();

  Referential* tmpref;

  tmpref = _source;
  _source = _dest;
  _dest = tmpref;

  if( reg )
    registerTrans();
}


Point3df Transformation::transform( const Point3df & pos ) const
{
  return _motion.transform( pos );
}


Point3df Transformation::transform( const Point3df & pos, 
                                    const Referential* orgRef,
                                    const Referential* dstRef,
                                    const Point3df & voxSizeOrg,
                                    const Point3df & voxSizeDst )
{
  Transformation *tra 
    = ATransformSet::instance()->transformation( orgRef, dstRef );

  if( tra )
    return( transform( pos, tra, voxSizeOrg, voxSizeDst ) );
  else
    return( transform( pos, voxSizeOrg, voxSizeDst ) );
}


Quaternion Transformation::quaternion() const
{
  Quaternion  q;
  q.buildFromMotion( _motion );
  return q;
}


void Transformation::setQuaternion( const Quaternion & q )
{
  AimsVector<float,16>	r = q.rotationMatrix();
  VolumeRef<float>       rotation = _motion.rotation();
  rotation(0,0) = r[0];
  rotation(0,1) = r[1];
  rotation(0,2) = r[2];
  rotation(1,0) = r[4];
  rotation(1,1) = r[5];
  rotation(1,2) = r[6];
  rotation(2,0) = r[8];
  rotation(2,1) = r[9];
  rotation(2,2) = r[10];
}


void Transformation::transformBoundingBox( const Point3df & pmin1, 
					   const Point3df & pmax1, 
					   Point3df & pmin2, Point3df & pmax2 )
{
	aims::transformBoundingBox(_motion, pmin1, pmax1, pmin2, pmax2);
}


void Transformation::registerTrans()
{
  ATransformSet	*ts = ATransformSet::instance();
  if( !ts->hasTransformation( this ) )
    ts->registerTransformation( this );
}


void Transformation::unregisterTrans()
{
  ATransformSet	*ts = ATransformSet::instance();
  if( ts->hasTransformation( this ) )
    ts->unregisterTransformation( this );
}


void Transformation::setRotation( float** r )
{
  VolumeRef<float>       rotation = _motion.rotation();
  rotation(0,0) = r[0][0];
  rotation(0,1) = r[0][1];
  rotation(0,2) = r[0][2];
  rotation(1,0) = r[1][0];
  rotation(1,1) = r[1][1];
  rotation(1,2) = r[1][2];
  rotation(2,0) = r[2][0];
  rotation(2,1) = r[2][1];
  rotation(2,2) = r[2][2];
}


void Transformation::setTranslation( float* t )
{
  aims::AffineTransformation3d::Table<float> & matrix = _motion.matrix();
  matrix(0, 3) = t[0];
  matrix(1, 3) = t[1];
  matrix(2, 3) = t[2];
}


void Transformation::setMatrix( float** m )
{
  aims::AffineTransformation3d::Table<float> & matrix = _motion.matrix();
  matrix(0,0) = m[0][0];
  matrix(0,1) = m[0][1];
  matrix(0,2) = m[0][2];
  matrix(1,0) = m[1][0];
  matrix(1,1) = m[1][1];
  matrix(1,2) = m[1][2];
  matrix(2,0) = m[2][0];
  matrix(2,1) = m[2][1];
  matrix(2,2) = m[2][2];
  matrix(0,3) = m[0][0];
  matrix(1,3) = m[1][0];
  matrix(2,3) = m[2][0];
}


void Transformation::setMatrixT( float m[4][3] )
{
  aims::AffineTransformation3d::Table<float> & matrix = _motion.matrix();
  matrix(0,0) = m[1][0];
  matrix(0,1) = m[1][1];
  matrix(0,2) = m[1][2];
  matrix(1,0) = m[2][0];
  matrix(1,1) = m[2][1];
  matrix(1,2) = m[2][2];
  matrix(2,0) = m[3][0];
  matrix(2,1) = m[3][1];
  matrix(2,2) = m[3][2];
  matrix(0,3) = m[0][0];
  matrix(1,3) = m[0][1];
  matrix(2,3) = m[0][2];
}

void
Transformation::addMotionToHistory( const AffineTransformation3d & motion )
{
  if (_motionHistoryIndex == -1)
  {
    _motionHistory.clear();
  }
  else if( _motionHistoryIndex >= 0 &&
           static_cast<size_t>(_motionHistoryIndex)
              < (_motionHistory.size() - 1))
  {
    _motionHistory.erase( _motionHistory.begin() + _motionHistoryIndex + 1,
                          _motionHistory.end() );
  }

  _motionHistory.push_back(motion);
  _motionHistoryIndex = _motionHistory.size() - 1;
}

void Transformation::undo()
{
  if (_motionHistoryIndex < 0 ||
          _motionHistory.empty())
    return;

  _motion = *_motionHistory[_motionHistoryIndex].inverse() * _motion;
  _motionHistoryIndex -= 1;
  notifyChange();
}

void Transformation::redo()
{
  if (static_cast<size_t>(_motionHistoryIndex) == (_motionHistory.size() - 1)
      || _motionHistory.empty())
    return;

  _motionHistoryIndex += 1;
  _motion = _motionHistory[_motionHistoryIndex] * _motion;
  notifyChange();
}

Transformation & Transformation::operator *= ( const Transformation & t )
{
  _motion *= t._motion;

  return( *this );
}


void Transformation::notifyChange()
{
  ATransformSet::instance()->updateTransformation( this );
}


#if 0
Transformation & Transformation::operator += ( const Transformation & t )
{
  _motion += t._motion;

  /*
  _translation[0] += t._translation[0];
  _translation[1] += t._translation[1];
  _translation[2] += t._translation[2];

  _rotation[0][0] += t._rotation[0][0];
  _rotation[0][1] += t._rotation[0][1];
  _rotation[0][2] += t._rotation[0][2];
  _rotation[1][0] += t._rotation[1][0];
  _rotation[1][1] += t._rotation[1][1];
  _rotation[1][2] += t._rotation[1][2];
  _rotation[2][0] += t._rotation[2][0];
  _rotation[2][1] += t._rotation[2][1];
  _rotation[2][2] += t._rotation[2][2];
  */

  return( *this );
}
#endif


Transformation Transformation::operator - () const
{
  Transformation t( 0, 0 );
  t._motion = -_motion;
  return t;
}


bool Transformation::isDirect() const
{
  return _motion.isDirect();
}

}

#include <cartobase/object/object_d.h>
INSTANTIATE_GENERIC_OBJECT_TYPE( anatomist::Transformation * )
INSTANTIATE_GENERIC_OBJECT_TYPE( set<anatomist::Transformation *> )
INSTANTIATE_GENERIC_OBJECT_TYPE( vector<anatomist::Transformation *> )
INSTANTIATE_GENERIC_OBJECT_TYPE( list<anatomist::Transformation *> )

