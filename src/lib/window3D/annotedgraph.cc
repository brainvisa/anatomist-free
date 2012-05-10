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

#include <anatomist/window3D/annotedgraph.h>
#include <anatomist/surface/textobject.h>
#include <anatomist/surface/transformedobject.h>
#include <anatomist/graph/Graph.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/controler/view.h>
#include <anatomist/mobject/ObjectList.h>
#include <aims/mesh/surfacegen.h>

using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;

namespace
{

  struct AnnotationProperties
  {
    AnnotationProperties() :
      usespheres( true ),
      colorlabels( true ),
      size( 1. )
    {
    }

    list<Point3df> lvert;
    list<AimsVector<unsigned,2> > lpoly;
    bool usespheres;
    bool colorlabels;
    Point3df center;
    Point3df bsize;
    float size;
  };


  void makelabel( const string & label, const Point3df &gc,
                  const Point3df & pos, const Point4df & color,
                  AnnotationProperties & props,
                  const map<string, Point4df> & colors,
                  AGraph *agraph, list<rc_ptr<AObject> > & objects )
  {
    TextObject *to = new TextObject( label );
    to->setScale( 0.1 );
    to->setName( string( "label: " ) + label );
    to->setReferentialInheritance( agraph );
    Point4df col = color;
    map<string, Point4df>::const_iterator ic = colors.find( label );
    if( ic != colors.end() )
    {
      col = ic->second;
      if( props.usespheres )
      {
        AimsSurfaceTriangle *sph = SurfaceGenerator::icosphere( gc, 2, 50 );
        ASurface<3> *asph = new ASurface<3>;
        asph->setSurface( sph );
        rc_ptr<AObject> osph( asph );
        theAnatomist->registerObject( asph, false );
        theAnatomist->releaseObject( asph );
        asph->setReferentialInheritance( agraph );
        Material & mat = asph->GetMaterial();
        mat.SetDiffuse( col[0], col[1], col[2], col[3] );
        asph->SetMaterial( mat );
        asph->setName( string( "gc: " ) + label );
        objects.push_back( osph );
      }
      if( props.colorlabels )
      {
        Material & mat = to->GetMaterial();
        mat.SetDiffuse( col[0], col[1], col[2], col[3] );
        to->SetMaterial( mat );
      }
    }
    vector<AObject *> vto;
    vto.push_back( to );
    TransformedObject *texto = new TransformedObject( vto, false, true, pos );
    texto->setReferentialInheritance( agraph );
    texto->setDynamicOffsetFromPoint( props.center );
    texto->setName( string( "annot: " ) + label );
    rc_ptr<AObject> otexto( texto );
    objects.push_back( otexto );
    props.lpoly.push_back( AimsVector<unsigned,2>( props.lvert.size(),
      props.lvert.size() + 1 ) );
    props.lvert.push_back( gc );
    props.lvert.push_back( pos );
    theAnatomist->registerObject( texto, false );
    theAnatomist->releaseObject( texto );
  }


  Point3df getExtremity( const Point3df & p0, const Point3df & v,
                         const AnnotationProperties & props )
  {
    /* ellipsoid intersect:
       p = p0 + alpha * v (alpha > 0 )
       p-center: (x,y,z): (x/a)^2 + (y/b)^2 + (z/c)^2 = r^2
       -> this gets to a 2nd order polynom aa * alpha^2 + bb * alpha + cc = 0
    */
    float a = props.bsize[0] * props.bsize[0] / 4;
    float b = props.bsize[1] * props.bsize[1] / 4;
    float c = props.bsize[2] * props.bsize[2] / 4;
    Point3df cp = p0 - props.center;
    float aa = b * c * v[0]*v[0] + a * c * v[1]*v[1] + a * b * v[2]*v[2];
    float bb = ( b * c * cp[0] * v[0] + a * c * cp[1] * v[1]
      + a * b * cp[2] * v[2] ) * 2;
    float cc = b * c * cp[0] * cp[0] + a * c * cp[1] * cp[1]
      + a * b * cp[2] * cp[2] - a * b * c * props.size;
    float delta = bb * bb - 4 * aa * cc; // discriminant
    if( delta < 0 )
      return p0 + v * props.size; // no intersection with ellipsoid.
    float alpha = ( - bb + sqrt( delta ) ) / ( aa * 2 ); // largest solution
    return p0 + v * alpha;
  }

}

// ----------------

struct AnnotationAction::PrivateShared
{
  PrivateShared() : isbyvertex( false )
  {
  }

  bool isbyvertex;
};


// -----------------

Action * AnnotationAction::creator()
{
  return new AnnotationAction;
}


AnnotationAction::AnnotationAction() : _built( false )
{
}


AnnotationAction::AnnotationAction( const AnnotationAction & a )
  : _built( false )
{
}


AnnotationAction::~AnnotationAction()
{
}


AnnotationAction::PrivateShared & AnnotationAction::pshared()
{
  static PrivateShared ps;
  return ps;
}


bool AnnotationAction::isByVertex() const
{
  return pshared().isbyvertex;
}


string AnnotationAction::name() const
{
  return "AnnotationAction";
}


void AnnotationAction::annotationSelected()
{
  AWindow *win = view()->window();
  set<AObject *> objs = win->Objects();
  list<AGraph *> graphs;
  _temp.clear();
  set<AObject *>::iterator io, eo = objs.end();
  for( io=objs.begin(); io!=eo; ++io )
  {
    if( dynamic_cast<AGraph *>( *io ) )
      graphs.push_back( static_cast<AGraph *>( *io ) );
  }
  list<AGraph *>::iterator ig, eg = graphs.end();
  for( ig=graphs.begin(); ig!=eg; ++ig )
    buildGraphAnnotations( *ig );
  if( !_temp.empty() )
    _built = true;
}


void AnnotationAction::cleanAnnotations()
{
  _built = false;
  _temp.clear();
}


void AnnotationAction::switchAnnotations()
{
  if( _built )
    cleanAnnotations();
  else
    annotationSelected();
}


void AnnotationAction::buildGraphAnnotations( AGraph * agraph )
{
  Graph *graph = agraph->graph();
  string labelatt = "name";
  graph->getProperty( "label_property", labelatt );
  Point3df bmin, bmax;
  agraph->boundingBox( bmin, bmax );
  AnnotationProperties props;
  props.center = ( bmin + bmax ) / 2;
  // float size = ( bmax - bmin ).norm() * 0.2;
  props.bsize = bmax - bmin;
//   float size = bsize[0] * bsize[1] * bsize[2];
  props.size = 2.; // ellipsoid going through the corner of the bbox
  Point3df vs = agraph->VoxelSize();
  list<rc_ptr<AObject> > objects;
  AimsTimeSurface<2, Void> *lines = new AimsTimeSurface<2, Void>;

  map<string, pair<Point3df, float> > elements;
  map<string, Point4df > colors;

  Graph::iterator iv, ev = graph->end();
  for( iv= graph->begin(); iv!=ev; ++iv )
  {
    Vertex *v = *iv;
    string label;
    if( v->getProperty( labelatt, label ) && label != "unknown" )
    {
      Point3df gc;
      Object ogc = none();
      try
      {
        ogc = v->getProperty( "gravity_center" );
        Object oi = ogc->objectIterator();
        int i = 0;
        while( oi->isValid() && i < 3)
        {
          gc[i] = oi->currentValue()->getScalar() * vs[i];
          ++i;
          oi->next();
        }
      }
      catch( ... )
      {
        ogc = none();
      }
      shared_ptr<AObject> av;
      v->getProperty( "ana_object", av );
      if( ogc.isNull() && av.get() )
      {
        Point3df gbbm, gbbM;
        av->boundingBox( gbbm, gbbM );
        gc = ( gbbm + gbbM ) / 2;
      }
      else if( ogc.isNull() )
        continue;
      map<string, pair<Point3df, float> >::iterator iel
        = elements.find( label );
      if( iel == elements.end() )
      {
        elements[ label ] = make_pair( Point3df( 0, 0, 0 ), 0. );
        iel = elements.find( label );
      }
      pair<Point3df, float> & elem = iel->second;
      float sz = 0;
      try
      {
        Object osz = v->getProperty( "size" );
        sz = osz->getScalar();
      }
      catch( ... )
      {
      }
      if( sz == 0 )
        sz = 1.;
      elem.first += gc * sz;
      elem.second += sz;
      Point4df color( 0, 0, 0, 1 );
      if( av.get() )
      {
        Material & mat = av->GetMaterial();
        color[0] = mat.Diffuse(0);
        color[1] = mat.Diffuse(1);
        color[2] = mat.Diffuse(2);
        color[3] = mat.Diffuse(3);
        colors[ label ] = color;
      }
      if( isByVertex() )
      {
//         Point3df pos = gc + ( gc - props.center ).normalize() * size;
        Point3df pos = getExtremity( gc, ( gc - props.center ).normalize(),
                                     props );
        makelabel( label, gc, pos, color, props, colors, agraph, objects );
      }
    }
  }

  if( ! isByVertex() )
  {
    Point3df gc, pos;
    Point4df color;
    map<string, pair<Point3df, float> >::iterator iel, eel = elements.end();
    for( iel=elements.begin(); iel!=eel; ++iel )
    {
      const string & label =iel->first;
      pair<Point3df, float> & elem = iel->second;
      if( elem.second != 0 )
        gc = elem.first / elem.second;
      else
        gc = elem.first;
//       pos = gc + ( gc - props.center ).normalize() * size;
      pos = getExtremity( gc, ( gc - props.center ).normalize(), props );
      if( colors.find( label ) != colors.end() )
        color = colors[ label ];
      else
        color = Point4df( 0, 0, 0, 1 );
      makelabel( label, gc, pos, color, props, colors, agraph, objects );
    }
  }

  if( objects.empty() )
    return;
  lines->vertex().insert( lines->vertex().begin(), props.lvert.begin(),
    props.lvert.end() );
  lines->polygon().insert( lines->polygon().begin(), props.lpoly.begin(),
    props.lpoly.end() );
  ASurface<2> *alines = new ASurface<2>;
  alines->setSurface( lines );
  rc_ptr<AObject> olines( alines );
  theAnatomist->registerObject( alines, false );
  theAnatomist->releaseObject( alines );
  Material & mat = alines->GetMaterial();
  mat.SetDiffuse( 0, 0, 0, 1 );
  alines->SetMaterial( mat );
  alines->setReferentialInheritance( agraph );
  objects.push_back( olines );
  // group
  ObjectList *labels = new ObjectList;
  labels->setName( theAnatomist->makeObjectName( "Labels" ) );
  list<rc_ptr<AObject> >::iterator io, eo = objects.end();
  for( io=objects.begin(); io!=eo; ++io )
    labels->insert( *io );
  rc_ptr<AObject> olabels( labels );
  theAnatomist->registerObject( labels, false );
  theAnatomist->releaseObject( labels );
  labels->setReferentialInheritance( agraph );
  AWindow *win = view()->window();
  win->registerObject( alines, true );
  for( io=objects.begin(); io!=eo; ++io )
    win->registerObject( io->get(), true );
  _temp.push_back( olabels );
}

