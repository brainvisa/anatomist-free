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

#include <anatomist/control/qObjTree.h>
#include <anatomist/surfmatcher/surfMatcher.h>
#include <anatomist/interface/qwSurfMatch.h>
#include <anatomist/surfmatcher/surfMatchMethod.h>
#include <anatomist/landmark/landmFactory.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/application/settings.h>
#include <anatomist/surface/triangulated.h>
#include <anatomist/window/Window.h>
#include <anatomist/object/actions.h>
#include <graph/tree/tree.h>
#include <qpixmap.h>
#include <stdio.h>
#include <assert.h>
#ifdef CARTO_USE_KDTREE
#include <kdtree++/kdtree.hpp>
#endif


using namespace anatomist;
using namespace carto;
using namespace std;


// internal formats & data

#ifdef CARTO_USE_KDTREE
namespace
{

  struct Bracket_accessor_PointIndex
  {
    typedef float result_type;
    typedef pair<uint, Point3df> _Val;

    result_type
    operator()(_Val const& V, size_t const N) const
    {
      return V.second[N];
    }
  };

  typedef KDTree::KDTree<3, pair<uint, Point3df>, Bracket_accessor_PointIndex >
    _KDTree;

}
#endif


namespace anatomist
{

  struct ASurfMatcher_processData
  {
    ASurfMatcher_processData() : nneighbours( 5 ), attraction( 2. ),
      pressure( 0.05 ), cohesion( 0.02 ), ctrlAttraction( 1. ),
      restLength( 0.8 ), maxLengthFactor( 5. ), meanLength( 0 ),
      reversenorm( false )
#ifdef CARTO_USE_KDTREE
      , kdtree( 0 )
#endif
    {}
#ifdef CARTO_USE_KDTREE
    ~ASurfMatcher_processData()
    { delete kdtree; }
#endif

    map<unsigned, set<unsigned> >	neigh3;
    unsigned	nneighbours;
    float		attraction;
    float		pressure;
    float		cohesion;
    float		ctrlAttraction;
    float		restLength;
    float		maxLengthFactor;

    float		meanLength;

    bool		reversenorm;
#ifdef CARTO_USE_KDTREE
    _KDTree              *kdtree;
#endif
  };


  struct ASurfMatcher_ctrlPts
  {
    vector<unsigned>	orgCtrlPts;
    vector<Point3df>	dstCtrlPts;
    float			ctrlPtsSize;
  };

}


int	ASurfMatcher::_classType = ASurfMatcher::registerClass();
Tree	*ASurfMatcher::_optionTree = 0;


int ASurfMatcher::registerClass()
{
  int		type = registerObjectType( "SURFMATCH" );
  FusionMethod	*m = new ASurfMatchMethod;

  if( !FusionFactory::registerMethod( m ) )
    delete m;

  return type;
}


//

ASurfMatcher::ASurfMatcher( AObject* o1, AObject* o2 )
  : ObjectVector(), _ascending( true ), _processFinished( false ),
    _record( false ), _mdata( 0 ), _ctrlPts( new ASurfMatcher_ctrlPts )
{
  _type = _classType;

  ATriangulated	*s1 = dynamic_cast<ATriangulated *>( o1 );
  ATriangulated	*s2 = dynamic_cast<ATriangulated *>( o2 );

  assert( s1 );
  assert( s2 );

  insert( o1 );
  insert( o2 );

  ATriangulated	*s3 = new ATriangulated( "matchsurf.mesh" );

  s3->setName( theAnatomist->makeObjectName( "Matchsurf" ) );
  s3->setReferential( s1->getReferential() );
  s3->setSurface( new AimsSurfaceTriangle( *s1->surface() ) );
  insert( s3 );
  theAnatomist->registerObject( s3, 0 );
  theAnatomist->registerSubObject( this, s3 );

  if( QObjectTree::TypeNames.find( _type ) == QObjectTree::TypeNames.end() )
    {
      string str = Settings::findResourceFile( "icons/list_surfmatcher.xpm" );
      if( !QObjectTree::TypeIcons[ _type ].load( str.c_str() ) )
      {
        QObjectTree::TypeIcons.erase( _type );
        cerr << "Icon " << str.c_str() << " not found\n";
      }

      QObjectTree::TypeNames[ _type ] = "SurfaceMatcher";
    }
}


ASurfMatcher::~ASurfMatcher()
{
  cleanup();
  deleteDstCtrlPointsSurf();
  deleteOrgCtrlPointsSurf();
  delete _mdata;
  delete _ctrlPts;
}


Tree* ASurfMatcher::optionTree() const
{
  if( !_optionTree )
    {
      Tree	*t, *t2;
      _optionTree = new Tree( true, "option tree" );
      t = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "File" ) );
      _optionTree->insert( t );
      t2 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu",
					      "Rename object" ) );
      t2->setProperty( "callback", &ObjectActions::renameObject );
      t->insert( t2 );

      t = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "Fusion" ) );
      _optionTree->insert( t );
      t2 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu",
					      "Surface matching window" ) );
      t2->setProperty( "callback", &surfMatchControl );
      t->insert( t2 );
    }
  return _optionTree;
}


void ASurfMatcher::setAscending( bool asc )
{
  _ascending = asc;
#ifdef CARTO_USE_KDTREE
  if( _mdata )
    delete _mdata->kdtree;
#endif
}


bool ASurfMatcher::CanRemove( AObject* )
{
  return false;
}


void ASurfMatcher::surfMatchControl( const set<AObject *> & obj )
{
  QSurfMatchWin	*w = new QSurfMatchWin( theAnatomist->getQWidgetAncestor(), (ASurfMatcher *) *obj.begin() );
  w->setWindowFlags(Qt::Window);
  w->show();
}


void ASurfMatcher::processStep()
{
  //cout << "matching step\n";
  // find objects
  iterator	io = begin();
  AObject	*o1, *o2;
  ATriangulated	*s1, *s2, *s3;

  o1 = *io;
  ++io;
  o2 = *io;
  if( !ascending() )
    {
      AObject	*o = o1;
      o1 = o2;
      o2 = o;
    }
  s1 = dynamic_cast<ATriangulated *>( o1 );
  s2 = dynamic_cast<ATriangulated *>( o2 );
  ++io;
  s3 = dynamic_cast<ATriangulated *>( *io );

  if( !s3->surface() )
    {
      cout << "creating new triangulation\n";
      s3->setSurface( new AimsSurfaceTriangle( *s1->surface() ) );
    }

  rc_ptr<AimsSurfaceTriangle>	surf = s3->surface();
  rc_ptr<AimsSurfaceTriangle>	surf2 = s2->surface();

  // time
  unsigned	time = (*surf->rbegin()).first;
  unsigned	oldtime = time;

  if( _record )	// record over time ?
    {
      // copy surface to next time step
      (*surf)[time+1] = (*surf)[time];
      ++time;
    }

  AimsSurface<3,Void>		& surf3 = (*surf)[time];
  const vector<Point3df>	& vert2 = surf2->vertex();
  vector<Point3df>		& vert3 = surf3.vertex();
  vector<Point3df>		& norm3 = surf3.normal();
  vector<AimsVector<uint, 3> >	& poly3 = surf3.polygon();
  unsigned			nv2 = vert2.size();
  unsigned			nv3 = vert3.size();
  vector<Point3df>		& norm2 = surf2->normal();

  //	store neighbourhoods of moving surface

  prepareNeighbourhood( s3, time );
  map<unsigned, set<unsigned> >	& neigh3 = _mdata->neigh3;
  set<unsigned>::const_iterator	in, fn;

  if( _mdata->meanLength == 0 )
    computeLength( s1, 0 );

  //	parameters

  unsigned	nneigh = _mdata->nneighbours;
  float		attraction = _mdata->attraction;
  float		pressure = _mdata->pressure;
  float		cohesion = _mdata->cohesion;
  float		ctrlAttraction = _mdata->ctrlAttraction;
  float		meanLength = _mdata->meanLength * _mdata->restLength;
  float		maxLengthFactor =  _mdata->maxLengthFactor;

  //	neighbourhoods in target surface

  vector<Point3df>		neigh(nneigh);
  float			sqd[nneigh];
  unsigned		ind[nneigh];
  unsigned      ver, vf, i;

#ifdef CARTO_USE_KDTREE
  unsigned long kdtmin = 2000; // under this number of nodes x searches,
  // the kd tree is slower than an exaustive neighbour search.
  if( !_mdata->kdtree && nv2 * nneigh > kdtmin )
  {
    cout << "generating kd tree..."  << flush;
    // make a kd tree of target vertices
    vector<pair<uint, Point3df> > *ivert2
      = new vector<pair<uint, Point3df> >( nv2 );
    for( vf=0; vf<nv2; ++vf )
    {
      (*ivert2)[vf].first = vf;
      (*ivert2)[vf].second = vert2[vf];
    }

    // make a KDtree with vertices list
    _mdata->kdtree = new _KDTree( ivert2->begin(), ivert2->end() );
    delete ivert2; // get rid of this temporary list
    cout << " done.\n";
  }
#endif

  //	processing

  float		dn, d2;
  Point3df	dif, norm, snorm, vec1, vec2;
  unsigned	maxi = 0;
  float		maxd = 1e38;
  unsigned	notherside = 0;

  //	move control points towards their aim

  const vector<unsigned>	& srcCtrl = _ctrlPts->orgCtrlPts;
  const vector<Point3df>	& dstCtrl = _ctrlPts->dstCtrlPts;
  unsigned			ncp = srcCtrl.size();

  if( dstCtrl.size() < ncp )
    ncp = dstCtrl.size();

  for( i=0; i<ncp; ++i )
    {
      Point3df		& pt = vert3[srcCtrl[i]];
      const Point3df	& pf = dstCtrl[i];

      pt[0] += ( pf[0] - pt[0] ) * ctrlAttraction;
      pt[1] += ( pf[1] - pt[1] ) * ctrlAttraction;
      pt[2] += ( pf[2] - pt[2] ) * ctrlAttraction;
    }

  //	move each point according to various forces

  for( ver=0; ver<nv3; ++ver )
  {
    Point3df	& pt = vert3[ver];

#ifdef CARTO_USE_KDTREE
    if( nv2 * nneigh > kdtmin )
    {
      for( i=0; i<nneigh; ++i )
      {
        if( i != 0 )
          // remove previous nearest neighbour
          _mdata->kdtree->erase( make_pair( ind[i-1], vert2[ind[i-1]] ) );
        pair<_KDTree::const_iterator, _KDTree::distance_type> in
          = _mdata->kdtree->find_nearest( make_pair( 0U, pt ) );
        ind[i] = in.first->first;
        neigh[i] = pt - vert2[ in.first->first ];
        sqd[i] = in.second * in.second;
      }
      // put back points which were removed
      for( i=0; i<nneigh-1; ++i )
        _mdata->kdtree->insert( make_pair( ind[i], vert2[ind[i]] ) );
    }
    else
    {
#endif

      for( i=0; i<nneigh; ++i )
      {
        //neigh[i] = Point3df( 1e38, 1e38, 1e38 );
        sqd[i] = 1e38;
      }
      maxi = 0;
      maxd = 1e38;

      // find the nneigh nearest points in s2

      for( vf=0; vf<nv2; ++vf )
      {
        const Point3df	& pf = vert2[vf];

        dif[0] = pt[0] - pf[0];
        dif[1] = pt[1] - pf[1];
        dif[2] = pt[2] - pf[2];
        dn = dif[0] * dif[0] + dif[1] * dif[1] + dif[2] * dif[2];

        if( dn < maxd )
        {
          neigh[maxi] = dif;
          sqd[maxi] = dn;
          maxd = dn;
          ind[maxi] = vf;

          for( i=0; i<nneigh; ++i )
          {
            if( sqd[i] > maxd )
            {
              maxi = i;
              maxd = sqd[i];
            }
          }
        }
      }
#ifdef CARTO_USE_KDTREE
    }
#endif

    // now apply displacements towards nearest neighbours

    vec1 = Point3df( 0, 0, 0 );

    for( i=0; i<nneigh; ++i )
      {
        vf = ind[i];
        Point3df		& nf = norm2[vf];
        Point3df		& d = neigh[i];

        /*if( sqd[i] > 0 )
          {*/
        dn = ( d[0] * nf[0] + d[1] * nf[1] + d[2] * nf[2] )
          / ( sqrt( sqd[i] ) + 1. );
        vec1[0] += dn * nf[0];
        vec1[1] += dn * nf[1];
        vec1[2] += dn * nf[2];
            /*}*/
      }
    dn = - attraction / nneigh;
    pt[0] += dn * vec1[0];
    pt[1] += dn * vec1[1];
    pt[2] += dn * vec1[2];

    // pressure force (along normal)
    Point3df	& nrm = norm3[ver];

    pt[0] += pressure * nrm[0];
    pt[1] += pressure * nrm[1];
    pt[2] += pressure * nrm[2];

    // cohesion force (along edges of triangles) (=smoothing)
    set<unsigned>	& nbr = neigh3[ver];

    vec2 = Point3df( 0, 0, 0 );

    for( in=nbr.begin(), fn=nbr.end(); in!=fn; ++in )
      {
        AimsVector<uint, 3>	& poly = poly3[*in];
        if( poly[0] == ver )
          i = 1;
        else
          i = 0;
        Point3df		&v1 = vert3[poly[i]];
        ++i;
        if( poly[i] == ver )
          ++i;
        Point3df		&v2 = vert3[poly[i]];

        vec1[0] = v1[0] - pt[0];
        vec1[1] = v1[1] - pt[1];
        vec1[2] = v1[2] - pt[2];
        d2 = sqrt( vec1[0] * vec1[0] + vec1[1] * vec1[1]
                    + vec1[2] * vec1[2] );
        if( d2 == 0 )
          {
            d2 = 1.;
            vec1[0] = 1.;
          }
        dn = ( d2 - meanLength ) * cohesion;
        if( dn < -maxLengthFactor )
          dn = -maxLengthFactor;
        else if( dn > maxLengthFactor )
          dn = maxLengthFactor;
        dn /= d2;

        vec2[0] += dn * vec1[0];
        vec2[1] += dn * vec1[1];
        vec2[2] += dn * vec1[2];

        vec1[0] = v2[0] - pt[0];
        vec1[1] = v2[1] - pt[1];
        vec1[2] = v2[2] - pt[2];
        d2 = sqrt( vec1[0] * vec1[0] + vec1[1] * vec1[1]
                    + vec1[2] * vec1[2] );
        if( d2 == 0 )
          {
            d2 = 1.;
            vec1[0] = 1.;
          }
        dn = ( d2 - meanLength ) * cohesion;
        if( dn < -maxLengthFactor )
          dn = -maxLengthFactor;
        else if( dn > maxLengthFactor )
          dn = maxLengthFactor;
        dn /= d2;

        vec2[0] += dn * vec1[0];
        vec2[1] += dn * vec1[1];
        vec2[2] += dn * vec1[2];
      }

    pt[0] += vec2[0];
    pt[1] += vec2[1];
    pt[2] += vec2[2];

    // after deplacements, recompute normal
    snorm = Point3df( 0, 0, 0 );

    for( in=nbr.begin(), fn=nbr.end(); in!=fn; ++in )
      {
        AimsVector<uint, 3>	& poly = poly3[*in];
        Point3df		&v1 = vert3[poly[0]];
        Point3df		&v2 = vert3[poly[1]];
        Point3df		&v3 = vert3[poly[2]];

        vec1[0] = v2[0] - v1[0];
        vec1[1] = v2[1] - v1[1];
        vec1[2] = v2[2] - v1[2];
        vec2[0] = v1[0] - v3[0];
        vec2[1] = v1[1] - v3[1];
        vec2[2] = v1[2] - v3[2];
        norm[0] = vec1[1] * vec2[2] - vec2[1] * vec1[2];
        norm[1] = vec1[2] * vec2[0] - vec2[2] * vec1[0];
        norm[2] = vec1[0] * vec2[1] - vec2[0] * vec1[1];

        dn = 1.; /* / sqrt( norm[0] * norm[0] + norm[1] * norm[1]
                    + norm[2] * norm[2] );*/
        if( _mdata->reversenorm )
          dn = -dn;
        if( ( norm[0] * nrm[0] + norm[1] * nrm[1] + norm[2] * nrm[2] )
            * ( _mdata->reversenorm ? -1 : 1 ) < 0 )
          ++notherside;
        snorm[0] += norm[0] * dn;
        snorm[1] += norm[1] * dn;
        snorm[2] += norm[2] * dn;
      }
    dn = 1. / sqrt( snorm[0] * snorm[0] + snorm[1] * snorm[1]
                    + snorm[2] * snorm[2] );
    nrm[0] = snorm[0] * dn;
    nrm[1] = snorm[1] * dn;
    nrm[2] = snorm[2] * dn;
  }

if( notherside >= nv3 / 2 )
  {
    _mdata->reversenorm = !_mdata->reversenorm;
    cout << "reversenorm : " << _mdata->reversenorm << endl;
    for( ver=0; ver<nv3; ++ver )
      {
        Point3df	&nrm = norm3[ver];
        nrm[0] *= -1;
        nrm[1] *= -1;
        nrm[2] *= -1;
      }
  }

  //

  s3->glSetChanged( GLComponent::glGEOMETRY );
  s3->UpdateMinAndMax();

  moveOrgCtrlPoints( oldtime, time );
}


void ASurfMatcher::prepareNeighbourhood( ATriangulated* s, unsigned time )
{
  if( !_mdata )
    _mdata = new ASurfMatcher_processData;

  rc_ptr<AimsSurfaceTriangle>	surf = s->surface();
  AimsSurface<3,Void>		& surf3 = (*surf)[time];
  map<unsigned, set<unsigned> >	& neigh3 = _mdata->neigh3;
  vector<AimsVector<uint, 3> >	& poly3 = surf3.polygon();
  unsigned			np3 = poly3.size();
  unsigned			pol;

  if( neigh3.size() == surf3.vertex().size() )
    return;	// already done & up to date

  neigh3.clear();

  for( pol=0; pol<np3; ++pol )
    {
      AimsVector<uint, 3>	&poly = poly3[pol];

      neigh3[ poly[0] ].insert( pol );
      neigh3[ poly[1] ].insert( pol );
      neigh3[ poly[2] ].insert( pol );
    }
  cout << "neighbourhoods done\n";
}


void ASurfMatcher::computeLength( ATriangulated* s, unsigned time )
{
  rc_ptr<AimsSurfaceTriangle>	surf = s->surface();
  AimsSurface<3,Void>		& surf1 = (*surf)[time];
  vector<Point3df>		& pts = surf1.vertex();
  vector<AimsVector<uint, 3> >	& poly1 = surf1.polygon();
  unsigned			np = poly1.size();
  unsigned			pol;
  float				d = 0;

  for( pol=0; pol<np; ++pol )
    {
      const AimsVector<uint, 3>	&poly = poly1[pol];
      const Point3df			& p1 = pts[ poly[0] ];
      const Point3df			& p2 = pts[ poly[1] ];
      const Point3df			& p3 = pts[ poly[2] ];

      d += ( p1[0] - p2[0] ) * ( p1[0] - p2[0] )
	+ ( p1[1] - p2[1] ) * ( p1[1] - p2[1] )
	+ ( p1[2] - p2[2] ) * ( p1[2] - p2[2] );
      d += ( p1[0] - p3[0] ) * ( p1[0] - p3[0] )
	+ ( p1[1] - p3[1] ) * ( p1[1] - p3[1] )
	+ ( p1[2] - p3[2] ) * ( p1[2] - p3[2] );
      d += ( p3[0] - p2[0] ) * ( p3[0] - p2[0] )
	+ ( p3[1] - p2[1] ) * ( p3[1] - p2[1] )
	+ ( p3[2] - p2[2] ) * ( p3[2] - p2[2] );
    }
  d /= np * 3;
  _mdata->meanLength = sqrt( d );
}


Tree ASurfMatcher::parameters()
{
  if( !_mdata )
    _mdata = new ASurfMatcher_processData;

  return ((const ASurfMatcher *) this)->parameters();
}


Tree ASurfMatcher::parameters() const
{
  Tree	par( true, "SurfMatcherParams" );

  const ASurfMatcher_processData	*data = _mdata;
  static ASurfMatcher_processData	dataS;

  if( !data )
    data = &dataS;

  par.setProperty( "nneighbours", (int) data->nneighbours );
  par.setProperty( "pressure", data->pressure );
  par.setProperty( "attraction", data->attraction );
  par.setProperty( "cohesion", data->cohesion );
  par.setProperty( "ctrlAttraction", data->ctrlAttraction );
  par.setProperty( "restLength", data->restLength );
  par.setProperty( "maxLengthFactor", data->maxLengthFactor );

  return par;
}


void ASurfMatcher::setParameters( const Tree & par )
{
  if( !_mdata )
    _mdata = new ASurfMatcher_processData;

  int	nn;

  par.getProperty( "nneighbours", nn );
  _mdata->nneighbours = (unsigned) nn;
  par.getProperty( "pressure", _mdata->pressure );
  par.getProperty( "attraction", _mdata->attraction );
  par.getProperty( "cohesion", _mdata->cohesion );
  par.getProperty( "ctrlAttraction", _mdata->ctrlAttraction );
  par.getProperty( "restLength", _mdata->restLength );
  par.getProperty( "maxLengthFactor", _mdata->maxLengthFactor );
}


SyntaxSet ASurfMatcher::paramSyntax() const
{
  SyntaxSet	ss;
  Syntax	& synt = ss[ "SurfMatcherParams" ];

  Semantic	sem;

  sem.type = "int";
  sem.needed = false;
  synt[ "nneighbours"     ] = sem;
  sem.type = "float";
  synt[ "attraction"      ] = sem;
  synt[ "pressure"        ] = sem;
  synt[ "cohesion"        ] = sem;
  synt[ "ctrlAttraction"  ] = sem;
  synt[ "restLength"      ] = sem;
  synt[ "maxLengthFactor" ] = sem;

  return ss;
}


void ASurfMatcher::resetProcess()
{
  ATriangulated	*s1 = 0;

  delete _mdata;
  _mdata = 0;
  iterator	i = begin();
  if( _ascending )
    s1 = (ATriangulated *) *i;
  ++i;
  if( !_ascending )
    s1 = (ATriangulated *) *i;
  ++i;
  ATriangulated	*surf = (ATriangulated *) *i;
  surf->setSurface( new AimsSurfaceTriangle( *s1->surface() ) );
  surf->setReferential( s1->getReferential() );

  //	reset control points
  vector<unsigned>	ocp = _ctrlPts->orgCtrlPts;
  setOrgControlPoints( ocp );
}


void ASurfMatcher::setOrgControlPoints( const vector<unsigned> & pts )
{
  deleteOrgCtrlPointsSurf();
  _ctrlPts->orgCtrlPts = pts;

  char					name[200];
  ATriangulated				*cp;
  unsigned				i, n = pts.size();
  ATriangulated				*dsurf = destSurface();
  ATriangulated				*osurf = orgSurface();
  const vector<Point3df>		& vert = osurf->surface()->vertex();
  float					sz;
  const set<AWindow *>			& wl = movingSurface()->WinList();
  set<AWindow *>::const_iterator	iw, fw=wl.end();
  Point3df				col( 0.3, 0.3, 1 ), bmin, bmax;

  dsurf->boundingBox( bmin, bmax );
  bmax -= bmin;
  sz = sqrt( bmax.dot( bmax ) ) * 0.01;
  _ctrlPts->ctrlPtsSize = sz;

  for( i=0; i<n; ++i )
    {
      sprintf( name, "OrgControlPoint%02d.mesh", i+1 );
      cp = ALandmarkFactory::createCube( vert[ pts[i] ], name, sz, col );
      insert( cp, i + 3 );
      theAnatomist->registerObject( cp, 0 );
      theAnatomist->registerSubObject( this, cp );
      for( iw=wl.begin(); iw!=fw; ++iw )
	(*iw)->registerObject( cp );
      cp->notifyObservers();
    }
}


void ASurfMatcher::setDestControlPoints( const vector<Point3df> & pts )
{
  deleteDstCtrlPointsSurf();
  _ctrlPts->dstCtrlPts = pts;

  char					name[200];
  ATriangulated				*cp;
  unsigned				i, n = pts.size();
  ATriangulated				*dsurf = destSurface();
  float					sz;
  const set<AWindow *>			& wl = dsurf->WinList();
  set<AWindow *>::const_iterator	iw, fw=wl.end();
  Point3df				col( 1, 0, 0 ), bmin, bmax;

  dsurf->boundingBox( bmin, bmax );
  bmax -= bmin;
  sz = sqrt( bmax.dot( bmax ) ) * 0.01;

  for( i=0; i<n; ++i )
    {
      sprintf( name, "DstControlPoint%02d.mesh", i+1 );
      cp = ALandmarkFactory::createCube( pts[i], name, sz, col );
      insert( cp );
      theAnatomist->registerObject( cp, 0 );
      theAnatomist->registerSubObject( this, cp );
      for( iw=wl.begin(); iw!=fw; ++iw )
	(*iw)->registerObject( cp );
      cp->notifyObservers();
    }
}


void ASurfMatcher::deleteDstCtrlPointsSurf()
{
  unsigned		n = 3 + _ctrlPts->orgCtrlPts.size(), k;
  datatype::iterator  i = _data.begin(), j;
  iterator  z;

  for( k=0; k<n-1; ++i, ++k ) {}
  while( size() > n  )
  {
    j = i;
    ++j;
    z = iterator( new ObjectVectorIterator( j ) );
    erase( z );
  }
}


void ASurfMatcher::deleteOrgCtrlPointsSurf()
{
  unsigned		n = 3 + _ctrlPts->dstCtrlPts.size();
  datatype::iterator	i = _data.begin(), j;
  iterator  z;

  ++i;
  ++i;	// 3rd element (surfmatch)

  while( size() > n  )
  {
    j = i;
    ++j;
    z = iterator( new ObjectVectorIterator( j ) );
    erase( z );
  }
}


const vector<unsigned> & ASurfMatcher::orgControlPoints() const
{
  return _ctrlPts->orgCtrlPts;
}


vector<unsigned> & ASurfMatcher::orgControlPoints()
{
  return _ctrlPts->orgCtrlPts;
}


const vector<Point3df> & ASurfMatcher::destControlPoints() const
{
  return _ctrlPts->dstCtrlPts;
}


vector<Point3df> & ASurfMatcher::destControlPoints()
{
  return _ctrlPts->dstCtrlPts;
}


ATriangulated* ASurfMatcher::orgSurface() const
{
  iterator	i = begin();
  if( !ascending() )
    ++i;
  return (ATriangulated *) *i;
}


ATriangulated* ASurfMatcher::destSurface() const
{
  iterator	i = begin();
  if( ascending() )
    ++i;
  return (ATriangulated *) *i;
}


ATriangulated* ASurfMatcher::movingSurface() const
{
  iterator	i = begin();
  ++i;
  ++i;
  return (ATriangulated *) *i;
}


void ASurfMatcher::moveOrgCtrlPoints( unsigned oldtime, unsigned time )
{
  datatype::const_iterator	io = _data.begin();
  unsigned			i, j, n = _ctrlPts->orgCtrlPts.size(), m;
  ATriangulated			*obj;
  ATriangulated			*s3 = movingSurface();
  const vector<Point3df>	& svert = (*s3->surface())[ time ].vertex();
  Point3df			pos;
  float				sz = _ctrlPts->ctrlPtsSize;
  Point3df			sp = Point3df( sz, sz, sz );
  rc_ptr<AimsSurfaceTriangle>	surf;

  ++io;
  ++io;
  ++io;
  for( i=0; i<n; ++i, ++io )
    {
      pos = svert[ _ctrlPts->orgCtrlPts[i] ];
      obj = (ATriangulated *) io->get();
      surf = obj->surface();

      if( oldtime != time )
	(*surf)[ time ] = (*surf)[ oldtime ];
      vector<Point3df>	& vert = (*surf)[ time ].vertex();
      pos -= vert[0] + sp;
      for( j=0, m=vert.size(); j<m; ++j )
	vert[j] += pos;
      obj->glSetChanged( GLComponent::glGEOMETRY );
      obj->UpdateMinAndMax();
    }
}
