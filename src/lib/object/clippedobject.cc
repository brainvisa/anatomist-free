#include <anatomist/object/clippedobject.h>
#include <anatomist/object/actions.h>
#include <anatomist/reference/Transformation.h>
#include <anatomist/reference/Referential.h>
#include <anatomist/window3D/window3D.h>
#include <anatomist/window3D/renderContext.h>
#include <anatomist/primitive/primitive.h>
#include <anatomist/control/qObjTree.h>
#include <anatomist/application/settings.h>
#include <anatomist/surface/glcomponent.h>
#include <anatomist/window/glwidget.h>
#include <anatomist/window/viewstate.h>
#include <aims/resampling/quaternion.h>
#include <qpixmap.h>
#include <graph/tree/tree.h>
#include <qtranslator.h>
#include <QOpenGLShaderProgram>

using namespace anatomist;
using namespace carto;
using namespace std;

namespace
{

struct GLClipScopeItem : public GLItem
{
  GLClipScopeItem( bool enable, int clipID, const Point4df & plane )
      : _enable( enable ), _clipID( clipID ), _plane( plane ) {}

  bool ghost() const override { return false; }

  void callList() const override
  {
    if( _enable )
    {
      glPushAttrib( GL_ENABLE_BIT );
      glEnable( GL_CLIP_DISTANCE2 + _clipID );

      GLdouble pl[4] = { _plane[0], _plane[1], _plane[2], _plane[3] };
      glClipPlane( GL_CLIP_PLANE2 + _clipID, pl );

      setClipPlaneUniform( true );
    }
    else
    {
      setClipPlaneUniform( false );
      glPopAttrib();
    }
  }

private:
  void setClipPlaneUniform( bool active ) const
  {
    GLint progID = 0;
    glGetIntegerv( GL_CURRENT_PROGRAM, &progID );
    if( !progID )
      return;

    if( active )
    {
      string planeName = "u_clipPlane" + to_string( 2 + _clipID );
      GLint loc = glGetUniformLocation( progID, planeName.c_str() );
      if( loc >= 0 )
      {
        GLfloat mv[16];
        glGetFloatv(GL_MODELVIEW_MATRIX, mv);
        QMatrix4x4 modelView(mv);
        modelView = modelView.transposed();
        QMatrix4x4 mvInvT = modelView.inverted().transposed();
        QVector4D planeEye = mvInvT * QVector4D(_plane[0], _plane[1], _plane[2], _plane[3]);
        glUniform4f( loc, planeEye.x(), planeEye.y(), planeEye.z(), planeEye.w() );
      }
    }

    string activeName = "u_clippedObjectActive" ;
    GLint aloc = glGetUniformLocation( progID, activeName.c_str() );
    if( aloc >= 0 )
      glUniform1i( aloc, active ? 1 : 0 );
  }

  bool     _enable;
  int      _clipID;
  Point4df _plane;
};

} 

struct ClippedObject::Private
{
  Private() : clipID( 0 ), worldPlane( 0.f, 0.f, 1.f, 0.f ) {}

  int      clipID;
  Point4df worldPlane;
};

int ClippedObject::registerClass()
{
  return registerObjectType( "ClippedObject" );
}

int ClippedObject::classType()
{
  static int _classType = registerClass();
  return _classType;
}

ClippedObject::ClippedObject( const vector<AObject *> & obj )
    : ObjectVector(), SelfSliceable(), d( new ClippedObject::Private )
{
  _type = classType();

  if( QObjectTree::TypeNames.find( _type ) == QObjectTree::TypeNames.end() )
  {
    string str = Settings::findResourceFile( "icons/list_clippedobject.png" );
    if( !QObjectTree::TypeIcons[ _type ].load( str.c_str() ) )
    {
        QObjectTree::TypeIcons.erase( _type );
        cerr << "Icon " << str << " not found\n";
    }
    QObjectTree::TypeNames[ _type ] = "ClippedObject";
  }

  AObject *o = nullptr;
  for( auto io = obj.begin(); io != obj.end(); ++io )
  {
    o = *io;
    insert( o );

    ClippedObject *co = dynamic_cast<ClippedObject *>( o );
    if( co && co->clipID() >= d->clipID )
        d->clipID = co->clipID() + 1;
  }

  if( size() > 0 )
  {
    o = *begin();
    setReferentialInheritance( o );

    vector<float> bmin, bmax;
    if( boundingBox( bmin, bmax ) )
      _offset = ( Point3df( bmin[0], bmin[1], bmin[2] )
                + Point3df( bmax[0], bmax[1], bmax[2] ) ) / 2.f;
  }
}

ClippedObject::~ClippedObject()
{
  iterator i = begin();
  erase( i );
  delete d;
}

int ClippedObject::clipID() const
{
  return d->clipID;
}

void ClippedObject::computeWorldPlane( const RenderContext & rc )
{
  const Point4df & p = plane();

  const Referential *objref = getReferential();
  const Referential *wr    = nullptr;

  const SliceViewState *svs = rc.getViewState().sliceVS();
  if( objref )
  {
    if( svs )
      wr = svs->winref;
    else if( rc.getViewState().window )
      wr = rc.getViewState().window->getReferential();
  }

  Transformation *trans = theAnatomist->getTransformation( objref, wr );

  if( trans )
  {
    Point3df normal( p[0], p[1], p[2] );
    normal.normalize();
    normal = trans->motion().transformUnitNormal( normal );

    d->worldPlane[0] = normal[0];
    d->worldPlane[1] = normal[1];
    d->worldPlane[2] = normal[2];

    Point3df p3( 0.f );
    if(      p[2] != 0.f ) p3[2] = -p[3] / p[2];
    else if( p[1] != 0.f ) p3[1] = -p[3] / p[1];
    else                   p3[0] = -p[3] / p[0];

    p3 = trans->transform( p3 );
    d->worldPlane[3] = -( d->worldPlane[0] * p3[0]
                        + d->worldPlane[1] * p3[1]
                        + d->worldPlane[2] * p3[2] );
  }
  else
  {
    d->worldPlane = p;
  }

  if( wr && wr->isDirect() )
  {
    d->worldPlane[0] *= -1;
    d->worldPlane[1] *= -1;
    d->worldPlane[2] *= -1;
    d->worldPlane[3] *= -1;
  }
}




bool ClippedObject::render( PrimList & prim, RenderContext & rc )
{

  const SliceViewState *osvs = rc.getViewState().sliceVS();
  SliceViewState svs;
  if( !osvs || !osvs->vieworientation )
  {
    if( osvs )
      svs = *osvs;
    else
      static_cast<ViewState &>( svs ) = rc.getViewState();

    const AWindow3D *w3 =
        dynamic_cast<const AWindow3D *>( rc.getViewState().window );
    if( w3 )
    {
      svs.orientation = &w3->sliceQuaternion();
      svs.winref = w3->getReferential();
      const GLWidgetManager *glv =
          dynamic_cast<const GLWidgetManager *>( w3->view() );
      if( glv )
          svs.vieworientation = &glv->quaternion();
    }
  }

  const bool firstlist = prim.empty();
  PrimList::iterator ip = firstlist ? prim.end() : std::prev( prim.end() );

  computeWorldPlane( rc );

  list<carto::shared_ptr<AObject>> subObjects;
  for( auto it = begin(); it != end(); ++it )
      subObjects.push_back( rc_ptr<AObject>( *it ) );

  RenderMode rcmode = RenderMode::Full;
  if( rc.getViewState().selectRenderMode != ViewState::glSELECTRENDER_NONE )
    rcmode = RenderMode::Selection;

  const bool hasRendered = rc.renderObjects( subObjects,
                              rcmode,
                              rc.getViewState().selectRenderMode );

  if( hasRendered )
  {
    auto insertPos = firstlist ? prim.begin() : std::next( ip );

    prim.insert( insertPos,
        rc_ptr<GLItem>( new GLClipScopeItem(
            true, d->clipID, d->worldPlane ) ) );

    prim.push_back(
        rc_ptr<GLItem>( new GLClipScopeItem(
            false, d->clipID, d->worldPlane ) ) );
  }

  return hasRendered;
}

void ClippedObject::sliceChanged()
{
  obsSetChanged( GLComponent::glGEOMETRY );
  setChanged();
}

void ClippedObject::update( const Observable *observable, void * )
{
  for( auto i = begin(), e = end(); i != e; ++i )
  {
    if( observable != *i )
      continue;

    const AObject *obj = static_cast<const AObject *>( observable );

    if( obj->obsHasChanged( GLComponent::glTEXIMAGE )
      || obj->obsHasChanged( GLComponent::glTEXENV )
      || obj->obsHasChanged( GLComponent::glREFERENTIAL )
      || obj->obsHasChanged( GLComponent::glBODY )
      || obj->obsHasChanged( GLComponent::glMATERIAL )
      || obj->obsHasChanged( GLComponent::glGEOMETRY ) )
    {
      setChanged();
    }

    updateSubObjectReferential( obj );
  }

  notifyObservers( this );
}

Material & ClippedObject::GetMaterial()
{
  return ( *begin() )->GetMaterial();
}

void ClippedObject::SetMaterial( const Material & mat )
{
  ( *begin() )->SetMaterial( mat );
}

const AObjectPalette * ClippedObject::palette() const
{
  return ( *begin() )->palette();
}

AObjectPalette * ClippedObject::palette()
{
  return ( *begin() )->palette();
}

void ClippedObject::setPalette( const AObjectPalette & pal )
{
  ( *begin() )->setPalette( pal );
  setChanged();
}

Tree * ClippedObject::optionTree() const
{
  static Tree *_optionTree = nullptr;
  if( !_optionTree )
  {
    Tree *t, *t2;
    _optionTree = new Tree( true, "option tree" );
    t  = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "File" ) );
    _optionTree->insert( t );
    t2 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "Save" ) );
    t2->setProperty( "callback", &ObjectActions::saveStatic );
    t->insert( t2 );
    t2 = new Tree( true,
                    QT_TRANSLATE_NOOP( "QSelectMenu", "Rename object" ) );
    t2->setProperty( "callback", &ObjectActions::renameObject );
    t->insert( t2 );
  }
  return _optionTree;
}

carto::Object ClippedObject::makeHeaderOptions() const
{
  carto::Object opts = ObjectVector::makeHeaderOptions();
  makeSliceHeaderOptions( opts );
  return opts;
}

void ClippedObject::setProperties( carto::Object options )
{
  ObjectVector::setProperties( options );
  setSliceProperties( options );
}