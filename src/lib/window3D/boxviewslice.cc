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

#include <anatomist/window3D/boxviewslice.h>
#include <anatomist/controler/action.h>
#include <anatomist/controler/view.h>
#include <anatomist/object/Object.h>
#include <anatomist/object/objectConverter.h>
#include <anatomist/window3D/window3D.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/surface/transformedobject.h>
#include <anatomist/window/glwidgetmanager.h>
#include <anatomist/surface/surface.h>
#include <anatomist/color/Material.h>
#include <aims/mesh/surfacegen.h>
#include <aims/resampling/quaternion.h>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>

using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;


struct BoxViewSlice::Private
{
  Private();

  Action* action;
  rc_ptr<AObject> cube;
  rc_ptr<AObject> rect;
  rc_ptr<AObject> smallobj;
  list<QGraphicsItem *> tmpitems;
  float cubecol[4];
  float planecol[4];
  int textcol[3];
  Referential *ref;
  bool planeEnabled;
  bool textEnabled;
  list<rc_ptr<AObject> > otherobjects;
};


BoxViewSlice::Private::Private()
  : ref( 0 ), planeEnabled( true ), textEnabled( true )
{
  cubecol[0] = 0.6;
  cubecol[1] = 0.6;
  cubecol[2] = 0.;
  cubecol[3] = 1.;
  planecol[0] = 1.;
  planecol[1] = 0.5;
  planecol[2] = 0.;
  planecol[3] = 1.;
  textcol[0] = 160;
  textcol[1] = 100;
  textcol[2] = 40;
}


BoxViewSlice::BoxViewSlice( Action* action )
  : d( new Private )
{
  d->action = action;
}


BoxViewSlice::~BoxViewSlice()
{
  delete d;
}


void BoxViewSlice::beginTrackball( int x, int y )
{
  initOjects();
}


void BoxViewSlice::moveTrackball( int x, int y )
{
  updateRect();
  QGraphicsView *gview = graphicsView();
  if( !gview || !d->textEnabled )
    return;
  AWindow3D *win = static_cast<AWindow3D *>( d->action->view()->aWindow() );
  const Quaternion & sliceq = win->sliceQuaternion();
  Point3df pos = win->GetPosition();
  Point3df normal = sliceq.transformInverse( Point3df( 0, 0, 1 ) );
  normal.normalize();
  QString text = QString( "<h4>" ) + ControlledWindow::tr( "slice plane:" )
    + QString( "</h4><p>" ) + ControlledWindow::tr( "normal: " )
    + QString::number( normal[0], 'f', 2 ) + QString( ", " )
    + QString::number( normal[1], 'f', 2 ) + QString( ", " )
    + QString::number( normal[2], 'f', 2 ) + QString( "<br/>" )
    + ControlledWindow::tr( "center: " )
    + QString::number( pos[0], 'f', 1 ) + QString( ", " )
    + QString::number( pos[1], 'f', 1 ) + QString( ", " )
    + QString::number( pos[2], 'f', 1 ) + QString( "</p>" );
  updateText( text );
}


void BoxViewSlice::endTrackball( int x, int y )
{
  removeObjects();
}


void BoxViewSlice::initOjects()
{
  buildCube();
  buildPlane();
  updateRect();
  buildSmallBox();
  AWindow* window = d->action->view()->aWindow();
  window->registerObject( d->cube.get(), true );
  if( !d->rect.isNull() )
    window->registerObject( d->rect.get(), true );
  window->registerObject( d->smallobj.get(), true );
}


void BoxViewSlice::buildSmallBox()
{
  if( !d->smallobj.isNull() )
    return; // already done

  vector<AObject *> objs;
  objs.reserve( 2 + d->otherobjects.size() );
  objs.push_back( d->cube.get() );
  if( !d->rect.isNull() )
    objs.push_back( d->rect.get() );
  list<rc_ptr<AObject> >::const_iterator io, eo = d->otherobjects.end();
  for( io=d->otherobjects.begin(); io!=eo; ++io )
    objs.push_back( io->get() );
  TransformedObject *smallobj 
    = new TransformedObject( objs, true, false, Point3df( 0, 0, 0 ), true );
  smallobj->setName( theAnatomist->makeObjectName( "smallbox" ) );
  d->smallobj.reset( smallobj );
  theAnatomist->registerObject( smallobj, false );
  theAnatomist->releaseObject( smallobj );
  smallobj->setScale( 1.3 / objectsSize() );
}


void BoxViewSlice::buildCube()
{
  GLWidgetManager *view = static_cast<GLWidgetManager *>( d->action->view() );
  Point3df bbmin = view->boundingMin();
  Point3df bbmax = view->boundingMax();
  AimsTimeSurface<2, Void> *mesh = SurfaceGenerator::parallelepiped_wireframe(
    bbmin, bbmax );
  if( !d->cube.isNull() )
  {
    ASurface<2> *acube = static_cast<ASurface<2> *>( d->cube.get() );
    acube->setSurface( mesh );
    acube->glAPI()->glSetChanged( GLComponent::glGEOMETRY );
  }
  else
  {
    ASurface<2> *acube = new ASurface<2>;
    acube->setSurface( mesh );
    acube->setName( theAnatomist->makeObjectName( "cube" ) );
    theAnatomist->registerObject( acube, false );
    if( d->ref )
      acube->setReferential( d->ref );
    else
      acube->setReferential( d->action->view()->aWindow()->getReferential() );
    Material mat = acube->GetMaterial();
    mat.SetDiffuse( d->cubecol[0], d->cubecol[1], d->cubecol[2], 
                    d->cubecol[3] );
    mat.setRenderProperty( Material::Ghost, 1 );
    mat.setRenderProperty( Material::RenderMode, Material::Wireframe );
    acube->SetMaterial( mat );
    d->cube.reset( acube );
    theAnatomist->releaseObject( acube );
  }
}


void BoxViewSlice::buildPlane()
{
  if( !d->planeEnabled || !d->rect.isNull() )
    return;

  GLWidgetManager *view = static_cast<GLWidgetManager *>( d->action->view() );
  Point3df bbmin = view->boundingMin();
  Point3df bbmax = view->boundingMax();
  AimsTimeSurface<2, Void> *mesh = new AimsTimeSurface<2, Void>;
  (*mesh)[0];
  ASurface<2> *arect = new ASurface<2>;
  arect->setSurface( mesh );
  arect->setName( theAnatomist->makeObjectName( "plane" ) );
  theAnatomist->registerObject( arect, false );
  if( d->ref )
    arect->setReferential( d->ref );
  else
    arect->setReferential( d->action->view()->aWindow()->getReferential() );
  Material mat = arect->GetMaterial();
  mat.SetDiffuse( d->planecol[0], d->planecol[1], d->planecol[2], 
                  d->planecol[3] );
  mat.setRenderProperty( Material::Ghost, 1 );
  mat.setRenderProperty( Material::RenderMode, Material::Wireframe );
  mat.setLineWidth( 2. );
  arect->SetMaterial( mat );
  d->rect.reset( arect );
  theAnatomist->releaseObject( arect );
}


void BoxViewSlice::removeObjects()
{
  AWindow *win = d->action->view()->aWindow();
  if( !d->smallobj.isNull() )
    win->unregisterObject( d->smallobj.get() );
  if( !d->rect.isNull() )
    win->unregisterObject( d->rect.get() );
  if( !d->cube.isNull() )
    win->unregisterObject( d->cube.get() );
  QGraphicsView *gview = graphicsView();
  if( !gview )
    return;
  QGraphicsScene *scene = gview->scene();
  if( !scene )
    return;
  clearTmpItems();
}


float BoxViewSlice::objectsSize()
{
  GLWidgetManager *view = static_cast<GLWidgetManager *>( d->action->view() );
  Point3df bbmin = view->boundingMin();
  Point3df bbmax = view->boundingMax();
  bbmax -= bbmin;
  return max( max( bbmax[0], bbmax[1] ), bbmax[2] );
}


namespace
{

  inline
  pair<bool, float> range_check( const pair<bool, float> & lamb )
  {
    if( lamb.first && lamb.second >= 0 && lamb.second <= 1 )
      return make_pair( true, lamb.second );
    return make_pair( false, 0 );
  }

}


void BoxViewSlice::updateRect()
{
  if( d->rect.isNull() )
    return;

  ASurface<2> *arect = static_cast<ASurface<2> *>( d->rect.get() );
  rc_ptr<AimsTimeSurface<2, Void> > rect = arect->surface();
  vector<Point3df> & vert = (*rect)[0].vertex();
  vector<AimsVector<uint32_t, 2> > & poly = (*rect)[0].polygon();
  GLWidgetManager *view = static_cast<GLWidgetManager *>( d->action->view() );
  AWindow3D *win = static_cast<AWindow3D *>( view->aWindow() );
  Point3df bbmin = view->boundingMin();
  Point3df bbmax = view->boundingMax();
  const Quaternion & sliceq = win->sliceQuaternion();
  Point3df pos = win->GetPosition();
  Point3df normal = sliceq.transformInverse( Point3df( 0, 0, 1 ) );
  float dplane = -normal.dot( pos );
  // intersect with 12 edge lines
  float x = bbmax[0] - bbmin[0]; // bbox len to normalize lambda intersects in [0-1]
  float y = bbmax[1] - bbmin[1];
  float z = bbmax[2] - bbmin[2];
  vector<pair<Point3df, Point3df> > edges( 12 );
  // each edge is a pair (vec, x0): direction, origin point
  edges[0] = make_pair( Point3df( 0, 0, z ), bbmin );
  edges[1] = make_pair( Point3df( 0, 0, z ),
                        Point3df( bbmin[0], bbmax[1], bbmin[2] ) );
  edges[2] = make_pair( Point3df( 0, 0, z ),
                        Point3df( bbmax[0], bbmin[1], bbmin[2] ) );
  edges[3] = make_pair( Point3df( 0, 0, z ),
                        Point3df( bbmax[0], bbmax[1], bbmin[2] ) );
  edges[4] = make_pair( Point3df( x, 0, 0 ), bbmin );
  edges[5] = make_pair( Point3df( x, 0, 0 ),
                        Point3df( bbmin[0], bbmax[1], bbmin[2] ) );
  edges[6] = make_pair( Point3df( x, 0, 0 ),
                        Point3df( bbmin[0], bbmin[1], bbmax[2] ) );
  edges[7] = make_pair( Point3df( x, 0, 0 ),
                        Point3df( bbmin[0], bbmax[1], bbmax[2] ) );
  edges[8] = make_pair( Point3df( 0, y, 0 ), bbmin );
  edges[9] = make_pair( Point3df( 0, y, 0 ),
                        Point3df( bbmax[0], bbmin[1], bbmin[2] ) );
  edges[10] = make_pair( Point3df( 0, y, 0 ),
                         Point3df( bbmin[0], bbmin[1], bbmax[2] ) );
  edges[11] = make_pair( Point3df( 0, y, 0 ),
                         Point3df( bbmax[0], bbmin[1], bbmax[2] ) );
  vector< pair<bool, float> > intersects( 12 );
  unsigned i;
  for( i=0; i<12; ++i )
  {
    pair<Point3df, Point3df> & e = edges[i];
    intersects[i] = range_check( lineIntersect( normal, dplane, e.first,
                                                e.second ) );
  }
  // intersect points
  vector<Point3df> pintersects( 12 );
  for( i=0; i<12; ++i )
  {
    pair<bool, float> & lambda = intersects[i];
    pair<Point3df, Point3df> & edge = edges[i];
    if( lambda.first )
      pintersects[i] = edge.second + lambda.second * edge.first;
  }

  // get intersects for each cube face
  const uint32_t cubefaces[][4] = {
    { 0, 1, 8, 10 },
    { 0, 2, 4, 6 },
    { 4, 5, 8, 9 },
    { 2, 3, 9, 11 },
    { 6, 7, 10, 11 },
    { 1, 3, 5, 7 }
  };

  map<int, int> edgelist; // edge point num -> final vertex num
  map<int, int>::iterator ielist, eelist = edgelist.end();
  vert.clear();
  poly.clear();
  unsigned face, edge, j;
  vector<int> fint;
  fint.reserve( 4 ); // face intersection list, to get them unique

  for( face=0; face<6; ++face ) // each face
  {
    fint.clear();
    const uint32_t* const & facedef = cubefaces[ face ];
    for( edge=0; edge<4; ++edge )
    {
      i = facedef[edge];
      if( intersects[i].first )
        fint.push_back( i );
    }
    if( fint.size() >= 2 ) // there is a line on this face
    {
      if( fint.size() > 2 && 
        ( pintersects[ fint[1] ] - pintersects[ fint[0] ] ).norm2() < 1e-5 )
        fint[1] = fint[2]; // keep only 2 distinct points
      ielist = edgelist.find( fint[0] );
      if( ielist == eelist ) // point not used yet
      {
        i = edgelist.size();
        edgelist[ fint[0] ] = i; // insert point, with a new vertex number
        vert.push_back( pintersects[ fint[0] ] );
      }
      else
        i = ielist->second; // use existing vertex num for this point
      ielist = edgelist.find( fint[1] );
      if( ielist == eelist ) // point not used yet
      {
        j = edgelist.size();
        edgelist[ fint[1] ] = j; // insert point, with a new vertex number
        vert.push_back( pintersects[ fint[1] ] );
      }
      else
        j = ielist->second; // use existing vertex num for this point
      poly.push_back( AimsVector<uint32_t, 2>( i, j ) );
    }
  }

  arect->UpdateMinAndMax();
  arect->glAPI()->glSetChanged( GLComponent::glGEOMETRY );
  arect->setChanged();
  arect->notifyObservers( this );
}


pair<bool, float> BoxViewSlice::lineIntersect( const Point3df & pnorm, float d,
  const Point3df & linedir, const Point3df & x0 )
{
  float pl = pnorm.dot( linedir );
  if( pl == 0 )
    return make_pair( false, 0.F );
  float lamb = - ( pnorm.dot( x0 ) + d ) / pl;
  return make_pair( true, lamb );
}


QGraphicsView* BoxViewSlice::graphicsView()
{
  GLWidgetManager* glw = static_cast<GLWidgetManager *>( d->action->view() );
  QWidget* parent = glw->qglWidget()->parentWidget();
  QGraphicsView* gview = dynamic_cast<QGraphicsView *>( parent );
  if( gview )
    return gview;
  return 0;
}


void BoxViewSlice::updateText( const QString & text )
{
  if( !d->textEnabled )
    return;

  QGraphicsView *gview = graphicsView();
  QGraphicsScene *scene = gview->scene();
  if( !scene )
  {
    scene = new QGraphicsScene( gview );
    gview->setScene( scene );
  }
  clearTmpItems();
  drawText( 0, 0, text );
}


void BoxViewSlice::drawText( float posx, float posy, const QString & text )
{
  QGraphicsView *gview = graphicsView();
  QGraphicsTextItem *gtext = new QGraphicsTextItem;
  gtext->setDefaultTextColor( QColor( d->textcol[0], d->textcol[1], 
                                      d->textcol[2] ) );
  QFont font = gtext->font();
  font.setPointSize( 8 );
  gtext->setFont( font );
  gtext->setHtml( text );
  QTransform tr = gtext->transform();
  tr.translate( posx, posy );
  gtext->setTransform( tr );
  gview->scene()->addItem( gtext );
  d->tmpitems.push_back( gtext );
}


void BoxViewSlice::clearTmpItems()
{
  QGraphicsView *gview = graphicsView();
  if( !gview )
    return;
  QGraphicsScene *scene = gview->scene();
  list<QGraphicsItem *>::iterator it, et = d->tmpitems.end();
  for( it=d->tmpitems.begin(); it!=et; ++it )
  {
    scene->removeItem( *it );
    delete *it;
  }
  d->tmpitems.clear();
}


void BoxViewSlice::setCubeColor( float r, float g, float b, float a )
{
  d->cubecol[0] = r;
  d->cubecol[1] = g;
  d->cubecol[2] = b;
  d->cubecol[3] = a;
  if( !d->cube.isNull() )
  {
    Material & mat = d->cube->GetMaterial();
    mat.SetDiffuse( r, g, b, a );
    d->cube->SetMaterial( mat );
  }
}


void BoxViewSlice::setPlaneColor( float r, float g, float b, float a )
{
  d->planecol[0] = r;
  d->planecol[1] = g;
  d->planecol[2] = b;
  d->planecol[3] = a;
  if( !d->rect.isNull() )
  {
    Material & mat = d->rect->GetMaterial();
    mat.SetDiffuse( r, g, b, a );
    d->rect->SetMaterial( mat );
  }
}


void BoxViewSlice::setTextColor( float r, float g, float b )
{
  d->textcol[0] = int( r * 255.99 );
  d->textcol[1] = int( g * 255.99 );
  d->textcol[2] = int( b * 255.99 );
}


void BoxViewSlice::setObjectsReferential( Referential* ref )
{
  if( d->ref == ref )
    return;
  d->ref = ref;
  if( !ref )
    ref = d->action->view()->aWindow()->getReferential();
  if( !d->cube.isNull() )
    d->cube->setReferential( ref );
  if( !d->rect.isNull() )
    d->rect->setReferential( ref );
}


void BoxViewSlice::enablePlane( bool x )
{
  d->planeEnabled = x;
  if( d->rect )
  {
    d->rect.reset();
    if( d->smallobj )
    {
      d->smallobj.reset();
      buildSmallBox();
    }
  }
}


void BoxViewSlice::enableText( bool x )
{
  d->textEnabled = x;
}


void BoxViewSlice::addObject( rc_ptr<AObject> obj )
{
  d->otherobjects.push_back( obj );
  if( !d->smallobj.isNull() )
  {
    d->smallobj.reset();
    buildSmallBox();
  }
}


