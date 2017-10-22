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
  _target_size( 4, 1 ),
  _resolution_level( 0 )
{
  _type = AVolumeView<T>::classType();
  insert( _myvolume.get() );
  theAnatomist->registerObject( _myvolume.get(), false );
  theAnatomist->releaseObject( _myvolume.get() );

  list<AObject *>::const_iterator io, eo = obj.end();
  for( io=obj.begin(); io!=eo; ++io )
  {
    AVolume<T> * vol = dynamic_cast<AVolume<T> *>( *obj.begin() );
    if( !vol )
      cerr << "View can only be built on a volume\n";
    else
    {
      insert( vol );
      _avolume.push_back( rc_ptr<AVolume<T> >( vol ) );
    }
  }

  if( !_avolume.empty() )
  {
    typename Volume<T>::Position4Di pos( 0, 0, 0, 0 ), size( 1, 1, 1, 1 );
    AVolume<T> * vol = _avolume.begin()->get();

    _myvolume->setVolume(
      rc_ptr<Volume<T> >( new Volume<T>( vol->volume(), pos, size ) ) );
  }

  setupTransformationFromView();
}


template <typename T>
AVolumeView<T>::AVolumeView( const string & filename )
 : anatomist::ObjectVector(),
  _myvolume( new AVolume<T>() ),
  _target_size( 4, 1 ),
  _resolution_level( 0 )
{
  _type = AVolumeView<T>::classType();
  insert( _myvolume.get() );
  theAnatomist->registerObject( _myvolume.get(), false );
  theAnatomist->releaseObject( _myvolume.get() );

  if( _myvolume->volume()->refVolume() )
  {
    _avolume.push_back( rc_ptr<AVolume<T> >(
      new AVolume<T>( _myvolume->volume()->refVolume() ) ) );
    insert( _avolume[0].get() );
    theAnatomist->registerObject( _avolume[0].get(), false );
    theAnatomist->releaseObject( _avolume[0].get() );
  }

  setupTransformationFromView();
}


template <typename T>
AVolumeView<T>::AVolumeView( rc_ptr<Volume<T> > vol )
 : anatomist::ObjectVector(),
  _myvolume( new AVolume<T>() ),
  _target_size( 4, 1 ),
  _resolution_level( 0 )
{
  vector<rc_ptr<Volume<T> > > vols( 1 );
  vols[0] = vol;
  init( vols );
}


template <typename T>
AVolumeView<T>::~AVolumeView()
{
}


template <typename T>
void AVolumeView<T>::init( const vector<rc_ptr<Volume<T> > > & vols )
{
  _type = AVolumeView<T>::classType();
  insert( _myvolume.get() );
  theAnatomist->registerObject( _myvolume.get(), false );
  theAnatomist->releaseObject( _myvolume.get() );

  if( !vols.empty() )
  {
    rc_ptr<Volume<T> > vol = vols[0];

    setVolume( vol );
    _target_size = vol->getSize();

    typename vector<rc_ptr<Volume<T> > >::const_iterator iv, ev = vols.end();
    for( iv=vols.begin(), ++iv; iv!=ev; ++iv )
    {
      rc_ptr<Volume<T> > rvol = (*iv)->refVolume();
      if( !rvol.get() )
        rvol = *iv;
      rc_ptr<AVolume<T> > avol( new AVolume<T>( rvol ) );
      _avolume.push_back( avol );
      insert( avol.get() );
      theAnatomist->registerObject( avol.get(), false );
      theAnatomist->releaseObject( avol.get() );
    }

    setupTransformationFromView();
  }
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
  while( size() >= 2 )
  {
    iterator i = begin();
    ++i;
    erase( i );
  }
  _avolume.clear();
  _myvolume->setVolume( vol );
  _myvolume->setReferentialInheritance( this );
  if( _myvolume->volume()->refVolume() )
  {
    rc_ptr<AVolume<T> > avol(
      new AVolume<T>( _myvolume->volume()->refVolume() ) );
    _avolume.push_back( avol );
    insert( avol.get() );
    theAnatomist->registerObject( avol.get(), false );
    theAnatomist->releaseObject( avol.get() );
  }
}


/*
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
*/


template <typename T>
void AVolumeView<T>::setupTransformationFromView()
{
  if( _avolume.empty() )
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
  rc_ptr<AVolume<T> > avol = _avolume[_resolution_level];

  if( at_origin && _myvolume->getReferential() == avol->getReferential() )
  {
    setReferentialInheritance( _myvolume.get() );
    return;
  }
  if( !avol->getReferential() )
  {
    // we need the reference volume to have a referential
    avol->setReferential( theAnatomist->centralReferential() );
  }

  /* WARNING: API mismatch: AObject::getReferential() const returns a
     non-const Referential *, while Sliceable::getReferential() const returns
     a const Referential *
  */
  Referential *ref = _myvolume->AObject::getReferential();
  if( !ref || ref == avol->getReferential() )
  {
    ref = new Referential;
    _myvolume->setReferential( ref );
  }
  Transformation *tr
    = theAnatomist->getTransformation( ref, avol->getReferential() );
  if( !tr )
    tr = new Transformation( ref, avol->AObject::getReferential() );
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
  if( _avolume.empty() )
    return;

  rc_ptr<AVolume<T> > avol = _avolume[_resolution_level];

  Transformation *tr
    = theAnatomist->getTransformation( _myvolume->getReferential(),
                                       avol->getReferential() );
  if( !tr )
    return;

  // TODO: erase all non-diagonal transform coefs

  rc_ptr<Volume<T> > vol = _myvolume->volume();
  vector<float> ivs = vol->getVoxelSize();
  vector<int> isz = vol->getSize();

  Point3df bbmin( 0, 0, 0 ),
    bbmax( ( isz[0] - 1 ) * ivs[0], ( isz[1] - 1 ) * ivs[1],
           ( isz[2] - 1 ) * ivs[2] );
  Point3df p0 = tr->transform( bbmin );
  Point3df p1 = tr->transform( bbmax );

  Point3df pos( min( p0[0], p1[0] ), min( p0[1], p1[1] ),
                min( p0[2], p1[2] ) );
  Point3df pmax( max( p0[0], p1[0] ), max( p0[1], p1[1] ),
                 max( p0[2], p1[2] ) );
  Point3df vs = avol->VoxelSize();
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
  nsize[0] = int( pmax[0] - pos[0] + 1 ); // ceil() ?
  nsize[1] = int( pmax[1] - pos[1] + 1);
  nsize[2] = int( pmax[2] - pos[2] + 1 );
  if( nsize[0] < 1 )
    nsize[0] = 1;
  if( nsize[1] < 1 )
    nsize[1] = 1;
  if( nsize[2] < 1 )
    nsize[2] = 1;

  cout << "calculated size: " << nsize[0] << ", " << nsize[1] << ", " << nsize[2] << endl;
  Point3df res( float( _target_size[0] ) / nsize[0],
                float( _target_size[1] ) / nsize[1],
                float( _target_size[2] ) / nsize[2] );
  cout << "scaling: " << res << endl;

  Point3df target_vs( vs[0] / res[0], vs[1] / res[1], vs[2] / res[2] );
  cout << "target vs: " << target_vs << endl;
  int resolution_level = selectBestResolutionLevel( target_vs );
  cout << "selected resolution_level: " << resolution_level << endl;
  if( resolution_level != _resolution_level )
  {
    _resolution_level = resolution_level;
    avol = _avolume[_resolution_level];
    vs = avol->VoxelSize();

    // recalculate position and size in the new ref volume
    Transformation *tr2
      = theAnatomist->getTransformation( _myvolume->getReferential(),
                                         avol->getReferential() );
    if( tr2 )
      tr = tr2;
    // if no tr2, assume the same as the previous one, which is probably wrong

    p0 = tr->transform( bbmin );
    p1 = tr->transform( bbmax );

    pos = Point3df( min( p0[0], p1[0] ), min( p0[1], p1[1] ),
                    min( p0[2], p1[2] ) );
    pmax = Point3df( max( p0[0], p1[0] ), max( p0[1], p1[1] ),
                     max( p0[2], p1[2] ) );
    pos[0] /= vs[0];
    pos[1] /= vs[1];
    pos[2] /= vs[2];
    pmax[0] /= vs[0];
    pmax[1] /= vs[1];
    pmax[2] /= vs[2];

    npos[0] = int( pos[0] );
    npos[1] = int( pos[1] );
    npos[2] = int( pos[2] );
  }

  nsize = _target_size;

  vector<int> maxsize = avol->volume()->getSize();
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
      = avol->volume()->allocatorContext().dataSourceInfo();
    cout << "read from file: " << dsi->url() << endl;
    Object options = Object::value( Dictionary() );
    options->setProperty( "ox", npos[0] );
    options->setProperty( "oy", npos[1] );
    options->setProperty( "oz", npos[2] );
    options->setProperty( "sx", nsize[0] );
    options->setProperty( "sy", nsize[1] );
    options->setProperty( "sz", nsize[2] );
    options->setProperty( "resolution_level", _resolution_level );
//     options->setProperty( "format", dsi->identifiedFormat() );

    Reader<Volume<T> > r( dsi->url() );
    r.setOptions( options );
    string format = dsi->identifiedFormat();
    VolumeRef<T> view = r.read( 0, &format );

//     VolumeRef<T> view( avol->volume(), npos, nsize );
    _myvolume->setVolume( view );

    setChanged();
  }
}


template <typename T>
int
AVolumeView<T>::selectBestResolutionLevel( const Point3df & target_vs ) const
{
  if( _avolume.size() < 2 )
    return 0; // no choice anyway

  Point3df ok_diff;
  Point3df subopt_diff;

  int level, n = _avolume.size();
  for( level=0; level<n; ++level )
  {
    Point3df vs = _avolume[level]->VoxelSize();
  }

  return 0; // TODO
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
void AVolumeView<T>::setTargetSize( const vector<int> & size )
{
  _target_size = size;
  setChanged();
}


template <typename T>
const vector<int> & AVolumeView<T>::targetSize() const
{
  return _target_size;
}


template <typename T>
void AVolumeView<T>::update( const Observable *observable, void *arg )
{
  const TransformationObserver *to
    = dynamic_cast<const TransformationObserver *>( observable );

  if( to && to->involves( _myvolume->getReferential() ) )
  {
    setupViewFromTransformation();
    if( _myvolume->hasChanged() )
      _myvolume->notifyObservers( this );
  }
  else if( observable == _myvolume.get() )
  {
    if( _myvolume->glHasChanged( GLComponent::glTEXIMAGE ) )
    {
      setChanged();
      notifyObservers( this );
    }
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

