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

#include <anatomist/surface/vectorfield.h>
#include <anatomist/reference/Transformation.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/surface/texsurface.h>
#include <anatomist/surface/texture.h>
#include <anatomist/volume/Volume.h>
#include <anatomist/object/actions.h>
#include <anatomist/surface/wvectorfield.h>
#include <aims/resampling/quaternion.h>

using namespace anatomist;
using namespace carto;
using namespace std;

//--------------------------------------------------------------

struct VectorField::Private
{
  enum DataType
  {
    NONE, INT8, UINT8, INT16, UINT16, INT32, UINT32, INT64, UINT64, FLOAT,
    DOUBLE, RGB, RGBA,
  };

  Private();
  ~Private() {}

  float getValue( int chan, const Point3d & pos );
  void setSize( int chan );
  static DataType getDataType( AObject* obj );

  rc_ptr<ASurface<2> > vecMesh;
  rc_ptr<ATexture> vecTexture;
  rc_ptr<ATexSurface> vecField;
  vector<Point3di> vardim;
  vector<vector<int> > fixeddim;
  vector<AObject *> refvol;
  vector<vector<int> > sizes;
  vector<DataType> datatype;
  float scaling;
};


VectorField::Private::Private()
  : vardim( 3, Point3di( 0 ) ), fixeddim( 3 ), refvol( 3, 0 ), sizes( 3 ),
    scaling( 0.01 ), datatype( 3, NONE )
{
}


VectorField::Private::DataType
VectorField::Private::getDataType( AObject* vol )
{
  if( dynamic_cast<AVolume<int8_t> *>( vol ) )
    return INT8;
  else if( dynamic_cast<AVolume<uint8_t> *>( vol ) )
    return UINT8;
  else if( dynamic_cast<AVolume<int16_t> *>( vol ) )
    return INT16;
  else if( dynamic_cast<AVolume<uint16_t> *>( vol ) )
    return UINT16;
  else if( dynamic_cast<AVolume<int32_t> *>( vol ) )
    return INT32;
  else if( dynamic_cast<AVolume<uint32_t> *>( vol ) )
    return UINT32;
  else if( dynamic_cast<AVolume<int64_t> *>( vol ) )
    return INT64;
  else if( dynamic_cast<AVolume<uint64_t> *>( vol ) )
    return UINT64;
  else if( dynamic_cast<AVolume<float> *>( vol ) )
    return FLOAT;
  else if( dynamic_cast<AVolume<double> *>( vol ) )
    return DOUBLE;
  else if( dynamic_cast<AVolume<AimsRGB> *>( vol ) )
    return RGB;
  else if( dynamic_cast<AVolume<AimsRGBA> *>( vol ) )
    return RGBA;
  return NONE;
}


float VectorField::Private::getValue( int chan, const Point3d & pos )
{
  AObject *vol = refvol[chan];
  if( !vol || pos[0] < 0 || pos[1] < 0 || pos[2] < 0 )
    return 0.;
  vector<int> ndpos = fixeddim[chan];
  if( sizes[chan].empty() )
  {
    setSize( chan );
    if( sizes[chan].empty() )
      return 0.;
  }
  const vector<int> & size = sizes[chan];
  const Point3di & dim = vardim[chan];
  if( size[dim[0]] <= pos[0]
      || size[dim[1]] <= pos[1]
      || size[dim[2]] <= pos[2] )
    return 0.;
  ndpos[dim[0]] = pos[0];
  ndpos[dim[1]] = pos[1];
  ndpos[dim[2]] = pos[2];
  switch( datatype[chan] )
  {
  case INT8:
    return static_cast<AVolume<int8_t> *>( vol )->volume()->at( ndpos );
  case UINT8:
    return static_cast<AVolume<uint8_t> *>( vol )->volume()->at( ndpos );
  case INT16:
    return static_cast<AVolume<int16_t> *>( vol )->volume()->at( ndpos );
  case UINT16:
    return static_cast<AVolume<uint16_t> *>( vol )->volume()->at( ndpos );
  case INT32:
    return static_cast<AVolume<int32_t> *>( vol )->volume()->at( ndpos );
  case UINT32:
    return static_cast<AVolume<uint32_t> *>( vol )->volume()->at( ndpos );
  case INT64:
    return static_cast<AVolume<int64_t> *>( vol )->volume()->at( ndpos );
  case UINT64:
    return static_cast<AVolume<uint64_t> *>( vol )->volume()->at( ndpos );
  case FLOAT:
    return static_cast<AVolume<float> *>( vol )->volume()->at( ndpos );
  case DOUBLE:
    return float(
      static_cast<AVolume<double> *>( vol )->volume()->at( ndpos ) );
  case RGB:
    return static_cast<AVolume<AimsRGB> *>( vol )->volume()->at( ndpos )[chan];
  case RGBA:
    return static_cast<AVolume<AimsRGBA> *>( vol )->volume()->at( ndpos )
      [chan];
  default:
    return 0.;
  }
}


void VectorField::Private::setSize( int chan )
{
  AObject *vol = refvol[chan];
  if( !vol )
    return;

  switch( datatype[chan] )
  {
  case INT8:
    sizes[chan]
      = static_cast<AVolume<int8_t> *>( vol )->volume()->getSize();
    break;
  case UINT8:
    sizes[chan]
      = static_cast<AVolume<uint8_t> *>( vol )->volume()->getSize();
    break;
  case INT16:
    sizes[chan]
      = static_cast<AVolume<int16_t> *>( vol )->volume()->getSize();
    break;
  case UINT16:
    sizes[chan]
      = static_cast<AVolume<uint16_t> *>( vol )->volume()->getSize();
    break;
  case INT32:
    sizes[chan]
      = static_cast<AVolume<int32_t> *>( vol )->volume()->getSize();
    break;
  case UINT32:
    sizes[chan]
      = static_cast<AVolume<uint32_t> *>( vol )->volume()->getSize();
    break;
  case INT64:
    sizes[chan]
      = static_cast<AVolume<int64_t> *>( vol )->volume()->getSize();
    break;
  case UINT64:
    sizes[chan]
      = static_cast<AVolume<uint64_t> *>( vol )->volume()->getSize();
    break;
  case FLOAT:
    sizes[chan]
      = static_cast<AVolume<float> *>( vol )->volume()->getSize();
    break;
  case DOUBLE:
    sizes[chan]
      = static_cast<AVolume<double> *>( vol )->volume()->getSize();
    break;
  case RGB:
    sizes[chan]
      = static_cast<AVolume<AimsRGB> *>( vol )->volume()->getSize();
    break;
  case RGBA:
    sizes[chan]
      = static_cast<AVolume<AimsRGBA> *>( vol )->volume()->getSize();
    break;
  default:
    break;
  }
}



//--------------------------------------------------------------
VectorField::VectorField( const vector<AObject *> & obj )
  : ObjectVector(), Sliceable(), d( new Private )
{
  _type = AObject::VECTORFIELD;

  // Insert the input objects
  vector<AObject *>::const_iterator io, eo=obj.end();
  size_t i = 0, n = obj.size();

  for( io=obj.begin(); io!=eo; ++io, ++i )
  {
    d->refvol[i] = *io;
    d->datatype[i] = d->getDataType( *io );
    d->vardim[i] = Point3di( 0, 1, 2 );
    d->setSize( i );
    d->fixeddim[i] = vector<int>( d->sizes[i].size(), 0 );
    insert( carto::shared_ptr<AObject>( carto::shared_ptr<AObject>::Strong,
                                        *io ) );
  }
  for( ; i<3; ++i )
  {
    if( d->sizes[i-1].size() < 4
        || d->sizes[i-1][3] <= d->fixeddim[i-1][3] + 1 )
    {
      d->refvol[i] = 0;
    }
    else
    {
      d->refvol[i] = d->refvol[i-1];
      d->datatype[i] = d->datatype[i-1];
      d->vardim[i] = d->vardim[i-1];
      d->sizes[i] = d->sizes[i-1];
      d->fixeddim[i] = d->fixeddim[i-1];
      ++d->fixeddim[i][3];
    }
  }

  // Create the merged surface object
  d->vecMesh.reset( new ASurface<2>( "" ) );
  d->vecMesh->setSurface( new AimsTimeSurface<2,Void> );
  Material & polmat = d->vecMesh->GetMaterial();
//   polmat.SetDiffuse( 1., 0., 0., 1. );
  polmat.setLineWidth( 2. );
//   polmat.setRenderProperty( Material::RenderFiltering, 1 );
  d->vecMesh->SetMaterial( polmat );
  d->vecMesh->setReferentialInheritance( *begin() );
  d->vecMesh->setName( theAnatomist->makeObjectName( "vector field mesh" ) );
  theAnatomist->registerObject( d->vecMesh.get(), false );
  theAnatomist->releaseObject( d->vecMesh.get() );

  d->vecTexture.reset( new ATexture );
  d->vecTexture->setTexture( rc_ptr<Texture1d>( new Texture1d ) );
  d->vecTexture->setName( theAnatomist->makeObjectName(
    "vector field texture" ) );
  theAnatomist->registerObject( d->vecTexture.get(), false );
  theAnatomist->releaseObject( d->vecTexture.get() );

  d->vecField.reset( new ATexSurface( d->vecMesh.get(),
                                      d->vecTexture.get() ) );
  d->vecField->setName( theAnatomist->makeObjectName( "vector field" ) );
  theAnatomist->registerObject( d->vecField.get(), false );
  theAnatomist->releaseObject( d->vecField.get() );

//   SetMaterial( polmat );
  setReferentialInheritance( *begin() );

  insert( rc_ptr<AObject>( d->vecField.get() ) );

//   buildMesh( SliceViewState() );
}

//--------------------------------------------------------------
VectorField::~VectorField()
{
  delete d;
}

const GLComponent* VectorField::glAPI() const
{
  return this;
}

GLComponent* VectorField::glAPI()
{
  return this;
}

//--------------------------------------------------------------
void VectorField::setVoxelSize( const Point3df & voxelSize )
{
  (*begin())->setVoxelSize( voxelSize );
}

//--------------------------------------------------------------
Point3df VectorField::VoxelSize() const
{
  return (*begin())->VoxelSize();
}

//--------------------------------------------------------------
Point3df VectorField::glVoxelSize() const
{
  return (*begin())->VoxelSize();
}

//--------------------------------------------------------------
void VectorField::update( const Observable * observable, void * arg )
{
  // TODO: check what changed to know if an update is needed
  glSetChanged( GLComponent::glGENERAL );
  glSetChanged( GLComponent::glBODY );

  AObject::update( observable, arg );

  obsSetChanged( GLComponent::glMATERIAL );
  setChanged();
  notifyObservers( (void*) this );
}

//--------------------------------------------------------------
void VectorField::SetMaterial( const Material & mat )
{
  d->vecMesh->SetMaterial( mat );
  AObject::SetMaterial( mat );
  glSetChanged( glMATERIAL );
  setChanged();
}

//--------------------------------------------------------------
Material & VectorField::GetMaterial()
{
  return d->vecMesh->GetMaterial();
}


const AObjectPalette* VectorField::glPalette( unsigned tex ) const
{
  return d->vecTexture->glPalette( tex );
}


AObjectPalette* VectorField::palette()
{
  return d->vecTexture->palette();
}


const AObjectPalette* VectorField::palette() const
{
  return d->vecTexture->palette();
}


void VectorField::setPalette( const AObjectPalette & palette )
{
  d->vecTexture->setPalette( palette );
  AObject::setPalette( palette );
  glSetChanged( glPALETTE );
}


void VectorField::createDefaultPalette( const std::string & name )
{
  d->vecTexture->createDefaultPalette( name );
  AObject::setPalette( *d->vecTexture->palette() );
  glSetChanged( glPALETTE );
}


//--------------------------------------------------------------
bool VectorField::render( PrimList & prim, const ViewState & state )
{
  buildMesh( state );

  return d->vecField->render( prim, state );
}

//--------------------------------------------------------------
Point4df VectorField::getPlane( const ViewState & state ) const
{
  const SliceViewState * st = state.sliceVS();
  if ( !st )
    return Point4df( 0., 0., 0., 0. );

  const aims::Quaternion & quat = *st->orientation;
  Point3df n = quat.transformInverse( Point3df( 0, 0, 1 ) );
  float d = -n.dot( st->position );

  // transform coordinates for plane if needed
  const Referential *wref = st->winref;
  const Referential *oref = getReferential();
  if( wref && oref )
  {
    const Transformation *tr
      = theAnatomist->getTransformation( wref, oref );
    if( tr )
    {
      // transform a trieder
      Point3df v3 = tr->transform( n ) - tr->transform( Point3df( 0. ) );
      Point3df v1, v2;
      v1 = Point3df( 1, 0, 0 );
      v1 = Point3df( 1, 0, 0 ) - n * n.dot( Point3df( 1, 0, 0 ) );
      if( v1.norm2() < 1e-5 )
        v1 = Point3df( 0, 1, 0 ) - n * n.dot( Point3df( 0, 1, 0 ) );
      v1.normalize();
      v2 = vectProduct( n, v1 );
      // transform v1 and v2
      v1 = tr->transform( v1 ) - tr->transform( Point3df( 0. ) );
      v2 = tr->transform( v2 ) - tr->transform( Point3df( 0. ) );
      n = vectProduct( v1, v2 );
      n.normalize();
      d = - n.dot( tr->transform( st->position ) );
    }
  }

  return Point4df( n[0], n[1], n[2], d );
}

//--------------------------------------------------------------
Point4df VectorField::glMin2D() const
{
//   if ( !d->vecMesh )
  return Point4df( 0, 0, 0, 0 );

//   Point3df min, max;
//   d->vecMesh->boundingBox( min, max );
//
//   return Point4df( min[0], min[1], min[2], 0 );
}

//--------------------------------------------------------------
Point4df VectorField::glMax2D() const
{
  Point4df p( 0. );
  int chan = 0;
  if( !d->refvol[0] )
  {
    chan = 1;
    if( !d->refvol[1] )
    {
      chan = 2;
      if( !d->refvol[2] )
        return p;
    }
  }
  p[0] = d->sizes[chan][d->vardim[chan][0]] - 1;
  p[1] = d->sizes[chan][d->vardim[chan][1]] - 1;
  p[2] = d->sizes[chan][d->vardim[chan][2]] - 1;
  return p;
}


//--------------------------------------------------------------
const Referential * VectorField::getReferential() const
{
  return AObject::getReferential();
}

//--------------------------------------------------------------
void VectorField::buildMesh( const ViewState & state )
{
//   cout << "buildMesh\n";

  // Update the merged surface polygons
  Point4df plane = getPlane( state );
  if( plane.norm2() == 0 )
    return;

  Point3df norm( plane[0], plane[1], plane[2] );
  norm.normalize();
  Point3df dir1 = vectProduct( norm, Point3df( 1., 0., 0. ) );
  if( dir1.norm2() < 1.e-6 )
    dir1 = dir1 = vectProduct( norm, Point3df( 0., 0., 1. ) );
  Point3df dir2 = vectProduct( norm, dir1 / dir1.norm() );
  Point3df vs = VoxelSize();
  dir1.normalize();
  dir2.normalize();

  AimsTimeSurface<2, Void> *new_surf = new AimsTimeSurface<2, Void>;
  Texture1d *new_tex = new Texture1d;

  vector<Point3df> & vert = new_surf->vertex();
  vector<AimsVector<uint32_t, 2> > & poly = new_surf->polygon();
  vector<float> & tex = (*new_tex)[0].data();

  int ymin, ymax, xmin, xmax, x, y;
  size_t n = 0;
  Point3df pos0, cpos, vec;
  Point3d voxpos;
  pos0 = Point3df( 0, 0, 0 );
  pos0 -= ( norm.dot(pos0) + plane[3] ) / norm.norm() * norm;

//   cout << "dir1: " << dir1 << ", dir2: " << dir2 << ", vs: " << vs << endl;

  // project extremities to get bounds
  float fxmin = dir1.dot( Point3df( 0., 0., 0. ) ), fxmax = fxmin;
  float fymin = dir2.dot( Point3df( 0., 0., 0. ) ), fymax = fymin;
  Point4df pmax4 = glMax2D();
  Point3df pmax( pmax4[0] * vs[0],
                 pmax4[1] * vs[1],
                 pmax4[2] * vs[2] );
  float fx = dir1.dot( Point3df( pmax[0], 0, 0 ) );
  if( fx < fxmin )
    fxmin = fx;
  if( fx > fxmax )
    fxmax = fx;
  fx = dir1.dot( Point3df( 0, pmax[1], 0 ) );
  if( fx < fxmin )
    fxmin = fx;
  if( fx > fxmax )
    fxmax = fx;
  fx = dir1.dot( Point3df( pmax[0], pmax[1], 0 ) );
  if( fx < fxmin )
    fxmin = fx;
  if( fx > fxmax )
    fxmax = fx;
  fx = dir1.dot( Point3df( 0, 0, pmax[2] ) );
  if( fx < fxmin )
    fxmin = fx;
  if( fx > fxmax )
    fxmax = fx;
  fx = dir1.dot( Point3df( pmax[0], 0, pmax[2] ) );
  if( fx < fxmin )
    fxmin = fx;
  if( fx > fxmax )
    fxmax = fx;
  fx = dir1.dot( Point3df( 0, pmax[1], pmax[2] ) );
  if( fx < fxmin )
    fxmin = fx;
  if( fx > fxmax )
    fxmax = fx;
  fx = dir1.dot( Point3df( pmax[0], pmax[1], pmax[2] ) );
  if( fx < fxmin )
    fxmin = fx;
  if( fx > fxmax )
    fxmax = fx;

  fx = dir2.dot( Point3df( pmax[0], 0, 0 ) );
  if( fx < fymin )
    fymin = fx;
  if( fx > fymax )
    fymax = fx;
  fx = dir2.dot( Point3df( 0, pmax[1], 0 ) );
  if( fx < fymin )
    fymin = fx;
  if( fx > fymax )
    fymax = fx;
  fx = dir2.dot( Point3df( pmax[0], pmax[1], 0 ) );
  if( fx < fymin )
    fymin = fx;
  if( fx > fymax )
    fymax = fx;
  fx = dir2.dot( Point3df( 0, 0, pmax[2] ) );
  if( fx < fymin )
    fymin = fx;
  if( fx > fymax )
    fymax = fx;
  fx = dir2.dot( Point3df( pmax[0], 0, pmax[2] ) );
  if( fx < fymin )
    fymin = fx;
  if( fx > fymax )
    fymax = fx;
  fx = dir2.dot( Point3df( 0, pmax[1], pmax[2] ) );
  if( fx < fymin )
    fymin = fx;
  if( fx > fymax )
    fymax = fx;
  fx = dir2.dot( Point3df( pmax[0], pmax[1], pmax[2] ) );
  if( fx < fymin )
    fymin = fx;
  if( fx > fymax )
    fymax = fx;

  // sampling step: min step to advance 1 voxel on an axis
  float psx = fabs( dir1.dot( Point3df( vs[0], 0, 0 ) ) );
  float psy = fabs( dir1.dot( Point3df( 0, vs[1], 0 ) ) );
  float psz = fabs( dir1.dot( Point3df( 0, 0, vs[2] ) ) );
  float step1 = 10000.;
  if( psx >= 1e-5 )
    step1 = std::min( step1, vs[0] * vs[0] / psx );
  if( psy >= 1e-5 )
    step1 = std::min( step1, vs[1] * vs[1] / psy );
  if( psz >= 1e-5 )
    step1 = std::min( step1, vs[2] * vs[2] / psz );
  psx = fabs( dir2.dot( Point3df( vs[0], 0, 0 ) ) );
  psy = fabs( dir2.dot( Point3df( 0, vs[1], 0 ) ) );
  psz = fabs( dir2.dot( Point3df( 0, 0, vs[2] ) ) );
  float step2 = 10000.;
  if( psx >= 1e-5 )
    step2 = std::min( step2, vs[0] * vs[0] / psx );
  if( psy >= 1e-5 )
    step2 = std::min( step2, vs[1] * vs[1] / psy );
  if( psz >= 1e-5 )
    step2 = std::min( step2, vs[2] * vs[2] / psz );

  xmin = int( floor( fxmin / step1 ) );
  xmax = int( ceil( fxmax / step1 ) ) + 1;
  ymin = int( floor( fymin / step2 ) );
  ymax = int( ceil( fymax / step2 ) ) + 1;

  dir1 *= step1;
  dir2 *= step2;

//   cout << "step1: " << step1 << ", step2: " << step2 << ", xmin: " << xmin << ", xmax: " << xmax << ", ymin: " << ymin << ", ymax: " << ymax << ", dir1: " << dir1 << ", dir2: " << dir2 << endl;

  size_t np = ( xmax - xmin ) * ( ymax - ymin );
  vert.reserve( np * 2 );
  poly.reserve( np );
  tex.reserve( np * 2 );
  float intensity;

  for( y=ymin; y<ymax; ++y )
  {
    cpos = pos0 + y * dir2 + xmin * dir1;
    for( x=xmin; x<xmax; ++x )
    {
      voxpos = Point3d( int( rint( cpos[0] / vs[0] ) ),
                        int( rint( cpos[1] / vs[1] ) ),
                        int( rint( cpos[2] / vs[2] ) ) );
      vec[0] = d->getValue( 0, voxpos );
      vec[1] = d->getValue( 1, voxpos );
      vec[2] = d->getValue( 2, voxpos );
      if( vec != Point3df( 0., 0., 0. ) )
      {
        intensity = vec.norm();
        vec *= d->scaling;

        vert.push_back( cpos - vec );
        vert.push_back( cpos + vec );
        poly.push_back( AimsVector<uint32_t, 2>( n, n + 1 ) );
        tex.push_back( -intensity );
        tex.push_back( intensity );
        n += 2;
      }
      cpos += dir1;
    }
  }

//   cout << "vertices: " << vert.size() << ", polygons: " << poly.size() << ", texture: " << tex.size() << endl;

  d->vecMesh->setSurface( new_surf );
  d->vecTexture->setTexture( rc_ptr<Texture1d>( new_tex ) );

  d->vecMesh->notifyObservers( this );
  d->vecTexture->notifyObservers( this );
}


int VectorField::canFusion( const set<AObject *> & obj )
{
  set<AObject *>::const_iterator	io, fo = obj.end();
  int i = 0;
  GLComponent *gl;
  for( io=obj.begin(); io!=fo && i<3; ++io, ++i )
  {
    if( Private::getDataType( *io ) == Private::NONE )
      return 0;
  }
  if( i >= 3 )
    return 0;

  return 50;
}



ObjectMenu* VectorField::optionMenu() const
{
  rc_ptr<ObjectMenu> om = getObjectMenu( objectFullTypeName() );
  if( !om )
  {
    om.reset( new ObjectMenu );
    vector<string>  vs;
    vs.reserve(1);
    vs.push_back(QT_TRANSLATE_NOOP( "QSelectMenu", "File"));
    om->insertItem(vs, QT_TRANSLATE_NOOP( "QSelectMenu",
                   "Rename object"),
                   &ObjectActions::renameObject);
    vs[0] = QT_TRANSLATE_NOOP("QSelectMenu", "Color");
    om->insertItem(vs, QT_TRANSLATE_NOOP("QSelectMenu", "Palette"),
                   &ObjectActions::colorPalette);
    om->insertItem(vs,QT_TRANSLATE_NOOP("QSelectMenu", "Material"),
                   &ObjectActions::colorMaterial);
    om->insertItem(vs,QT_TRANSLATE_NOOP("QSelectMenu", "Rendering"),
                   &ObjectActions::colorRendering);
    om->insertItem(vs,QT_TRANSLATE_NOOP("QSelectMenu", "Texturing"),
                   &ObjectActions::textureControl);
    vs[0] = QT_TRANSLATE_NOOP("QSelectMenu", "Referential");
    om->insertItem(vs,QT_TRANSLATE_NOOP("QSelectMenu", "Load"),
                   &ObjectActions::referentialLoad);
    vs[0] = QT_TRANSLATE_NOOP("QSelectMenu", "Fusion");
    om->insertItem(vs, QT_TRANSLATE_NOOP("QSelectMenu",
                                         "Edit vector field properties"),
                   &VectorField::editVectorFieldProperties);
    setObjectMenu( objectFullTypeName(), om );
  }
  return AObject::optionMenu();
}


Tree* VectorField::optionTree() const
{
  return AObject::optionTree();
}


void VectorField::editVectorFieldProperties( const set<AObject *> & obj )
{
  VectorFieldEditionWindow *vfw = new VectorFieldEditionWindow( obj );
  vfw->show();
}


AObject *VectorField::volume( int channel ) const
{
  if( channel >= 3 )
    return 0;
  return d->refvol[ channel ];
}


void VectorField::setVolume( int channel, AObject* obj )
{
  if( channel >= 3 )
    return;

  if( d->refvol[ channel ] == obj )
    return; // not changed

  MObject::iterator io = find( obj );
  if( io == end() )
  {
    cerr << "VectorField::setVolume: object not part of the vector field\n";
    return;
  }

  //   AObject *old = d->refvol[ channel ];

  d->refvol[ channel ] = obj;
  d->datatype[ channel ] = d->getDataType( obj );
  d->setSize( channel );
  d->vardim[channel] = Point3di( 0, 1, 2 );
  d->fixeddim[channel] = vector<int>( d->sizes[channel].size(), 0 );

  glSetChanged( glBODY );
  setChanged();
}


Point3di VectorField::spaceCoordsDimensions( int channel ) const
{
  if( channel >= 3 )
    return Point3di( 0, 1, 2 );

  return d->vardim[ channel ];
}


void VectorField::setSpaceCoordsDimensions( int channel,
                                            const Point3di & dims )
{
  if( channel >= 3 )
    return;

  d->vardim[ channel ] = dims;
  size_t n = d->sizes[channel].size();
  if( d->vardim[ channel ][0] >= n )
    d->vardim[ channel ][0] = n - 1;
  if( d->vardim[ channel ][1] >= n )
    d->vardim[ channel ][1] = n - 1;
  if( d->vardim[ channel ][2] >= n )
    d->vardim[ channel ][2] = n - 1;
  glSetChanged( glBODY );
  setChanged();
}


vector<int> VectorField::vectorChannelPosition( int channel ) const
{
  if( channel >= 3 )
    return vector<int>();

  return d->fixeddim[ channel ];
}


void VectorField::setVectorChannelPosition( int channel,
                                            const vector<int> & pos )
{
  if( channel >= 3 )
    return;

  d->fixeddim[ channel ] = pos;
  size_t n = d->sizes[ channel ].size();
  if( d->fixeddim[ channel ].size() > n )
    d->fixeddim[ channel ].resize( n );
  glSetChanged( glBODY );
  setChanged();
}


float VectorField::scaling() const
{
  return d->scaling;
}


void VectorField::setScaling( float scaling )
{
  d->scaling = scaling;
  glSetChanged( glBODY );
  setChanged();
}


vector<int> VectorField::volumeSize( int channel ) const
{
  if( channel >= 3 )
    return vector<int>();

  if( !d->refvol[channel ] )
    return vector<int>();
  return d->sizes[channel];
}


