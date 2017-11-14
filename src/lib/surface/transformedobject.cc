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

#include <anatomist/surface/transformedobject.h>
#include <anatomist/window/glwidgetmanager.h>
#include <anatomist/window/controlledWindow.h>
#include <anatomist/reference/Transformation.h>
#include <aims/transformation/affinetransformation3d.h>

using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;

struct TransformedObject::Private
{
  Private( bool followorientation,
           bool followposition,
           const Point3df & pos );

  bool followorientation;
  bool followposition;
  Point3df posoffset;
  Point3df pos;
  bool dynamicoffset;
  Point3df offsetfrom;
  float scale;
};


TransformedObject::Private::Private( bool followorientation,
                                     bool followposition,
                                     const Point3df & pos )
  : followorientation( followorientation ), followposition( followposition ),
    posoffset( 0, 0, 0 ), pos( pos ), dynamicoffset( false ),
    offsetfrom( 0, 0, 0 ), scale( 1. )
{
}


TransformedObject::TransformedObject( const vector<AObject *> & obj,
                                      bool followorientation,
                                      bool followposition,
                                      const Point3df & pos, bool strongref )
  : d( new Private( followorientation, followposition, pos ) )
{
  using carto::shared_ptr;

  vector<AObject *>::const_iterator io, eo = obj.end();
  shared_ptr<AObject>::ReferenceType rtype = shared_ptr<AObject>::WeakShared;
  if( strongref )
    rtype = shared_ptr<AObject>::Strong;
  for( io=obj.begin(); io!=eo; ++io )
    insert( shared_ptr<AObject>( rtype, *io ) );
}


TransformedObject::TransformedObject(
  const vector<carto::shared_ptr<AObject> > & obj,
  bool followorientation,
  bool followposition,
  const Point3df & pos )
  : d( new Private( followorientation, followposition, pos ) )
{
  using carto::shared_ptr;

  vector<shared_ptr<AObject> >::const_iterator io, eo = obj.end();
  for( io=obj.begin(); io!=eo; ++io )
    insert( *io );
}


TransformedObject::~TransformedObject()
{
  delete d;
}


bool TransformedObject::renderingIsObserverDependent() const
{
  return true;
}


bool TransformedObject::render( PrimList & pl, const ViewState & vs )
{
  // change transformation matrixes
  setupTransforms( pl, vs );

  // render sub-objects
  bool res = false;
  iterator io, eo = end();
  for( io=begin(); io!=eo; ++io )
    res |= (*io)->render( pl, vs );

  // pop matrixes
  popTransformationMatrixes( pl );

  return res;
}


void TransformedObject::setupTransforms( GLPrimitives & pl,
                                         const ViewState & vs )
{
  if( d->followorientation && d->followposition )
    return;
  GLList *gll = new GLList;
  gll->generate();
  pl.push_back( RefGLItem( gll ) );
  glNewList( gll->item(), GL_COMPILE );

  // save matrixes
  glMatrixMode( GL_MODELVIEW );
  glPushMatrix();
  glMatrixMode( GL_PROJECTION );
  glPushMatrix();

  if( !d->followposition )
  {
    int winDim = 70;
    glPushAttrib( GL_VIEWPORT_BIT );
    glViewport( 0, 0, winDim, winDim );
    glLoadIdentity();
    GLfloat orthoMinX = - 1.5;
    GLfloat orthoMinY = - 1.5;
    GLfloat orthoMinZ = - 1.5;
    GLfloat orthoMaxX =   1.5;
    GLfloat orthoMaxY =   1.5;
    GLfloat orthoMaxZ =   1.5;
    glOrtho( orthoMinX, orthoMaxX, orthoMinY, orthoMaxY, orthoMinZ,
             orthoMaxZ );
  }
  AWindow *win = vs.window;
  GLWidgetManager *view = 0;
  if( win )
  {
    ControlledWindow *cw = dynamic_cast<ControlledWindow *>( win );
    view = dynamic_cast<GLWidgetManager *>( cw->view() );
  }
  if( !view )
    return;

  Transformation *otov = theAnatomist->getTransformation(
    getReferential(), view->aWindow()->getReferential() );
  Point3df posoffset = d->posoffset;
  Point3df pos = d->pos;
  Point3df offsetfrom = d->offsetfrom;
  if( otov )
  {
    posoffset = otov->transform( posoffset )
                - otov->transform( Point3df( 0., 0., 0. ) );
    pos = otov->transform( pos );
    offsetfrom = otov->transform( offsetfrom );
  }
//   cout << "posoffset: " << d->posoffset << ": " << posoffset << endl;
//   cout << "offsetfrom: " << d->offsetfrom << ": " << offsetfrom << endl;
//   cout << "pos: " << d->pos << ": " << pos << endl;

  if( !d->followorientation )
  {
    glTranslatef( posoffset[0], posoffset[1], posoffset[2] );
    if( d->dynamicoffset )
    {
//       cout << "dynamicoffset\n";
      // project offsetfrom point into camera coords
      AffineTransformation3d r \
        = AffineTransformation3d( view->quaternion() ); //.inverse();
      AffineTransformation3d p;
      if( view->invertedZ() )
        p.rotation()( 2, 2 ) = -1.;
      r = ( p * r ).inverse();
      Point3df proj = r.transform( pos - offsetfrom );

      // align correct corner
      vector<float> bmin( 3, 0.f ), bmax( 3, 0.f ), bmt, bmat;
      iterator io, eo = end();
      bool first = true;
      for( io=begin(); io!=eo; ++io ) // TODO: use refs
      {
        if( (*io)->boundingBox( bmt, bmat ) )
        {
          if( first )
          {
            bmin = bmt;
            bmax = bmat;
            first = false;
          }
          else
          {
            if( bmin[0] > bmt[0] )
              bmin[0] = bmt[0];
            if( bmin[1] > bmt[1] )
              bmin[1] = bmt[1];
            if( bmin[2] > bmt[2] )
              bmin[2] = bmt[2];
            if( bmax[0] < bmat[0] )
              bmax[0] = bmat[0];
            if( bmax[0] < bmat[1] )
              bmax[0] = bmat[0];
            if( bmax[0] < bmat[0] )
              bmax[0] = bmat[0];
          }
        }
      }
      Point3df dynoffset( 0, 0, 0 );
      if( proj[0] < 0 )
        dynoffset[0] = -bmax[0];
      else
        dynoffset[0] = -bmin[0];
      if( proj[1] > 0 )
        dynoffset[1] = bmax[1];
      else
        dynoffset[1] = bmin[1];
//       cout << "dynoffset: " << dynoffset << endl;
      glTranslatef( dynoffset[0], dynoffset[1], dynoffset[2] );
    }
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    /* keep the translation part of the view orientation
    (mut apply the inverse rotation to it) */
    Point3df trans = pos - view->rotationCenter();
    AffineTransformation3d r \
      = AffineTransformation3d( view->quaternion() ).inverse();
    AffineTransformation3d p;
    p.translation()[0] = trans[0];
    p.translation()[1] = trans[1];
    p.translation()[2] = trans[2];
    AffineTransformation3d inv;
    if( view->invertedZ() )
      inv.rotation()( 2, 2 ) = -1;
    r = r * inv * p;
    glTranslatef( r.translation()[0], r.translation()[1],
      r.translation()[2] );
    float sym = ( view->invertedZ() ? 1 : -1 );
    glScalef( d->scale * sym, -d->scale * sym, d->scale * sym );
  }
  else
  {
    glMatrixMode( GL_MODELVIEW );
    /* keep the rotation part of the view orientation, removing the
      translation part */
    Point3df trans = view->rotationCenter();
    // TODO: find a better center than this -0.5
    glTranslatef( trans[0] - 0.5, trans[1] - 0.5, trans[2] + 0.5 );
    glScalef( d->scale, d->scale, d->scale );
  }

    /* counter the auto transfo done by underlying objects
       glHandleTransformation()
       FIXME: each object will transfor from its own ref to the view's
       so this code only works if all children are in the same ref.
       The right solution would be to disable glHandleTransformation on
       children, but our API doesn't allow it.
    */
  Transformation *vtoo = 0;
  iterator i = begin();
  if( i != end() )
    vtoo = theAnatomist->getTransformation(
      view->aWindow()->getReferential(), (*i)->getReferential() );
  if( vtoo )
  {
    GLfloat mat[16];

    // write 4x4 matrix in column
    mat[0] = vtoo->Rotation( 0, 0 );
    mat[1] = vtoo->Rotation( 1, 0 );
    mat[2] = vtoo->Rotation( 2, 0 );
    mat[3] = 0;
    mat[4] = vtoo->Rotation( 0, 1 );
    mat[5] = vtoo->Rotation( 1, 1 );
    mat[6] = vtoo->Rotation( 2, 1 );
    mat[7] = 0;
    mat[8] = vtoo->Rotation( 0, 2 );
    mat[9] = vtoo->Rotation( 1, 2 );
    mat[10] = vtoo->Rotation( 2, 2 );
    mat[11] = 0;
    mat[12] = vtoo->Translation( 0 );
    mat[13] = vtoo->Translation( 1 );
    mat[14] = vtoo->Translation( 2 );
    mat[15] = 1;

    glMultMatrixf( mat );
  }

  glEndList();
}


void TransformedObject::setScale( float scale )
{
  d->scale = scale;
}


float TransformedObject::scale() const
{
  return d->scale;
}


void TransformedObject::popTransformationMatrixes( GLPrimitives & pl )
{
  if( d->followorientation && d->followposition )
    return;
  GLList *gll = new GLList;
  gll->generate();
  pl.push_back( RefGLItem( gll ) );
  glNewList( gll->item(), GL_COMPILE );
  glMatrixMode( GL_PROJECTION );
  if( !d->followposition )
    glPopAttrib();
  glPopMatrix();
  glMatrixMode( GL_MODELVIEW );
  glPopMatrix();
  glEndList();
}


void TransformedObject::setPosition( const Point3df & pos )
{
  d->pos = pos;
}


Point3df TransformedObject::position() const
{
  return d->pos;
}


void TransformedObject::setOffset( const Point3df & posoffset )
{
  d->posoffset = posoffset;
}


Point3df TransformedObject::offset() const
{
  return d->posoffset;
}


void TransformedObject::setDynamicOffsetFromPoint( const Point3df & pos )
{
  d->offsetfrom = pos;
  d->dynamicoffset = true;
}


void TransformedObject::removeDynamicOffset()
{
  d->dynamicoffset = false;
}


bool TransformedObject::usesDynamicOffset() const
{
  return d->dynamicoffset;
}


Point3df TransformedObject::dynamicOffsetFromPoint() const
{
  return d->offsetfrom;
}


