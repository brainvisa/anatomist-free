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
#include <anatomist/graph/pythonAObject.h>
#include <anatomist/window/viewstate.h>
#include <aims/resampling/quaternion.h>
#include <time.h>

//#define ANA_DEBUG
#ifdef ANA_DEBUG
#include <typeinfo>
#endif

using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;


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

  PrimList		sharedprim;
  bool		cleanupdone;
  // texture stuff should be in GLComponent but...
  TextureMode		texturemode;
  TextureFiltering	texturefilter;
  Referential		*oldref;
  time_t		loaddate;
  bool                  copy;

  AObject               *inheritref;
  // bool                  cycling;
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
    oldref( 0 ), loaddate( time( 0 ) ), copy( false ), inheritref( 0 )
{
}


AObject::Private::Private( const Private & x )
  : cleanupdone( false ), texturemode( x.texturemode ),
    texturefilter( x.texturefilter ),
    oldref( 0 ), loaddate( x.loaddate ), inheritref( x.inheritref )
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
  : Observable(), Observer(), _type( x._type ), _id( 0 ), _name( x._name ),
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


float AObject::TimeStep() const
{
  return 1.;
}


Point3df AObject::VoxelSize() const
{
  return Point3df( 1, 1, 1 );
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
      _material = mat;
      if( glAPI() )
        glAPI()->glSetChanged( GLComponent::glMATERIAL );
      else
        setChanged();
    }
}


void AObject::clearHasChangedFlags() const
{
  _referenceHasChanged = false;
}


AObject* AObject::ObjectAt( float x, float y, float z, float t, float tol, 
			    const Referential* orgref, const Point3df & )
{
  if( !orgref || !getReferential() )
    return( ObjectAt( x, y, z, t, tol ) );

  const Transformation	*tra 
    = theAnatomist->getTransformation( orgref, getReferential() );
  if( !tra )
    return( ObjectAt( x, y, z, t, tol ) );

  Point3df	tp = tra->transform( Point3df( x, y, z ) );

  return( ObjectAt( tp[0], tp[1], tp[2], t, tol ) );
}


AObject* AObject::ObjectAt( float x, float y, float z, float t, float tol )
{
  if( tol < 0 ) tol *= -1;

  //	default behaviour is really primitive...
  Point3df	bmin, bmax;
  if( boundingBox( bmin, bmax ) 
      && t>=MinT() && t<=MaxT() && x>=bmin[0]-tol && x<=bmax[0]+tol 
      && y>=bmin[1]-tol && y<=bmax[1]+tol && z>=bmin[2]-tol && z<=bmax[2]+tol )
    return this ;
  else
    return 0;
}


AObject* AObject::load( const string & filename )
{
  ObjectReader::PostRegisterList subobjects;
  return( ObjectReader::reader()->load( filename, subobjects ) );
}


bool AObject::reload( AObject* object, bool onlyoutdated )
{
  return( ObjectReader::reader()->reload( object, onlyoutdated ) );
}


bool AObject::reload( const string & )
{
  return( false );
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


float AObject::mixedTexValue( const Point3df & pos, float time, 
			      const Referential* orgRef, 
			      const Point3df & orgVoxSz ) const
{
  Transformation	*trans 
    = theAnatomist->getTransformation( orgRef, getReferential() );
  Point3df		pt;

  if( trans )
    pt = Transformation::transform( pos, trans, orgVoxSz, VoxelSize() );
  else
    pt = Transformation::transform( pos, orgVoxSz, VoxelSize() );
  return( mixedTexValue( pt, time ) );
}


vector<float> AObject::texValues( const Point3df & pos, float time, 
				  const Referential* orgRef, 
				  const Point3df & orgVoxSz ) const
{
  Transformation	*trans 
    = theAnatomist->getTransformation( orgRef, getReferential() );
  Point3df		pt;

  if( trans )
    pt = Transformation::transform( pos, trans, orgVoxSz, VoxelSize() );
  else
    pt = Transformation::transform( pos, orgVoxSz, VoxelSize() );
  //cout << "AObject::texValues, point org: " << pos << ", dst: " << pt 
  //     << endl;
  return( texValues( pt, time ) );
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
      (*current)->erase(this) ;
    }

    // TEMPORARY until SelectFactory is an observer of this
    SelectFactory			*sf = SelectFactory::factory();
    map<unsigned, set<AObject *> >	sel = sf->selected();
    map<unsigned, set<AObject *> >::iterator	is, es = sel.end();
    set<AObject *>			so;
    so.insert( this );
    for( is=sel.begin(); is!=es; ++is )
      sf->unselect( is->first, so );
  }
}


bool AObject::boundingBox( Point3df & bmin, Point3df & bmax ) const
{
  bmin = Point3df( 0, 0, 0 );
  bmax = Point3df( 0, 0, 0 );
  return( false );
}


bool AObject::boundingBox2D( Point3df & bmin, Point3df & bmax ) const
{
  Point3df vs = VoxelSize();
  bmin = Point3df( MinX2D() * vs[0], MinY2D() * vs[1], MinZ2D() * vs[2] );
  bmax = Point3df( MaxX2D() * vs[0], MaxY2D() * vs[1], MaxZ2D() * vs[2] );
  return true;
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


void AObject::setHeaderOptions()
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
            setReferential( theAnatomist->centralReferential() );

          // material
          GLComponent	*g = glAPI();
          try
            {
              m = o->getProperty( "material" );
              GetMaterial().set( *m );
              if( g )
                g->glSetChanged( GLComponent::glMATERIAL );
              else
                setChanged();
            }
          catch( ... )
            {
            }

          // palette
          try
            {
              m = o->getProperty( "palette" );
              getOrCreatePalette();
              if( palette() && palette()->set( *m ) )
                {
                  setPalette( *palette() );
                  palette()->fill();
                }
            }
          catch( ... )
            {
            }

          // texture properties
          if( g && g->glNumTextures() > 0 )
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
                    try
                      {
                        Object	tp = t->getProperty( "generation_params_1" );
                        //gc->glSetAutoTexParams( &_genparams_1[0], 0, i );
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
    }
}


void AObject::storeHeaderOptions()
{
  PythonAObject	*pao = dynamic_cast<PythonAObject *>( this );
  if( pao )
    {
      GenericObject	*o = pao->attributed();
      Object	m;

      if( o )
        {
          // material
          o->setProperty( "material", GetMaterial().genericDescription() );
          // palette
          AObjectPalette	*pal = palette();
          if( pal )
            o->setProperty( "palette", pal->genericDescription() );

          // texture params
          GLComponent	*g = glAPI();
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

          setChanged();
        }
    }
}


void AObject::update( const Observable* obs, void* )
{
  if( obs == d->inheritref )
  {
    AObject  *o = const_cast<AObject *>( (const AObject *) obs );
    if( o->hasReferenceChanged() )
    {
      clearReferentialInheritance();
      setReferentialInheritance( o );
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

// ----

AObject::Private::MenuRegistrersMap &
AObject::Private::objectMenuRegistrers()
{
  static MenuRegistrersMap reg;
  return reg;
}


#include <cartobase/object/object_d.h>
INSTANTIATE_GENERIC_OBJECT_TYPE( AObject * )
INSTANTIATE_GENERIC_OBJECT_TYPE( set<AObject *> )
INSTANTIATE_GENERIC_OBJECT_TYPE( vector<AObject *> )
INSTANTIATE_GENERIC_OBJECT_TYPE( list<AObject *> )
INSTANTIATE_GENERIC_OBJECT_TYPE( shared_ptr<AObject> )


