// /* This software and supporting documentation are distributed by
//  *     Institut Federatif de Recherche 49
//  *     CEA/NeuroSpin, Batiment 145,
//  *     91191 Gif-sur-Yvette cedex
//  *     France
//  *
//  * This software is governed by the CeCILL-B license under
//  * French law and abiding by the rules of distribution of free software.
//  * You can  use, modify and/or redistribute the software under the
//  * terms of the CeCILL-B license as circulated by CEA, CNRS
//  * and INRIA at the following URL "http://www.cecill.info".
//  *
//  * As a counterpart to the access to the source code and  rights to copy,
//  * modify and redistribute granted by the license, users are provided only
//  * with a limited warranty  and the software's author,  the holder of the
//  * economic rights,  and the successive licensors  have only  limited
//  * liability.
//  *
//  * In this respect, the user's attention is drawn to the risks associated
//  * with loading,  using,  modifying and/or developing or reproducing the
//  * software by the user in light of its specific status of free software,
//  * that may mean  that it is complicated to manipulate,  and  that  also
//  * therefore means  that it is reserved for developers  and  experienced
//  * professionals having in-depth computer knowledge. Users are therefore
//  * encouraged to load and test the software's suitability as regards their
//  * requirements in conditions enabling the security of their systems and/or
//  * data to be ensured and,  more generally, to use and operate it in the
//  * same conditions as regards security.
//  *
//  * The fact that you are presently reading this means that you have had
//  * knowledge of the CeCILL-B license and that you accept its terms.
//  */

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

using namespace anatomist;
using namespace carto;
using namespace std;

struct ClippedObject::Private
{
  Private( AObject* obj ) : clipID( 0 ), object( obj ) {}

  int      clipID;
  AObject* object;
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

ClippedObject::ClippedObject( AObject* obj )
  : d( new Private( obj ) )
{
  _type = classType();

  if( QObjectTree::TypeNames.find( _type ) == QObjectTree::TypeNames.end() )
  {
    string str = Settings::findResourceFile( "icons/list_clippedobject.png" );
    if( !QObjectTree::TypeIcons[ _type ].load( str.c_str() ) )
    {
      QObjectTree::TypeIcons.erase( _type );
      cerr << "Icon " << str.c_str() << " not found\n";
    }
    QObjectTree::TypeNames[ _type ] = "ClippedObject";
  }

  if( obj )
  {
    obj->addObserver( this );
    setReferentialInheritance( obj );

    vector<float> bmin, bmax;
    if( obj->boundingBox( bmin, bmax ) )
      _offset = ( Point3df( bmin[0], bmin[1], bmin[2] )
                  + Point3df( bmax[0], bmax[1], bmax[2] ) ) / 2;
  }
}

ClippedObject::~ClippedObject()
{
  if( d->object )
    d->object->deleteObserver( this );

  delete d;
}

int ClippedObject::clipID() const
{
  return d->clipID;
}


AObject* ClippedObject::wrappedObject() const
{
  return d->object;
}

GLComponent* ClippedObject::glAPI()
{
  return d->object ? d->object->glAPI() : nullptr;
}

const GLComponent* ClippedObject::glAPI() const
{
  return d->object ? d->object->glAPI() : nullptr;
}

void ClippedObject::objectUniforms(
  carto::rc_ptr<QOpenGLShaderProgram> shader ) const
{
  //std::cout << "ClippedObject::objectUniforms\n";
  if( d->object )
    d->object->objectUniforms( shader );

  // const Point4df & p = plane();
  // GLdouble pl[4] = { p[0], p[1], p[2], p[3] };

  // const Referential *objref = getReferential();
  // if( objref )
  // {
  //   const Referential *wr = nullptr;
  //   if( !_winList.empty() )
  //     wr = (*_winList.begin())->getReferential();

  //   Transformation *trans = theAnatomist->getTransformation( objref, wr );
  //   if( trans )
  //   {
  //     Point3df p2( p[0], p[1], p[2] );
  //     p2.normalize();
  //     p2 = trans->motion().transformUnitNormal( p2 );
  //     pl[0] = p2[0];
  //     pl[1] = p2[1];
  //     pl[2] = p2[2];

  //     Point3df p3( 0.f );
  //     if     ( p[2] != 0 ) p3[2] = -p[3] / p[2];
  //     else if( p[1] != 0 ) p3[1] = -p[3] / p[1];
  //     else                 p3[0] = -p[3] / p[0];

  //     p3    = trans->transform( p3 );
  //     pl[3] = -( pl[0]*p3[0] + pl[1]*p3[1] + pl[2]*p3[2] );
  //   }

  //   if( wr && wr->isDirect() )
  //   {
  //     pl[0] *= -1; pl[1] *= -1; pl[2] *= -1; pl[3] *= -1;
  //   }
  // }

  // shader->setUniformValue( "u_objectClipPlane",
  //                          (float)pl[0], (float)pl[1],
  //                          (float)pl[2], (float)pl[3] );
  // shader->setUniformValue( "u_useObjectClip", 1 );
}

bool ClippedObject::render( PrimList & prim, RenderContext & rc )
{
  if( !d->object )
    return false;

    std::cout << "clipped subobject address : "<<d->object << std::endl;

  // GLList *gll_on = new GLList;
  // gll_on->generate();
  // glNewList( gll_on->item(), GL_COMPILE );
  // glEnable( GL_CLIP_DISTANCE2 + d->clipID );
  // glEndList();
  // prim.push_back( RefGLItem( gll_on ) );

  size_t before = prim.size();
  bool result = d->object->render( prim, rc );
  std::cout << "ClippedObject::render, prim size before: " << before << ", after: " << prim.size() << std::endl;

  // GLList *gll_off = new GLList;
  // gll_off->generate();
  // glNewList( gll_off->item(), GL_COMPILE );
  // glDisable( GL_CLIP_DISTANCE2 + d->clipID );
  // glEndList();
  // prim.push_back( RefGLItem( gll_off ) );

  return result;
}

bool ClippedObject::Is2DObject()
{
  return d->object ? d->object->Is2DObject() : false;
}

bool ClippedObject::Is3DObject()
{
  return d->object ? d->object->Is3DObject() : false;
}

Material & ClippedObject::GetMaterial()
{
  return d->object->GetMaterial();
}


void ClippedObject::SetMaterial( const Material & mat )
{
  d->object->SetMaterial( mat );
}


const AObjectPalette* ClippedObject::palette() const
{
  return d->object->palette();
}


AObjectPalette* ClippedObject::palette()
{
  return d->object->palette();
}


void ClippedObject::setPalette( const AObjectPalette & palette )
{
  d->object->setPalette( palette );
  setChanged();
}


void ClippedObject::sliceChanged()
{
  obsSetChanged( GLComponent::glGEOMETRY );
  setChanged();
}

void ClippedObject::update( const Observable *observable, void * )
{
  if( observable != d->object )
  {
    notifyObservers( this );
    return;
  }

  const AObject *obj = static_cast<const AObject *>( observable );

  if( obj->obsHasChanged( GLComponent::glTEXIMAGE ) )
    setChanged();

  if( obj->obsHasChanged( GLComponent::glTEXENV ) )
    setChanged();

  if( obj->obsHasChanged( GLComponent::glBODY )
      || obj->obsHasChanged( GLComponent::glREFERENTIAL ) )
    setChanged();

  if( obj->obsHasChanged( GLComponent::glMATERIAL ) )
    setChanged();

  if( obj->obsHasChanged( GLComponent::glGEOMETRY )
      || obj->obsHasChanged( GLComponent::glREFERENTIAL ) )
    setChanged();

  if( obj->obsHasChanged( GLComponent::glREFERENTIAL ) )
    setReferentialInheritance( d->object );

  notifyObservers( this );
}


Tree* ClippedObject::optionTree() const
{
  static Tree *_optionTree = nullptr;

  if( !_optionTree )
  {
    Tree *t, *t2;
    _optionTree = new Tree( true, "option tree" );

    t = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "File" ) );
    _optionTree->insert( t );

    t2 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "Save" ) );
    t2->setProperty( "callback", &ObjectActions::saveStatic );
    t->insert( t2 );

    t2 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "Rename object" ) );
    t2->setProperty( "callback", &ObjectActions::renameObject );
    t->insert( t2 );
  }
  return _optionTree;
}

Object ClippedObject::makeHeaderOptions() const
{
  Object opts = AObject::makeHeaderOptions();
  makeSliceHeaderOptions( opts );
  return opts;
}


void ClippedObject::setProperties( Object options )
{
  AObject::setProperties( options );
  setSliceProperties( options );
}