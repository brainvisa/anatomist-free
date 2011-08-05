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

using namespace anatomist;
using namespace std;

TransformedObject::TransformedObject( const vector<AObject *> & obj,
                                      bool followorientation,
                                      bool followposition )
  : _followorientation( followorientation ), _followposition( followposition )
{
  vector<AObject *>::const_iterator io, eo = obj.end();
  for( io=obj.begin(); io!=eo; ++io )
    insert( *io );
}


TransformedObject::~TransformedObject()
{
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
  if( _followorientation && _followposition )
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

  if( !_followposition )
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

  glEndList();
}


void TransformedObject::popTransformationMatrixes( GLPrimitives & pl )
{
  if( _followorientation && _followposition )
    return;
  GLList *gll = new GLList;
  gll->generate();
  pl.push_back( RefGLItem( gll ) );
  glNewList( gll->item(), GL_COMPILE );
  glMatrixMode( GL_PROJECTION );
  if( !_followposition )
    glPopAttrib();
  glPopMatrix();
  glMatrixMode( GL_MODELVIEW );
  glPopMatrix();
  glEndList();
}


