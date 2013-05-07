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

#include <anatomist/sparsematrix/connectivitymatrix.h>
#include <anatomist/sparsematrix/sparsematrix.h>
#include <anatomist/surface/texsurface.h>
#include <anatomist/surface/texture.h>
#include <anatomist/surface/mtexture.h>
#include <anatomist/application/settings.h>
#include <anatomist/control/qObjTree.h>
#include <anatomist/color/objectPalette.h>
#include <anatomist/color/paletteList.h>
#include <anatomist/fusion/fusionFactory.h>
#include <anatomist/application/Anatomist.h>
#include <aims/sparsematrix/sparseMatrix.h>
#include <aims/mesh/surfacegen.h>
#include <aims/mesh/surfaceOperation.h>
#include <aims/utility/converter_texture.h>

using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;


struct AConnectivityMatrix::Private
{
  Private();

  AConnectivityMatrix::PatchMode patchmode;
  int patchnum;
  ASparseMatrix *sparse;
  ATriangulated *mesh;
  ATexture *patch;
  ATexture *texture;
  AMTexture *mtexture;
  ATexSurface *texsurface;
  ATriangulated *marker;
  vector<uint32_t> patchindices;
  uint32_t vertex;
};


AConnectivityMatrix::Private::Private()
  : patchmode( AConnectivityMatrix::ALL_BUT_ONE ), patchnum( 0 ),
  sparse( 0 ), mesh( 0 ), patch( 0 ), texture( 0 ), mtexture( 0 ),
  texsurface( 0 ), marker( 0 ), vertex( 0 )
{
}

namespace
{
  int connectivityMatrixType()
  {
    static int type = 0;
    if( type == 0 )
    {
      type = AObject::registerObjectType( "ConnectivityMatrix" );
      QObjectTree::setObjectTypeName( type, string( "Connectivity Matrix" ) );
      string str = Settings::findResourceFile( 
        "icons/connectivitymatrix.png" );
      QObjectTree::setObjectTypeIcon( type, str );
    }
    return type;
  }
}


// ---

AConnectivityMatrix::AConnectivityMatrix( const vector<AObject *> & obj )
  : ObjectVector(), d( new Private )
{
  _type = connectivityMatrixType();

  list<AObject *> filteredobj;
  set<AObject *> sobj;
  sobj.insert( obj.begin(), obj.end() );

  if( !checkObjects( sobj, filteredobj, d->patchmode, d->patchnum ) )
  {
    cerr << "AConnectivityMatrix: inconsistency in fusion objects types\n";
    return;
  }

  list<AObject *>::iterator io, eo = filteredobj.end();
  io=filteredobj.begin();
  d->sparse = static_cast<ASparseMatrix *>( *io );
  insert( rc_ptr<AObject>( *io ) );
  ++io;
  d->mesh = static_cast<ATriangulated *>( *io );
  insert( rc_ptr<AObject>( *io ) );
  setReferentialInheritance( d->mesh );
  ++io;
  if( io != eo )
  { 
    d->patch = static_cast<ATexture *>( *io );
    insert( rc_ptr<AObject>( *io ) );
    d->patch->getOrCreatePalette();
    AObjectPalette *pal = d->patch->palette();
    rc_ptr<Texture1d> aimstex = d->patch->texture<float>( false, false );
    string apal = "BLUE-ufusion";
    // check if texture has only one patch (2 values max)
    vector<float> t( 2, 0 );
    vector<int32_t> n(3, 0);
    vector<float>::const_iterator it, 
      et = aimstex->begin()->second.data().end();
    for( it=aimstex->begin()->second.data().begin(); it!=et && n[2] == 0; 
      ++it )
    {
      if( n[0] == 0 || t[0] == *it )
      {
        ++n[0];
        t[0] = *it;
      }
      else if( n[1] == 0 || t[1] == *it )
      {
        ++n[1];
        t[1] = *it;
      }
      else // here's a 3rd value
        ++n[2];
    }
    if( n[2] != 0 )
      // if texture has several values, use a non-binary palette
      apal = "Blue-Red-fusion";
    pal->setRefPalette( theAnatomist->palettes().find( apal ) );
    d->patch->setPalette( *pal );
    d->patch->glSetTexRGBInterpolation( true, 0 );
  }
  else
  {
    d->patch = 0;
    d->patchnum = 0;
  }

  // make a texture to store connectivity data
  rc_ptr<TimeTexture<float> > tex( new TimeTexture<float> );
  size_t nvert = d->mesh->surface()->vertex().size();
  vector<float> & tex0 = (*tex)[0].data();
  tex0.resize( nvert, 0 );
  d->texture = new ATexture;
  d->texture->setTexture( tex );
  d->texture->setName( "ConnectivityTexture" );
//   theAnatomist->registerObject( d->texture, false );
  d->texture->getOrCreatePalette();
  AObjectPalette *pal = d->texture->palette();
  pal->setRefPalette( theAnatomist->palettes().find( "yellow-red-fusion" ) );
  d->texture->setPalette( *pal );
  insert( rc_ptr<AObject>( d->texture ) );

  // make a textured surface
  FusionFactory *ff = FusionFactory::factory();
  AObject *texture = 0;
  if( d->patch ) // with 2 textures
  {
    vector<AObject *> objf( 2 );
    objf[0] = d->patch;
    objf[1] = d->texture;
    d->mtexture = static_cast<AMTexture *>( ff->method( 
      "FusionMultiTextureMethod" )->fusion( objf ) );
    d->mtexture->setName( "PatchAndConnectivity" );
//     theAnatomist->registerObject( d->mtexture, false );
    texture = d->mtexture;
  }
  else // no patch: complete matrix
    texture = d->texture;

  vector<AObject *> objf( 2 );
  objf[0] = d->mesh;
  objf[1] = texture;
  d->texsurface = static_cast<ATexSurface *>( 
    ff->method( "FusionTexSurfMethod" )->fusion( objf ) );
  d->texsurface->setName( "ConnectivityTextureMesh" );
  theAnatomist->registerObject( d->texsurface, false );
  insert( rc_ptr<AObject>( d->texsurface ) );

  // build vertices indices on patch
  buildPatchIndices();
  // take initial vertex
  uint32_t vertex = 0;
  if( !d->patchindices.empty() )
    vertex = d->patchindices[0];
  d->vertex = vertex;

  // build small sphere to point starting vertex
  AimsSurfaceTriangle *sph = SurfaceGenerator::icosahedron( 
    d->mesh->surface()->vertex()[ vertex ], 1.5 );
  d->marker = new ATriangulated;
  d->marker->setSurface( rc_ptr<AimsSurfaceTriangle>( sph ) );
  d->marker->setName( "StartingPoint" );
//   theAnatomist->registerObject( d->marker, false );
  insert( rc_ptr<AObject>( d->marker ) );

  // build connectivity texture contents
  buildTexture( vertex );
}


AConnectivityMatrix::~AConnectivityMatrix()
{
  if( d->texsurface )
    theAnatomist->unregisterObject( d->texsurface );
  delete d;
}


bool AConnectivityMatrix::render( PrimList & plist, const ViewState & vs )
{
  if( d->texsurface )
    d->texsurface->render( plist, vs );
  if( d->marker )
    d->marker->render( plist, vs );
  return true;
}


void AConnectivityMatrix::update( const Observable *observable, void *arg )
{
  if( observable == d->patch )
  {
    buildPatchIndices();
    setChanged();
    notifyObservers( this );
  }
  else if( observable != d->texture )
  {
    setChanged();
    notifyObservers( this );
  }
}


bool AConnectivityMatrix::checkObjects( const set<AObject *> & objects, 
  list<AObject *> & ordered, PatchMode & patchnummode, int & patchnum )
{
  ASparseMatrix *sparse = 0;
  ATriangulated *mesh = 0;
  ATexture *tex = 0;

  set<AObject *>::const_iterator io, eo = objects.end();
  for( io=objects.begin(); io!=eo; ++io )
  {
    if( dynamic_cast<ASparseMatrix *>( *io ) )
    {
      if( sparse )
        return false;
      sparse = static_cast<ASparseMatrix *>( *io );
    }
    else if( dynamic_cast<ATriangulated *>( *io ) )
    {
      if( mesh )
        return false;
      mesh = static_cast<ATriangulated *>( *io );
    }
    else if( dynamic_cast<ATexture *>( *io ) )
    {
      if( tex )
        return false;
      tex = static_cast<ATexture *>( *io );
    }
    else
      return false;
  }

  if( !sparse || !mesh )
    return false;

  unsigned nvert1 = mesh->surface()->vertex().size();
  unsigned nvert2 = nvert1;
  rc_ptr<SparseMatrix> smat = sparse->matrix();
  unsigned texsize = smat->getSize2();
  if( nvert1 != smat->getSize1() )
  {
    if( nvert1 != texsize )
      return false;
    texsize = smat->getSize1();
  }
  patchnummode = ALL_BUT_ONE;
  patchnum = 0;

  if( tex )
  {
    /* texture should have nvert1 elements, with texsize "useable" as a ROI
       patch, or combination of patches (all but one value) */
    if( tex->glTexCoordSize( ViewState() ) != nvert1 )
      return false; // incompatible texture

    rc_ptr<Texture1d> aimstex = tex->texture<float>( true, false );
    if( !aimstex || aimstex->size() == 0 )
      return false;

    // texture histogram
    map<int32_t, int32_t> histo;
    vector<float>::const_iterator it, 
      et = aimstex->begin()->second.data().end();
    int32_t nonzero = 0, t;
    for( it=aimstex->begin()->second.data().begin(); it!=et; ++it )
    {
      t = int32_t( rint( *it ) );
      ++histo[ t ];
      if( t != 0 )
        ++nonzero;
    }
    // try all but 0 (all non-null values) first
    if( nonzero == texsize )
      nvert2 = nonzero;
    else // doesn't work
    {
      // now try one patch value
      map<int32_t, int32_t>::const_iterator ih, eh = histo.end();
      list<int32_t> compatiblepatches;
      cout << "compatible patches: ";
      for( ih=histo.begin(); ih!=eh; ++ih )
        if( ih->second == texsize )
        {
          compatiblepatches.push_back( ih->first );
          cout << ih->first << ", ";
        }
      cout << endl;
      if( !compatiblepatches.empty() )
      {
        patchnummode = ONE;
        patchnum = *compatiblepatches.begin();
        nvert2 = texsize;
      }
      else //no patch has the good size
      {
        // check all values but one
        cout << "compatible compl. patches: ";
        for( ih=histo.begin(); ih!=eh; ++ih )
          if( ih->second == nvert1 - texsize )
          {
            compatiblepatches.push_back( ih->first );
            cout << ih->first << ", ";
          }
        cout << endl;
        if( compatiblepatches.empty() )
          return false; // no matching patch[es]
        patchnummode = ALL_BUT_ONE;
        patchnum = *compatiblepatches.begin();
        nvert2 = texsize;
      }
    }
  }
  else
  {
    if( smat->getSize1() != smat->getSize2() )
      return false;
  }

  cout << "matrix size: " << nvert1 << ", " << nvert2 << endl;

  ordered.push_back( sparse );
  ordered.push_back( mesh );
  if( tex )
    ordered.push_back( tex );

  return true;
}


void AConnectivityMatrix::buildPatchIndices()
{
  cout << "buildPatchIndices\n";
  if( !d->patch )
  {
    // no patch texture: no indices
    d->patchindices.clear();
    return;
  }
  rc_ptr<TimeTexture<float> > tx = d->patch->texture<float>( true, false );
  const vector<float> & tex = (*tx)[0].data();
  vector<float>::const_iterator it, et = tex.end();
  uint32_t i = 0, j = 0;
  d->patchindices.resize( min( d->sparse->matrix()->getSize1(), 
                               d->sparse->matrix()->getSize2() ) );
  cout << "patch size: " << d->patchindices.size() << endl;
  if( d->patchmode == ONE )
  {
    for( it=tex.begin(); it!=et; ++it, ++j )
      if( *it == d->patchnum )
        d->patchindices[ i++ ] = j;
  }
  else // ALL_BUT_ONE
  {
    for( it=tex.begin(); it!=et; ++it, ++j )
      if( *it != d->patchnum )
        d->patchindices[ i++ ] = j;
  }
}


void AConnectivityMatrix::buildTexture( uint32_t startvertex )
{
  cout << "buildTexture\n";
  uint32_t vertex = startvertex;
  cout << "patchindices: " << d->patchindices.size() << endl;
  if( !d->patchindices.empty() )
  {
    // find vertex in indices
    vector<uint32_t>::const_iterator ipi, epi = d->patchindices.end();
    uint32_t i = 0;
    for( ipi=d->patchindices.begin(); ipi!=epi; ++ipi, ++i )
      if( *ipi == startvertex )
      {
        vertex = i;
        break;
      }
    if( ipi == epi )
    {
      // clicked outside patch: show column connections, to the patch
      buildColumnTexture( startvertex );
      return;
    }
  }
  cout << "line: " << vertex << endl;

  d->vertex = startvertex;
  rc_ptr<SparseMatrix> mat = d->sparse->matrix();
  vector<double> row = mat->getRow( vertex );
  rc_ptr<TimeTexture<float> > tex( new TimeTexture<float> );
  vector<float> & tex0 = (*tex)[0].data();
  tex0.insert( tex0.end(), row.begin(), row.end() );
  d->texture->setTexture( tex );

  // update sphere marker
  if( d->marker )
  {
    AimsSurfaceTriangle *sph = SurfaceGenerator::icosahedron( 
      d->mesh->surface()->vertex()[ startvertex ], 1.5 );
    d->marker->setSurface( rc_ptr<AimsSurfaceTriangle>( sph ) );
    Material & mat = d->marker->GetMaterial();
    mat.SetDiffuse( 0.6, 1., 0., 1. );
    d->marker->SetMaterial( mat );
    d->marker->glSetChanged( GLComponent::glMATERIAL );
  }
}


void AConnectivityMatrix::buildColumnTexture( uint32_t startvertex )
{
  cout << "buildColumnTexture\n";
  d->vertex = startvertex;
  rc_ptr<SparseMatrix> mat = d->sparse->matrix();
  vector<double> col = mat->getColumn( startvertex );
  rc_ptr<TimeTexture<float> > tex( new TimeTexture<float> );
  vector<float> & tex0 = (*tex)[0].data();
  tex0.resize( d->mesh->surface()->vertex().size(), 0. );
  vector<uint32_t>::const_iterator ip, ep = d->patchindices.end();
  vector<double>::const_iterator iv, ev = col.end();
  for( ip=d->patchindices.begin(), iv=col.begin(); ip!=ep; ++ip, ++iv )
    tex0[*ip] = *iv;
  d->texture->setTexture( tex );

  // update sphere marker
  if( d->marker )
  {
    AimsSurfaceTriangle *sph = SurfaceGenerator::icosahedron(
      d->mesh->surface()->vertex()[ startvertex ], 1.5 );
    d->marker->setSurface( rc_ptr<AimsSurfaceTriangle>( sph ) );
    Material & mat = d->marker->GetMaterial();
    mat.SetDiffuse( 0., .6, 1., 1. );
    d->marker->SetMaterial( mat );
    d->marker->glSetChanged( GLComponent::glMATERIAL );
  }
}


void AConnectivityMatrix::buildPatchTexture( uint32_t startvertex )
{
  if( !d->patch )
    return;

  // find vertex in patch
  vector<uint32_t>::const_iterator ip, ep = d->patchindices.end();
  uint32_t vertex = 0, n, texsize = d->mesh->surface()->vertex().size(), i;
  for( ip=d->patchindices.begin(); ip!=ep; ++ip, ++vertex )
    if( *ip == startvertex )
      break;
  if( ip == ep )
  {
    // outside patch
    buildColumnPatchTexture( startvertex );
    return;
  }

  d->vertex = startvertex;
  rc_ptr<TimeTexture<int16_t> > tx = d->patch->texture<int16_t>( true, false );
  rc_ptr<TimeTexture<float> > ptex( new TimeTexture<float> );
  TimeTexture<int16_t>::const_iterator it, et = tx->end();
  int timestep;
  int16_t patchval, tval;
  vector<int16_t> pvals;
  pvals.reserve( tx->size() );
  rc_ptr<SparseMatrix> mat = d->sparse->matrix();

  for( it=tx->begin(); it!=et; ++it )
  {
    timestep = it->first;
    const vector<int16_t> & tex = it->second.data();
    patchval = tex[ startvertex ];
    cout << "patch at time " << timestep << ": " << patchval << endl;
    pvals.push_back( patchval );
    vector<uint32_t> patchindices;
    n = d->patchindices.size();
    patchindices.reserve( n ); // sub-patch vertices
    // get sub-patch
    for( vertex=0; vertex<n; ++vertex )
    {
      tval = tex[ d->patchindices[vertex] ];
      if( tval == patchval )
        patchindices.push_back( vertex ); // indices in d->patchindices
    }

    // sum rows in sparse matrix
    vector<float> & ptext = (*ptex)[timestep].data();
    ptext.resize( texsize, 0. );
    for( vertex=0, n=patchindices.size(); vertex<n; ++vertex )
    {
      vector<double> row = mat->getRow( patchindices[vertex] );
      for( i=0; i<texsize; ++i )
        ptext[i] += row[i];
    }
  }

  // store texture with timesteps
  d->texture->setTexture( ptex );

  // mesh patch marker
  if( d->marker )
  {
    // is the patch time-dependent ?
    patchval = pvals[0];
    for( i=1, n=pvals.size(); i<n; ++i )
      if( pvals[i] != patchval )
        break;
    AimsSurfaceTriangle *mesh = 0;
    if( i == n ) // only one patch value
    {
      mesh = SurfaceManip::meshExtract( *d->mesh->surface(), *tx, patchval );
    }
    else
    {
      // build sub-mesh for each timestep
      for( it=tx->begin(), i=0; it!=et; ++it, ++i )
      {
        timestep=it->first;
        cout << "mesh timestep: " << timestep << endl;
        // get patch texture for meshExtract
        TimeTexture<int16_t> tex16;
        tex16[0] = it->second;
        cout << "pval: " << pvals[i] << endl;
        AimsSurfaceTriangle *tmesh = SurfaceManip::meshExtract(
          *d->mesh->surface(), tex16, pvals[i] );
        cout << "vertices: " << tmesh->vertex().size() << endl;
        if( !mesh )
          mesh = tmesh;
        else
        {
          // merge meshes
          (*mesh)[timestep] = (*tmesh)[0];
          delete tmesh;
        }
      }
    }

    // set marker mesh
    d->marker->setSurface( rc_ptr<AimsSurfaceTriangle>( mesh ) );
    Material & mat = d->marker->GetMaterial();
    mat.SetDiffuse( 1., 0.6, 0., 1. );
    d->marker->SetMaterial( mat );
    d->marker->glSetChanged( GLComponent::glMATERIAL );
  }
}


void AConnectivityMatrix::buildColumnPatchTexture( uint32_t startvertex )
{
}


const rc_ptr<ATriangulated> AConnectivityMatrix::mesh() const
{
  return rc_ptr<ATriangulated>( d->mesh );
}


const rc_ptr<ATexture> AConnectivityMatrix::texture() const
{
  return rc_ptr<ATexture>( d->texture );
}


const rc_ptr<ATriangulated> AConnectivityMatrix::marker() const
{
  return rc_ptr<ATriangulated>( d->marker );
}


