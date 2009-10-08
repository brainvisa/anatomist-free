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

#ifndef ANA_SURFACE_CURVE_H
#define ANA_SURFACE_CURVE_H

#include <anatomist/surface/globject.h>
#include <aims/curve/curve.h>

namespace anatomist
{

  class ACurve : public AGLObject
  {
  public:
    typedef aims::Curve<float, 3>	CurveType;
    typedef std::map<unsigned, std::vector<rc_ptr<CurveType> > >	Bundle;

    ACurve( const std::string & filename );
    virtual ~ACurve();

    virtual Tree* optionTree() const;
    static Tree*	_optionTree;

    virtual void UpdateMinAndMax();

    const Bundle* curves();
    Bundle* curves();
    void addCurve( CurveType* c, unsigned t );
    const std::vector<CurveType>* curveOfTime( float time ) const;
    Bundle* curveOfTime( float time );
    float actualTime( float time ) const;

    virtual float MinT() const;
    virtual float MaxT() const;

    virtual bool boundingBox( Point3df & bmin, Point3df & bmax ) const;

    virtual AObject* ObjectAt( float x, float y, float z, float t, 
			       float tol = 0 );

    bool Is2DObject() { return false; }
    bool Is3DObject() { return true; }

    virtual unsigned glNumVertex( const ViewState & ) const;
    virtual const GLfloat* glVertexArray( const ViewState & ) const;
    virtual unsigned glNumNormal( const ViewState & ) const;
    virtual const GLfloat* glNormalArray( const ViewState & ) const;
    virtual unsigned glNumPolygon( const ViewState & ) const;
    virtual const GLuint* glPolygonArray( const ViewState & ) const;

    virtual bool loadable() const { return true; }
    virtual bool savable() const { return true; }
    virtual bool save( const std::string & filename );
    virtual bool reload( const std::string & filename );

    bool isPlanar() const;

    virtual void notifyObservers( void * = 0 );

  protected:
    void freeCurve();

  private:
    struct Private;

    Private	*d;
  };

}

#endif


