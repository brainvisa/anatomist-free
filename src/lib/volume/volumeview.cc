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

#include <anatomist/volume/volumeview.h>
#include <anatomist/reference/Transformation.h>
#include <anatomist/reference/Referential.h>
#include <anatomist/reference/transformobserver.h>
#include <anatomist/application/Anatomist.h>

using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;


namespace
{
  int registerClass()
  {
    int   type = AObject::registerObjectType( "VolumeView" );
    return type;
  }
}


template <typename T>
AVolumeView<T>::AVolumeView( const list<AObject *> & obj )
 : anatomist::ObjectVector(),
  _myvolume( new AVolume<T>( "" ) ),
  _avolume( 0 )
{
  _type = AVolumeView<T>::classType();

  if( obj.size() != 1 )
  {
    cerr << "Can only build view on a single volume\n";
  }
  AVolume<T> * vol = dynamic_cast<AVolume<T> *>( *obj.begin() );
  if( !vol )
  {
    cerr << "View can only be built on a volume\n";
  }
  insert( _myvolume.get() );
  insert( vol );
  _avolume.reset( vol );

  typename Volume<T>::Position4Di pos( 0, 0, 0, 0 ), size( 1, 1, 1, 1 );

  _myvolume->setVolume(
    rc_ptr<Volume<T> >( new Volume<T>( vol->volume(), pos, size ) ) );

  theAnatomist->registerObject( _myvolume.get(), false );
  theAnatomist->releaseObject( _myvolume.get() );
  theAnatomist->registerObject( _avolume.get(), false );
  theAnatomist->releaseObject( _avolume.get() );

  setupTransformationFromView();
}


template <typename T>
AVolumeView<T>::AVolumeView( const string & filename )
 : anatomist::ObjectVector(),
  _myvolume( new AVolume<T>() ),
  _avolume( 0 )
{
  _type = AVolumeView<T>::classType();
  insert( _myvolume.get() );

  if( _myvolume->volume()->refVolume() )
  {
    _avolume = rc_ptr<AVolume<T> >(
      new AVolume<T>( _myvolume->volume()->refVolume() ) );
    insert( _avolume.get() );
  }

  theAnatomist->registerObject( _myvolume.get(), false );
  theAnatomist->releaseObject( _myvolume.get() );
  theAnatomist->registerObject( _avolume.get(), false );
  theAnatomist->releaseObject( _avolume.get() );

  setupTransformationFromView();
}


template <typename T>
AVolumeView<T>::AVolumeView( rc_ptr<Volume<T> > vol )
 : anatomist::ObjectVector(),
  _myvolume( new AVolume<T>() ),
  _avolume( 0 )
{
  _type = AVolumeView<T>::classType();
  insert( _myvolume.get() );

  setVolume( vol );

  theAnatomist->registerObject( _myvolume.get(), false );
  theAnatomist->releaseObject( _myvolume.get() );
  theAnatomist->registerObject( _avolume.get(), false );
  theAnatomist->releaseObject( _avolume.get() );

  setupTransformationFromView();
}


template <typename T>
AVolumeView<T>::~AVolumeView()
{
}


template <typename T>
int AVolumeView<T>::classType()
{
  static int    _classType = registerClass();
  return _classType;
}


template <typename T>
void AVolumeView<T>::setVolume( rc_ptr<Volume<T> > vol )
{
  if( size() >= 2 )
  {
    iterator i = begin();
    ++i;
    erase( i );
  }
  _avolume.reset( 0 );
  _myvolume->setVolume( vol );
  _myvolume->setReferentialInheritance( this );
  if( _myvolume->volume()->refVolume() )
  {
    _avolume = rc_ptr<AVolume<T> >(
      new AVolume<T>( _myvolume->volume()->refVolume() ) );
    insert( _avolume.get() );
  }
}


template <typename T>
void AVolumeView<T>::move( const Point3d & pos, const Point3d & size )
{
  typename Volume<T>::Position4Di pos4( pos[0], pos[1], pos[2], pos[3] );
  typename Volume<T>::Position4Di size4( size[0], size[1], size[2], 1 );
  _myvolume->setVolume(
    rc_ptr<Volume<T> >( new Volume<T>( _avolume->volume(), pos4, size4 ) ) );
  _myvolume->glSetChanged( GLComponent::glBODY );
  _myvolume->glSetChanged( GLComponent::glGEOMETRY );
  _myvolume->glSetChanged( GLComponent::glTEXIMAGE );
}


template <typename T>
void AVolumeView<T>::setupTransformationFromView()
{
  if( !_avolume.get() )
    return;
  const typename Volume<T>::Position & pos
    = _myvolume->volume()->posInRefVolume();
  typename Volume<T>::Position::const_iterator ip, ep = pos.end();
  Point3d ipos;
  int i = 0;
  bool at_origin = true;
  for( ip=pos.begin(); ip!=ep && i<3; ++ip, ++i )
    if( *ip != 0 )
    {
      at_origin = false;
      ipos[i] = *ip;
    }
  if( at_origin && _myvolume->getReferential() == _avolume->getReferential() )
  {
    setReferentialInheritance( _myvolume.get() );
    return;
  }
  if( !_avolume->getReferential() )
  {
    // we need the reference volume to have a referential
    _avolume->setReferential( theAnatomist->centralReferential() );
  }

  /* WARNING: API mismatch: AObject::getReferential() const returns a
     non-const Referential *, while Sliceable::getReferential() const returns
     a const Referential *
  */
  Referential *ref = _myvolume->AObject::getReferential();
  if( !ref || ref == _avolume->getReferential() )
  {
    ref = new Referential;
    _myvolume->setReferential( ref );
  }
  Transformation *tr
    = theAnatomist->getTransformation( ref, _avolume->getReferential() );
  if( !tr )
    tr = new Transformation( ref, _avolume->AObject::getReferential() );
  AffineTransformation3d & atr = tr->motion();
  atr.setToIdentity();
  Point3df vs = _myvolume->VoxelSize();
  Point3df trans( ipos[0] * vs[0], ipos[1] * vs[1], ipos[2] * vs[2] );
  atr.setTranslation( trans );
  tr->registerTrans();

  setReferentialInheritance( _myvolume.get() );
}


template <typename T>
void AVolumeView<T>::setupViewFromTransformation()
{
  if( !_avolume.get() )
    return;
  Transformation *tr
    = theAnatomist->getTransformation( _myvolume->getReferential(),
                                       _avolume->getReferential() );
  if( !tr )
    return;

  // TODO: erase all non-diagonal transform coefs

  rc_ptr<Volume<T> > vol = _myvolume->volume();
  vector<float> ivs = vol->getVoxelSize();
  vector<int> isz = vol->getSize();

  Point3df bbmin( 0, 0, 0 ),
    bbmax( ( isz[0] - 1 ) * ivs[0], ( isz[1] - 1 ) * ivs[1],
           ( isz[2] - 1 ) * ivs[2] );
  cout << "bb: " << bbmin << " - " << bbmax << endl;
  Point3df p0 = tr->transform( bbmin );
  Point3df p1 = tr->transform( bbmax );
  cout << "after trans: " << p0 << " - " << p1 << endl;

  Point3df pos( min( p0[0], p1[0] ), min( p0[1], p1[1] ),
                min( p0[2], p1[2] ) );
  Point3df pmax( max( p0[0], p1[0] ), max( p0[1], p1[1] ),
                 max( p0[2], p1[2] ) );
  Point3df vs = _avolume->VoxelSize();
  pos[0] /= vs[0];
  pos[1] /= vs[1];
  pos[2] /= vs[2];
  pmax[0] /= vs[0];
  pmax[1] /= vs[1];
  pmax[2] /= vs[2];
  cout << "new pos: " << pos << endl;
  cout << "new size: " << pmax - pos << endl;

  vector<int> npos( 3 );
  npos[0] = int( pos[0] );
  npos[1] = int( pos[1] );
  npos[2] = int( pos[2] );

  vector<int> nsize( 3 );
  nsize[0] = ceil( pmax[0] - pos[0] + 1 );
  nsize[1] = ceil( pmax[1] - pos[1] + 1);
  nsize[2] = ceil( pmax[2] - pos[2] + 1 );
  if( nsize[0] < 1 )
    nsize[0] = 1;
  if( nsize[1] < 1 )
    nsize[1] = 1;
  if( nsize[2] < 1 )
    nsize[2] = 1;

  vector<int> maxsize = _avolume->volume()->getSize();
  if( npos[0] + nsize[0] > maxsize[0] )
    nsize[0] = maxsize[0] - npos[0];
  if( npos[1] + nsize[1] > maxsize[1] )
    nsize[1] = maxsize[1] - npos[1];
  if( npos[2] + nsize[2] > maxsize[2] )
    nsize[2] = maxsize[2] - npos[2];

  vector<int> volpos = vol->posInRefVolume();
  vector<int> volsz = vol->getSize();
  if( volpos[0] != npos[0] || volpos[1] != npos[1] || volpos[2] != npos[2]
      || volsz[0] != nsize[0] || volsz[1] != nsize[1] || volsz[2] != nsize[2] )
  {
    int i, n = volpos.size();
    for( i=3; i<n; ++i )
      npos.push_back( volpos[i] );
    for( i=3, n=volsz.size(); i<n; ++i )
      nsize.push_back( volsz[i] );

    // FIXME: totally suboptimal
    rc_ptr<DataSourceInfo> dsi
      = _avolume->volume()->allocatorContext().dataSourceInfo();
    cout << "read from file: " << dsi->url() << endl;
    Object options = Object::value( Dictionary() );
    options->setProperty( "ox", npos[0] );
    options->setProperty( "oy", npos[1] );
    options->setProperty( "oz", npos[2] );
    options->setProperty( "sx", nsize[0] );
    options->setProperty( "sy", nsize[1] );
    options->setProperty( "sz", nsize[2] );
//     options->setProperty( "format", dsi->identifiedFormat() );

    Reader<Volume<T> > r( dsi->url() );
    r.setOptions( options );
    string format = dsi->identifiedFormat();
    VolumeRef<T> view = r.read( 0, &format );

//     VolumeRef<T> view( _avolume->volume(), npos, nsize );
    _myvolume->setVolume( view );
  }
}


template <typename T>
bool AVolumeView<T>::render( PrimList & prim, const ViewState & vs )
{
  _myvolume->render( prim, vs );
}



template <typename T>
void AVolumeView<T>::setFileName( const string & fname )
{
  ObjectVector::setFileName( fname );
  _myvolume->setFileName( fname );
}


template <typename T>
void AVolumeView<T>::SetExtrema()
{
  _myvolume->SetExtrema();
}


template <typename T>
void AVolumeView<T>::adjustPalette()
{
  _myvolume->adjustPalette();
}


template <typename T>
void AVolumeView<T>::update( const Observable *observable, void *arg )
{
  const TransformationObserver *to
    = dynamic_cast<const TransformationObserver *>( observable );

  if( to && to->involves( _myvolume->getReferential() ) )
  {
    setupViewFromTransformation();
  }

  ObjectVector::update( observable, arg );
}


// instanciations

namespace anatomist
{

  template class AVolumeView<int8_t>;
  template class AVolumeView<uint8_t>;
  template class AVolumeView<int16_t>;
  template class AVolumeView<uint16_t>;
  template class AVolumeView<int32_t>;
  template class AVolumeView<uint32_t>;
  template class AVolumeView<int64_t>;
  template class AVolumeView<uint64_t>;
  template class AVolumeView<float>;
  template class AVolumeView<double>;
  template class AVolumeView<AimsRGB>;
  template class AVolumeView<AimsRGBA>;

}

