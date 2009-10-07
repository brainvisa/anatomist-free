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


#include <cstdlib>
#include <anatomist/surface/curve.h>
#include <aims/curve/bundles.h>

using namespace anatomist;
using namespace aims;
using namespace sid;


struct ACurve::Private
{
  Private();

  Bundle	curve;
  bool		planar;
};


ACurve::Private::Private() : planar( false )
{
}


ACurve::ACurve( const string & filename )
  : AGLObject(), d( new Private )
{
}


ACurve::ACurve()
{
  delete d;
}


void ACurve::freeCurve()
{
  d->curve.clear();
}


const vector<CurveType>* ACurve::curveOfTime( float time ) const
{
  if( d->curve.empty() )
    return( 0 );

  unsigned	t = (unsigned) ( time / TimeStep() );
  Bundle::const_iterator	is = d->curve.lower_bound( t );

  if( is == d->curve.end() )
    is = d->curve.begin();
  return( &is->second );
}


vector<CurveType>* ACurve::curveOfTime( float time )
{
  unsigned	t = (unsigned) ( time / TimeStep() );

  if( d->curve.empty() )
    return( &(*d->curve)[t] );

  Bundle::iterator	is = d->curve.lower_bound( t );

  if( is == d->curve.end() )
    is = d->curve.begin();
  return( &is->second );
}


void ACurve::UpdateMinAndMax()
{
  if( d->curve.empty() )
    return;

  d->planar = true;

  CurveSampler<float, 3>	sam = d-curve.begin()->second.sampler();

  const vector<Point3df>	& norm = d->curve[0].normal();
  std::size_type i, n = norm.size();
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


float ACurve::MaxT() const
{
  if( d->curve.empty() )
    return( 0. );
  return( TimeStep() * (*d->curve.rbegin()).first );
}


float ACurve::MinT() const
{
  if(d->curve.empty() )
    return( 0. );
  return( TimeStep() * (*d->curve.begin()).first );
}


void ACurve::addCurve( CurveType* c, unsigned t )
{
  freeSurface();
  d->curve = surf;
  UpdateMinAndMax();
  glSetChanged( glGEOMETRY );
  setChanged();
}


unsigned ACurve::glNumVertex( const ViewState & s ) const
{
  const vector<CurveType>	*surf = curveOfTime( s.time );

  if( !surf )
    return 0;
  return surf->vertex().size();
}


const GLfloat* ACurve::glVertexArray( const ViewState & s ) const
{
  const CurveType	*surf = curveOfTime( s.time );

  if( !surf )
    return 0;
  return &surf->vertex()[0][0];
}


unsigned ACurve::glNumNormal( const ViewState & s ) const
{
  const CurveType	*surf = curveOfTime( s.time );

  if( !surf )
    return 0;
  return surf->normal().size();
}


const GLfloat* ACurve::glNormalArray( const ViewState & s ) const
{
  const CurveType	*surf = curveOfTime( s.time );

  if( !surf )
    return( 0 );
  return( &surf->normal()[0][0] );
}


const GLuint* ACurve::glPolygonArray( const ViewState & s ) const
{
  const CurveType	*surf = curveOfTime( s.time );

  if( !surf )
    return( 0 );
  return( (GLuint *) &surf->polygon()[0][0] );
}


unsigned ACurve::glNumPolygon( const ViewState & s ) const
{
  const CurveType	*surf = curveOfTime( s.time );

  if( !surf )
    return( 0 );
  return( surf->polygon().size() );
}


Tree* ACurve::optionTree() const
{
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

      t = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "Color" ) );
      _optionTree->insert( t );
      t2 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "Material" ) );
      t2->setProperty( "callback", &ObjectActions::colorMaterial );
      t->insert( t2 );
      t = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "Referential" ) );
      _optionTree->insert( t );
      t2 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "Load" ) );
      t2->setProperty( "callback", &ObjectActions::referentialLoad );
      t->insert( t2 );
      t = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "Geometry" ) );
      _optionTree->insert( t );
      t2 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", 
					      "Invert polygon orientation" ) );
      t2->setProperty( "callback", &invertTrianglesStatic );
      t->insert( t2 );
    }
  return( _optionTree );
}


AObject* ACurve::ObjectAt( float, float, float, float, 
				  float )
{
  return( 0 );
}


bool ACurve::save( const string & filename )
{
  if( !d->curve )
    return( false );

  try
    {
      Writer<map<unsigned, CurveType>>	sw( filename );
      sw << *d->curve;
    }
  catch( exception & e )
    {
      cerr << e.what() << "\nsave aborted\n";
      return( false );
    }
  return( true );
}


bool ACurve::reload( const string & filename )
{
  Reader<map<unsigned, CurveType>>	reader( filename );
  map<unsigned, CurveType>		*obj = new map<unsigned, CurveType>;
  if( !reader.read( *obj ) )
    {
      delete obj;
      return( false );
    }

  setSurface( obj );
  return( true );
}


void ACurve::invertTrianglesStatic( const set<AObject *> & obj )
{
  set<AObject *>::const_iterator	io, fo=obj.end();

  for( io=obj.begin(); io!=fo; ++io )
    ((ACurve *) *io)->invertTriangles();
}


void ACurve::invertTriangles()
{
  aims::SurfaceManip::invertSurfacePolygons( *d->curve );
  glSetChanged( glBODY );
  setChanged();
  notifyObservers( this );
}


bool ACurve::boundingBox( Point3df & bmin, Point3df & bmax ) const
{
  if( !d->curve )
    return( false );

  bmin = d->curve.minimum();
  bmax = d->curve.maximum();
  return( true );
}


float ACurve::actualTime( float time ) const
{
  if( !d->curve )
    return( 0 );

  unsigned	t = (unsigned) ( time / TimeStep() );
  map<unsigned, CurveType>::const_iterator	is = d->curve.lower_bound( t );

  if( is == d->curve.end() )
    is = d->curve.begin();
  return( is->first * TimeStep() );
}


void ACurve::notifyObservers( void * arg )
{
  AGLObject::notifyObservers( arg );
  glSetChanged( glGEOMETRY, false );
}


