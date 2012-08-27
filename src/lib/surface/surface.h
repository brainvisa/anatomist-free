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

#ifndef ANA_SURFACE_SURFACE_H
#define ANA_SURFACE_SURFACE_H


#include <anatomist/surface/globject.h>
#include <anatomist/color/Material.h>
#include <aims/mesh/surface.h>
#include <anatomist/graph/pythonAObject.h>

/*
class QGLShaderProgram;
class QGLShader;*/


namespace anatomist
{

  /**	Template surface type
   */
  template<int D>
  class ASurface : public AGLObject, public PythonAObject
  {
  public:

    typedef AimsVector<uint,D>	Triangle;
    typedef AimsSurface<D,Void> SurfaceType;
    typedef AimsTimeSurface<D,Void> TimeSurfaceType;

    ASurface( const char *filename = "" );
    ~ASurface();

    virtual AObject* clone( bool shallow = true );

    virtual Tree* optionTree() const;

    virtual void UpdateMinAndMax();

    virtual bool render( anatomist::PrimList& primitiveList,
                                   const anatomist::ViewState& viewState );
    virtual bool glMakeBodyGLL( const anatomist::ViewState& viewState,
                                     const anatomist::GLList& glList ) const;

    const carto::rc_ptr<TimeSurfaceType> surface() const 
    { return( _surface ); }
    carto::rc_ptr<TimeSurfaceType> surface() { return( _surface ); }
    void setSurface( carto::rc_ptr<TimeSurfaceType> surf );
    /** Using this method, the ASurface takes ownership of the surface 
        (by making a new rc_ptr on it) */
    void setSurface( TimeSurfaceType *surf );
    const SurfaceType* surfaceOfTime( float time ) const;
    float actualTime( float time ) const;

    virtual float MinT() const;
    virtual float MaxT() const;

    virtual bool boundingBox( Point3df & bmin, Point3df & bmax ) const;

    /// selection is disabled for surfaces
    virtual AObject* ObjectAt( float x, float y, float z, float t, 
			       float tol = 0 );

    /// Can't be display in 2D windows.
    bool Is2DObject() { return false; }
    /// Can be display in 3D windows.
    bool Is3DObject() { return true; }

    virtual unsigned glNumVertex( const ViewState & ) const;
    virtual const GLfloat* glVertexArray( const ViewState & ) const;
    virtual unsigned glNumNormal( const ViewState & ) const;
    virtual const GLfloat* glNormalArray( const ViewState & ) const;
    virtual unsigned glNumPolygon( const ViewState & ) const;
    virtual unsigned glPolygonSize( const ViewState & ) const;
    virtual const GLuint* glPolygonArray( const ViewState & ) const;

    virtual bool loadable() const { return( true ); }
    virtual bool savable() const { return( true ); }
    virtual bool save( const std::string & filename );
    virtual bool reload( const std::string & filename );

    SurfaceType* surfaceOfTime( float time );

    bool isPlanar() const { return _planar; }

    void invertPolygons();
    static void invertPolygonsStatic( const std::set<AObject *> & obj );

    virtual void notifyObservers( void * = 0 );

    virtual carto::GenericObject* attributed();
    virtual const carto::GenericObject* attributed() const;
    virtual void setInternalsChanged();

  protected:
    carto::rc_ptr<TimeSurfaceType>	_surface;
    bool				_planar;
/*
    QGLShaderProgram			*_shader_program;
    QGLShader				*_vertex_shader;
    QGLShader				*_fragment_shader;
*/
    void freeSurface();
  };

}


#endif
