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

  AConnectivityMatrix::PatchMode patchmode;
  set<int> patchnums;
  ASparseMatrix *sparse;
  ATriangulated *mesh;
  ATexture *patch;
  ATexture *basins;
  ATexture *texture;
  AMTexture *mtexture;
  ATexSurface *texsurface;
  ATriangulated *marker;
  vector<uint32_t> patchindices;
  uint32_t vertex;
  double connectivity_max;
};


AConnectivityMatrix::Private::Private()
  : patchmode( AConnectivityMatrix::ALL_BUT_ONE ),
  sparse( 0 ), mesh( 0 ), patch( 0 ), basins( 0 ), texture( 0 ), mtexture( 0 ),
  texsurface( 0 ), marker( 0 ), vertex( 0 ), connectivity_max( 0 )
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

  list<AObject *> filteredobj;
  set<AObject *> sobj;
  sobj.insert( obj.begin(), obj.end() );

  if( !checkObjects( sobj, filteredobj, d->patchmode, d->patchnums ) )
  {
    cerr << "AConnectivityMatrix: inconsistency in fusion objects types\n";
    return;
  }

  list<AObject *>::iterator io, eo = filteredobj.end();
  io=filteredobj.begin();
  AObject *matrix = *io;
  // matrix transformation / transposition delayed after textures analysis
  ++io;
  d->mesh = static_cast<ATriangulated *>( *io );
  setReferentialInheritance( d->mesh );
  ++io;
  if( io != eo )
  {
    d->patch = static_cast<ATexture *>( *io );
    d->patch->getOrCreatePalette();
    AObjectPalette *pal = d->patch->palette();
    rc_ptr<Texture1d> aimstex = d->patch->texture<float>( false, false );
    GLComponent::TexExtrema & text = d->patch->glTexExtrema( 0 );
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
    d->patch->setPalette( *pal );
    d->patch->glSetTexRGBInterpolation( true, 0 );

    ++io;
    if( io != eo )
    {
      // 2nd texture for matrix reduction along basins
      d->basins = static_cast<ATexture *>( *io );
    }
  }
  else
  {
    d->patch = 0;
    d->patchnums.clear();
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

  // make a texture to store connectivity data
  rc_ptr<TimeTexture<float> > tex( new TimeTexture<float> );
  size_t nvert = d->mesh->surface()->vertex().size();
  vector<float> & tex0 = (*tex)[0].data();
  tex0.resize( nvert, 0 );
  d->texture = new ATexture;
  d->texture->setTexture( tex );
  d->texture->setName( "ConnectivityTexture" );
  theAnatomist->registerObject( d->texture, false );
  d->texture->getOrCreatePalette();
  AObjectPalette *pal = d->texture->palette();
  pal->setRefPalette( theAnatomist->palettes().find( "yellow-red-fusion" ) );
  d->texture->setPalette( *pal );

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
    theAnatomist->registerObject( d->mtexture, false );
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
  if( d->mtexture )
    theAnatomist->releaseObject( d->mtexture );

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
  d->marker->setReferentialInheritance( d->mesh );
  theAnatomist->registerObject( d->marker, false );

  // insert in MObject
  insert( rc_ptr<AObject>( d->sparse ) );
  insert( rc_ptr<AObject>( d->mesh ) );
  if( d->patch )
  {
    insert( rc_ptr<AObject>( d->patch ) );
    if( d->basins )
      insert( rc_ptr<AObject>( d->basins ) );
  }
  /* if the matrix object has been created here, release it (remove from
     the main GUI) now, after it is inserted in the MObject.
  */
  if( builtnewmatrix )
    theAnatomist->releaseObject( d->sparse );
  insert( rc_ptr<AObject>( d->texture ) );
  theAnatomist->releaseObject( d->texture );
  insert( rc_ptr<AObject>( d->texsurface ) );
  theAnatomist->releaseObject( d->texsurface );
  insert( rc_ptr<AObject>( d->marker ) );
  theAnatomist->releaseObject( d->marker );

  d->connectivity_max = SparseMatrixUtil::max( *d->sparse->matrix() );

  // build connectivity texture contents
  buildTexture( vertex );
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
  if( d->texsurface )
    d->texsurface->render( plist, vs );
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
    if( mesh_ids.size() != meshes.size() )
      return false;
    if( tex.size() < mesh_ids.size() )
      return false;

    cols = get_cifti_mesh_cols( mat, mesh_ids );
    list<ATriangulated *> ordered_meshes;
    list<string>::const_iterator imid, emid = mesh_ids.end();
    list<ATriangulated *>::iterator im, em = meshes.end(), bm;
    list<ATexture *> ordered_tex;
    list<ATexture *>::iterator it, et = tex.end(), bt;
    list<pair<vector<int>, size_t> >::iterator icol;
    set<ATriangulated *> done_meshes;
    set<ATriangulated *>::iterator mesh_not_done = done_meshes.end();
    set<ATexture *> done_tex;
    set<ATexture *>::iterator tex_not_done = done_tex.end();
    ViewState vs;

    for( imid=mesh_ids.begin(), icol=cols.begin(); imid!=emid; ++imid, ++icol )
    {
      // look for matching mesh(es)
      bm = em;
      for( im=meshes.begin(); im!=em; ++im )
      {
        if( done_meshes.find( *im ) != mesh_not_done )
          continue;
        rc_ptr<AimsSurfaceTriangle> mesh = (*im)->surface();
        if( mesh->vertex().size() == icol->second )
        {
          if( bm == em )
            bm = im;
          try
          {
            string name = mesh->header().getProperty( "cifti_mesh_name" )
              ->getString();
            if( name == *imid )
              break;
          }
          catch( ... )
          {
          }
        }
      }
      if( bm == em )
        return false;
      ordered_meshes.push_back( *bm );
      done_meshes.insert( *bm );

      // look for matching texture(s)
      bt = et;
      for( it=tex.begin(); it!=et; ++it )
      {
        if( done_tex.find( *it ) != tex_not_done )
          continue;
        if( (*it)->glTexCoordSize( vs, 0 ) == icol->second )
        {
          if( bt == et )
            bt = it;
          try
          {
            string name = (*it)->attributed()->getProperty( "cifti_mesh_name" )
              ->getString();
            if( name == *imid )
              break;
          }
          catch( ... )
          {
          }
        }
      }
      if( bt == et )
        return false;
      ordered_tex.push_back( *bt );
      done_tex.insert( *bt );
    }

    meshes = ordered_meshes;
    tex = ordered_tex;
    return true;
  }

}


bool AConnectivityMatrix::checkObjects( const set<AObject *> & objects, 
  list<AObject *> & ordered, PatchMode & patchnummode,
  set<int> & patchnums )
{
  ASparseMatrix *sparse = 0;
  AVolume<float> *dense = 0;
  AVolume<double> *ddense = 0;
  list<ATriangulated *> mesh;
  ATexture *tex = 0, *tex2 = 0;

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
      if( !mesh.empty() )
        // a mesh should be the only one unless they are fusionned with
        // textures
        return false;
      mesh.push_back( static_cast<ATriangulated *>( *io ) );
    }
    else if( dynamic_cast<ATexture *>( *io ) )
    {
      if( tex )
      {
        if( tex2 )
          return false;
        tex2 = static_cast<ATexture *>( *io );
      }
      else
        tex = static_cast<ATexture *>( *io );
    }
    else
      return false;
  }

  if( ( !sparse && !dense && !ddense ) || mesh.size() != 1 )
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
    ordered.push_back( sparse );
  else if( dense )
    ordered.push_back( dense );
  else
    ordered.push_back( ddense );
  for( imesh=mesh.begin(); imesh!=emesh; ++imesh )
  ordered.push_back( *imesh );
  if( tex )
    ordered.push_back( tex );
  if( tex2 )
    ordered.push_back( tex2 );

  cout << "OK for connectivity matrix fusion.\n";
  return true;
}


void AConnectivityMatrix::buildPatchIndices()
{
  if( !d->patch )
  {
    // no patch texture: no indices
    d->patchindices.clear();
    return;
  }
  rc_ptr<TimeTexture<float> > tx = d->patch->texture<float>( false, false );
  const vector<float> & tex = (*tx)[0].data();
  vector<float>::const_iterator it, et = tex.end();
  uint32_t i = 0, j = 0;
  d->patchindices.resize( d->sparse->matrix()->getSize1() );
  const GLComponent::TexExtrema & te = d->patch->glTexExtrema( 0 );
  float scale = 1., offset = 0.;
  if( te.scaled && te.max[0] != te.min[0] )
  {
    scale = ( te.maxquant[0] - te.minquant[0] ) / ( te.max[0] - te.min[0] );
    offset = te.minquant[0] - te.min[0] * scale;
  }
  if( d->patchmode == ONE )
  {
    for( it=tex.begin(); it!=et; ++it, ++j )
      if( d->patchnums.find( int( rint( *it * scale + offset ) ) )
          != d->patchnums.end() )
        d->patchindices[ i++ ] = j;
  }
  else // ALL_BUT_ONE
  {
    for( it=tex.begin(); it!=et; ++it, ++j )
      if( d->patchnums.find( ( *it * scale + offset ) ) == d->patchnums.end() )
        d->patchindices[ i++ ] = j;
  }
}


void AConnectivityMatrix::buildTexture( uint32_t startvertex, float time_pos )
{
  uint32_t vertex = startvertex;
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
      buildColumnTexture( startvertex, time_pos );
      return;
    }
  }

  d->vertex = startvertex;
  rc_ptr<SparseOrDenseMatrix> mat = d->sparse->matrix();
  mat->readRow( vertex );
  vector<double> row = mat->getRow( vertex );
  rc_ptr<TimeTexture<float> > tex( new TimeTexture<float> );
  vector<float> & tex0 = (*tex)[0].data();
  const AimsSurface<3, Void> *surf = d->mesh->surfaceOfTime( time_pos );
  float texmax = -numeric_limits<float>::max();
  float colscale = 0.F;

  if( !d->basins )
  {
    tex0.insert( tex0.end(), row.begin(), row.end() );
    texmax = 1.F;
    colscale = 1.F;
  }
  else
  {
    // expand matrix to basins
    tex0.insert( tex0.end(), surf->vertex().size(), 0 );
    // FIXME needs to cache this int texture
    rc_ptr<Texture1d> basintex = d->basins->texture<float>( true, false );
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
        if( *ic > texmax )
          texmax = *ic;
      }
    }
  }

  if( texmax == -numeric_limits<float>::max() )
    colscale = 1.F;
  else if( colscale == 0.F )
    colscale = d->connectivity_max / texmax;

  d->texture->setTexture( tex );
  d->texture->palette()->setMax1( colscale );

  // update sphere marker
  if( d->marker )
  {
    AimsSurfaceTriangle *sph = SurfaceGenerator::icosahedron( 
      surf->vertex()[ startvertex ], 1.5 );
    d->marker->setSurface( rc_ptr<AimsSurfaceTriangle>( sph ) );
    Material & mat = d->marker->GetMaterial();
    mat.SetDiffuse( 0.6, 1., 0., 1. );
    d->marker->SetMaterial( mat );
    d->marker->glSetChanged( GLComponent::glMATERIAL );
  }
}


void AConnectivityMatrix::buildColumnTexture( uint32_t startvertex,
                                              float time_pos )
{
  d->vertex = startvertex;
  rc_ptr<SparseOrDenseMatrix> mat = d->sparse->matrix();
  rc_ptr<TimeTexture<float> > tex( new TimeTexture<float> );
  vector<float> & tex0 = (*tex)[0].data();
  const AimsSurface<3, Void> *surf = d->mesh->surfaceOfTime( time_pos );
  tex0.resize( surf->vertex().size(), 0. );
  bool valid = true;
  uint32_t column = startvertex;
  float texmax = -numeric_limits<float>::max();
  float colscale = 0.F;

  if( d->basins )
  {
    // expand matrix to basins
    // FIXME needs to cache this int texture
    rc_ptr<Texture1d> basintex = d->basins->texture<float>( true, false );
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
    vector<uint32_t>::const_iterator ip, ep = d->patchindices.end();
    vector<double>::const_iterator iv, ev = col.end();
    for( ip=d->patchindices.begin(), iv=col.begin(); ip!=ep; ++ip, ++iv )
    {
      tex0[*ip] = *iv;
      if( *iv > texmax )
        texmax = *iv;
    }
  }

  if( texmax == -numeric_limits<float>::max() )
    colscale = 1.F;
  else if( colscale == 0.F )
    colscale = d->connectivity_max / texmax;

  d->texture->setTexture( tex );
  d->texture->palette()->setMax1( colscale );

  // update sphere marker
  if( d->marker )
  {
    AimsSurfaceTriangle *sph = SurfaceGenerator::icosahedron(
      surf->vertex()[ startvertex ], 1.5 );
    d->marker->setSurface( rc_ptr<AimsSurfaceTriangle>( sph ) );
    Material & mat = d->marker->GetMaterial();
    mat.SetDiffuse( 0., .6, 1., 1. );
    d->marker->SetMaterial( mat );
    d->marker->glSetChanged( GLComponent::glMATERIAL );
  }
}


void AConnectivityMatrix::buildPatchTexture( uint32_t startvertex,
                                             float time_pos )
{
  if( !d->patch )
    return;

  // find vertex in patch
  vector<uint32_t>::const_iterator ip, ep = d->patchindices.end();
  const AimsSurface<3, Void> *surf = d->mesh->surfaceOfTime( time_pos );
  uint32_t vertex = 0, n, texsize = surf->vertex().size(), i;
  for( ip=d->patchindices.begin(); ip!=ep; ++ip, ++vertex )
    if( *ip == startvertex )
      break;
  if( ip == ep )
  {
    // outside patch
    buildColumnPatchTexture( startvertex, time_pos );
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
  rc_ptr<SparseOrDenseMatrix> mat = d->sparse->matrix();

  for( it=tx->begin(); it!=et; ++it )
  {
    timestep = it->first;
    const vector<int16_t> & tex = it->second.data();
    patchval = tex[ startvertex ];
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

    // if using basins, prepare a translation table
    rc_ptr<TimeTexture<int> > basinstex;
    Texture<int> *basins = 0;
    int b;
    if( d->basins )
    {
      basinstex = d->basins->texture<int>( true, false );
      basins = &basinstex->begin()->second;
    }

    // sum rows in sparse matrix
    vector<float> & ptext = (*ptex)[timestep].data();
    ptext.resize( texsize, 0. );
    for( vertex=0, n=patchindices.size(); vertex<n; ++vertex )
    {
      mat->readRow( patchindices[vertex] );
      vector<double> row = mat->getRow( patchindices[vertex] );
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
  d->texture->setTexture( ptex );
  d->texture->palette()->setMax1( 1. );

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
        // get patch texture for meshExtract
        TimeTexture<int16_t> tex16;
        tex16[0] = it->second;
        AimsSurfaceTriangle *tmesh = SurfaceManip::meshExtract(
          *d->mesh->surface(), tex16, pvals[i] );
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


void AConnectivityMatrix::buildColumnPatchTexture( uint32_t /*startvertex*/,
                                                   float /*time_pos*/ )
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


