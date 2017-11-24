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
#include <anatomist/reference/transfSet.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/control/qObjTree.h>
#include <anatomist/application/settings.h>

using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;


namespace
{
  int registerClass()
  {
    int   type = AObject::registerObjectType( "VolumeView" );

    if( QObjectTree::TypeNames.find( type ) == QObjectTree::TypeNames.end() )
    {
      string str = Settings::findResourceFile( "icons/list_volmultires.xpm" );
      if( !QObjectTree::TypeIcons[ type ].load( str.c_str() ) )
      {
        QObjectTree::TypeIcons.erase( type );
        cerr << "Icon " << str.c_str() << " not found\n";
      }

      QObjectTree::TypeNames[ type ] = "Multi-res volume view";
    }

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

  cout << "Warning:\n";
  cout << "--------\n";
  cout << "the AVolumeView object is still in development. Basic functionalities for multi-resolution sub-volume views are here but are largely sub-optimal and not integrated in a convenient way. There are also still bugs and Anatomist may crash when using this object.\n";

  insert( _myvolume.get() );
  theAnatomist->registerObject( _myvolume.get(), false );
  theAnatomist->releaseObject( _myvolume.get() );
  Referential *ref = new Referential;
  _myvolume->setReferential( ref );

  list<AObject *>::const_iterator io, eo = obj.end();
  for( io=obj.begin(); io!=eo; ++io )
  {
    AVolume<T> * vol = dynamic_cast<AVolume<T> *>( *obj.begin() );
    if( !vol->getReferential() )
      vol->setReferential( theAnatomist->centralReferential() );
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
    Transformation *tr
      = new Transformation( ref, vol->AObject::getReferential() );
    tr->motion().setToIdentity();
    tr->registerTrans();

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
  Referential *ref = new Referential;
  _myvolume->setReferential( ref );

  if( _myvolume->volume()->refVolume() )
  {
    _avolume.push_back( rc_ptr<AVolume<T> >(
      new AVolume<T>( _myvolume->volume()->refVolume() ) ) );
    insert( _avolume[0].get() );
    theAnatomist->registerObject( _avolume[0].get(), false );
    theAnatomist->releaseObject( _avolume[0].get() );
    _avolume[0]->setReferential( theAnatomist->centralReferential() );
    Transformation *tr
      = new Transformation( ref, _avolume[0]->AObject::getReferential() );
    tr->motion().setToIdentity();
    tr->registerTrans();
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
AVolumeView<T>::AVolumeView( const vector<rc_ptr<Volume<T> > > & vols )
  : anatomist::ObjectVector(),
  _myvolume( new AVolume<T>() ),
  _target_size( 4, 1 ),
  _resolution_level( 0 )
{
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
  Referential *ref = new Referential;
  _myvolume->setReferential( ref );


  if( !vols.empty() )
  {
    vector<rc_ptr<Volume<T> > > cvols;
    rc_ptr<Volume<T> > vol = vols[0];

    if( vol->header().hasProperty( "resolutions_dimension" ) )
    {
      VolumeRef<T> refvol = vol->refVolume();
      if( !refvol.get() )
        refvol = vol;
      try
      {
        Object all_res = vol->header().getProperty( "resolutions_dimension" );
        Object it;
        int rl = 0;
        cvols.reserve( all_res->size() + vols.size() - 1 );

        for( it=all_res->objectIterator(); it->isValid(); it->next(), ++rl )
        {
          Object res = it->currentValue();
          vector<int> vres;
          Object rit;
          cout << "resolution level dims: ";
          for( rit=res->objectIterator(); rit->isValid(); rit->next() )
          {
            vres.push_back( int( rint( rit->currentValue()->getScalar() ) ) );
            cout << vres[ vres.size() -1 ] << ", ";
          }
          cout << endl;
          if( vres == refvol->getSize() )
          {
            cout << "found current one\n";
            _resolution_level = rl;
            cvols.push_back( vol );
          }
          else
          {
            rc_ptr<DataSourceInfo> dsi
              = refvol->allocatorContext().dataSourceInfo();
            cout << "read from file: " << dsi->url() << endl;
            Object options = Object::value( Dictionary() );
            options->setProperty( "sx", 1 );
            options->setProperty( "sy", 1 );
            options->setProperty( "sz", 1 );
            options->setProperty( "resolution_level", rl );
//               options->setProperty( "unallocated", true );

            Reader<Volume<T> > r( dsi->url() );
            r.setOptions( options );
            string format = dsi->identifiedFormat();
            VolumeRef<T> view = r.read( 0, &format );
            cvols.push_back( view );
          }
        }
      }
      catch( exception & e )
      {
        cerr << "error while reading resolutions_dimension of the volume: "
          << e.what() << endl;
      }
      cvols.insert( cvols.end(), vols.begin() + 1, vols.end() );
    }
    else
      cvols.insert( cvols.end(), vols.begin(), vols.end() );

    setVolume( cvols[0] );
    if( _resolution_level != 0 )
      _myvolume->setVolume( vol );
    _target_size = vol->getSize();
    Transformation *tr
      = new Transformation( ref, _avolume[0]->AObject::getReferential() );
    tr->motion().setToIdentity();
    tr->registerTrans();

    typename vector<rc_ptr<Volume<T> > >::const_iterator iv, ev = cvols.end();
    for( iv=cvols.begin(), ++iv; iv!=ev; ++iv )
    {
      rc_ptr<Volume<T> > rvol = (*iv)->refVolume();
      if( !rvol.get() )
        rvol = *iv;
      rc_ptr<AVolume<T> > avol( new AVolume<T>( rvol ) );
      _avolume.push_back( avol );
      insert( avol.get() );
      theAnatomist->registerObject( avol.get(), false );
      theAnatomist->releaseObject( avol.get() );
      avol->setReferential( theAnatomist->centralReferential() );
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
  if( _myvolume->volume()->refVolume() )
  {
    rc_ptr<AVolume<T> > avol(
      new AVolume<T>( _myvolume->volume()->refVolume() ) );
    _avolume.push_back( avol );
    avol->setReferential( theAnatomist->centralReferential() );
    insert( avol.get() );
    theAnatomist->registerObject( avol.get(), false );
    theAnatomist->releaseObject( avol.get() );
  }
  _myvolume->SetExtrema();
  _myvolume->adjustPalette();
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
  Point3d ipos( 0, 0, 0 );
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
  vector<float> vs = _myvolume->voxelSize();
  vector<int> size = _myvolume->volume()->getSize();
  _initial_fov = Point3df( size[0] * vs[0], size[1] * vs[1], size[2] * vs[2] );
  Point3df trans( ipos[0] * vs[0], ipos[1] * vs[1], ipos[2] * vs[2] );
  atr.setTranslation( trans );
  tr->registerTrans();
}


template <typename T>
void AVolumeView<T>::setupViewFromTransformation()
{
  if( _avolume.empty() )
    return;

  rc_ptr<AVolume<T> > avol = _avolume[_resolution_level];

  Transformation *tr
    = theAnatomist->getTransformation( getReferential(),
                                       avol->getReferential() );
  if( !tr )
    return;

  // TODO: erase all non-diagonal transform coefs

  rc_ptr<Volume<T> > vol = _myvolume->volume();

  Point3df p0 = tr->transform( Point3df( 0, 0, 0 ) );
  Point3df p1 = tr->transform( _initial_fov );

  Point3df pos( min( p0[0], p1[0] ), min( p0[1], p1[1] ),
                min( p0[2], p1[2] ) );
  Point3df pmax( max( p0[0], p1[0] ), max( p0[1], p1[1] ),
                 max( p0[2], p1[2] ) );
  vector<float> vs = avol->voxelSize();

  Point3df vsize = pmax - pos;
  Point3df target_vs( vsize[0] / _target_size[0], vsize[1] / _target_size[1],
                      vsize[2] / _target_size[2] );
  cout << "target vs: " << Point3df( target_vs ) << endl;
  int resolution_level = selectBestResolutionLevel( target_vs );
  cout << "selected resolution_level: " << resolution_level << endl;

  if( resolution_level != _resolution_level )
  {
    avol = _avolume[resolution_level];
    vs = avol->voxelSize();
    cout << "new vs: " << Point3df( vs ) << endl;

    // recalculate position and size in the new ref volume
    Transformation *tr2
      = theAnatomist->getTransformation( getReferential(),
                                         avol->getReferential() );
    if( tr2 )
      tr = tr2;
    // if no tr2, assume the same as the previous one, which is probably wrong
  }

  pos[0] /= vs[0];
  pos[1] /= vs[1];
  pos[2] /= vs[2];
  pmax[0] /= vs[0];
  pmax[1] /= vs[1];
  pmax[2] /= vs[2];
  cout << "new pos: " << pos << endl;
  cout << "new size: " << pmax - pos << endl;

//   Point3df res( float( vsize[0] / _target_size[0] ),
//                 float( vsize[1] / _target_size[1] ),
//                 float( vsize[2] / _target_size[2] ) );
//   cout << "scaling: " << res << endl;

  vector<int> npos( 3 );
  Point3df neg_trans( 0, 0, 0 );
  npos[0] = int( pos[0] );
  npos[1] = int( pos[1] );
  npos[2] = int( pos[2] );
  if( npos[0] < 0 )
  {
    neg_trans[0] = npos[0] * vs[0];
    npos[0] = 0;
  }
  if( npos[1] < 0 )
  {
    neg_trans[1] = npos[1] * vs[1];
    npos[1] = 0;
  }
  if( npos[2] < 0 )
  {
    neg_trans[2] = npos[2] * vs[2];
    npos[2] = 0;
  }

  vector<int> nsize( 3 );
  nsize[0] = int( pmax[0] ) - npos[0] + 1; // ceil() ?
  nsize[1] = int( pmax[1] ) - npos[1] + 1;
  nsize[2] = int( pmax[2] ) - npos[2] + 1;
  if( nsize[0] < 1 )
    nsize[0] = 1;
  if( nsize[1] < 1 )
    nsize[1] = 1;
  if( nsize[2] < 1 )
    nsize[2] = 1;

  cout << "calculated size: " << nsize[0] << ", " << nsize[1] << ", " << nsize[2] << endl;

//   nsize = _target_size;

  vector<int> maxsize = avol->volume()->getSize();

  if( npos[0] >= maxsize[0] )
    npos[0] = maxsize[0] - 1;
  if( npos[1] >= maxsize[1] )
    npos[1] = maxsize[1] - 1;
  if( npos[2] >= maxsize[2] )
    npos[2] = maxsize[2] - 1;

  if( npos[0] + nsize[0] > maxsize[0] )
    nsize[0] = maxsize[0] - npos[0];
  if( npos[1] + nsize[1] > maxsize[1] )
    nsize[1] = maxsize[1] - npos[1];
  if( npos[2] + nsize[2] > maxsize[2] )
    nsize[2] = maxsize[2] - npos[2];

  Transformation *tr2
    = theAnatomist->getTransformation( _myvolume->getReferential(),
                                       avol->getReferential() );
  if( !tr2 )
  {
    tr2 = new Transformation( _myvolume->AObject::getReferential(),
                              avol->AObject::getReferential() );
  }
  AffineTransformation3d & mtr = tr2->motion();
  mtr.setToIdentity();
  Point3df new_trans = tr->transform( Point3df( 0, 0, 0 ) ) - neg_trans;
  new_trans[0] = rint( new_trans[0] / vs[0] ) * vs[0];
  new_trans[1] = rint( new_trans[1] / vs[1] ) * vs[1];
  new_trans[2] = rint( new_trans[2] / vs[2] ) * vs[2];
  mtr.setTranslation( new_trans );

  cout << "npos: " << Point3df( npos[0], npos[1], npos[2] ) << ", neg_trans: " << neg_trans << endl;
  cout << "trans: " << mtr.translation() << endl;
  cout << "size: " << Point3d( nsize[0], nsize[1], nsize[2] ) << endl;

  vector<int> volpos = vol->posInRefVolume();
  vector<int> volsz = vol->getSize();
  if( resolution_level != _resolution_level
      || volpos[0] != npos[0] || volpos[1] != npos[1] || volpos[2] != npos[2]
      || volsz[0] != nsize[0] || volsz[1] != nsize[1] || volsz[2] != nsize[2] )
  {
    _resolution_level = resolution_level;
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
    _myvolume->SetExtrema();
    _myvolume->adjustPalette();

    setChanged();
  }
  tr2->registerTrans();
  ATransformSet *tset = ATransformSet::instance();
  tset->updateTransformation( tr2 );
}


template <typename T>
int
AVolumeView<T>::selectBestResolutionLevel( const Point3df & target_vs ) const
{
  if( _avolume.size() < 2 )
    return 0; // no choice anyway

  Point3df ok_diff;
  Point3df subopt_diff;
  bool found_ok = false;
  int best_level = -1;

  int level, n = _avolume.size();
  for( level=0; level<n; ++level )
  {
    Point3df vs = Point3df( _avolume[level]->voxelSize() );
    Point3df diff = vs - target_vs;
    // make it relative
    diff[0] /= target_vs[0];
    diff[1] /= target_vs[1];
    diff[2] /= target_vs[2];

    if( diff[0] >= 0 && diff[1] >= 0 && diff[2] >= 0 )
    {
      if( !found_ok )
      {
        found_ok = true;
        best_level = level;
        ok_diff = diff;
      }
      else
      {
        if( max( ok_diff[0], max( ok_diff[1], ok_diff[2] ) )
            > max( diff[0], max( diff[1], diff[2] ) ) )
        {
          best_level = level;
          ok_diff = diff;
        }
      }
    }
    else if( !found_ok )
    {
      if( best_level < 0 )
      {
        subopt_diff = diff;
        best_level = level;
      }
      else
      {
        if( max( fabs( diff[0] ), max( fabs( diff[1] ), fabs( diff[2] ) ) )
            < max( fabs( subopt_diff[0] ),
                   max( fabs( subopt_diff[1] ), fabs( subopt_diff[2] ) ) ) )
        {
          subopt_diff = diff;
          best_level = level;
        }
      }
    }
  }

  if( best_level < 0 )
    best_level = n - 1; // take the lowest, it's safer

  return best_level;
}


template <typename T>
bool AVolumeView<T>::render( PrimList & prim, const ViewState & vs )
{
  return _myvolume->render( prim, vs );
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
void AVolumeView<T>::setInitialFOV( const Point3df & fov )
{
  _initial_fov = fov;
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

