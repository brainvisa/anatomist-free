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

#include <anatomist/object/clippedobject.h>
#include <anatomist/object/actions.h>
#include <anatomist/reference/Transformation.h>
#include <anatomist/window3D/window3D.h>
#include <anatomist/control/qObjTree.h>
#include <anatomist/application/settings.h>
#include <anatomist/surface/glcomponent.h>
#include <anatomist/window/glwidget.h>
#include <anatomist/window/viewstate.h>
#include <aims/resampling/quaternion.h>
#include <qpixmap.h>
#include <graph/tree/tree.h>
#include <qtranslator.h>

using namespace anatomist;
using namespace carto;
using namespace std;


struct ClippedObject::Private
{
  Private() : clipID( 0 ) {}

  int clipID;
};


int ClippedObject::registerClass()
{
  int	type = registerObjectType( "ClippedObject" );
  return type;
}


ClippedObject::ClippedObject( const vector<AObject *> & obj )
  : ObjectVector(), SelfSliceable(), d( new ClippedObject::Private )
{
  _type = classType();

  if( QObjectTree::TypeNames.find( _type ) == QObjectTree::TypeNames.end() )
  {
    string str = Settings::globalPath() + "/icons/list_clippedobject.png";
    if( !QObjectTree::TypeIcons[ _type ].load( str.c_str() ) )
    {
      QObjectTree::TypeIcons.erase( _type );
      cerr << "Icon " << str.c_str() << " not found\n";
    }

    QObjectTree::TypeNames[ _type ] = "ClippedObject";
  }

  vector<AObject *>::const_iterator	io, fo=obj.end();
  vector<AObject *>			vol;
  AObject				*o = 0;

  for( io=obj.begin(); io!=fo; ++io )
  {
    o = *io;
    insert( o );
    ClippedObject *co = dynamic_cast<ClippedObject *>( o );
    if( co )
    {
      if( co->clipID() >= d->clipID )
        d->clipID = co->clipID() + 1;
    }
  }

  if( size() > 0 )
  {
    o = *begin();
    setReferentialInheritance( o );

/*    Point3df	vs = o->VoxelSize();
    _offset = Point3df( ( o->MinX2D() + o->MaxX2D() ) * vs[0] / 2, 
                        ( o->MinY2D() + o->MaxY2D() ) * vs[1] / 2, 
                        ( o->MinZ2D() + o->MaxZ2D() ) * vs[2] / 2 );*/
    Point3df bmin, bmax;
    if( boundingBox( bmin, bmax ) )
      _offset = (bmin + bmax) / 2;

  }
}


ClippedObject::~ClippedObject()
{
  iterator	i = begin();
  erase( i );
  delete d;
}


int ClippedObject::classType()
{
  static int	_classType = registerClass();
  return _classType;
}


int ClippedObject::clipID() const
{
  return d->clipID;
}


bool ClippedObject::render( PrimList & prim, const ViewState & state )
{
  // cout << "ClippedObject::render " << quaternion().vector() << endl;
  /* always use a SliceViewState since the underlying object may need
     orientation information (a VolRender needs view orientation) */
  const SliceViewState *osvs = state.sliceVS();
  SliceViewState svs;
  if( !osvs || !osvs->vieworientation )
  {
    if( osvs )
      svs = *osvs;
    const AWindow3D * w3 = dynamic_cast<const AWindow3D *>( state.window );
    if( w3 )
    {
      svs.orientation = &w3->sliceQuaternion();
      svs.winref = w3->getReferential();
      const GLWidgetManager
          *glv = dynamic_cast<const GLWidgetManager *>( w3->view() );
      if( glv )
        svs.vieworientation = &glv->quaternion();
    }
    osvs = &svs;
  }

  bool firstlist = false;
  PrimList::iterator ip = prim.end();
  if( ip == prim.begin() )
    firstlist = true;
  else
    --ip;
  bool hasrendered = false;
  iterator i, e = end();

  for( i=begin(); i!=e; ++i )
  {
    AObject* obj = *i;
    if( obj->render( prim, *osvs ) )
      hasrendered = true;
  }
  if( hasrendered )
  {
    // clipping
    if( !firstlist )
      ++ip;
    else
      ip = prim.begin();
    GLList* gll = new GLList;
    gll->generate();
    glNewList( gll->item(), GL_COMPILE );
    glPushAttrib( GL_ENABLE_BIT );
    glEnable( GL_CLIP_PLANE2 + d->clipID );
    GLdouble pl[4];

    const SliceViewState  *svs = state.sliceVS();
    const Referential *wr = 0, *objref = getReferential();
    if( objref )
    {
      if( svs )
        wr = svs->winref;
      else if( state.window )
        wr = state.window->getReferential();
    }
    Transformation *trans = theAnatomist->getTransformation( objref, wr );
    const Point4df & p = plane();
    if( trans )
    {
      // transform clipping plane
      Point3df p2( p[0], p[1], p[2] );
      p2.normalize();
      p2 = trans->motion().transformUnitNormal( p2 );
      pl[0] = p2[0];
      pl[1] = p2[1];
      pl[2] = p2[2];
      Point3df p3( 0. );
      if( p[2] != 0 )
        p3[2] = -p[3] / p[2];
      else if( p[1] != 0 )
        p3[1] = -p[3] / p[1];
      else
        p3[0] = -p[3] / p[0];
      p3 = trans->transform( p3 );
      pl[3] = - ( pl[0] * p3[0] + pl[1] * p3[1] + pl[2] * p3[2] );
    }
    else
    {
      pl[0] = p[0];
      pl[1] = p[1];
      pl[2] = p[2];
      pl[3] = p[3];
    }
    glClipPlane( GL_CLIP_PLANE2 + d->clipID, pl );
    glEndList();
    prim.insert( ip, rc_ptr<GLItem>( gll ) );

    // finish clipping
    GLList *gll2 = new GLList;
    gll2->generate();
    glNewList( gll2->item(), GL_COMPILE );
    glPopAttrib();
    glEndList();
    prim.insert( prim.end(), rc_ptr<GLItem>( gll2 ) );
  }
  return hasrendered;
}


Tree* ClippedObject::optionTree() const
{
  static Tree*	_optionTree = 0;

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
    }
  return( _optionTree );
}


Material & ClippedObject::GetMaterial()
{
  return( (*begin())->GetMaterial() );
}


void ClippedObject::SetMaterial( const Material & mat )
{
  (*begin())->SetMaterial( mat );
}


const AObjectPalette* ClippedObject::palette() const
{
  const AObject	*obj = *begin();
  return obj->palette();
}


AObjectPalette* ClippedObject::palette()
{
  AObject	*obj = *begin();
  return obj->palette();
}


void ClippedObject::setPalette( const AObjectPalette & palette )
{
  AObject	*obj= *begin();
  obj->setPalette( palette );
  setChanged();
}


void ClippedObject::sliceChanged()
{
  obsSetChanged( GLComponent::glGEOMETRY );
  setChanged();
}


void ClippedObject::update( const Observable *observable, void * )
{
  // cout << "ClippedObject::update\n";

  iterator i, e = end();
  for( i=begin(); i!=e; ++i )
    if( observable == *i )
    {
      const AObject *obj = static_cast<const AObject *>( observable );
      // cout << "obj: " << obj << ", : " << obj->name() << endl;

      if( obj->obsHasChanged( GLComponent::glTEXIMAGE ) )
      {
        setChanged();
        // g->glSetTexImageChanged( true, 0 );
      }
      if( obj->obsHasChanged( GLComponent::glTEXENV ) )
      {
        setChanged();
        // g->glSetTexEnvChanged( true, 0 );
      }
      if( obj->obsHasChanged( GLComponent::glREFERENTIAL )
          || obj->obsHasChanged( GLComponent::glBODY ) )
      {
        setChanged();
        // g->glSetChanged( GLComponent::glBODY );
      }
      if( obj->obsHasChanged( GLComponent::glMATERIAL ) )
      {
        setChanged();
        // g->glSetChanged( GLComponent::glMATERIAL );
      }
      if( obj->obsHasChanged( GLComponent::glREFERENTIAL )
          || obj->obsHasChanged( GLComponent::glGEOMETRY ) )
      {
        setChanged();
        // g->glSetChanged( GLComponent::glGEOMETRY );
      }
      updateSubObjectReferential( obj );
    }

  notifyObservers( this );
}

