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
#include <anatomist/volume/Volume.h>
#include <aims/sparsematrix/sparsematrixutil.h>
#include <aims/mesh/surfacegen.h>
#include <aims/mesh/surfaceOperation.h>
#include <aims/utility/converter_texture.h>
#include <aims/sparsematrix/ciftitools.h>

using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;


struct AConnectivityMatrix::Private
{
  Private();
  ~Private();

  AConnectivityMatrix::PatchMode patchmode;
  set<int> patchnums;
  ASparseMatrix *sparse;
  vector<ATriangulated *> meshes;
  vector<ATexture *> patches;
  vector<ATexture *> basins;
  vector<ATexture *> textures;
  vector<AMTexture *> mtextures;
  vector<ATexSurface *> texsurfaces;
  ATriangulated *marker;
  // index in mesh -> index in matrix table
  vector<map<uint32_t, uint32_t> > patchindices;
//   uint32_t vertex;
  double connectivity_max;
};


AConnectivityMatrix::Private::Private()
  : patchmode( AConnectivityMatrix::ALL_BUT_ONE ),
  sparse( 0 ), marker( 0 ), connectivity_max( 0 )
{
}


AConnectivityMatrix::Private::~Private()
{
  vector<ATexSurface *>::iterator its, ets = texsurfaces.end();
  for( its=texsurfaces.begin(); its!=ets; ++its )
    delete *its;
  vector<AMTexture *>::iterator imt, emt = mtextures.end();
  for( imt=mtextures.begin(); imt!=emt; ++imt )
    delete *imt;
  vector<ATexture *>::iterator it, et = textures.end();
  for( it=textures.begin(); it!=et; ++it )
    delete *it;
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


  ASparseMatrix* matrixFromVolume( AVolume<float>* vol,
                                   bool transpose = false )
  {
    VolumeRef<float> aimsvol = vol->volume();
    VolumeRef<double> dvol;
    // transpose volume and
    if( transpose )
    {
      dvol = new Volume<double>( aimsvol->getSizeY(), aimsvol->getSizeX() );
      unsigned i, j, nx = aimsvol->getSizeX(), ny = aimsvol->getSizeY();
      for( j=0; j<ny; ++j )
        for( i=0; i<nx; ++i )
          dvol->at( j, i ) = aimsvol->at( i, j );
    }
    else
    {
      dvol = new Volume<double>( aimsvol->getSizeX(), aimsvol->getSizeY() );
      unsigned i, j, nx = aimsvol->getSizeX(), ny = aimsvol->getSizeY();
      for( j=0; j<ny; ++j )
        for( i=0; i<nx; ++i )
          dvol->at( i, j ) = aimsvol->at( i, j );
    }
    SparseOrDenseMatrix *sdmat = new SparseOrDenseMatrix;
    sdmat->setMatrix( dvol );
    ASparseMatrix *sparse = new ASparseMatrix;
    sparse->setMatrix( rc_ptr<SparseOrDenseMatrix>( sdmat ) );
    sparse->setName( "Matrix_" + vol->name() );
    theAnatomist->registerObject( sparse, false );
    return sparse;
  }


  ASparseMatrix* matrixFromVolume( AVolume<double>* vol,
                                   bool transpose = false )
  {
    VolumeRef<double> aimsvol = vol->volume();
    VolumeRef<double> dvol;
    // transpose volume and
    if( transpose )
    {
      dvol = new Volume<double>( aimsvol->getSizeY(), aimsvol->getSizeX() );
      unsigned i, j, nx = aimsvol->getSizeX(), ny = aimsvol->getSizeY();
      for( j=0; j<ny; ++j )
        for( i=0; i<nx; ++i )
          dvol->at( j, i ) = aimsvol->at( i, j );
    }
    else
      dvol = aimsvol;
    SparseOrDenseMatrix *sdmat = new SparseOrDenseMatrix;
    sdmat->setMatrix( dvol );
    ASparseMatrix *sparse = new ASparseMatrix;
    sparse->setMatrix( rc_ptr<SparseOrDenseMatrix>( sdmat ) );
    sparse->setName( "Matrix_" + vol->name() );
    theAnatomist->registerObject( sparse, false );
    return sparse;
  }

}


// ---

AConnectivityMatrix::AConnectivityMatrix( const vector<AObject *> & obj )
  : ObjectVector(), d( new Private )
{
  _type = connectivityMatrixType();

  set<AObject *> sobj;
  sobj.insert( obj.begin(), obj.end() );
  AObject *matrix = 0;
  list<ATriangulated *> meshes;
  list<ATexture *> patch_textures, basin_textures;

  if( !checkObjects( sobj, matrix, meshes, patch_textures, basin_textures,
                     d->patchmode, d->patchnums ) )
  {
    cerr << "AConnectivityMatrix: inconsistency in fusion objects types\n";
    return;
  }

  // matrix handling / transformation
  bool builtnewmatrix = false;
  if( dynamic_cast<ASparseMatrix *>( matrix ) )
  {
    d->sparse = static_cast<ASparseMatrix *>( matrix );
  }
  else if( dynamic_cast<AVolume<float> *>( matrix ) )
  {
    AVolume<float> *vol = static_cast<AVolume<float> *>( matrix );
    bool transpose = false;
    /* TODO: check matrix size for transposition
    if( d->basins )
    {
    }
    */
    d->sparse = matrixFromVolume( vol, transpose );
    builtnewmatrix = true;
  }
  else
  {
    AVolume<double> *vol = static_cast<AVolume<double> *>( matrix );
    // TODO: check matrix size for transposition
    d->sparse = matrixFromVolume( vol );
    builtnewmatrix = true;
  }

  // matrix transformation / transposition delayed after textures analysis
  d->meshes.insert( d->meshes.end(), meshes.begin(), meshes.end() );
  setReferentialInheritance( d->meshes[0] );
  if( !patch_textures.empty() )
  {
    d->patches.insert( d->patches.end(), patch_textures.begin(),
                       patch_textures.end() );
    d->patches[0]->getOrCreatePalette();
    AObjectPalette *pal = d->patches[0]->palette();
    rc_ptr<Texture1d> aimstex = d->patches[0]->texture<float>( false, false );
    GLComponent::TexExtrema & text = d->patches[0]->glTexExtrema( 0 );
    unsigned nval
      = unsigned( rint( text.maxquant[0] - text.minquant[0] ) ) + 1;
    rc_ptr<APalette> apal( new APalette( "batch_bin", nval ) );
    unsigned i, minpatch = unsigned( rint( text.minquant[0] ) );
    if( d->patchmode == ONE )
      for( i=0; i<nval; ++i )
        if( d->patchnums.find( i + minpatch ) != d->patchnums.end() )
          (*apal)( i ) = AimsRGBA( 200, 200, 255, 255 );
        else
          (*apal)( i ) = AimsRGBA( 255, 255, 255, 255 );
    else
      for( i=0; i<nval; ++i )
        if( d->patchnums.find( i + minpatch ) == d->patchnums.end() )
          (*apal)( i ) = AimsRGBA( 200, 200, 255, 255 );
        else
          (*apal)( i ) = AimsRGBA( 255, 255, 255, 255 );
    pal->setRefPalette( apal );
    d->patches[0]->setPalette( *pal );
    d->patches[0]->glSetTexRGBInterpolation( true, 0 );
  }
  else
  {
    d->patches.clear();
    d->patchnums.clear();
  }
  if( !basin_textures.empty() )
  {
    // 2nd texture for matrix reduction along basins
    d->basins.insert( d->basins.begin(), basin_textures.begin(),
                      basin_textures.end() );
  }

  // make a texture to store connectivity data
  unsigned count = 0;
  vector<ATriangulated *>::iterator im, em = d->meshes.end();
  for( im=d->meshes.begin(); im!=em; ++im, ++count )
  {
    rc_ptr<TimeTexture<float> > tex( new TimeTexture<float> );
    size_t nvert = (*im)->surface()->vertex().size();
    vector<float> & tex0 = (*tex)[0].data();
    tex0.resize( nvert, 0 );
    ATexture *texture = new ATexture;
    d->textures.push_back( texture );
    texture->setTexture( tex );
    stringstream name;
    name << "ConnectivityTexture_" << count;
    texture->setName( name.str() );
    theAnatomist->registerObject( texture, false );
    texture->getOrCreatePalette();
    AObjectPalette *pal = texture->palette();
    pal->setRefPalette( theAnatomist->palettes().find( "yellow-red-fusion" ) );
    texture->setPalette( *pal );
  }

  // make a textured surface
  FusionFactory *ff = FusionFactory::factory();
  vector<AObject *> textures;
  if( !d->patches.empty() ) // with 2 textures
  {
    size_t ntex = std::min( d->patches.size(), d->textures.size() );
    textures.reserve( ntex );
    vector<ATexture *>::iterator ipt, it, ept = d->patches.end(),
      et = d->textures.end();
    for( ipt=d->patches.begin(), it=d->textures.begin(), count=0;
        it!=et && ipt!=ept; ++it, ++ipt, ++count )
    {
      vector<AObject *> objf( 2 );
      objf[0] = *ipt;
      objf[1] = *it;
      AMTexture *mtexture = static_cast<AMTexture *>( ff->method(
        "FusionMultiTextureMethod" )->fusion( objf ) );
      d->mtextures.push_back( mtexture );
      stringstream name;
      name << "PatchAndConnectivity_" << count;
      mtexture->setName( name.str() );
      theAnatomist->registerObject( mtexture, false );
      textures.push_back( mtexture );
    }
  }
  else // no patch: complete matrix
    textures.insert( textures.end(), d->textures.begin(), d->textures.end() );

  // textured surfaces fusions
  vector<AObject *>::iterator iot, eot = textures.end();
  for( im=d->meshes.begin(), iot=textures.begin(), count=0; im!=em && iot!=eot;
      ++im, ++iot, ++count )
  {
    vector<AObject *> objf( 2 );
    objf[0] = *im;
    objf[1] = *iot;
    ATexSurface *texsurface = static_cast<ATexSurface *>(
      ff->method( "FusionTexSurfMethod" )->fusion( objf ) );
    d->texsurfaces.push_back( texsurface );
    stringstream name;
    name << "ConnectivityTextureMesh_" << count;
    texsurface->setName( name.str() );
    theAnatomist->registerObject( texsurface, false );
    if( !d->mtextures.empty() )
      theAnatomist->releaseObject( *iot );
  }

  // build vertices indices on patch
  buildPatchIndices();
  // take initial vertex
  uint32_t vertex = 0;
  if( !d->patchindices.empty() && !d->patchindices[0].empty() )
    vertex = d->patchindices[0].begin()->first;
//   d->vertex = vertex;

  // build small sphere to point starting vertex
  AimsSurfaceTriangle *sph = SurfaceGenerator::icosahedron( 
    d->meshes[0]->surface()->vertex()[ vertex ], 1.5 );
  d->marker = new ATriangulated;
  d->marker->setSurface( rc_ptr<AimsSurfaceTriangle>( sph ) );
  d->marker->setName( "StartingPoint" );
  d->marker->setReferentialInheritance( d->meshes[0] );
  theAnatomist->registerObject( d->marker, false );

  // insert in MObject
  insert( rc_ptr<AObject>( d->sparse ) );
  insert( rc_ptr<AObject>( d->meshes[0] ) );
  if( !d->patches.empty() )
  {
    insert( rc_ptr<AObject>( d->patches[0] ) );
    if( !d->basins.empty() )
      insert( rc_ptr<AObject>( d->basins[0] ) );
  }
  /* if the matrix object has been created here, release it (remove from
     the main GUI) now, after it is inserted in the MObject.
  */
  if( builtnewmatrix )
    theAnatomist->releaseObject( d->sparse );
  vector<ATexture *>::iterator it, et = d->textures.end();
  for( it=d->textures.begin(), et; it!=et; ++it )
  {
    insert( rc_ptr<AObject>( *it ) );
    theAnatomist->releaseObject( *it );
  }
  vector<ATexSurface *>::iterator its, ets = d->texsurfaces.end();
  for( its=d->texsurfaces.begin(); its!=ets; ++its )
  {
    insert( rc_ptr<AObject>( *its ) );
    theAnatomist->releaseObject( *its );
  }
  insert( rc_ptr<AObject>( d->marker ) );
  theAnatomist->releaseObject( d->marker );

  d->connectivity_max = SparseMatrixUtil::max( *d->sparse->matrix() );
  if( d->connectivity_max == 0. )
    d->connectivity_max = 1.;

  // build connectivity texture contents
  buildTexture( 0, vertex );
}


AConnectivityMatrix::~AConnectivityMatrix()
{
//   if( d->texsurface )
//     theAnatomist->unregisterObject( d->texsurface );
  delete d;
}


bool AConnectivityMatrix::render( PrimList & plist, const ViewState & vs )
{
  if( d->marker )
    d->marker->render( plist, vs );
  vector<ATexSurface *>::iterator its, ets = d->texsurfaces.end();
  for( its=d->texsurfaces.begin(); its!=ets; ++its )
    (*its)->render( plist, vs );
  return true;
}


void AConnectivityMatrix::update( const Observable *observable, void *arg )
{
  if( !d->patches.empty() && observable == d->patches[0] )
  {
    buildPatchIndices();
    setChanged();
    notifyObservers( this );
  }
  else if( observable != d->textures[0] )
  {
    setChanged();
    notifyObservers( this );
  }
}


namespace
{

  bool checkTextureAsBasins( const Texture1d & tex, GenericObject* header,
                             size_t nvert, size_t ncol )
  {
    bool canbebasins = false;
    string dtype;
    if( header->getProperty( "data_type", dtype )
        && ( dtype != "S32" && dtype != "S16" ) )
      return false;
    if( tex.size() != 1 )
      return false;

    // check for basins labels
    set<int> uniquevals;
    const vector<float> & tex0 = tex.begin()->second.data();
    if( tex0.size() != nvert )
      return false; // doesn't match mesh size
    vector<float>::const_iterator it, et = tex0.end();
    for( it=tex0.begin(); it!=et; ++it )
    {
      if( uniquevals.size() >= 100000 )
        // too many labels: obviously not basins
        return false;
      uniquevals.insert( int( rint( *it ) ) );
    }
    uniquevals.erase( 0 ); // remove background
    if( uniquevals.size() == ncol ) // match matrix columns
      canbebasins = true;

    return canbebasins;
  }


  bool checkTextureAsConnectivity(
    const Texture1d & tex, size_t nvert, size_t nlines,
    AConnectivityMatrix::PatchMode & patchnummode, set<int> & patchnum,
    Object patchlabels, Object & patchintlabels, Object texheader )
  {
    bool canbeconn = false;
    patchnummode = AConnectivityMatrix::ALL_BUT_ONE;
    patchnum.clear();
    set<int> patches;

    if( patchlabels.get() )
    {
      /* patchlabels provides a list of (string) labels to be used as initial
         region.
      */
      try
      {
        Object labels_table = texheader->getProperty( "GIFTI_labels_table" );
        cout << "GIFTI_labels_table found\n";
        set<string> labels_set;

        Object it = patchlabels->objectIterator();
        try
        {
          for( ; it->isValid(); it->next() )
            labels_set.insert( it->currentValue()->getString() );
        }
        catch( ... )
        {
        }

        for( it=labels_table->objectIterator(); it->isValid(); it->next() )
        {
          try
          {
            string label = it->currentValue()->getString();
            if( labels_set.find( label ) != labels_set.end() )
              patches.insert( it->intKey() );
          }
          catch( ... )
          {
          }
        }
      }
      catch( ... )
      {
      }
    }

    if( patches.empty() && patchintlabels.get() )
    {
      /* patchintlabels provides a list of (int) labels to be used as initial
         region.
      */
      Object iter = patchintlabels->objectIterator();
      for( ; iter->isValid(); iter->next() )
      {
        try
        {
          patches.insert( int( rint(
            iter->currentValue()->getScalar() ) ) );
        }
        catch( ... )
        {
        }
      }
    }

    // texture histogram
    map<int32_t, size_t> histo;
    vector<float>::const_iterator it,
      et = tex.begin()->second.data().end();
    int32_t t;
    size_t nonzero = 0;
    for( it=tex.begin()->second.data().begin(); it!=et; ++it )
    {
      t = int32_t( rint( *it ) );
      ++histo[ t ];
      if( t != 0 )
        ++nonzero;
    }

    if( !patches.empty() )
    {
      set<int>::const_iterator i, n = patches.end();
      size_t sum = 0;
      for( i=patches.begin(); i!=n; ++i )
      {
        sum += histo[*i];
      }
      if( sum == nlines )
      {
        canbeconn = true;
        patchnum = patches;
        patchnummode = AConnectivityMatrix::ONE;
      }
    }

    // try all but 0 (all non-null values) first
    if( !canbeconn && nonzero == nlines )
      canbeconn = true;
    else if( !canbeconn ) // doesn't work
    {
      // now try one patch value
      map<int32_t, size_t>::const_iterator ih, eh = histo.end();
      list<int32_t> compatiblepatches;
      cout << "compatible patches: ";
      for( ih=histo.begin(); ih!=eh; ++ih )
        if( ih->second == nlines )
        {
          compatiblepatches.push_back( ih->first );
          cout << ih->first << ", ";
        }
      cout << endl;
      if( !compatiblepatches.empty() )
      {
        patchnummode = AConnectivityMatrix::ONE;
        patchnum.insert( *compatiblepatches.begin() );
        canbeconn = true;
      }
      else //no patch has the good size
      {
        // check all values but one
        cout << "compatible compl. patches: ";
        for( ih=histo.begin(); ih!=eh; ++ih )
          if( ih->second == nvert - nlines )
          {
            compatiblepatches.push_back( ih->first );
            cout << ih->first << ", ";
          }
        cout << endl;
        if( compatiblepatches.empty() )
          return false; // no matching patch[es]
        patchnummode = AConnectivityMatrix::ALL_BUT_ONE;
        patchnum.insert( *compatiblepatches.begin() );
        canbeconn = true;
      }
    }

    return canbeconn;
  }


  list<string> get_cifti_mesh_ids( rc_ptr<SparseOrDenseMatrix> mat,
                                   int dim = 1 )
  {
    list<string> mesh_ids;
    CiftiTools ctools( mat );
    list<string> bstruct = ctools.getBrainStructures( dim, true, false );
    list<string>::iterator ibs, ebs = bstruct.end();
    const CiftiTools::BrainStuctureToMeshMap & meshmap
      = ctools.brainStructureMap();
    CiftiTools::BrainStuctureToMeshMap::const_iterator imm,
      emm = meshmap.end();

    for( ibs=bstruct.begin(); ibs!=ebs; ++ibs )
    {
      imm = meshmap.find( *ibs );
      if( imm != emm )
      {
        while( mesh_ids.size() <= imm->second )
          mesh_ids.push_back( "" );
        list<string>::iterator is = mesh_ids.begin();
        for( int i=0; i<imm->second; ++i )
          ++is;
        *is = imm->first;
      }
    }

    if( mesh_ids.empty() )
      mesh_ids.push_back( "CORTEX" );
    return mesh_ids;
  }


  list<pair<vector<int>, size_t> > get_cifti_mesh_cols(
    rc_ptr<SparseOrDenseMatrix> mat, const list<string> mesh_ids )
  {
    list<pair<vector<int>, size_t> > cols;
    CiftiTools ctools( mat );
    list<string> tmp_mesh_ids;
    const list<string> *pmesh_ids = &mesh_ids;
    const CiftiTools::BrainStuctureToMeshMap & meshmap
      = ctools.brainStructureMap();
    CiftiTools::BrainStuctureToMeshMap::const_iterator imm,
      emm = meshmap.end();
    try
    {
      if( ctools.dimensionType( 1 ) == "brain_models" )
      {
        if( mesh_ids.empty() )
        {
          tmp_mesh_ids = get_cifti_mesh_ids( mat, 1 );
          pmesh_ids = &tmp_mesh_ids;
        }
        list<string>::const_iterator im, em=pmesh_ids->end();

        for( im=pmesh_ids->begin(); im!=em; ++im )
        {
          vector<int> scols = ctools.getIndicesForBrainStructure( 1, *im );
          imm = meshmap.find( *im );
          int index = 0;
          if( imm != emm )
            index = imm->second;
          while( cols.size() <= index )
            cols.push_back( make_pair( vector<int>(), 0 ) );
          list<pair<vector<int>, size_t> >::iterator icv = cols.begin();
          for( int i=0; i<index; ++i )
            ++icv;
          icv->first.insert( icv->first.end(), scols.begin(), scols.end() );
          icv->second = ctools.getBrainStructureMeshNumberOfNodes( 1, *im );
        }
      }
    }
    catch( ... )
    {
    }
    return cols;
  }


  bool check_meshes( rc_ptr<SparseOrDenseMatrix> mat,
                     const list<string> & mesh_ids,
                     list<ATriangulated *> & meshes, list<ATexture *> & tex,
                     list<pair<vector<int>, size_t> > & cols )
  {
    cout << "check_meshes " << mesh_ids.size() << ", " << meshes.size() << ", " << tex.size() << endl;
    if( mesh_ids.size() != meshes.size() )
      return false;
    if( tex.size() < mesh_ids.size() )
      return false;

    CiftiTools ctools( mat );
    cols = get_cifti_mesh_cols( mat, mesh_ids );
    list<ATriangulated *> ordered_meshes;
    list<ATriangulated *>::iterator im, em = meshes.end(), bm;
    list<ATexture *> ordered_tex;
    list<ATexture *>::iterator it, et = tex.end(), bt;
//     set<ATexture *> done_tex;
    ViewState vs;

    // find / order matching meshes
    for( im=meshes.begin(); im!=em; ++im )
    {
      list<string> structs;
      int mesh_num, i;
      if( !ctools.isMatchingSurface( 0, *(*im)->surface(), structs,
                                     mesh_num ) )
      { cout << "unmatching mesh on dim 0\n";
        return false;
      }
      if( !ctools.isMatchingSurface( 1, *(*im)->surface(), structs,
                                     mesh_num ) )
      { cout << "unmatching mesh on dim 1\n";
        return false;
      }
      while( ordered_meshes.size() <= mesh_num )
        ordered_meshes.push_back( 0 );
      for( bm=ordered_meshes.begin(), i=0; i<mesh_num; ++i )
        ++bm;
      if( *bm )
      {
        // already found
        cout << "mesh already found\n";
        return false;
      }
      *bm = *im;
    }

    for( it=tex.begin(); it!=et; ++it )
    {
      list<string> structs;
      int mesh_num, i;
      size_t nv = (*it)->glTexCoordSize( vs, 0 );
      if( !ctools.isMatchingSurfaceOrTexture( 0, nv, (*it)->attributed(),
                                              structs, mesh_num ) )
      { cout << "unmatching texture\n";
        return false;
      }
      while( ordered_tex.size() <= mesh_num )
        ordered_tex.push_back( 0 );
      for( bt=ordered_tex.begin(), i=0; i<mesh_num; ++i )
        ++bt;
      if( *bt )
      { cout << "tex already found\n";
        // already found
        return false;
      }
      *bt = *it;
//       done_tex.insert( *it );
    }
    for( bt=ordered_tex.begin(), et=ordered_tex.end(); bt!=et; ++bt )
      if( !*bt )
      { cout << "missing texture\n";
        return false;
      }

    meshes = ordered_meshes;
    tex = ordered_tex;
    return true;
  }

}


bool AConnectivityMatrix::checkObjects( const set<AObject *> & objects,
                                        AObject * & matrix,
                                        list<ATriangulated *> & meshes,
                                        list<ATexture *> & patch_textures,
                                        list<ATexture *> & basin_textures,
                                        PatchMode & patchnummode,
                                        set<int> & patchnums )
{
  ASparseMatrix *sparse = 0;
  AVolume<float> *dense = 0;
  AVolume<double> *ddense = 0;
  list<ATriangulated *> mesh;
  list<ATexture *> tex_list;
  ATexture *tex = 0, *tex2 = 0;
  cout << "checkObjects\n";

  set<AObject *>::const_iterator io, eo = objects.end();
  for( io=objects.begin(); io!=eo; ++io )
  {
    if( dynamic_cast<ASparseMatrix *>( *io ) )
    {
      if( sparse || dense || ddense )
        return false;
      sparse = static_cast<ASparseMatrix *>( *io );
    }
    else if( dynamic_cast<AVolume<float> *>( *io ) )
    {
      if( sparse || dense || ddense )
        return false;
      dense = static_cast<AVolume<float> *>( *io );
    }
    else if( dynamic_cast<AVolume<double> *>( *io ) )
    {
      if( sparse || dense || ddense )
        return false;
      ddense = static_cast<AVolume<double> *>( *io );
    }
    else if( dynamic_cast<ATriangulated *>( *io ) )
    {
//       if( !mesh.empty() )
//         // a mesh should be the only one unless they are fusionned with
//         // textures
//         return false;
      mesh.push_back( static_cast<ATriangulated *>( *io ) );
    }
    else if( dynamic_cast<ATexture *>( *io ) )
    {
      if( tex )
      {
//         if( tex2 )
//           return false;
        tex2 = static_cast<ATexture *>( *io );
      }
      else
        tex = static_cast<ATexture *>( *io );
      tex_list.push_back( static_cast<ATexture *>( *io ) );
    }
    else
      return false;
  }

  if( ( !sparse && !dense && !ddense ) || mesh.empty() )
    return false;

  list<unsigned> nvert_list;
  unsigned nvert = 0;
  list<ATriangulated *>::iterator imesh, emesh = mesh.end();
  for( imesh=mesh.begin(); imesh!=emesh; ++imesh )
  {
    unsigned n = (*imesh)->surface()->vertex().size();
    nvert_list.push_back( n );
    nvert += n;
  }
  unsigned lines, texsize;
  list<unsigned> texsize_list;
  Object patchlabels;
  Object patchintlabels;
  list<string> cifti_meshes_ids;
  if( sparse )
  {
    rc_ptr<SparseOrDenseMatrix> smat = sparse->matrix();
    texsize = smat->getSize2();
    lines = smat->getSize1();
    try
    {
      patchlabels = smat->header()->getProperty( "row_labels" );
    }
    catch( ... )
    {
    }
    try
    {
      patchintlabels = smat->header()->getProperty( "row_int_labels" );
    }
    catch( ... )
    {
    }
    cifti_meshes_ids = get_cifti_mesh_ids( smat );
    list<pair<vector<int>, size_t> > cols;
    if( !check_meshes( smat, cifti_meshes_ids, mesh, tex_list, cols ) )
    {
      cout << "check_meshes is false.\n";
    }
    cout << "check_meshes OK\n";
  }
  else if( dense )
  {
    texsize = dense->volume()->getSizeX();
    lines = dense->volume()->getSizeY();
    try
    {
      patchlabels = dense->attributed()->getProperty( "row_labels" );
    }
    catch( ... )
    {
    }
    try
    {
      patchintlabels = dense->attributed()->getProperty( "row_int_labels" );
    }
    catch( ... )
    {
    }
  }
  else
  {
    texsize = ddense->volume()->getSizeX();
    lines = ddense->volume()->getSizeY();
    try
    {
      patchlabels = ddense->attributed()->getProperty( "row_labels" );
    }
    catch( ... )
    {
    }
    try
    {
      patchintlabels = ddense->attributed()->getProperty( "row_int_labels" );
    }
    catch( ... )
    {
    }
  }

  cout << "matrix size: " << lines << ", " << texsize << endl;
  cout << "mesh vertices: " << nvert << endl;
  if( tex2 )
    cout << "using 2 textures\n";

  if( tex )
  {
    /* texture should have nvert elements, with texsize "useable" as a ROI
       patch, or combination of patches (all but one value) */
    if( tex->glTexCoordSize( ViewState() ) != nvert )
      return false; // incompatible texture
    rc_ptr<Texture1d> aimstex = tex->texture<float>( true, false );
    if( !aimstex || aimstex->size() == 0 )
      return false;

    if( !tex2 )
    {
      if( nvert != texsize )
      {
        cout << "matrix columns do not correspond to mesh vertices\n";
        return false;
      }
      if( !checkTextureAsConnectivity( *aimstex, nvert, lines,
                                       patchnummode, patchnums,
                                       patchlabels, patchintlabels,
                                       tex->attributed() ) )
      {
        cout << "incompatible texture\n";
        return false;
      }
    }
    else
    {
      // 2nd texture for reduced matrix
      cout << "reduced connectivity matrix\n";
      if( tex2->glTexCoordSize( ViewState() ) != nvert )
        return false; // incompatible texture
      rc_ptr<Texture1d> aimstex2;
      aimstex2 = tex2->texture<float>( true, false );
      if( !aimstex2 || aimstex2->size() == 0 )
        return false;

      bool t1asbasins = checkTextureAsBasins( *aimstex, tex->attributed(),
                                              nvert, texsize );
      bool t2asbasins = checkTextureAsBasins( *aimstex2, tex2->attributed(),
                                              nvert, texsize );
      if( !t1asbasins && !t2asbasins )
      {
        cout << "no texture can be used for basins reduction\n";
        return false;
      }
      AConnectivityMatrix::PatchMode patchmode1, patchmode2;
      set<int> patchnum1, patchnum2;

      bool t1asconn = checkTextureAsConnectivity( *aimstex, nvert, lines,
                                                  patchmode1, patchnum1,
                                                  patchlabels,
                                                  patchintlabels,
                                                  tex->attributed() );
      bool t2asconn = checkTextureAsConnectivity( *aimstex2, nvert, lines,
                                                  patchmode2, patchnum2,
                                                  patchlabels,
                                                  patchintlabels,
                                                  tex2->attributed() );
      //cout << "t1asconn: " << t1asconn << ", t2asconn: " << t2asconn << endl;
      if( t1asconn && t2asbasins )
      {
        patchnummode = patchmode1;
        patchnums = patchnum1;
      }
      else if( t1asbasins && t2asconn )
      {
        // swap textures
        ATexture * ttmp = tex;
        tex = tex2;
        tex2 = ttmp;
        patchnummode = patchmode2;
        patchnums = patchnum2;
      }
      else
        return false;
    }
  }
  else
  {
    if( lines != texsize && nvert != lines )
      return false;
  }

  if( sparse )
    matrix = sparse;
  else if( dense )
    matrix = dense;
  else
    matrix = ddense;
  meshes = mesh;
  if( tex )
    patch_textures.push_back( tex );
  if( tex2 )
    basin_textures.push_back( tex2 );

  cout << "OK for connectivity matrix fusion.\n";
  return true;
}


void AConnectivityMatrix::buildPatchIndices()
{
  d->patchindices.clear();

  CiftiTools ctools( d->sparse->matrix() );

  if( d->patches.empty() )
  {
    // no patch texture: no indices

    CiftiTools::RoiTextureList *roitex = ctools.roiTextureFromDimension( 0 );
    CiftiTools::RoiTextureList::iterator irt, ert = roitex->end();
    int index = 0, i;

    for( irt=roitex->begin(); irt!=ert; ++irt, ++index )
    {
      const vector<int32_t> & vtex = (**irt)[0].data();
      size_t i, n = vtex.size();
      vector<int> indices_vec;

      for( i=0; i<n; ++i )
        if( vtex[i] != 0 )
          indices_vec.push_back( i );

      vector<int> mat_ind = ctools.getIndicesForSurfaceIndices( 0, index,
                                                                indices_vec );
      d->patchindices.push_back( map<uint32_t, uint32_t>() );
      map<uint32_t, uint32_t> & pind = *d->patchindices.rbegin();
      for( i=0; i<n; ++i )
        pind[ indices_vec[i] ] = mat_ind[i];
    }

    delete roitex;
    return;
  }

  vector<ATexture *>::iterator ipt, ept = d->patches.end();
  d->patchindices.reserve( d->patches.size() );
  int surf_index = 0;
  for( ipt=d->patches.begin(); ipt!=ept; ++ipt, ++surf_index )
  {
    rc_ptr<TimeTexture<float> > tx = (*ipt)->texture<float>( false, false );
    const vector<float> & tex = (*tx)[0].data();
    vector<float>::const_iterator it, et = tex.end();
    uint32_t i = 0, j = 0;
    d->patchindices.push_back( map<uint32_t, uint32_t>() );
    map<uint32_t, uint32_t> & patchindices = *d->patchindices.rbegin();
    // FIXME: use cifti cols info
    const GLComponent::TexExtrema & te = (*ipt)->glTexExtrema( 0 );
    float scale = 1., offset = 0.;
    if( te.scaled && te.max[0] != te.min[0] )
    {
      scale = ( te.maxquant[0] - te.minquant[0] ) / ( te.max[0] - te.min[0] );
      offset = te.minquant[0] - te.min[0] * scale;
    }
    vector<int> indices_vec;
    if( d->patchmode == ONE )
    {
      for( it=tex.begin(); it!=et; ++it, ++j )
        if( d->patchnums.find( int( rint( *it * scale + offset ) ) )
            != d->patchnums.end() )
          indices_vec.push_back( j );
//           patchindices[j] = i++;
    }
    else // ALL_BUT_ONE
    {
      for( it=tex.begin(); it!=et; ++it, ++j )
        if( d->patchnums.find( ( *it * scale + offset ) )
            == d->patchnums.end() )
          indices_vec.push_back( j );
//           patchindices[ j ] = i++;
    }
    vector<int> mat_ind = ctools.getIndicesForSurfaceIndices( 0, surf_index,
                                                              indices_vec );
    size_t n = mat_ind.size();
    for( i=0; i<n; ++i )
      patchindices[ indices_vec[i] ] = mat_ind[i];
  }
}


void AConnectivityMatrix::buildTexture( int mesh_index, uint32_t startvertex,
                                        float time_pos )
{
  uint32_t row = startvertex;
  if( !d->patchindices.empty() )
  {
    // find vertex in indices
    map<uint32_t, uint32_t>::const_iterator
      ipi = d->patchindices[ mesh_index ].find( startvertex );
    if( ipi == d->patchindices[ mesh_index ].end() )
    {
      // clicked outside patch: show column connections, to the patch
      buildColumnTexture( mesh_index, startvertex, time_pos );
      return;
    }
    // startvertex found in patch: regular display by line
    row = ipi->second;
  }

  rc_ptr<SparseOrDenseMatrix> mat = d->sparse->matrix();
  CiftiTools::TextureList texlist;
  CiftiTools ctools( mat );
  vector<int> pos( 2, 0 );
  pos[0] = row;
  ctools.expandedValueTextureFromDimension( 1, pos, &texlist );
  float texmax = -numeric_limits<float>::max(),
    texmin = 0;
  float colscale = 0.F, colscalemin = 0.F;

  vector<float> tmax( d->meshes.size(), -numeric_limits<float>::max() );
  vector<float> tmin( d->meshes.size(), numeric_limits<float>::max() );

  vector<ATriangulated *>::iterator im, em = d->meshes.end();
  unsigned index = 0;
  for( im=d->meshes.begin(); im!=em; ++im, ++index )
  {
    vector<float> & row = (*texlist[index])[0].data();
    rc_ptr<TimeTexture<float> > tex( new TimeTexture<float> );
    vector<float> & tex0 = (*tex)[0].data();
    const AimsSurface<3, Void> *surf = (*im)->surfaceOfTime( time_pos );

    if( d->basins.empty() )
    {
      unsigned i = 0, n = row.size();
      tex0.resize( row.size() );
      for( i=0; i<n; ++ i )
      {
        float x = row[i];
        if( isinf( x ) )
          x = 0.;
        if( x > tmax[index] )
          tmax[index] = x;
        if( x < tmin[index] )
          tmin[index] = x;
        tex0[i] = x;
      }
//       texmax = 1.F;
//       colscale = 1.F;
    }
    else
    {
      // expand matrix to basins
      tex0.insert( tex0.end(), surf->vertex().size(), 0 );
      // FIXME needs to cache this int texture
      rc_ptr<Texture1d> basintex = d->basins[index]->texture<float>( true,
                                                                     false );
      const vector<float> & basinv = basintex->begin()->second.data();
      vector<float>::const_iterator ib, eb = basinv.end();
      vector<float>::iterator ic = tex0.begin();
      int basin;

      for( ib=basinv.begin(); ib!=eb; ++ib, ++ic )
      {
        basin = int( rint( *ib ) );
        if( *ib != 0 )
        {
          *ic = row[ *ib - 1 ];
          if( isinf( *ic ) )
            *ic = 0.;
          if( *ic > tmax[index] )
            tmax[index] = *ic;
          if( *ic < tmin[index] )
            tmin[index] = *ic;
        }
      }
    }

    d->textures[index]->setTexture( tex );
    if( tmax[index] > texmax )
      texmax = tmax[index];
    if( tmin[index] < texmin )
      texmin = tmin[index];
  }

  vector<ATexture *>::iterator it, et = d->textures.end();
  for( it=d->textures.begin(), index=0; it!=et; ++it, ++index )
  {
    if( texmax == -numeric_limits<float>::max() )
    {
      colscale = 1.F;
      colscalemin = 0.F;
    }
    else
    {
      colscale = ( texmax - tmin[index] ) / ( tmax[index] - tmin[index] );
      colscalemin = -tmin[index] / ( tmax[index] - tmin[index] );
    }
    (*it)->palette()->setMax1( colscale );
    (*it)->palette()->setMin1( colscalemin );
    (*it)->glSetTexImageChanged();
  }

  // update sphere marker
  if( d->marker )
  {
    AimsSurfaceTriangle *sph = SurfaceGenerator::icosahedron(
      d->meshes[ mesh_index ]->surfaceOfTime( time_pos )->vertex()
        [ startvertex ], 1.5 );
    d->marker->setSurface( rc_ptr<AimsSurfaceTriangle>( sph ) );
    Material & mat = d->marker->GetMaterial();
    mat.SetDiffuse( 0.6, 1., 0., 1. );
    d->marker->SetMaterial( mat );
    d->marker->glSetChanged( GLComponent::glMATERIAL );
  }
}


void AConnectivityMatrix::buildColumnTexture( int mesh_index,
                                              uint32_t startvertex,
                                              float time_pos )
{
//   d->vertex = startvertex;
  rc_ptr<SparseOrDenseMatrix> mat = d->sparse->matrix();
  rc_ptr<TimeTexture<float> > tex( new TimeTexture<float> );
  vector<float> & tex0 = (*tex)[0].data();
  CiftiTools ctools( d->sparse->matrix() );
  vector<int> cind( 1, startvertex );
  cout << "buildColumnTexture " << startvertex << ": " << cind[0] << endl;
  vector<int> col_indices = ctools.getIndicesForSurfaceIndices( 1, mesh_index,
                                                                cind );
  cout << "col_indices: " << col_indices.size() << endl;
  vector<ATriangulated *>::iterator im, em = d->meshes.end();
  unsigned index = 0;

  if( col_indices.empty() )
  {
    // not in the matrix: clear textures
    for( im=d->meshes.begin(); im!=em; ++im, ++index )
    {
      const AimsSurface<3, Void> *surf = (*im)->surfaceOfTime( time_pos );
      tex0.resize( surf->vertex().size(), 0. );
      d->textures[index]->setTexture( tex );
    }
    return;
  }
  uint32_t column = col_indices[0];

  for( im=d->meshes.begin(); im!=em; ++im, ++index )
  {
    const AimsSurface<3, Void> *surf = (*im)->surfaceOfTime( time_pos );
    tex0.resize( surf->vertex().size(), 0. );
    bool valid = true;
    float texmax = -numeric_limits<float>::max();
    float colscale = 0.F;

    if( !d->basins.empty() )
    {
      // expand matrix to basins
      // FIXME needs to cache this int texture
      rc_ptr<Texture1d> basintex = d->basins[index]->texture<float>( true,
                                                                     false );
      const vector<float> & basinv = basintex->begin()->second.data();
      int basin = int( rint( basinv[ startvertex ] ) );
      if( basin == 0 )
      {
        valid = false;
        colscale = 1.F;
      }
      else
        column = basin - 1; // new index in matrix columns
    }
    if( valid )
    {
      /* TODO: here we should mat->readColumn( column );
        but on large CIFTI files, this operation is too slow to allow
        interactive reading. So for now, this will not dislay good data on
        CIFTI files.
        Another way would be to read all rows for the selected ROIs.
      */
      vector<double> col = mat->getColumn( column );
      map<uint32_t, uint32_t>::const_iterator
        ip, ep = d->patchindices[index].end();
      for( ip=d->patchindices[index].begin(); ip!=ep; ++ip )
      {
        double val = col[ ip->second ];
        tex0[ ip->first ] = val;
        if( val > texmax )
          texmax = val;
      }
    }

    if( texmax == -numeric_limits<float>::max() )
      colscale = 1.F;
    else if( colscale == 0.F )
      colscale = d->connectivity_max / texmax;

    d->textures[index]->setTexture( tex );
    d->textures[index]->palette()->setMax1( colscale );
  }

  // update sphere marker
  if( d->marker )
  {
    AimsSurfaceTriangle *sph = SurfaceGenerator::icosahedron(
      d->meshes[ mesh_index ]->surfaceOfTime( time_pos )->vertex()
        [ startvertex ], 1.5 );
    d->marker->setSurface( rc_ptr<AimsSurfaceTriangle>( sph ) );
    Material & mat = d->marker->GetMaterial();
    mat.SetDiffuse( 0., .6, 1., 1. );
    d->marker->SetMaterial( mat );
    d->marker->glSetChanged( GLComponent::glMATERIAL );
  }
}


void AConnectivityMatrix::buildPatchTexture( int mesh_index,
                                             uint32_t startvertex,
                                             float time_pos )
{
  if( d->patches.empty() )
    return;

  // find vertex in patch
  if( !d->patchindices.empty() )
  {
    map<uint32_t, uint32_t>::const_iterator
      ip = d->patchindices[mesh_index].find( startvertex );
    if( ip == d->patchindices[mesh_index].end() )
    {
      // outside patch
      buildColumnPatchTexture( mesh_index, startvertex, time_pos );
      return;
    }
  }

  // get patch values at clicked vertex
  vector<int16_t> pvals;
  int16_t patchval;
  rc_ptr<TimeTexture<int16_t> > tx0 = d->patches[mesh_index]->texture<int16_t>(
    true, false );
  int timestep;
  uint32_t i, n;
  TimeTexture<int16_t>::const_iterator it, et = tx0->end();
  for( it=tx0->begin(); it!=et; ++it )
  {
    timestep = it->first;
    const vector<int16_t> & tex = it->second.data();
    patchval = tex[ startvertex ];
    pvals.push_back( patchval );
  }

  vector<ATriangulated *>::iterator im, em = d->meshes.end();
  unsigned index = 0;
  for( im=d->meshes.begin(); im!=em; ++im, ++index )
  {
    const AimsSurface<3, Void> *surf = (*im)->surfaceOfTime( time_pos );
    uint32_t vertex, texsize = surf->vertex().size();

    rc_ptr<TimeTexture<int16_t> > tx;
    if( index == mesh_index )
      tx = tx0;
    else
      tx = d->patches[index]->texture<int16_t>( true, false );
    rc_ptr<TimeTexture<float> > ptex( new TimeTexture<float> );
    int16_t tval;
    if( index == mesh_index )
      pvals.reserve( tx->size() );
    rc_ptr<SparseOrDenseMatrix> mat = d->sparse->matrix();

    for( it=tx->begin(), et=tx->end(); it!=et; ++it )
    {
      timestep = it->first;
      const vector<int16_t> & tex = it->second.data();
      patchval = pvals[ timestep ];
      map<uint32_t, uint32_t> patchindices;
      map<uint32_t, uint32_t>::const_iterator
        ip, ep = d->patchindices[index].end();
      n = d->patchindices[index].size();
      // get sub-patch
      for( ip=d->patchindices[index].begin(); ip!=ep; ++ip )
      {
        tval = tex[ ip->first ];
        if( tval == patchval )
          patchindices[ ip->first ] = ip->second; // indices in d->patchindices
      }

      // if using basins, prepare a translation table
      rc_ptr<TimeTexture<int> > basinstex;
      Texture<int> *basins = 0;
      int b;
      if( !d->basins.empty() )
      {
        basinstex = d->basins[index]->texture<int>( true, false );
        basins = &basinstex->begin()->second;
      }

      // sum rows in sparse matrix
      vector<float> & ptext = (*ptex)[timestep].data();
      ptext.resize( texsize, 0. );
      for( ip=patchindices.begin(), ep=patchindices.end(); ip!=ep; ++ip )
      {
        mat->readRow( ip->second );
        // FIXME: use Cifti expandedValueTextureFromDimension
        vector<double> row = mat->getRow( ip->second );
        if( !basins )
          for( i=0; i<texsize; ++i )
            ptext[i] += row[i];
        else
          for( i=0; i<texsize; ++i )
          {
            b = (*basins)[i];
            if( b != 0 )
              ptext[i] += row[ b - 1 ];
          }
      }
    }

    // store texture with timesteps
    d->textures[index]->setTexture( ptex );
    d->textures[index]->palette()->setMax1( 1. );
  }

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
      mesh = SurfaceManip::meshExtract( *d->meshes[mesh_index]->surface(),
                                        *tx0, patchval );
    }
    else
    {
      // build sub-mesh for each timestep
      for( it=tx0->begin(), et=tx0->end(), i=0; it!=et; ++it, ++i )
      {
        timestep=it->first;
        // get patch texture for meshExtract
        TimeTexture<int16_t> tex16;
        tex16[0] = it->second;
        AimsSurfaceTriangle *tmesh = SurfaceManip::meshExtract(
          *d->meshes[mesh_index]->surface(), tex16, pvals[i] );
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


void AConnectivityMatrix::buildColumnPatchTexture( int /* mesh_index */,
                                                   uint32_t /*startvertex*/,
                                                   float /*time_pos*/ )
{
}


vector<rc_ptr<ATriangulated> > AConnectivityMatrix::meshes() const
{
  unsigned i, n = d->meshes.size();
  vector<rc_ptr<ATriangulated> > meshes( n );
  for( i=0; i<n; ++i )
    meshes[i].reset( d->meshes[i] );
  return meshes;
}


vector<rc_ptr<ATexture> > AConnectivityMatrix::textures() const
{
  unsigned i, n = d->textures.size();
  vector<rc_ptr<ATexture> > tex( n );
  for( i=0; i<n; ++i )
    tex[i].reset( d->textures[i] );
  return tex;
}


const rc_ptr<ATriangulated> AConnectivityMatrix::marker() const
{
  return rc_ptr<ATriangulated>( d->marker );
}


