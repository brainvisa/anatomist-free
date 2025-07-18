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


#include <anatomist/window/glwidget.h>	// needed for GL context
#include <anatomist/window/glcaps.h>
#include <anatomist/control/wControl.h>
#include <anatomist/object/Object.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/window/Window.h>
#include <anatomist/mobject/MObject.h>
#include <anatomist/mobject/Fusion2D.h>
#include <anatomist/mobject/Fusion3D.h>
#include <anatomist/reference/Referential.h>
#include <anatomist/reference/Transformation.h>
#include <anatomist/reference/Geometry.h>
#include <anatomist/object/oReader.h>
#include <anatomist/color/objectPalette.h>
#include <anatomist/color/paletteList.h>
#include <anatomist/misc/error.h>
#include <anatomist/primitive/primitive.h>
#include <anatomist/selection/selectFactory.h>
#include <anatomist/surface/glcomponent.h>
#include <anatomist/surface/glcomponent_internals.h> // for TexInfo struct
#include <anatomist/graph/pythonAObject.h>
#include <anatomist/window/viewstate.h>
#include <anatomist/object/mobjectio.h>
#include <anatomist/object/sliceable.h>
#include <anatomist/graph/attribAObject.h>
#include <aims/resampling/quaternion.h>
#include <aims/mesh/texturetools.h>
#include <cartobase/stream/fileutil.h>
#include <cartobase/thread/mutex.h>
#include <time.h>

//#define ANA_DEBUG
#ifdef ANA_DEBUG
#include <typeinfo>
#endif

using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;


namespace anatomist
{

struct AObject::Private
{
  enum TextureMode
  {
    Geometric, Linear, Replace
  };

  enum TextureFiltering
  {
    F_Nearest, F_Linear
  };

  Private();
  Private( const Private & );
  ~Private();

  GLint glTexMode() const;
  GLint glTexFiltering() const;

  typedef map<rc_ptr<ObjectMenuRegistrerClass>, set<string> >
    MenuRegistrersMap;
  static MenuRegistrersMap & objectMenuRegistrers();

  PrimList          sharedprim;
  bool              cleanupdone;
  // texture stuff should be in GLComponent but...
  TextureMode       texturemode;
  TextureFiltering  texturefilter;
  Referential       *oldref;
  time_t            loaddate;
  bool              copy;

  AObject           *inheritref;
  // bool             cycling;
  bool              userModified;
  bool              allowsOverwriteOnSave;
};

std::map<std::string, rc_ptr<ObjectMenu> >	AObject::_objectmenu_map;

// -----

void AObject::cleanStatic()
{
  _objectmenu_map.clear();
  _objectTypes.clear();
  _objectTypeNames.clear();
}


AObject::Private::Private() 
  : cleanupdone( false ), texturemode( Replace ), texturefilter( F_Nearest ), 
    oldref( 0 ), loaddate( time( 0 ) ), copy( false ), inheritref( 0 ),
    userModified( false ), allowsOverwriteOnSave( false )
{
}


AObject::Private::Private( const Private & x )
  : cleanupdone( false ), texturemode( x.texturemode ),
    texturefilter( x.texturefilter ),
    oldref( 0 ), loaddate( x.loaddate ), inheritref( x.inheritref ), userModified( x.userModified ),
    allowsOverwriteOnSave( x.allowsOverwriteOnSave )
    // cycling( x.cycling )
{
}


AObject::Private::~Private()
{
}


GLint AObject::Private::glTexMode() const
{
  static GLint	mode[] = { GL_MODULATE, GL_DECAL, GL_REPLACE };
  return mode[ texturemode ];
}


GLint AObject::Private::glTexFiltering() const
{
  static GLint	mode[] = { GL_NEAREST, GL_LINEAR };
  return mode[ texturefilter ];
}



// --- AObject


AObject::AObject( const string & filename ) 
  : Observable(), Observer(), _filename( filename ), _material(), 
    _palette( 0 ), 
    d( new AObject::Private )
{
  // cout << "AObject::AObject " << filename << ", " << this << endl;
  _id   = 0;
  _inMemory = 0;
  _visible = 0;
  _referenceHasChanged = false;
  _referential = 0;
  //setReferential( theAnatomist->centralReferential() );
}


AObject::AObject( const AObject & x )
  : SharedObject(), Observable(), Observer(), _type( x._type ), _id( 0 ), _name( x._name ),
    _filename( x._filename ), _inMemory( x._inMemory ), _visible( x._visible ),
    _material( x._material ), _referential( 0 ), _referenceHasChanged( false ),
    _palette( 0 ),
    d( new Private( *x.d ) )
{
}


AObject::~AObject()
{
  // cout << "AObject::~AObject " << name() << ", " << this << endl;
  cleanup();

  if( !_name.empty() )
    theAnatomist->unregisterObjectName( _name );

  delete _palette;
  delete d;
}



bool AObject::IsFusion2DAllowed() 
{
  return true;
}


int AObject::CanBeDestroyed() 
{
  if( !testDeletable() )
    return 0;
  ParentList::iterator par;
  for( par=_parents.begin(); par!=_parents.end(); ++par )
    if( !(*par)->CanRemove( this ) )
      return 0;
  return 1;
}


void AObject::setName( const string & n )
{
  if( _name == n ) return;
  theAnatomist->lockObjects( true );
  if( !_name.empty() )
    theAnatomist->unregisterObjectName( _name );
  _name = n;

  theAnatomist->registerObjectName( _name, this );
  theAnatomist->lockObjects( false );
}


void AObject::setFileName( const string & filename )
{
  _filename = filename;
}


void AObject::setId(int id)
{
  if( id < 0 )
    AWarning("void AObject::SetId(int) : negative identifier");
  _id = id;
}


void AObject::registerWindow(AWindow *window)
{
  _winList.insert( window );
}


void AObject::unregisterWindow(AWindow *window)
{
  set<AWindow*>::iterator win = _winList.find( window );

  if( win == _winList.end() )
    {
      AWarning( "AObject::unregisterWindow: window not registered" );
    }
  else
    _winList.erase( win );
}


bool AObject::render( PrimList & prim, const ViewState & vs )
{
  GLComponent *gl = glAPI();
  if( gl )
  {
    GLPrimitives	pl = gl->glMainGLL( vs );
    if( !pl.empty() )
    {
      const Referential *ref = getReferential();
      GLPrimitives p2 = GLComponent::glHandleTransformation( vs, ref );
      bool hastr = !p2.empty();
      prim.insert( prim.end(), p2.begin(), p2.end() );
      prim.insert( prim.end(), pl.begin(), pl.end() );
      if( hastr )
      {
        pl = GLComponent::glPopTransformation( vs, ref );
        prim.insert( prim.end(), pl.begin(), pl.end() );
      }
      gl->glClearHasChangedFlags();
      return true;
    }
  }
  return false;
}


vector<float> AObject::voxelSize() const
{
  return vector<float>( 4, 1.f );
}


void AObject::setReferenceChanged()
{
  _referenceHasChanged = true;
  const GLComponent	*glc = glAPI();
  if( glc )
    glc->glSetChanged( GLComponent::glREFERENTIAL );
  obsSetChanged( GLComponent::glREFERENTIAL ); 
}


AObject *AObject::referentialInheritance() const
{
  return d->inheritref;
}


AObject *& AObject::_referentialInheritance()
{
  return d->inheritref;
}


Referential* AObject::getReferential() const
{
  return _referential;
}


void AObject::setReferentialInheritance( AObject* p )
{
  if( !p )
  {
    setReferential( 0 );
    return;
  }

  Referential  *ref = p->getReferential();
  if( ref && ref->destroying() )
    ref = theAnatomist->centralReferential();
  if( p == d->inheritref && ref == _referential )
    return;

  const AObject *old = d->inheritref;
  clearReferentialInheritance();
  d->inheritref = p;
    d->inheritref->addObserver( this );
  if( !p && old )
  {
    setReferenceChanged();
    if( theAnatomist->getControlWindow() != 0 )
      theAnatomist->getControlWindow()->NotifyObjectChange(this);
    return;
  }

  Referential  *r = ref;
  if( ref != _referential )
  {
    if( _referential )
      _referential->RemoveObject(this);
    d->oldref = _referential;
    r = _referential;
    _referential = ref;
    if( _referential )
      _referential->AddObject(this);
    setReferenceChanged();
  }

  if( r!= ref && theAnatomist->getControlWindow() != 0 )
    theAnatomist->getControlWindow()->NotifyObjectChange(this);
}


void AObject::setReferential( Referential *ref )
{
  if( ref == _referential && !d->inheritref )
    return;

  bool change = ( d->inheritref != 0 );
  clearReferentialInheritance();
  
  Referential *r = _referential;
  /*  cout << "_setReferential " << name() << " " << this << ": ref: "
      << ref << ", old: "
      << r << ", own: " << ownref << ", oldown: " << oldown << endl;*/
  if( ref != _referential )
  {
    if( _referential )
      _referential->RemoveObject(this);
    d->oldref = _referential;
    _referential = ref;
    if( _referential )
      _referential->AddObject(this);
    setReferenceChanged();
  }

  if( ( change || r!= ref ) && theAnatomist->getControlWindow() != 0 )
    theAnatomist->getControlWindow()->NotifyObjectChange(this);
  // cout << "_setReferential end\n";
}


void AObject::clearReferentialInheritance()
{
  if( d->inheritref )
    d->inheritref->deleteObserver( this );
  d->inheritref = 0;
}


AObject* AObject::fallbackReferentialInheritance() const
{
  return 0;
}


bool AObject::hasReferenceChanged() const
{
  return _referenceHasChanged;
}


void AObject::SetMaterial( const Material & mat )
{
  if( mat != _material )
  {
    bool changeshader = false;
    GLComponent* glc = glAPI();
    if( glc )
    {
      if( _material.renderProperty( Material::UseShader )
          != mat.renderProperty( Material::UseShader ) )
        changeshader = true;
      if( !changeshader && mat.renderProperty( Material::UseShader ) )
      {
        Material::RenderProperty props[] = { Material::RenderLighting,
          Material::RenderSmoothShading,
          Material::ShaderColorNormals,
          Material::NormalIsDirection };
        for( int p=0; p<4; ++p )
          if( _material.renderProperty( props[p] )
            != mat.renderProperty( props[p] ) )
          {
            changeshader = true;
            break;
          }
      }
    }
    _material = mat;
    if( changeshader )
    {
      glc->setupShader();
      glc->glSetChanged( GLComponent::glBODY );
    }
    if( glc )
      glc->glSetChanged( GLComponent::glMATERIAL );
    else
      setChanged();
  }
}


void AObject::clearHasChangedFlags() const
{
  _referenceHasChanged = false;
}


AObject* AObject::objectAt( const vector<float> & pos, float tol,
                            const Referential* orgref, const Point3df & )
{
  if( !orgref || !getReferential() )
    return( objectAt( pos, tol ) );

  const Transformation	*tra 
    = theAnatomist->getTransformation( orgref, getReferential() );
  if( !tra )
    return( objectAt( pos, tol ) );

  Point3df	tp = tra->transform( Point3df( pos[0], pos[1], pos[2] ) );

  vector<float> new_pos = pos;
  new_pos[0] = tp[0];
  new_pos[1] = tp[1];
  new_pos[2] = tp[2];

  return( objectAt( new_pos, tol ) );
}


AObject* AObject::objectAt( const vector<float> & pos, float tol )
{
  if( tol < 0 ) tol *= -1;

  //	default behaviour is really primitive...
  vector<float> bmin, bmax;
  if( boundingBox( bmin, bmax ) )
  {
    unsigned i, n = std::min( bmax.size(), pos.size() );
    for( i=0; i<n; ++i )
      if( pos[i] < bmin[i] || pos[i] > bmax[i] )
        return 0;
    for( n=pos.size(); i<n; ++i )
      if( pos[i] != 0 )
        // extra dimensions in pos should be 0 to be acceptable
        return 0;
    return this;
  }
  else
    return 0;
}


const AObject* AObject::nearestVertex( const std::vector<float> & pos,
                                       int *vertex, float *distance,
                                       float tol, int *polygon,
                                       bool tex_only, int target_poly ) const
{
  // cout << "AObject::nearestVertex " << pos[0] << ", " << pos[1] << ", " << pos[2] << ", tol: " << tol << ", tex_only: " << tex_only << ", target_poly: " << target_poly << ", poly: " << polygon << endl;
  const GLComponent *glc = glAPI();
  if( !glc || ( tex_only && glc->glNumTextures() == 0 ) )
    return 0;

  Point3df p0( pos ), p1;
  vector<float> pt;
  unsigned i;
  for( i=3; i<pos.size(); ++i )
    pt.push_back( pos[i] );
  if( pt.empty() )
    pt.push_back( 0 );
  ViewState vs( pt );
  int iv = -1, nv = (int) glc->glNumVertex( vs );
  if( nv == 0 )
    return 0;
  const GLfloat* vert0 = glc->glVertexArray( vs ), *vert = vert0;
  float dist = -1, d, tol2 = -1;
  if( tol >= 0 )
    tol2 = tol * tol;

  if( target_poly >= 0 )
  {
    unsigned np = glc->glNumPolygon( vs ), ps = glc->glPolygonSize( vs );
    if( target_poly >= np )
      return 0;

    const GLuint *vpoly = glc->glPolygonArray( vs ) + ps * target_poly;
    unsigned v;

    for( i=0; i<ps; ++i )
    {
      v = *vpoly++;
      p1[0] = vert0[v * 3];
      p1[1] = vert0[v * 3 + 1];
      p1[2] = vert0[v * 3 + 2];
      d = ( p1 - p0 ).norm2();
      if( ( dist < 0 || d < dist ) && ( tol2 < 0 || d <= tol2 ) )
      {
        dist = d;
        iv = v;
      }
    }
    if( iv >= 0 )
    {
      *distance = sqrt( dist );
      *vertex = iv;
      if( polygon )
        *polygon = target_poly;
      return this;
    }

  }
  else
  {
    for( i=0; i<nv; ++i )
    {
      p1[0] = *vert++;
      p1[1] = *vert++;
      p1[2] = *vert++;
      d = ( p1 - p0 ).norm2();
      if( ( dist < 0 || dist > d ) && ( tol2 < 0 || d <= tol2 ) )
      {
        dist = d;
        iv = i;
      }
    }
    if( iv >= 0 )
    {
      *distance = sqrt( dist );
      *vertex = iv;
      if( polygon )
      {
        unsigned np = glc->glNumPolygon( vs );
        unsigned ps = glc->glPolygonSize( vs );
        const GLuint* poly = glc->glPolygonArray( vs );
        unsigned j, p;
        int ip = -1;
        float d2;
        dist = -1;
        for( i=0; i<np; ++i )
        {
          for( j=0; j<ps; ++j )
            if( *(poly + j) == iv )
              break;
          if( j != ps )
          {
            d2 = -1;
            // max distance among points of the polygon
            for( j=0; j<ps; ++j )
            {
              p = *poly++;
              if( p != iv )
              {
                p *= 3;
                p1[0] = vert0[p++];
                p1[1] = vert0[p++];
                p1[2] = vert0[p++];
                d = ( p1 - p0 ).norm2();
                if( d2 < d )
                  d2 = d;
              }
            }
            if( dist < 0 || d2 < dist )
            {
              // keep the poly with minimum d2 (max dist)
              dist = d2;
              ip = int(i);
            }
          }
          else
            poly += ps;
        }
        *polygon = ip;
      }
      return this;
    }
  }

  return 0;
}


list<AObject *> AObject::load( const string & filename )
{
  ObjectReader::PostRegisterList subobjects;
  list<AObject *> obj = ObjectReader::reader()->load( filename, subobjects );
//   if( !obj.empty() )
//     return obj;
//   Object objs = MObjectIO::readMObject( filename );
//   if( objs )
//   {
//     Object it = objs->objectIterator();
//     for( ; it->isValid(); it->next() )
//       obj.insert( it->currentValue()->value<AObject *>() );
//   }
  return obj;
}


bool AObject::reload( AObject* object, bool onlyoutdated )
{
  bool res = ObjectReader::reader()->reload( object, onlyoutdated );
  if( res )
    object->setUserModified( false );
  return res;
}


bool AObject::reload( const string & )
{
  return false;
}


void AObject::internalUpdate()
{
  if( hasChanged() )
  {
    ParentList::iterator i, f=Parents().end();
    for( i=Parents().begin(); i!=f; ++i )
    {
      (*i)->setChanged();
      (*i)->internalUpdate();
    }
  }
}


int AObject::registerObjectType( const string & id )
{
  map<string, int>::const_iterator	i = _objectTypes.find( id );

  if( i != _objectTypes.end() )
    {
      return( (*i).second );
    }
  int	n;
  if( _objectTypeNames.empty() )
    n = OTHER + 1;
  else
    n = (*_objectTypeNames.rbegin()).first + 1;
  _objectTypes[ id ] = n;
  _objectTypeNames[ n ] = id;
  return( n );
}


string AObject::objectTypeName( int type )
{
  map<int, string>::const_iterator	i = _objectTypeNames.find( type );
  if( i != _objectTypeNames.end() )
    return( (*i).second );
  else
    {
      if( type >= 0 && type <= OTHER )	// not initialized yet
	{
	  _objectTypeNames[ VOLUME        ] = "VOLUME";
	  _objectTypeNames[ BUCKET        ] = "BUCKET";
	  _objectTypeNames[ FACET         ] = "FACET";
	  _objectTypeNames[ TRIANG        ] = "SURFACE";
	  _objectTypeNames[ LIST          ] = "LIST";
	  _objectTypeNames[ VECTOR        ] = "VECTOR";
	  _objectTypeNames[ MAP           ] = "MAP";
	  _objectTypeNames[ SET           ] = "SET";
	  _objectTypeNames[ GRAPH         ] = "GRAPH";
	  _objectTypeNames[ GRAPHOBJECT   ] = "GRAPHOBJECT";
	  _objectTypeNames[ VOLSURF       ] = "VOLSURF";
	  _objectTypeNames[ MULTISURF     ] = "MULTISURF";
	  _objectTypeNames[ MULTIBUCKET   ] = "MULTIBUCKET";
	  _objectTypeNames[ MULTIVOLUME   ] = "MULTIVOLUME";
	  _objectTypeNames[ FUSION2D      ] = "FUSION2D";
	  _objectTypeNames[ FUSION3D      ] = "FUSION3D";
	  _objectTypeNames[ FASCICLE      ] = "FASCICLE";
	  _objectTypeNames[ FASCICLEGRAPH ] = "FASCIC. GRAPH";
	  _objectTypeNames[ TEXTURE       ] = "TEXTURE";
	  _objectTypeNames[ TEXSURFACE    ] = "TEXTURED SURF.";
	  _objectTypeNames[ FUSION2DMESH  ] = "FUSION2D MESH";
	  _objectTypeNames[ VECTORFIELD   ] = "VECTOR FIELD";
	  _objectTypeNames[ OTHER         ] = "UNREGISTERED";
	  return( _objectTypeNames[ type ] );
	}
      else
	return( "UNKNOWN" );
    }
}


void AObject::setPalette( const AObjectPalette & pal )
{
  if( &pal != _palette )
    {
      int sx = -1, sy = -1, gsx = -1, gsy = -1;
      if( _palette )
      {
        // keep same dimension limits
        sx = _palette->maxSizeX();
        sy = _palette->maxSizeY();
        gsx = _palette->glMaxSizeX();
        gsy = _palette->glMaxSizeY();
      }
      delete _palette;
      _palette = pal.clone();
      _palette->setMaxSize( sx, sy );
      _palette->glSetMaxSize( gsx, gsy );
      _palette->copyColors( pal );
    }
  GLComponent	*glc = glAPI();
  if( glc )
    glc->glSetTexImageChanged();
}


const AObjectPalette* AObject::getOrCreatePalette() const
{
  if( !palette() )
    const_cast<AObject *>(this)->createDefaultPalette();
  return( palette() );
}


void AObject::createDefaultPalette( const string & name )
{
  const PaletteList	& palettes = theAnatomist->palettes();
  rc_ptr<APalette>	refpal;

  if( palettes.size() == 0 )
    {
      cerr << "Palette list empty. Check your install path\n";
      return;
    }

  if( name.empty() )
    refpal = palettes.find( "B-W LINEAR" );
  else
    refpal = palettes.find( name );

  if( !refpal )
    {
      cerr << "Palette " << name << " not found.\n";
      refpal = palettes.palettes().front();
    }

  setPalette( AObjectPalette( refpal ) );
  palette()->create( 256 );
  palette()->fill();
}


float AObject::mixedTexValue( const vector<float> & pos,
                              const Referential* orgRef, int poly ) const
{
  Transformation *trans
    = theAnatomist->getTransformation( orgRef, getReferential() );
  vector<float> pt = pos;

  if( trans )
  {
    Point3df ptp = trans->transform( Point3df( pos[0], pos[1], pos[2] ) );
    pt[0] = ptp[0];
    pt[1] = ptp[1];
    pt[2] = ptp[2];
  }
  return mixedTexValue( pt, poly );
}


vector<float> AObject::texValues( const vector<float> & pos,
                                  const Referential* orgRef, int poly ) const
{
  Transformation	*trans 
    = theAnatomist->getTransformation( orgRef, getReferential() );
  vector<float> pt = pos;

  if( trans )
  {
    Point3df ptp = trans->transform( Point3df( pos[0], pos[1], pos[2] ) );
    pt[0] = ptp[0];
    pt[1] = ptp[1];
    pt[2] = ptp[2];
 }

  return texValues( pt, poly );
}


std::vector<float>
anatomist::AObject::texValues( const std::vector<float> & pos, int poly ) const
{
  int vertex;
  float distance;
  float tol = -1;  // FIXME

  const AObject *no = this;
  no = nearestVertex( pos, &vertex, &distance, tol, 0, true, poly );
  if( !no )
    return vector<float>();

  const GLComponent *glc = no->glAPI();
  if( !glc || glc->glNumTextures() == 0 )
    return vector<float>();  // would be strange: should not happen...

  Point3df p0( pos ), p1;
  vector<float> pt;
  unsigned i;
  for( i=3; i<pos.size(); ++i )
    pt.push_back( pos[i] );
  if( pt.empty() )
    pt.push_back( 0 );
  ViewState vs( pt );

  unsigned nt = glc->glNumTextures( vs ), ts;
  vector<float> values;
  values.reserve( nt );

  for( i=0; i<nt; ++i )
  {
    unsigned dt = glc->glDimTex( vs, i ), j;
    const GLfloat* tc = glc->glTexCoordArray( vs, i );
    const GLComponent::TexExtrema & te = glc->glTexExtrema( i );
    for( j=0; j<dt; ++j )
    {
      float val = tc[vertex * dt + j];
      if( te.scaled )
        val = ( val - te.min[j] ) * ( te.maxquant[j] - te.minquant[j] )
          / ( te.max[j] - te.min[j] ) + te.minquant[j];
      values.push_back( val );
    }
  }
  return values;
}


rc_ptr<Volume<float> > AObject::texValuesSeries( const vector<float> & pos,
                                                 int axis,
                                                 const Referential* orgRef,
                                                 int poly ) const
{
  Transformation	*trans
    = theAnatomist->getTransformation( orgRef, getReferential() );
  vector<float> pt = pos;

  if( trans )
  {
    Point3df ptp = trans->transform( Point3df( pos[0], pos[1], pos[2] ) );
    pt[0] = ptp[0];
    pt[1] = ptp[1];
    pt[2] = ptp[2];
 }

  return texValuesSeries( pt, axis, poly );
}


rc_ptr<Volume<float> >
anatomist::AObject::texValuesSeries( const std::vector<float> & pos, int axis,
                                     int poly ) const
{
  int vertex;
  float distance;
  float tol = -1;  // FIXME

  const AObject *no = this;
  no = nearestVertex( pos, &vertex, &distance, tol, 0, true, poly );
  if( !no )
    return VolumeRef<float>();

  const GLComponent *glc = no->glAPI();
  if( !glc || glc->glNumTextures() == 0 )
    return VolumeRef<float>();  // would be strange: should not happen...

  Point3df p0( pos ), p1;
  vector<float> pt;
  unsigned i;
  for( i=3; i<pos.size(); ++i )
    pt.push_back( pos[i] );
  if( pt.empty() )
    pt.push_back( 0 );
  ViewState vs( pt );

  unsigned nt = glc->glNumTextures( vs ), ts;
  unsigned ns, td = 0;
  for( i=0; i<nt; ++i )
    td += glc->glDimTex( vs, i );

  vector<float> voxs = no->voxelSize();

  while( voxs.size() <= axis )
    voxs.push_back( 1. );
  unsigned n = 1, t, k;
  if( axis == 3 )
  {
    vector<float> bmin, bmax;
    if( no->boundingBox( bmin, bmax ) )
      n = ( bmax[3] - bmin[3] ) / voxs[3] + 1;
  }

  VolumeRef<float> values( n, td );
  values->setVoxelSize( voxs[axis] );

  for( t=0; t<n; ++t )
  {
    if( axis >= 3 )
      pt[axis - 3] = t * voxs[axis];
    vs = ViewState( pt );

    k = 0;
    for( i=0; i<nt; ++i )
    {
      unsigned dt = glc->glDimTex( vs, i ), j;
      const GLfloat* tc = glc->glTexCoordArray( vs, i );
      const GLComponent::TexExtrema & te = glc->glTexExtrema( i );
      for( j=0; j<dt; ++j )
      {
        float val = tc[vertex * dt + j];
        if( te.scaled )
          val = ( val - te.min[j] ) * ( te.maxquant[j] - te.minquant[j] )
            / ( te.max[j] - te.min[j] ) + te.minquant[j];
        values->at( t, k++ ) = val;
      }
    }
  }
  return values;
}


void AObject::getTextureLabels( const vector<float> & texvalues,
                                vector<string> & texlabels,
                                string & textype ) const
{
  if( texvalues.empty() )
    return;

  const AttributedAObject *
    aao = dynamic_cast<const AttributedAObject *> ( this );
  Object labels;

  // is there a labels table (int -> string map)
  if( aao )
  {
    aao->attributed()->getProperty("data_type", textype);
    try
    {
      labels = aao->attributed()->getProperty( "labels" );
    }
    catch( ... )
    {
    }
  }
  if( !labels.get() )
  {
    // try another texture object
    const MObject *mo = dynamic_cast<const MObject *>( this );
    if( mo )
    {
      MObject::const_iterator im, em = mo->end();
      for( im=mo->begin(); im!=em; ++im )
      {
        aao = dynamic_cast<const AttributedAObject *>( *im );
        if( aao )
        {
          try
          {
            labels = aao->attributed()->getProperty( "labels" );
            break;
          }
          catch( ... )
          {
          }
        }
      }
    }
  }

  unsigned i, ntex = texvalues.size();
  int itval;

  if( labels.get() )
  {
    texlabels.reserve( ntex );
    for( i = 0; i < ntex; ++i )
    {
      itval = int( rint( texvalues[i] ) );
      // get string label if any
      string label;
      try
      {
        if( labels->hasItem( itval ) )
        {
          Object olabel = labels->getArrayItem( itval );
          if( olabel )
            label = olabel->getString();
        }
      }
      catch( ... )
      {
      }
      texlabels.push_back( label );
    }
  }
}


PrimList & AObject::primitives()
{
  return( d->sharedprim );
}


const PrimList & AObject::primitives() const
{
  return( d->sharedprim );
}


void AObject::cleanup()
{
  if( !d->cleanupdone )	// don't do this several times
  {
#ifdef ANA_DEBUG
    cout << "AObject::cleanup for object " << this << " ("
        << typeid( *this ).name() << ") - " << name() << endl;
#endif
    d->cleanupdone = true;

    disableRefCount();
    clearReferentialInheritance();
    theAnatomist->unregisterObject( this );
    notifyUnregisterObservers();

    setReferential( 0 );

    ParentList::iterator par( _parents.begin() ), current ;
    while( par != _parents.end() )
    {
      current = par ;
      ++par ;
      (*current)->eraseObject(this) ;
    }

    if( !theAnatomist->destroying() )
    {
      // TEMPORARY until SelectFactory is an observer of this
      SelectFactory *sf = SelectFactory::factory();
      const map<unsigned, set<AObject *> > & sel = sf->selected();
      map<unsigned, set<AObject *> >::const_iterator is, es = sel.end();
      set<AObject *> so;
      so.insert( this );
      vector<unsigned> groups;
      groups.reserve( sel.size() );
      for( is=sel.begin(); is!=es; ++is )
        groups.push_back( is->first );
      vector<unsigned>::iterator iv, ev=groups.end();
      for( iv=groups.begin(); iv!=ev; ++iv )
        sf->unselect( *iv, so );
    }
  }
}


bool AObject::boundingBox( vector<float> & /*bmin*/, vector<float> & /*bmax*/ ) const
{
  return false;
}


bool AObject::boundingBox2D( vector<float> & bmin, vector<float> & bmax ) const
{
  return boundingBox( bmin, bmax );
}


float AObject::MinT() const
{
  vector<float> bmin, bmax;
  boundingBox( bmin, bmax );
  if( bmin.size() >= 4 )
    return bmin[3];
  return 0.f;
}


float AObject::MaxT() const
{
  vector<float> bmin, bmax;
  boundingBox( bmin, bmax );
  if( bmax.size() >= 4 )
    return bmax[3];
  return 0.f;
}


bool AObject::isTransparent() const
{
  return material().IsBlended() 
    || ( palette() && palette()->isTransparent() );
}


const Referential* AObject::previousReferential() const
{
  return d->oldref;
}


namespace
{

  // flip y in texture image
  void _flipTexImage( VolumeRef<AimsRGBA> teximage )
  {
    int i, j, z, t,
      nx = teximage->getSizeX(), ny = teximage->getSizeY(), my = ny / 2,
      nz = teximage->getSizeZ(), nt = teximage->getSizeT();
    std::vector<AimsRGBA> line( nx );
    AimsRGBA *ptr1, *ptr2, *ptr3;

    for( t=0; t<nt; ++t )
      for( z=0; z<nz; ++z )
        for( i=0; i<my; ++i )
        {
          ptr1 = &teximage->at( 0, i, z, t );
          line.assign( ptr1, ptr1 + nx );
          ptr2 = &teximage->at( 0, ny - i - 1, z, t );
          ptr3 = &line[0];
          for( j=0; j<nx; ++j )
          {
            *ptr1++ = *ptr2;
            *ptr2++ = *ptr3++;
          }
        }
  }

}


void AObject::setHeaderOptions()
{
  PythonAObject	*pao = dynamic_cast<PythonAObject *>( this );
  if( pao )
  {
    const GenericObject	*o = pao->attributed();
    if( o )
      setProperties( Object::reference( *o ) );
  }
}


void AObject::setProperties( Object /*options*/ )
{
   /* cout << "setHeaderOptions on " << objectTypeName( type() ) << ": "
     << "name: " << name() << ", filename: " << fileName() << endl; */
  PythonAObject	*pao = dynamic_cast<PythonAObject *>( this );
  if( pao )
    {
      const GenericObject	*o = pao->attributed();
      Object	m;

      if( o )
        {
          // referential
          string t = objectTypeName( type() );
          if( t != "NOMENCLATURE" && t != "TEXTURE" )
          {
            try
            {
              Object ref = o->getProperty( "referential" );
              string refstr = ref->getString();
              Referential *oref = Referential::referentialOfName( refstr );
              if( oref )
                setReferential( oref );
              else
                setReferential( theAnatomist->centralReferential() );
            }
            catch( ... )
            {
              setReferential( theAnatomist->centralReferential() );
            }
          }

          // material
          GLComponent	*g = glAPI();
          try
            {
              m = o->getProperty( "material" );
              GetMaterial().set( *m );
              if( g )
              {
                g->setupShader();
                g->glSetChanged( GLComponent::glMATERIAL );
              }
              else
                setChanged();
            }
          catch( ... )
            {
            }

          // GIFTI colormap
          VolumeRef<AimsRGBA> cmap = giftiColormap( Object::reference( *o ) );
          if( cmap.get() && cmap->getSizeX() > 1 )
          {
            string palname = name();
            rc_ptr<APalette>      pal( new APalette( palname ) );

            pal->Volume<AimsRGBA>::operator = ( *cmap );

//             theAnatomist->palettes().push_back( pal );

            getOrCreatePalette();
            AObjectPalette  *opal = palette();
            if( opal )
            {
              opal->setRefPalette( pal );
              GLComponent *glc = glAPI();
              if( glc && glc->glNumTextures() != 0 )
              {
                const GLComponent::TexExtrema & te = glc->glTexExtrema( 0 );
                float vmin = te.minquant[0], vmax = te.maxquant[0];
                // ue max dynamics from volume and cmap
                map<int, Object> labels;
                cmap->header().getProperty( "labels", labels );
                if( !labels.empty() )
                {
                  vmin = labels.begin()->first;
                  vmax = labels.rbegin()->first;
                }
                else if( cmap->getSizeX() > vmax - vmin + 1 )
                  vmax = cmap->getSizeX() + vmin - 1;
                float den = te.maxquant[0] - te.minquant[0];
                if( den == 0. )
                  den = 1.;
                opal->setMin1( ( vmin - te.minquant[0] ) / den );
                opal->setMax1( ( vmax - te.minquant[0] + 0.99 ) / den );
              }
              else
              {
                opal->setMin1( 0. );
                opal->setMax1( pal->getSizeX() + 0.99 / pal->getSizeX() );
              }
              opal->setMin2( 0. );
              opal->setMax2( 1. );
              setPalette( *opal );

              // copy labels table, if any
              try
              {
                Object labels = cmap->header().getProperty( "labels" );
                pao->attributed()->setProperty( "labels", labels );
              }
              catch( ... )
              {
              }
            }
          }

          // palette
          try
            {
              m = o->getProperty( "palette" );
              getOrCreatePalette();
              m->setProperty( "image_directory",
                              FileUtil::dirname( fileName() ) );
              if( palette() && palette()->set( *m ) )
              {
                setPalette( *palette() );
                palette()->fill();
              }
            }
          catch( ... )
            {
            }

          // if labels are present, set the texture properties to labels mode,
          // using RGB interpolation, or volumeInterpolation to 0
          if( cmap.get() && cmap->getSizeX() > 1 )
          {
            if( dynamic_cast<SliceableObject *>( this ) )
            {
              // for volumes / sliceables, labels interpolation is disabled
              // in value space
              pao->attributed()->setProperty( "volumeInterpolation", 0 );
            }
            else
            {
              // for textures, on the contrary, use RGB interpolation
              // (instead of colormap space interpolation)
              try
              {
                Object tprop = Object::value( carto::ObjectVector() );
                try
                {
                  tprop = pao->attributed()->getProperty( "texture_properties" );
                }
                catch( ... )
                {
                }
                if( tprop->size() < 1 )
                  tprop->insertArrayItem( 0, Object::value( Dictionary() ) );
                Object t0prop = tprop->getArrayItem( 0 );
                t0prop->setProperty( "interpolation", "rgb" );
                pao->attributed()->setProperty( "texture_properties", tprop );
              }
              catch( ... )
              {
              }
            }
          }

          // texture properties
          if( g && g->glNumTextures() > 0 )
          {
            try
              {
                m = o->getProperty( "texture_properties" );
                unsigned	i, n = m->size();
                if( n > g->glNumTextures() )
                  n = g->glNumTextures();
                Object	iter;
                for( i=0, iter=m->objectIterator(); i<n && iter->isValid();
                     ++i, iter->next() )
                {
                  Object	t = iter->currentValue();
                  if( !t.isNone() )
                  {
                    try
                    {
                        Object	tp = t->getProperty( "mode" );
                        static map<string, GLComponent::glTextureMode> modes;
                        if( modes.empty() )
                          {
                            modes[ "geometric"        ]
                              = GLComponent::glGEOMETRIC;
                            modes[ "linear"           ]
                              = GLComponent::glLINEAR;
                            modes[ "replace"          ]
                              = GLComponent::glREPLACE;
                            modes[ "decal"            ]
                              = GLComponent::glDECAL;
                            modes[ "blend"            ]
                              = GLComponent::glBLEND;
                            modes[ "add"              ]
                              = GLComponent::glADD;
                            modes[ "combine"          ]
                              = GLComponent::glCOMBINE;
                            modes[ "linear_on_nonnul" ]
                              = GLComponent::glLINEAR_ON_DEFINED;
                          }
                        map<string, GLComponent::glTextureMode>::
                          const_iterator
                          im = modes.find( tp->getString() );
                        if( im != modes.end() )
                          g->glSetTexMode( im->second, i );
                    }
                    catch( ... )
                    {
                    }
                    try
                    {
                      Object	tp = t->getProperty( "filtering" );
                      static map<string, GLComponent::glTextureFiltering>
                        filters;
                      if( filters.empty() )
                        {
                          filters[ "nearest"        ]
                            = GLComponent::glFILT_NEAREST;
                          filters[ "linear"         ]
                            = GLComponent::glFILT_LINEAR;
                        }
                      map<string, GLComponent::glTextureFiltering>::
                        const_iterator
                        im = filters.find( tp->getString() );
                      if( im != filters.end() )
                        g->glSetTexFiltering( im->second, i );
                    }
                    catch( ... )
                    {
                    }
                    try
                    {
                      Object	tp = t->getProperty( "generation" );
                      static map<string, GLComponent::glAutoTexturingMode>
                        gens;
                      if( gens.empty() )
                        {
                          gens[ "none"           ]
                            = GLComponent::glTEX_MANUAL;
                          gens[ "object_linear"  ]
                            = GLComponent::glTEX_OBJECT_LINEAR;
                          gens[ "eye_linear"     ]
                            = GLComponent::glTEX_EYE_LINEAR;
                          gens[ "sphere_map"     ]
                            = GLComponent::glTEX_SPHERE_MAP;
                          gens[ "reflection_map" ]
                            = GLComponent::glTEX_REFLECTION_MAP;
                          gens[ "normal_map"     ]
                            = GLComponent::glTEX_NORMAL_MAP;
                        }
                      map<string, GLComponent::glAutoTexturingMode>::
                        const_iterator
                        im = gens.find( tp->getString() );
                      if( im != gens.end() )
                        g->glSetAutoTexMode( im->second, i );
                    }
                    catch( ... )
                    {
                    }
                    try
                    {
                      Object	tp = t->getProperty( "rate" );
                      g->glSetTexRate( tp->getScalar(), i );
                    }
                    catch( ... )
                    {
                    }
                    try
                    {
                      Object	tp = t->getProperty( "interpolation" );
                      static map<string, bool>	inters;
                      if( inters.empty() )
                        {
                          inters[ "palette" ] = false;
                          inters[ "rgb"     ] = true;
                        }
                      map<string, bool>::const_iterator
                        im = inters.find( tp->getString() );
                      if( im != inters.end() )
                        g->glSetTexRGBInterpolation( im->second, i );
                    }
                    catch( ... )
                    {
                    }
                    for( int p=0; p<3; ++p )
                      try
                      {
                        stringstream pname;
                        pname << "generation_params_" << p;
                        Object	tp = t->getProperty( pname.str() );
                        if( tp && tp->size() >= 4 )
                        {
                          vector<float> gpar( 4, 0. );
                          Object it = tp->objectIterator();
                          int k;
                          for( k=0; k<4 && it->isValid(); it->next(), ++k )
                            gpar[k] = float( it->currentValue()->getScalar() );
                          g->glSetAutoTexParams( &gpar[0], 0, i );
                        }
                      }
                      catch( ... )
                      {
                      }
                    try
                    {
                      Object	tp = t->getProperty( "wrapmode" );
                      static map<string, GLComponent::glTextureWrapMode>
                        wrapmodes;
                      if( wrapmodes.empty() )
                        {
                          wrapmodes[ "clamp_to_edge" ]
                            = GLComponent::glTEXWRAP_CLAMP_TO_EDGE;
                          wrapmodes[ "clamp_to_border" ]
                            = GLComponent::glTEXWRAP_CLAMP_TO_BORDER;
                          wrapmodes[ "repeat" ]
                            = GLComponent::glTEXWRAP_REPEAT;
                          wrapmodes[ "mirrored_repeat" ]
                            = GLComponent::glTEXWRAP_MIRRORED_REPEAT;
                          wrapmodes[ "mirror_clamp_to_edge" ]
                            = GLComponent::glTEXWRAP_MIRROR_CLAMP_TO_EDGE;
                        }
                      Object tpi = tp->objectIterator();
                      if( tpi.get() )
                      {
                        int k;
                        for( k=0; k<3 && tpi->isValid(); tpi->next(), ++k )
                        {
                          map<string, GLComponent::glTextureWrapMode>::
                            const_iterator
                            im = wrapmodes.find(
                              tpi->currentValue()->getString() );
                          if( im != wrapmodes.end() )
                            g->glSetTexWrapMode( im->second, k, i );
                        }
                      }
                    }
                    catch( ... )
                    {
                    }
                  }
                }
              }
            catch( ... )
              {
              }

            try
            {
              map<int, Object> palettes;
              if( o->getProperty( "_texture_palettes", palettes )
                  && palettes.size() != 0 )
              {
                map<int, Object>::const_iterator ip/*, ep = palettes.end()*/;
                ip=palettes.begin();
                rc_ptr<Volume<AimsRGBA> > teximage
                  = ip->second->value<rc_ptr<Volume<AimsRGBA> > >();

                /* We apply custom palettes only if
                   * it is a 2D texture palette
                   * we don't already have palette indications in the minf
                     header.

                  The thing is that applying these palettes will break
                  anatomist fancy palette dynamics settings since the saved
                  texture image is a static version of the palette system,
                  and is only suitable with range (min/max) settings of [0,1].

                  So if we can detect that the object has been saved by
                  anatomist with reliable 1D palette indications, we use
                  them, and not the texture image.
                */
                if( teximage->getSizeY() > 1 || !o->hasProperty( "palette" ) )
                {
                  _flipTexImage( teximage );

                  string palname = name();
                  rc_ptr<APalette>      pal( new APalette( palname ) );

                  pal->Volume<AimsRGBA>::operator = ( *teximage );

                  theAnatomist->palettes().push_back( pal );

                  getOrCreatePalette();
                  AObjectPalette  *opal = palette();
                  opal->setRefPalette( pal );
                  opal->setMin1( 0. );
                  opal->setMax1( 1. );
                  opal->setMin2( 0. );
                  opal->setMax2( 1. );
                  setPalette( *opal );
                }
              }
            }
            catch( ... )
            {
            }
          }
        }
    }
}


void AObject::storeHeaderOptions()
{
  PythonAObject	*pao = dynamic_cast<PythonAObject *>( this );
  if( pao )
  {
    GenericObject	*o = pao->attributed();

    if( o )
    {
      Object options = makeHeaderOptions();
      o->copyProperties( options );
      setChanged();
    }
  }
}


Object AObject::makeHeaderOptions() const
{
  Object o = Object::value( Dictionary() );

  // material
  o->setProperty( "material", material().genericDescription() );
  // palette
  const AObjectPalette	*pal = palette();
  if( pal )
    o->setProperty( "palette", pal->genericDescription() );

  // texture params
  const GLComponent	*g = glAPI();
  if( g && g->glNumTextures() > 0 )
  {
    Object	tp = Object::value( carto::ObjectVector() );

    unsigned	i, n = g->glNumTextures();
    for( i=0; i<n; ++i )
    {
      Object	tpx = Object::value( Dictionary() );
      static string	mode[] = { "geometric", "linear", "replace",
                                "decal", "blend", "add",
                                "combine" };
      static string filt[] = { "nearest", "linear" };
      static string gene[] = { "none", "object_linear",
                                "eye_linear", "sphere_map",
                                "reflection_map", "normal_map" };
      static string inter[] = { "palette", "rgb" };
      tpx->setProperty( "mode", mode[ g->glTexMode( i ) ] );
      tpx->setProperty( "filtering",
                        filt[ g->glTexFiltering( i ) ] );
      tpx->setProperty( "generation",
                        gene[ g->glAutoTexMode( i ) ] );
      tpx->setProperty( "rate", g->glTexRate( i ) );
      tpx->setProperty( "interpolation",
                        inter[ g->glTexRGBInterpolation( i ) ] );
      const float	*x = g->glAutoTexParams( 0, i );
      vector<float>	y(4);
      if( x )
      {
        y[0] = x[0];
        y[1] = x[1];
        y[2] = x[2];
        y[3] = x[3];
        tpx->setProperty( "generation_params_1", y );
      }
      x = g->glAutoTexParams( 1, i );
      if( x )
      {
        y[0] = x[0];
        y[1] = x[1];
        y[2] = x[2];
        y[3] = x[3];
        tpx->setProperty( "generation_params_2", y );
      }
      x = g->glAutoTexParams( 2, i );
      if( x )
      {
        y[0] = x[0];
        y[1] = x[1];
        y[2] = x[2];
        y[3] = x[3];
        tpx->setProperty( "generation_params_3", y );
      }
      tp->insertArrayItem( -1, tpx );
    }

    o->setProperty( "texture_properties", tp );
  }

  return o;
}


void AObject::update( const Observable* obs, void* )
{
  if( obs == d->inheritref )
  {
    const AObject  *o = dynamic_cast<const AObject *>( obs );
    assert( o != NULL );
    if( o->hasReferenceChanged() )
    {
      clearReferentialInheritance();
      setReferentialInheritance( const_cast<AObject*>( o ) );
    }
  }
}


void AObject::unregisterObservable( Observable* obs )
{
  if( obs == d->inheritref )
  {
    d->inheritref = 0;
    setReferenceChanged();
    if( theAnatomist->getControlWindow() != 0 )
      theAnatomist->getControlWindow()->NotifyObjectChange(this);
  }
  Observer::unregisterObservable( obs );
}


long AObject::loadDate() const
{
  return d->loaddate;
}


void AObject::setLoadDate( long t )
{
  d->loaddate = t;
}


void AObject::setInternalsChanged()
{
}


bool AObject::renderingIsObserverDependent() const
{
  return false;
}


AObject* AObject::clone( bool )
{
  return 0;
}


bool AObject::isCopy() const
{
  return d->copy;
}


void AObject::setCopyFlag( bool x )
{
  d->copy = x;
}


Tree* AObject::optionTree() const
{
  ObjectMenu *om = optionMenu();
  if (om)   return om->tree();
  else      return NULL;
}


ObjectMenu* AObject::optionMenu() const
{
  std::string   id = objectFullTypeName();
  std::map<std::string, rc_ptr<ObjectMenu> >::const_iterator
  i = _objectmenu_map.find(id);

  rc_ptr<ObjectMenu> om( 0 );
  if( i != _objectmenu_map.end() )
    om = i->second;
  if( om.isNull() )
    return 0;

  // register callbacks that have not been yet on this type
  Private::MenuRegistrersMap & regmap = Private::objectMenuRegistrers();
  Private::MenuRegistrersMap::iterator ir, er = regmap.end();
  for( ir=regmap.begin(); ir!=er; ++ir )
  {
    bool done = ( ir->second.find( id ) != ir->second.end() );
    if( !done )
    {
      ir->second.insert( id );
      om.reset( ir->first->doit( this, om.get() ) );
      if( om && i == _objectmenu_map.end() )
        _objectmenu_map[ id ] = om;
    }
  }

  return om.get();
}


void AObject::addObjectMenuRegistration( ObjectMenuRegistrerFunction f )
{
  addObjectMenuRegistration( new ObjectMenuRegistrerFuncClass( f ) );
}

void AObject::addObjectMenuRegistration( ObjectMenuRegistrerClass * f )
{
  Private::objectMenuRegistrers()[ rc_ptr<ObjectMenuRegistrerClass>( f ) ];
}


namespace
{

  template <int D, typename T>
  Object makeMesh( const AObject* ao, const GLComponent *gl )
  {
    AimsTimeSurface<D, T>* surf = new AimsTimeSurface<D, T>;
    Object meshobj = Object::value( rc_ptr<AimsTimeSurface<D, T> >( surf ) );

    int timestep = 0;
    ViewState state( timestep );

    AimsSurface<D, T> & s0 = (*surf)[timestep];

    vector<Point3df> & vert = s0.vertex();
    vector<Point3df> & norm = s0.normal();
    vector<AimsVector<uint, D> > & poly = s0.polygon();

    PythonHeader & ph = surf->header();

    unsigned i, nv = gl->glNumVertex( state );
    const GLfloat* glvert = gl->glVertexArray( state );
    vert.resize( nv );

    for( i=0; i<nv; ++i )
    {
      vert[i][0] = *glvert++;
      vert[i][1] = *glvert++;
      vert[i][2] = *glvert++;
    }

    const GLfloat* glnorm = gl->glNormalArray( state );
    if( glnorm != 0 )
    {
      norm.resize( nv );

      for( i=0; i<nv; ++i )
      {
        norm[i][0] = *glnorm++;
        norm[i][1] = *glnorm++;
        norm[i][2] = *glnorm++;
      }
    }

    unsigned j, np = gl->glNumPolygon( state );
    const GLuint* glpoly = gl->glPolygonArray( state );
    poly.resize( np );

    for( i=0; i<np; ++i )
    {
      AimsVector<uint, D> & pol = poly[i];
      for( j=0; j<D; ++j )
        pol[j] = *glpoly++;
    }

    unsigned tex = 0;
    unsigned nt = gl->glTexCoordSize( state, tex );
    if( nt != 0 )
    {
      // palette
      //int nsteps = 256;
      if( ao->palette() )
      {
        const rc_ptr<Volume<AimsRGBA> > teximage
          = gl->glBuildTexImage( state, tex, -1, -1, false );
        _flipTexImage( teximage );
        std::map<int, Object> pal_list;
        ph.getProperty( "_texture_palettes", pal_list );
        pal_list[timestep] = Object::value( teximage );
        ph.setProperty( "_texture_palettes", pal_list );
        //nsteps = teximage->getSizeX();
      }

      const GLfloat* gltex = gl->glTexCoordArray( state, tex );
      const T* ttex = reinterpret_cast<const T *>( gltex );
      vector<T> & texture = s0.texture();
      texture.resize( nt );

      // rescale texture
//       const GLComponent::TexInfo & t = gl->glTexInfo( tex );

      //float m = 1. / ( nsteps * 2 );
      for( i=0; i<nt; ++i )
      {
        texture[i] = *ttex++;
//         texture[i] = (T) ( ( *gltex++ + t.texoffset[0] ) * t.texscale[0] ); // FIXME: use object-space scaling
      }

    }

    const PythonAObject *pao = dynamic_cast<const PythonAObject *>( ao );
    if( pao )
      ph.copyProperties( Object::reference( *pao->attributed() ) );

    ph.setProperty( "object_type",
                    DataTypeCode< AimsTimeSurface<D, T> >::objectType() );
    ph.setProperty( "data_type",
                    DataTypeCode< AimsTimeSurface<D, T> >::dataType() );
    Object opts = ao->makeHeaderOptions();
    ph.copyProperties( opts );
//     ph.setProperty( "material", gl->glMaterial()->genericDescription() );
//     const AObjectPalette *pal = gl->glPalette();
//     if( pal )
//       ph.setProperty( "palette", pal->genericDescription() );

    return meshobj;
  }


  template <int D, typename T>
  void _write_mesh( Object meshobj, const string & filename )
  {
    rc_ptr< AimsTimeSurface<D, T> > mesh
      = meshobj->value< rc_ptr< AimsTimeSurface<D, T> > >();
    string dt;
    mesh->header().getProperty( "data_type", dt );
    Writer<AimsTimeSurface<D, T> > w( filename );
    w.write( *mesh );
  }


  void saveMesh( Object meshobj, const string & filename )
  {
    string mtype = meshobj->type();

    if( mtype == "rc_ptr of Mesh of VOID" )
      _write_mesh<3, Void>( meshobj, filename );
    else if( mtype == "rc_ptr of Mesh of FLOAT" )
      _write_mesh<3, float>( meshobj, filename );
    else if( mtype == "rc_ptr of Mesh of POINT2DF" )
      _write_mesh<3, Point2df>( meshobj, filename );
    else if( mtype == "rc_ptr of Segments of VOID" )
      _write_mesh<2, Void>( meshobj, filename );
    else if( mtype == "rc_ptr of Segments of FLOAT" )
      _write_mesh<2, float>( meshobj, filename );
    else if( mtype == "rc_ptr of Segments of POINT2DF" )
      _write_mesh<2, Point2df>( meshobj, filename );
    else if( mtype == "rc_ptr of Mesh4 of VOID" )
      _write_mesh<4, Void>( meshobj, filename );
    else if( mtype == "rc_ptr of Mesh4 of FLOAT" )
      _write_mesh<4, float>( meshobj, filename );
    else if( mtype == "rc_ptr of Mesh4 of POINT2DF" )
      _write_mesh<4, Point2df>( meshobj, filename );
    else
      cerr << "Cannot save mesh of type " << mtype << endl;
  }

}


carto::Object AObject::aimsMeshFromGLComponent()
{
  Object meshobj;

  const GLComponent *gl = glAPI();
  if( gl )
  {
    ViewState state( 0 );

    if( gl->glNumVertex( state ) != 0 )
    {
      int poly_size = gl->glPolygonSize( state );

      if( gl->glNumTextures( state ) == 0 )
      {
        // mesh only
        switch( poly_size )
        {
          case 2:
            meshobj = makeMesh<2, Void>( this, gl );
            break;
          case 3:
            meshobj = makeMesh<3, Void>( this, gl );
            break;
          case 4:
            meshobj = makeMesh<4, Void>( this, gl );
            break;
          default:
            break;
        }
      }
      else
      {
        // textured mesh
        int texdim = gl->glDimTex( state, 0 );

        if( texdim == 2 )
        {
          switch( poly_size )
          {
            case 2:
              meshobj = makeMesh<2, Point2df>( this, gl );
              break;
            case 3:
              meshobj = makeMesh<3, Point2df>( this, gl );
              break;
            case 4:
              meshobj = makeMesh<4, Point2df>( this, gl );
              break;
            default:
              break;
          }
        }
        else
        {
          switch( poly_size )
          {
            case 2:
              meshobj = makeMesh<2, float>( this, gl );
              break;
            case 3:
              meshobj = makeMesh<3, float>( this, gl );
              break;
            case 4:
              meshobj = makeMesh<4, float>( this, gl );
              break;
            default:
              break;
          }
        }
      }
    }
  }

  return meshobj;
}

bool AObject::save( const std::string & filename )
{
  if( filename.substr( filename.length() - 5, 5 ) == ".aobj" )
    if( MObjectIO::writeMObject( Object::value( this ), filename ) )
      return true;

  Object meshobj = aimsMeshFromGLComponent();
  if( meshobj )
  {
    saveMesh( meshobj, filename );
    return true;
  }
  return false;
}


string AObject::toolTip() const
{
  return string();
}


bool AObject::userModified() const
{
  return d->userModified;
}


void AObject::setUserModified( bool state )
{
  d->userModified = state;
}


bool AObject::allowsOverwriteOnSave() const
{
  return d->allowsOverwriteOnSave;
}


void AObject::setAllowsOverwriteOnSave( bool x )
{
  d->allowsOverwriteOnSave = x;
}


bool AObject::save( const std::string & filename, bool onlyIfModified )
{
  if( onlyIfModified && !userModified() )
    return true;
  bool res = save( filename );
  if( res )
  {
    setUserModified( false );
    setAllowsOverwriteOnSave( true );
  }
  return res;
}


void AObject::adjustPalette()
{
  getOrCreatePalette();
  AObjectPalette *pal = palette();
  if( !pal )
    return;
  pal->setMin1( 0. );
  pal->setMax1( 1. );
  pal->setMin2( 0. );
  pal->setMax2( 1. );
}

// ----

AObject::Private::MenuRegistrersMap &
AObject::Private::objectMenuRegistrers()
{
  static MenuRegistrersMap reg;
  return reg;
}

}


#include <cartobase/object/object_d.h>
INSTANTIATE_GENERIC_OBJECT_TYPE( AObject * )
INSTANTIATE_GENERIC_OBJECT_TYPE( set<AObject *> )
INSTANTIATE_GENERIC_OBJECT_TYPE( vector<AObject *> )
INSTANTIATE_GENERIC_OBJECT_TYPE( list<AObject *> )
INSTANTIATE_GENERIC_OBJECT_TYPE( carto::shared_ptr<AObject> )


