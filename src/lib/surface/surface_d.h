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

#ifndef ANA_SURFACE_SURFACE_D_H
#define ANA_SURFACE_SURFACE_D_H


#include <cstdlib>
#include <anatomist/surface/surface.h>
#include <anatomist/application/Anatomist.h> 
#include <anatomist/window/Window.h>
#include <anatomist/window/viewstate.h>
#include <anatomist/object/actions.h>
#include <anatomist/primitive/primitive.h>
#include <graph/tree/tree.h>
#include <aims/mesh/surfaceOperation.h>
#include <aims/io/writer.h>
#include <aims/io/reader.h>
#include <qtranslator.h>

namespace
{
  void generateTexture1D( const std::set<anatomist::AObject *> & obj );
  void generateTexture2D( const std::set<anatomist::AObject *> & obj );

  template <int D> inline
  void _completeMeshDirections( AimsTimeSurface<D, Void> & )
  {
  }


  template <> inline
  void _completeMeshDirections( AimsTimeSurface<2, Void> & surf )
  {
    if( surf.normal().empty() )
    {
      try
      {
        carto::Object mat = surf.header().getProperty( "material" );
        carto::Object cnorm = mat->getProperty( "shader_color_normals" );
        if( cnorm->getScalar() )
        {
          carto::Object nisd = mat->getProperty( "normal_is_direction" );
          if( nisd->getScalar() )
          {
            std::cout << "normal_is_direction ON\n";
            std::vector<AimsVector<float, 3> > *dirs
              = aims::SurfaceManip::lineDirections( surf );
            std::cout << "generate dirs: " << dirs->size() << std::endl;
            surf.normal() = *dirs;
            delete dirs;
          }
        }
      }
      catch( ... )
      {
      }
    }
  }


}


namespace anatomist
{

  template<int D>
  ASurface<D>::ASurface( const char * )
    : AGLObject(), _surface( 0 ), _planar( false )
  {
    _type = AObject::TRIANG;
  }

template<int D>
bool ASurface<D>::render( anatomist::PrimList& primitiveList,
                                   const anatomist::ViewState& viewState )
{
	return anatomist::AObject::render( primitiveList, viewState );
}


template<int D>
bool ASurface<D>::glMakeBodyGLL( const anatomist::ViewState& viewState,
                                     const anatomist::GLList& glList ) const
{
	return AGLObject::glMakeBodyGLL(viewState, glList);
}

  template<int D>
  ASurface<D>::~ASurface()
  {
    cleanup();
    freeSurface();
  }


  template <int D>
  AObject* ASurface<D>::clone( bool shallow )
  {
    ASurface<D> *s = new ASurface<D>;
    s->setFileName( fileName() );
    if( shallow )
      s->setSurface( surface() );
    else
      s->setSurface( new TimeSurfaceType( *surface() ) );
    return s;
  }


  template<int D>
  void ASurface<D>::freeSurface()
  {
    _surface.reset( 0 );
  }


  template<int D>
  const typename ASurface<D>::SurfaceType* 
  ASurface<D>::surfaceOfTime( float time ) const
  {
    if( !_surface )
      return 0;

    unsigned	t = (unsigned) ( time / TimeStep() );
    typename TimeSurfaceType::const_iterator is = _surface->lower_bound( t );

    if( is == _surface->end() )
    {
      if( is != _surface->begin() )
        --is;
    }
    return( &(*is).second );
  }


  template<int D>
  typename ASurface<D>::SurfaceType* ASurface<D>::surfaceOfTime( float time )
  {
    if( !_surface )
      return 0;

    unsigned	t = (unsigned) ( time / TimeStep() );

    if( _surface->empty() )
      return( &(*_surface)[t] );

    typename TimeSurfaceType::iterator	is = _surface->lower_bound( t );

    if( is == _surface->end() )
    {
      if( is != _surface->begin() )
        --is;
    }
    return( &(*is).second );
  }


  template<int D>
  void ASurface<D>::UpdateMinAndMax()
  {
    _surface->setMini();
    _surface->setMaxi();
    _planar = true;

    const std::vector<Point3df>	& norm = _surface->normal();
    std::vector<Point3df>::size_type i, n = norm.size();
    if( n < _surface->vertex().size() )
      _surface->updateNormals();
    if( n < 2 )
      return;
    Point3df nref = norm[0];
    const float eps = 1e-5;

    for( i=1; i<n; ++i )
      if( fabs( norm[i][0] - nref[0] ) > eps 
          || fabs( norm[i][1] - nref[1] ) > eps 
          || fabs( norm[i][2] - nref[2] ) > eps )
        {
          _planar = false;
          break;
        }
    //cout << name() << " - planar: " << _planar << endl;
  }


  template<int D>
  float ASurface<D>::MaxT() const
  {
    if( !_surface || _surface->size() == 0 )
      return( 0. );
    return( TimeStep() * (*_surface->rbegin()).first );
  }


  template<int D>
  float ASurface<D>::MinT() const
  {
    if( !_surface || _surface->size() == 0 )
      return( 0. );
    return( TimeStep() * (*_surface->begin()).first );
  }


  template<int D>
  void ASurface<D>::setSurface( carto::rc_ptr<TimeSurfaceType> surf )
  {
    bool hasold = _surface.get() != 0;
    freeSurface();
    _surface = surf;
    UpdateMinAndMax();
    _completeMeshDirections( *surf );
    if( D == 2 && !_surface->normal().empty() )
    {
      GetMaterial().setRenderProperty( Material::UseShader, 1 );
      GetMaterial().setRenderProperty( Material::NormalIsDirection, 1 );
      setupShader();
    }
    try
    {
      carto::Object omat = _surface->header().getProperty( "material" );
      Material & mat = GetMaterial();
      mat.set( *omat );
      SetMaterial( mat );
    }
    catch( ... )
    {
    }
    glSetChanged( glGEOMETRY );
    setChanged();
    if( !hasold )
      setHeaderOptions();
  }


  template<int D>
  void ASurface<D>::setSurface( TimeSurfaceType* surf )
  {
    bool hasold = _surface.get() != 0;
    freeSurface();
    _surface.reset( surf );
    UpdateMinAndMax();
    _completeMeshDirections( *surf );
    if( D == 2 && !_surface->normal().empty() )
    {
      GetMaterial().setRenderProperty( Material::UseShader, 1 );
      GetMaterial().setRenderProperty( Material::NormalIsDirection, 1 );
      setupShader();
    }
    try
    {
      carto::Object omat = _surface->header().getProperty( "material" );
      Material & mat = GetMaterial();
      mat.set( *omat );
      SetMaterial( mat );
    }
    catch( ... )
    {
    }
    glSetChanged( glGEOMETRY );
    setChanged();
    if( !hasold )
      setHeaderOptions();
  }


  template<int D>
  unsigned ASurface<D>::glNumVertex( const ViewState & s ) const
  {
    const SurfaceType	*surf = surfaceOfTime( s.timedims[0] );

    if( !surf )
      return 0;
    return surf->vertex().size();
  }


  template<int D>
  const GLfloat* ASurface<D>::glVertexArray( const ViewState & s ) const
  {
    const SurfaceType	*surf = surfaceOfTime( s.timedims[0] );

    if( !surf )
      return 0;
    return &surf->vertex()[0][0];
  }


  template<int D>
  unsigned ASurface<D>::glNumNormal( const ViewState & s ) const
  {
    const SurfaceType	*surf = surfaceOfTime( s.timedims[0] );

    if( !surf )
      return 0;
    return surf->normal().size();
  }


  template<int D>
  const GLfloat* ASurface<D>::glNormalArray( const ViewState & s ) const
  {
    const SurfaceType	*surf = surfaceOfTime( s.timedims[0] );

    if( !surf )
      return( 0 );
    const std::vector<Point3df> & n = surf->normal();
    if( n.size() < surf->vertex().size() )
      return 0;
    return( &n[0][0] );
  }


  template<int D>
  const GLuint* ASurface<D>::glPolygonArray( const ViewState & s ) const
  {
    const SurfaceType	*surf = surfaceOfTime( s.timedims[0] );

    if( !surf )
      return( 0 );
    return( (GLuint *) &surf->polygon()[0][0] );
  }


  template<int D>
  unsigned ASurface<D>::glNumPolygon( const ViewState & s ) const
  {
    const SurfaceType	*surf = surfaceOfTime( s.timedims[0] );

    if( !surf )
      return( 0 );
    return( surf->polygon().size() );
  }


  template<int D>
  Tree* ASurface<D>::optionTree() const
  {
    static Tree*	_optionTree = 0;

    if( !_optionTree )
      {
        Tree	*t, *t2;
        _optionTree = new Tree( true, "option tree" );
        t = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "File" ) );
        _optionTree->insert( t );
        t2 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "Reload" ) );
        t2->setProperty( "callback", &ObjectActions::fileReload );
        t->insert( t2 );
        t2 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "Save" ) );
        t2->setProperty( "callback", &ObjectActions::saveStatic );
        t->insert( t2 );
        t2 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu",
                                                "Rename object" ) );
        t2->setProperty( "callback", &ObjectActions::renameObject );
        t->insert( t2 );
        t2 = new Tree( true,
                       QT_TRANSLATE_NOOP( "QSelectMenu",
                                          "Create generated 1D texture" ) );
        t2->setProperty( "callback", &ObjectActions::generateTexture1D );
        t->insert( t2 );
        t2 = new Tree( true,
                       QT_TRANSLATE_NOOP( "QSelectMenu",
                                          "Create generated 2D texture" ) );
        t2->setProperty( "callback", &ObjectActions::generateTexture2D );
        t->insert( t2 );

        t = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "Color" ) );
        _optionTree->insert( t );
        t2 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "Material" ) );
        t2->setProperty( "callback", &ObjectActions::colorMaterial );
        t->insert( t2 );
        t2 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "Rendering" ) );
        t2->setProperty( "callback", &ObjectActions::colorRendering);
        t->insert( t2 );
        t = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu",
                                               "Referential" ) );
        _optionTree->insert( t );
        t2 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "Load" ) );
        t2->setProperty( "callback", &ObjectActions::referentialLoad );
        t->insert( t2 );
        t2 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu",
                       "Load information from file header") );
        t2->setProperty( "callback", &ObjectActions::setAutomaticReferential );
        t->insert( t2 );
        t = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "Geometry" ) );
        _optionTree->insert( t );
        t2 = new Tree( true,
                       QT_TRANSLATE_NOOP( "QSelectMenu",
                                          "Invert polygon orientation" ) );
        t2->setProperty( "callback", &invertPolygonsStatic );
        t->insert( t2 );
        setObjectMenu( objectFullTypeName(),
                       carto::rc_ptr<ObjectMenu>
                           ( new ObjectMenu( *_optionTree )  ));
      }
    return AObject::optionTree();
  }


  template<int D>
  AObject* ASurface<D>::ObjectAt( float, float, float, float, 
				  float )
  {
    return( 0 );
  }


  template<int D>
  bool ASurface<D>::save( const std::string & filename )
  {
    if( !_surface )
      return false;

    try
      {
        storeHeaderOptions();
        aims::Writer<TimeSurfaceType>	sw( filename );
        sw << *_surface;
      }
    catch( std::exception & e )
      {
        std::cerr << e.what() << "\nsave aborted\n";
        return false;
      }
    return true;
  }


  template<int D>
  bool ASurface<D>::reload( const std::string & filename )
  {
    aims::Reader<TimeSurfaceType>	reader( filename );
    carto::rc_ptr<TimeSurfaceType>	obj( new TimeSurfaceType );
    if( !reader.read( *obj ) )
      return false;

    setSurface( obj );
    return true;
  }


  template<int D>
  void ASurface<D>::invertPolygonsStatic( const std::set<AObject *> & obj )
  {
    std::set<AObject *>::const_iterator	io, fo=obj.end();

    for( io=obj.begin(); io!=fo; ++io )
      ((ASurface<D> *) *io)->invertPolygons();
  }


  template<int D>
  void ASurface<D>::invertPolygons()
  {
    aims::SurfaceManip::invertSurfacePolygons( *_surface );
    glSetChanged( glGEOMETRY );
    setChanged();
    notifyObservers( this );
  }


  template<int D>
  bool ASurface<D>::boundingBox( std::vector<float> & bmin,
                                 std::vector<float> & bmax ) const
  {
    if( !_surface )
      return( false );

    Point3df bminp = _surface->minimum();
    Point3df bmaxp = _surface->maximum();
    if( bminp == Point3df( 1e38, 1e38, 1e38 )
        && bmaxp == Point3df( -1e38, -1e38, -1e38) )
      // emty meshes return a crappy bounding box...
      return false;
    bmin.resize( 4 );
    bmax.resize( 4 );
    bmin[0] = bminp[0];
    bmin[1] = bminp[1];
    bmin[2] = bminp[2];
    bmax[0] = bmaxp[0];
    bmax[1] = bmaxp[1];
    bmax[2] = bmaxp[2];
    if( _surface->empty() )
    {
      bmin[3] = 0.f;
      bmax[3] = 0.f;
    }
    else
    {
      bmin[3] = _surface->begin()->first * TimeStep();
      bmax[3] = _surface->rbegin()->first * TimeStep();
    }
    return true;
  }


  template<int D>
  float ASurface<D>::actualTime( float time ) const
  {
    if( !_surface )
      return( 0 );

    unsigned	t = (unsigned) ( time / TimeStep() );
    typename TimeSurfaceType::const_iterator is = _surface->lower_bound( t );

    if( is == _surface->end() )
      is = _surface->begin();
    return( is->first * TimeStep() );
  }


  template<int D>
  void ASurface<D>::notifyObservers( void * arg )
  {
    AGLObject::notifyObservers( arg );
    glSetChanged( glGEOMETRY, false );
  }


  template<int D>
  unsigned ASurface<D>::glPolygonSize( const ViewState & ) const
  {
    return D;
  }


  template<int D> 
  carto::GenericObject *ASurface<D>::attributed()
  {
    if( _surface )
      {
        aims::PythonHeader	*ah 
          = dynamic_cast<aims::PythonHeader *>( &_surface->header() );
        if( ah )
          return ah;
      }
    return 0;
  }


  template<int D> 
  const carto::GenericObject *ASurface<D>::attributed() const
  {
    if( _surface )
      {
        const aims::PythonHeader	*ah 
          = dynamic_cast<const aims::PythonHeader *>( &_surface->header() );
        if( ah )
          return ah;
      }
    return 0;
  }


  template <int D>
  void ASurface<D>::setInternalsChanged()
  {
    UpdateMinAndMax();
    glSetChanged( glGEOMETRY );
    setChanged();
    setHeaderOptions();
  }


}


#endif
