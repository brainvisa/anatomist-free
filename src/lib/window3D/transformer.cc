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


#include <anatomist/window3D/transformer.h>
#include <anatomist/window3D/window3D.h>
#include <anatomist/window/glwidget.h>
#include <anatomist/selection/selectFactory.h>
#include <anatomist/reference/Transformation.h>
#include <anatomist/reference/transfSet.h>
#include <anatomist/commands/cAssignReferential.h>
#include <anatomist/commands/cLoadTransformation.h>
#include <anatomist/processor/Processor.h>
#include <anatomist/window3D/boxviewslice.h>
#include <anatomist/surface/surface.h>
#include <anatomist/reference/Referential.h>
#include <anatomist/reference/wChooseReferential.h>
#include <anatomist/ui/ui_transform_control.h>
#include <aims/mesh/surfacegen.h>
#include <aims/mesh/surfaceOperation.h>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <iostream>

using namespace anatomist;
using namespace anatomist::internal;
using namespace aims;
using namespace carto;
using namespace std;


TransformerActionData::TransformerActionData()
  : QObject(), _maintrans( 0 ), rotationAxis( 0, 0, 1 ),
    _rotationAngleEdited( false ), _rotationScaleEdited( false )
{

}


TransformerActionData::TransformerActionData(
  const TransformerActionData & other )
  : QObject(), _maintrans( other._maintrans ), _trans( other._trans ),
    _itrans( other._itrans ),
    _rotationAngleEdited( false ), _rotationScaleEdited( false ),
    _pendingMotion( other._pendingMotion ),
    _centerOnObjects( false )
{
}


TransformerActionData::~TransformerActionData()
{
}


anatomist::Transformation* TransformerActionData::mainTransformation() const
{
  return _maintrans;
}


void TransformerActionData::selectTransformations( AWindow * win )
{
  Action* action = dynamic_cast<AWindow3D *>( win )->view()
    ->controlSwitch()->getAction( "Transformer" );
  if( action )
  {
    Transformer *ta = dynamic_cast<Transformer *>( action );
    if( ta && ta != this )
    {
      ta->selectTransformations( win );
      _maintrans = ta->_maintrans;
      _trans = ta->_trans;
      _itrans = ta->_itrans;
      _centerOnObjects = ta->_centerOnObjects;
      return;
    }
  }

  const std::map<unsigned, set<AObject *> >
    & sel = SelectFactory::factory()->selected();
  map<unsigned, set<AObject *> >::const_iterator is = sel.find( win->Group() );
  if( is == sel.end() )
    return;

  const set<AObject *>                  & obj = is->second;
  set<AObject *>                        nobj, iobj, cobj;
  set<AObject *>::const_iterator        io, eo = obj.end();
  Referential   *ref = 0, *cref = theAnatomist->centralReferential();
  Transformation                        *t;

  if( _maintrans )
  {
    ref = _maintrans->source();
    cref = _maintrans->destination();
  }

  _trans.clear();
  _itrans.clear();

  for( io=obj.begin(); io!=eo; ++io )
  {
    ref = (*io)->getReferential();
    t = theAnatomist->getTransformation( ref, cref );

    if( t && !t->isGenerated() )
    {
      cobj.insert( *io );
      _trans[ t ] = *t;
    }
    else
    {
      t = theAnatomist->getTransformation( cref, ref );
      if( t && !t->isGenerated() )
      {
        iobj.insert( *io );
        _itrans[ t ] = *t;
      }
      else
        nobj.insert( *io );
    }
  }

  if( !nobj.empty() )
  {
    set<AWindow *> wins;
    AssignReferentialCommand    *com = new AssignReferentialCommand( 0, nobj,
                                                                     wins );
    theProcessor->execute( com );
    ref = com->ref();
    cobj.insert( nobj.begin(), nobj.end() );
    float               matvec[4][3];
    matvec[0][0] = 0;
    matvec[0][1] = 0;
    matvec[0][2] = 0;
    matvec[1][0] = 1;
    matvec[1][1] = 0;
    matvec[1][2] = 0;
    matvec[2][0] = 0;
    matvec[2][1] = 1;
    matvec[2][2] = 0;
    matvec[3][0] = 0;
    matvec[3][1] = 0;
    matvec[3][2] = 1;
    LoadTransformationCommand   *tc
      = new LoadTransformationCommand( matvec, ref, cref );
    theProcessor->execute( tc );
    Transformation *tr = tc->trans();
    _trans[ tc->trans() ] = *tr;
    if( !_maintrans )
      _maintrans = tr;
  }

  if( _maintrans )
  {
    if( _trans.find( _maintrans ) == _trans.end()
      && _itrans.find( _maintrans ) == _itrans.end() )
    {
      // if the inverse transform is found, it is OK.
      Transformation *itr
        = theAnatomist->getTransformation( _maintrans->destination(),
                                           _maintrans->source() );
      if( _trans.find( itr ) == _trans.end()
      && _itrans.find( itr ) == _itrans.end() )
        _maintrans = 0;
    }
  }
  if( !_maintrans )
  {
    if( !_trans.empty() )
      _maintrans = _trans.begin()->first;
    else if( !_itrans.empty() )
      _maintrans = _itrans.begin()->first;
  }
}


void TransformerActionData::setMainTransformation( Transformation* t )
{
  _maintrans = t;
}


bool TransformerActionData::isMainTransDirect() const
{
  if( !_maintrans )
    return true;
  if( _trans.find( _maintrans ) == _trans.end() )
    return false;
  return true;
}


Referential* TransformerActionData::mainSourceRef() const
{
  if( !_maintrans )
    return 0;
  if( isMainTransDirect() )
    return _maintrans->source();
  else
    return _maintrans->destination();
}


Referential* TransformerActionData::mainDestRef() const
{
  if( !_maintrans )
    return 0;
  if( isMainTransDirect() )
    return _maintrans->destination();
  else
    return _maintrans->source();
}


void TransformerActionData::setTransformData(const Transformation & t,
                                             bool absolute,
                                             bool addToHistory)
{
  ATransformSet *tset = ATransformSet::instance();
  map<Transformation*, Transformation>::iterator        it, et = _trans.end();
  for( it=_trans.begin(); it!=et; ++it )
  {
//     it->first->unregisterTrans();
//     *it->first = t;
    it->first->motion() = t.motion();
    if( !absolute )
      *it->first *= it->second;

//     it->first->registerTrans();
    tset->updateTransformation( it->first );

    if (addToHistory)
    {
    	Motion mot = t.motion();
    	transmitValidatedMotion(mot);
    }
  }

  if( !_itrans.empty() )
  {
    Transformation t2( 0, 0, t );
    t2.invert();
    for( it=_itrans.begin(), et=_itrans.end(); it!=et; ++it )
    {
//       it->first->unregisterTrans();
//       *it->first = t2;
      it->first->motion() = t2.motion();
      if( !absolute )
        *it->first *= it->second;
//       it->first->registerTrans();
      tset->updateTransformation( it->first );
    }
  }

  emitTransformationChanged();
}


void TransformerActionData::emitTransformationChanged()
{
	emit transformationChanged();
}

void TransformerActionData::undo()
{
    selectTransformations(tadView()->aWindow());
    if (!mainTransformation())
    {
        return;
    }

    mainTransformation()->undo();
    emitTransformationChanged();

    AWindow3D * w = dynamic_cast<AWindow3D *>(tadView()->aWindow());
    if (w)
    {
        w->refreshNow();
    }
}

void TransformerActionData::redo()
{
    selectTransformations(tadView()->aWindow());
    if (!mainTransformation())
    {
        return;
    }

    mainTransformation()->redo();
    emitTransformationChanged();

    AWindow3D * w = dynamic_cast<AWindow3D *>(tadView()->aWindow());
    if (w)
    {
        w->refreshNow();
    }
}

bool TransformerActionData::undoable()
{
    selectTransformations(tadView()->aWindow());
    if (!_maintrans)
    {
        return false;
    }

    return (_maintrans->motionHistorySize() > 0 &&
            _maintrans->motionHistoryIndex() >= 0);
}

bool TransformerActionData::redoable()
{
    selectTransformations(tadView()->aWindow());
    if (!_maintrans)
    {
        return false;
    }

    return (_maintrans->motionHistorySize() > 0 &&
            static_cast<size_t>(_maintrans->motionHistoryIndex()) < _maintrans->motionHistorySize() - 1);
}

void TransformerActionData::resetTransform()
{
  selectTransformations( tadView()->aWindow() );
  if( !mainTransformation() )
    return;

  transmitValidatedMotion(mainTransformation()->motion().inverse());
  Transformation t( 0, 0 );
  setTransformData( t, true );
  Quaternion q( initialQuaternion() );
  updateTemporaryObjects( q );
  updateGVInfo( q );
  AWindow3D    *w3 = dynamic_cast<AWindow3D *>( tadView()->aWindow() );
  if( w3 )
    w3->refreshNow();
}


void TransformerActionData::resetRotation()
{
  selectTransformations( tadView()->aWindow() );
  if( !mainTransformation() )
    return;
  Transformation t( 0, 0 );
  t.setTranslation( &mainTransformation()->translation()[0] );
  setTransformData( t, true );
  Quaternion q( initialQuaternion() );
  updateTemporaryObjects( q );
  updateGVInfo( q );
  AWindow3D    *w3 = dynamic_cast<AWindow3D *>( tadView()->aWindow() );
  if( w3 )
    w3->refreshNow();
}


void TransformerActionData::matrixCellChanged( int row, int col,
                                               QTableWidget* twid )
{
  selectTransformations( tadView()->aWindow() );
  Transformation *t = mainTransformation();
  if( !t )
    return;
  twid->blockSignals( true );
  QTableWidgetItem *item = twid->item( row, col );
  QString text = item->text();
  bool ok = false;
  double value = text.toDouble( &ok );
  AffineTransformation3d & atr = t->motion();
  if( !ok )
  {
    if( col < 3 )
      item->setText( QString::number( atr.rotation()( row, col ), 'f', 2 ) );
    else
      item->setText( QString::number( atr.translation()[ row ], 'f', 2 ) );
  }
  else
  {
    if( col < 3 )
      atr.rotation()( row, col ) = value;
    else
      atr.translation()[ row ] = value;

    setTransformData( *t, true );
    Quaternion q( initialQuaternion() );
    updateTemporaryObjects( q );
    updateGVInfo( q );
    AWindow3D    *w3 = dynamic_cast<AWindow3D *>( tadView()->aWindow() );
    if( w3 )
      w3->refreshNow();
  }
  twid->blockSignals( false );
}


void TransformerActionData::axisCellChanged( int row, int col,
                                             QTableWidget* twid )
{
  selectTransformations( tadView()->aWindow() );
  Transformation *t = mainTransformation();
  if( !t )
    return;
  QTableWidgetItem *item = twid->item( row, col );
  QString text = item->text();
  bool ok = false;
  double value = text.toDouble( &ok );
  AffineTransformation3d & atr = t->motion();
  if( !ok )
  {
    twid->blockSignals( true );
    item->setText( QString::number( rotationAxis[ col ], 'f', 2 ) );
    twid->blockSignals( false );
  }
  else
  {
    rotationAxis[ col ] = value;
  }
}


void TransformerActionData::centerCellChanged( int row, int col,
                                               QTableWidget* twid )
{
  AWindow3D    *w3 = dynamic_cast<AWindow3D *>( tadView()->aWindow() );
  if( !w3 )
    return;
  QTableWidgetItem *item = twid->item( row, col );
  QString text = item->text();
  bool ok = false;
  double value = text.toDouble( &ok );
  GLWidgetManager *w = static_cast<GLWidgetManager *>( tadView() );
  Point3df cent = w->rotationCenter();
  if( !ok )
  {
    item->setText( QString::number( cent[ col ], 'f', 2 ) );
    return;
  }
  cent[ col ] = value;
  w->setRotationCenter( cent );
}


void TransformerActionData::rotationAngleChanged( QLineEdit* ledit,
                                                  QComboBox* unitbox )
{
  if( !_rotationAngleEdited )
    return;
  _rotationAngleEdited = false;
  selectTransformations( tadView()->aWindow() );
  Transformation *t = mainTransformation();
  if( !t )
    return;
  ledit->blockSignals( true );
  QString text = ledit->text();
  bool ok = false;
  double value = text.toDouble( &ok );
  AffineTransformation3d & atr = t->motion();
  if( !ok )
    ledit->setText( QString::number( 0., 'f', 2 ) );
  else
  {
    if( unitbox->currentIndex() == 1 ) // degrees
      value *= M_PI / 180.;
    Quaternion q;
    q.fromAxis( rotationAxis, value );
    Transformation        t2( 0, 0 );
    t2.setQuaternion( q.inverse() );
    AWindow3D    *w3 = dynamic_cast<AWindow3D *>( tadView()->aWindow() );
    if( !w3 )
      return;
    Point3df t0
      = static_cast<GLWidgetManager *>( tadView() )->rotationCenter();
    t0 -= t2.transform( t0 ); // (I-R) t0
    t2.SetTranslation( 0, t0[0] );
    t2.SetTranslation( 1, t0[1] );
    t2.SetTranslation( 2, t0[2] );

    setTransformData( t2 );
    updateTemporaryObjects( q );
    updateGVInfo( q );
    w3->refreshNow();
  }
  ledit->blockSignals( false );
}


void TransformerActionData::rotationScaleChanged( QLineEdit* ledit )
{
  if( !_rotationScaleEdited )
    return;
  _rotationScaleEdited = false;
  selectTransformations( tadView()->aWindow() );
  Transformation *t = mainTransformation();
  if( !t )
    return;
  ledit->blockSignals( true );
  QString text = ledit->text();
  bool ok = false;
  double value = text.toDouble( &ok );
  if( !ok )
    ledit->setText( QString::number( 1., 'f', 2 ) );
  else
  {
    Quaternion q;
    Transformation        t2( 0, 0 );
    AWindow3D    *w3 = dynamic_cast<AWindow3D *>( tadView()->aWindow() );
    if( !w3 )
      return;
    Point3df t0
      = static_cast<GLWidgetManager *>( tadView() )->rotationCenter();
    t2.SetTranslation( 0, t0[0] * ( 1 - value ) );
    t2.SetTranslation( 1, t0[1] * ( 1 - value ) );
    t2.SetTranslation( 2, t0[2] * ( 1 - value ) );
    AffineTransformation3d & atr = t2.motion();
    atr.rotation()( 0, 0 ) = value;
    atr.rotation()( 1, 1 ) = value;
    atr.rotation()( 2, 2 ) = value;

    setTransformData( t2 );
    updateTemporaryObjects( q );
    updateGVInfo( q );
    w3->refreshNow();
  }
  ledit->blockSignals( false );
}


void TransformerActionData::rotationAngleEdited( const QString & )
{
  _rotationAngleEdited = true;
}


void TransformerActionData::rotationScaleEdited( const QString & )
{
  _rotationScaleEdited = true;
}


void TransformerActionData::clearEditionFlags()
{
    _rotationAngleEdited = false;
    _rotationScaleEdited = false;
}


void TransformerActionData::fromRefButtonClicked()
{
  set<AObject *> obj;
  ChooseReferentialWindow chref( obj, "Source referential" );
  chref.exec();
  Referential *ref = chref.selectedReferential();
  if( ref )
  {
    Referential *dref = theAnatomist->centralReferential();
    if( _maintrans )
      dref = _maintrans->destination();
    Transformation *tr = theAnatomist->getTransformation( ref, dref );
    if( tr )
    {
      setMainTransformation( tr );
      updateGVInfo( initialQuaternion() );
    }
  }
}


void TransformerActionData::toRefButtonClicked()
{
  set<AObject *> obj;
  ChooseReferentialWindow chref( obj, "Destination referential" );
  chref.exec();
  Referential *ref = chref.selectedReferential();
  if( ref )
  {
    Referential *sref = theAnatomist->centralReferential();
    if( _maintrans )
      sref = _maintrans->source();
    Transformation *tr = theAnatomist->getTransformation( sref, ref );
    if( tr )
    {
      setMainTransformation( tr );
      updateGVInfo( initialQuaternion() );
    }
  }
}


void TransformerActionData::invertTransformationClicked()
{
  selectTransformations(tadView()->aWindow());

  Transformation* t = mainTransformation();
  if( t )
  {
    Transformation *itr
      = theAnatomist->getTransformation( t->destination(),
                                         t->source() );
    if( itr )
    {
      setMainTransformation( itr );
      updateGVInfo( initialQuaternion() );
    }
  }
}


void TransformerActionData::centerOnObjectsToggled( int state )
{
  _centerOnObjects = state;
}


bool TransformerActionData::getCurrentMotion(Motion& motion)
{
    selectTransformations(tadView()->aWindow());
    Transformation* t = mainTransformation();
    if (!t)
    {
        return false;
    }

    motion = t->motion();

    return true;
}

void TransformerActionData::updatePendingMotion(const Motion & motion)
{
    _pendingMotion = motion;
}

void TransformerActionData::transmitValidatedMotion(Motion motion, bool notify)
{
    if (!_maintrans)
    {
        return;
    }

    _maintrans->addMotionToHistory(motion);

    if (notify)
    {
    	emitTransformationChanged();
    }
}

// ---

struct Transformer::Private
{
  Private() : trans_ui( 0 ), show_info( true ) {}

  rc_ptr<BoxViewSlice> box1;
  rc_ptr<BoxViewSlice> box2;
  list<QGraphicsItem *> gvitems;
  Ui::transform_feedback* trans_ui;
  bool show_info;
};


struct TranslaterAction::Private : public Transformer::Private
{
//   rc_ptr<BoxViewSlice> box1;
//   rc_ptr<BoxViewSlice> box2;
//   list<QGraphicsItem *> gvitems;
};


namespace
{

  rc_ptr<AObject> trieder( float colr, float colg, float colb, float cola )
  {
    AimsTimeSurface<2,Void> *cross = new AimsTimeSurface<2,Void>;
    vector<Point3df> & vert = (*cross)[0].vertex();
    vector<AimsVector<uint32_t,2> > & poly = (*cross)[0].polygon();
    vert.reserve(4);
    poly.reserve(3);
    vert.push_back( Point3df( 0, 0, 0 ) );
    vert.push_back( Point3df( 100, 0, 0 ) );
    vert.push_back( Point3df( 0, 100, 0 ) );
    vert.push_back( Point3df( 0, 0, 100 ) );
    poly.push_back( AimsVector<uint32_t,2>( 0, 1 ) );
    poly.push_back( AimsVector<uint32_t,2>( 0, 2 ) );
    poly.push_back( AimsVector<uint32_t,2>( 0, 3 ) );
    ASurface<2>* amesh = new ASurface<2>;
    rc_ptr<AObject> rmesh( amesh );
    amesh->setSurface( cross );
    Material & mat = amesh->GetMaterial();
    mat.SetDiffuse( colr, colg, colb, cola );
    mat.setRenderProperty( Material::Ghost, 1 );
    mat.setRenderProperty( Material::RenderMode, Material::Wireframe );
    mat.setLineWidth( 2. );
    amesh->SetMaterial( mat );
    amesh->setName( theAnatomist->makeObjectName( "trieder" ) );
    theAnatomist->registerObject( amesh, false );
    theAnatomist->releaseObject( amesh );
    return rmesh;
  }

  void initBoxes( BoxViewSlice & box1, BoxViewSlice & box2 )
  {
    box2.setCubeColor( 0., 1., 0.5, 1. );
    box2.setPlaneColor( 0.2, 0.6, 0.2, 1. );
    box1.enablePlane( false );
    box2.enablePlane( false );
    box1.enableText( false );
    box2.enableText( false );
    box1.addObject( trieder( 1., 0.5, 0., 1. ) );
    box2.addObject( trieder( 0.2, 0.6, 0.2, 1. ) );
  }


  void updateAxisWithCircles( AimsTimeSurface<2,Void> &mesh,
                              const Point3df & p0, const Quaternion & rotation,
                              float radius, float circlespacing )
  {
    Point3df axis = rotation.axis();
    float angle = rotation.angle();
    mesh[0].vertex().clear();
    mesh[0].polygon().clear();
    mesh[0].normal().clear();
    Point3df startdir = rotation.transformInverse( Point3df( 1, 0, 0 ) );
    AimsTimeSurface<2,Void> *mesh2
      = SurfaceGenerator::circle_wireframe( p0, radius, 20, axis, startdir,
                                            angle, M_PI*2 );
    SurfaceManip::meshMerge( mesh, *mesh2 );
    delete mesh2;
    mesh2 = SurfaceGenerator::circle_wireframe( p0 + axis * circlespacing,
                                                radius, 20, axis, startdir,
                                                angle, M_PI*2 );
    SurfaceManip::meshMerge( mesh, *mesh2 );
    delete mesh2;
    mesh2 = SurfaceGenerator::circle_wireframe( p0 - axis * circlespacing,
                                                radius, 20, axis, startdir,
                                                angle, M_PI*2 );
    SurfaceManip::meshMerge( mesh, *mesh2 );
    delete mesh2;
    // add axis
    vector<Point3df> & vert = mesh[0].vertex();
    vector<AimsVector<uint32_t,2> > & poly = mesh[0].polygon();
    poly.push_back( AimsVector<uint32_t,2>( vert.size(), vert.size()+1 ) );
    vert.push_back( p0 - axis * circlespacing * 2 );
    vert.push_back( p0 + axis * circlespacing * 2 );
  }


  void updateCirclesAngles( AimsTimeSurface<2,Void> &mesh,
                            const Point3df & p0, const Quaternion & rotation,
                            float radius, float circlespacing )
  {
    Point3df axis = rotation.axis();
    float angle = rotation.angle();
    vector<Point3df> & vert = mesh[0].vertex();
    vector<AimsVector<uint32_t,2> > & poly = mesh[0].polygon();
    vert.clear();
    poly.clear();
    mesh[0].normal().clear();
    Point3df startdir = rotation.transformInverse( Point3df( 1, 0, 0 ) );
    Point3df arrowdir = crossed( axis, startdir );
    Point3df raydir;
    float arrowlen = 15;
    float arrowthick = arrowlen * 0.3;
    size_t nvert = 0;
    bool doarrow = true;
    if( arrowdir.norm2() < 1e-5 )
      doarrow = false;
    else
    {
      raydir = crossed( arrowdir, axis );
      arrowdir = crossed( axis, raydir );
      arrowdir.normalize();
      raydir.normalize();
      if( angle >= 0 )
        arrowdir *= -1;
    }
    AimsTimeSurface<2,Void> *mesh2
      = SurfaceGenerator::circle_wireframe( p0, radius, 20, axis, startdir,
                                            0, angle );
    SurfaceManip::meshMerge( mesh, *mesh2 );
    delete mesh2;
    if( doarrow )
    {
      // add arrows
      Point3df lastp = vert[ nvert ];
      poly.push_back( AimsVector<uint32_t,2>( nvert, vert.size() ) );
      poly.push_back( AimsVector<uint32_t,2>( nvert, vert.size()+1 ) );
      vert.push_back( lastp - arrowdir * arrowlen + raydir * arrowthick );
      vert.push_back( lastp - arrowdir * arrowlen - raydir * arrowthick );
    }
    nvert = vert.size();
    mesh2 = SurfaceGenerator::circle_wireframe( p0 + axis * circlespacing,
                                                radius, 20, axis, startdir,
                                                0, angle );
    SurfaceManip::meshMerge( mesh, *mesh2 );
    delete mesh2;
    if( doarrow )
    {
      // add arrows
      Point3df lastp = vert[ nvert ];
      poly.push_back( AimsVector<uint32_t,2>( nvert, vert.size() ) );
      poly.push_back( AimsVector<uint32_t,2>( nvert, vert.size()+1 ) );
      vert.push_back( lastp - arrowdir * arrowlen + raydir * arrowthick );
      vert.push_back( lastp - arrowdir * arrowlen - raydir * arrowthick );
    }
    nvert = vert.size();
    mesh2 = SurfaceGenerator::circle_wireframe( p0 - axis * circlespacing,
                                                radius, 20, axis, startdir,
                                                0, angle );
    SurfaceManip::meshMerge( mesh, *mesh2 );
    delete mesh2;
    if( doarrow )
    {
      // add arrows
      Point3df lastp = vert[ nvert ];
      poly.push_back( AimsVector<uint32_t,2>( nvert, vert.size() ) );
      poly.push_back( AimsVector<uint32_t,2>( nvert, vert.size()+1 ) );
      vert.push_back( lastp - arrowdir * arrowlen + raydir * arrowthick );
      vert.push_back( lastp - arrowdir * arrowlen - raydir * arrowthick );
    }
  }


  rc_ptr<AObject> ameshFromMesh( AimsTimeSurface<2,Void>* mesh,
                                 const string & name, float colr, float colg,
                                 float colb, float cola, float linewidth )
  {
    ASurface<2>* amesh = new ASurface<2>;
    rc_ptr<AObject> rmesh( amesh );
    amesh->setSurface( mesh );
    Material & mat = amesh->GetMaterial();
    mat.SetDiffuse( colr, colg, colb, cola );
    mat.setRenderProperty( Material::Ghost, 1 );
    mat.setRenderProperty( Material::RenderMode, Material::Wireframe );
    mat.setLineWidth( linewidth );
    amesh->SetMaterial( mat );
    amesh->setName( theAnatomist->makeObjectName( name ) );
    theAnatomist->registerObject( amesh, false );
    theAnatomist->releaseObject( amesh );
    return rmesh;
  }


  rc_ptr<AObject> axisWithCircles( const Point3df & p0,
                                   const Quaternion & rotation,
                                   float radius, float circlespacing,
                                   float colr, float colg,
                                   float colb, float cola )
  {
    Point3df axis = rotation.axis();
    float angle = rotation.angle();
    Point3df startdir = rotation.transformInverse( Point3df( 1, 0, 0 ) );
    AimsTimeSurface<2,Void> *mesh = new AimsTimeSurface<2,Void>;
    updateAxisWithCircles( *mesh, p0, rotation, radius, circlespacing );

    rc_ptr<AObject> rmesh = ameshFromMesh( mesh, "trieder", colr, colg, colb,
                                           cola, 2. );
    return rmesh;
  }


  rc_ptr<AObject> circlesWithAngle( const Point3df & p0,
                                    const Quaternion & rotation,
                                    float radius, float circlespacing,
                                    float colr, float colg,
                                    float colb, float cola )
  {
    Point3df axis = rotation.axis();
    float angle = rotation.angle();
    Point3df startdir = rotation.transformInverse( Point3df( 1, 0, 0 ) );
    AimsTimeSurface<2,Void> *mesh = new AimsTimeSurface<2,Void>;
    updateCirclesAngles( *mesh, p0, rotation, radius, circlespacing );

    rc_ptr<AObject> rmesh = ameshFromMesh( mesh, "rotationangle", colr, colg,
                                           colb, cola, 3. );
    return rmesh;
  }


  rc_ptr<AObject> zoomMarkers( const Point3df & p0, float radius,
                               float colr, float colg, float colb, float cola )
  {
    AimsTimeSurface<2, Void> *mesh = new AimsTimeSurface<2, Void>;
    vector<Point3df> & vert = (*mesh)[0].vertex();
    vector<AimsVector<uint32_t,2> > & poly = (*mesh)[0].polygon();

    vert.reserve( 16 );
    poly.reserve( 8 );
    vert.push_back( Point3df( p0 + Point3df( 1, 1, 1 ) * radius * 0.3535 ) );
    vert.push_back( Point3df( p0 + Point3df( 1, 1, 1 ) * radius * 0.7071 ) );
    vert.push_back( Point3df( p0 + Point3df( -1, -1, -1 ) * radius * 0.3535
      ) );
    vert.push_back( Point3df( p0 + Point3df( -1, -1, -1 ) * radius * 0.7071
      ) );
    vert.push_back( Point3df( p0 + Point3df( -1, 1, 1 ) * radius * 0.3535 ) );
    vert.push_back( Point3df( p0 + Point3df( -1, 1, 1 ) * radius * 0.7071 ) );
    vert.push_back( Point3df( p0 + Point3df( 1, -1, -1 ) * radius * 0.3535 ) );
    vert.push_back( Point3df( p0 + Point3df( 1, -1, -1 ) * radius * 0.7071 ) );
    vert.push_back( Point3df( p0 + Point3df( 1, -1, 1 ) * radius * 0.3535 ) );
    vert.push_back( Point3df( p0 + Point3df( 1, -1, 1 ) * radius * 0.7071 ) );
    vert.push_back( Point3df( p0 + Point3df( -1, 1, -1 ) * radius * 0.3535 ) );
    vert.push_back( Point3df( p0 + Point3df( -1, 1, -1 ) * radius * 0.7071 ) );
    vert.push_back( Point3df( p0 + Point3df( 1, 1, -1 ) * radius * 0.3535 ) );
    vert.push_back( Point3df( p0 + Point3df( 1, 1, -1 ) * radius * 0.7071 ) );
    vert.push_back( Point3df( p0 + Point3df( -1, -1, 1 ) * radius * 0.3535 ) );
    vert.push_back( Point3df( p0 + Point3df( -1, -1, 1 ) * radius * 0.7071 ) );
    poly.push_back( AimsVector<uint32_t,2>( 0, 1 ) );
    poly.push_back( AimsVector<uint32_t,2>( 2, 3 ) );
    poly.push_back( AimsVector<uint32_t,2>( 4, 5 ) );
    poly.push_back( AimsVector<uint32_t,2>( 6, 7 ) );
    poly.push_back( AimsVector<uint32_t,2>( 8, 9 ) );
    poly.push_back( AimsVector<uint32_t,2>( 10, 11 ) );
    poly.push_back( AimsVector<uint32_t,2>( 12, 13 ) );
    poly.push_back( AimsVector<uint32_t,2>( 14, 15 ) );

    rc_ptr<AObject> rmesh = ameshFromMesh( mesh, "zoomMarkers", colr, colg,
                                           colb, cola, 2. );
    return rmesh;
  }


  void updateZoomStateMarkers( AimsTimeSurface<2,Void> &mesh,
                               const Point3df & p0, float radius, float zoom )
  {
    vector<Point3df> & vert = mesh[0].vertex();
    vector<AimsVector<uint32_t,2> > & poly = mesh[0].polygon();
    vert.clear();
    poly.clear();
    float startfac = 1., endfac;

//     if( zoom >= 1 )
//       startfac = radius * 0.3535;
//     else
      startfac = radius * 0.7071;
    endfac = startfac * zoom;

    vert.reserve( 32 );
    poly.reserve( 24 );
    vert.push_back( p0 + Point3df( 1, 1, 1 ) * startfac );
    vert.push_back( p0 + Point3df( 1, 1, 1 ) * endfac );
    vert.push_back( p0 + Point3df( -1, -1, -1 ) * startfac );
    vert.push_back( p0 + Point3df( -1, -1, -1 ) * endfac );
    vert.push_back( p0 + Point3df( -1, 1, 1 ) * startfac );
    vert.push_back( p0 + Point3df( -1, 1, 1 ) * endfac );
    vert.push_back( p0 + Point3df( 1, -1, -1 ) * startfac );
    vert.push_back( p0 + Point3df( 1, -1, -1 ) * endfac );
    vert.push_back( p0 + Point3df( 1, -1, 1 ) * startfac );
    vert.push_back( p0 + Point3df( 1, -1, 1 ) * endfac );
    vert.push_back( p0 + Point3df( -1, 1, -1 ) * startfac );
    vert.push_back( p0 + Point3df( -1, 1, -1 ) * endfac );
    vert.push_back( p0 + Point3df( 1, 1, -1 ) * startfac );
    vert.push_back( p0 + Point3df( 1, 1, -1 ) * endfac );
    vert.push_back( p0 + Point3df( -1, -1, 1 ) * startfac );
    vert.push_back( p0 + Point3df( -1, -1, 1 ) * endfac );
    poly.push_back( AimsVector<uint32_t,2>( 0, 1 ) );
    poly.push_back( AimsVector<uint32_t,2>( 2, 3 ) );
    poly.push_back( AimsVector<uint32_t,2>( 4, 5 ) );
    poly.push_back( AimsVector<uint32_t,2>( 6, 7 ) );
    poly.push_back( AimsVector<uint32_t,2>( 8, 9 ) );
    poly.push_back( AimsVector<uint32_t,2>( 10, 11 ) );
    poly.push_back( AimsVector<uint32_t,2>( 12, 13 ) );
    poly.push_back( AimsVector<uint32_t,2>( 14, 15 ) );
    // arrows
    float arrowlen = 15 * 0.7071;
    float arrowthick = arrowlen * 0.3;
    float sign = zoom >= 1 ? 1. : -1;
    vert.push_back( p0 + Point3df( 1, 1, 1 ) * ( endfac - arrowlen * sign )
      + Point3df( -1, -1, 1 ) * arrowthick );
    vert.push_back( p0 + Point3df( 1, 1, 1 ) * ( endfac - arrowlen * sign )
      - Point3df( -1, -1, 1 ) * arrowthick );
    vert.push_back( p0 + Point3df( -1, -1, -1 ) * ( endfac - arrowlen * sign )
      + Point3df( 1, 1, -1 ) * arrowthick );
    vert.push_back( p0 + Point3df( -1, -1, -1 ) * ( endfac - arrowlen * sign )
      - Point3df( 1, 1, -1 ) * arrowthick );
    vert.push_back( p0 + Point3df( -1, 1, 1 ) * ( endfac - arrowlen * sign )
      + Point3df( 1, -1, 1 ) * arrowthick );
    vert.push_back( p0 + Point3df( -1, 1, 1 ) * ( endfac - arrowlen * sign )
      - Point3df( 1, -1, 1 ) * arrowthick );
    vert.push_back( p0 + Point3df( 1, -1, -1 ) * ( endfac - arrowlen * sign )
      + Point3df( -1, 1, -1 ) * arrowthick );
    vert.push_back( p0 + Point3df( 1, -1, -1 ) * ( endfac - arrowlen * sign )
      - Point3df( -1, 1, -1 ) * arrowthick );
    vert.push_back( p0 + Point3df( 1, -1, 1 ) * ( endfac - arrowlen * sign )
      + Point3df( -1, 1, 1 ) * arrowthick );
    vert.push_back( p0 + Point3df( 1, -1, 1 ) * ( endfac - arrowlen * sign )
      - Point3df( -1, 1, 1 ) * arrowthick );
    vert.push_back( p0 + Point3df( -1, 1, -1 ) * ( endfac - arrowlen * sign )
      + Point3df( 1, -1, -1 ) * arrowthick );
    vert.push_back( p0 + Point3df( -1, 1, -1 ) * ( endfac - arrowlen * sign )
      - Point3df( 1, -1, -1 ) * arrowthick );
    vert.push_back( p0 + Point3df( 1, 1, -1 ) * ( endfac - arrowlen * sign )
      + Point3df( -1, -1, -1 ) * arrowthick );
    vert.push_back( p0 + Point3df( 1, 1, -1 ) * ( endfac - arrowlen * sign )
      - Point3df( -1, -1, -1 ) * arrowthick );
    vert.push_back( p0 + Point3df( -1, -1, 1 ) * ( endfac - arrowlen * sign )
      + Point3df( 1, 1, 1 ) * arrowthick );
    vert.push_back( p0 + Point3df( -1, -1, 1 ) * ( endfac - arrowlen * sign )
      - Point3df( 1, 1, 1 ) * arrowthick );
    poly.push_back( AimsVector<uint32_t,2>( 1, 16 ) );
    poly.push_back( AimsVector<uint32_t,2>( 1, 17 ) );
    poly.push_back( AimsVector<uint32_t,2>( 3, 18 ) );
    poly.push_back( AimsVector<uint32_t,2>( 3, 19 ) );
    poly.push_back( AimsVector<uint32_t,2>( 5, 20 ) );
    poly.push_back( AimsVector<uint32_t,2>( 5, 21 ) );
    poly.push_back( AimsVector<uint32_t,2>( 7, 22 ) );
    poly.push_back( AimsVector<uint32_t,2>( 7, 23 ) );
    poly.push_back( AimsVector<uint32_t,2>( 9, 24 ) );
    poly.push_back( AimsVector<uint32_t,2>( 9, 25 ) );
    poly.push_back( AimsVector<uint32_t,2>( 11, 26 ) );
    poly.push_back( AimsVector<uint32_t,2>( 11, 27 ) );
    poly.push_back( AimsVector<uint32_t,2>( 13, 28 ) );
    poly.push_back( AimsVector<uint32_t,2>( 13, 29 ) );
    poly.push_back( AimsVector<uint32_t,2>( 15, 30 ) );
    poly.push_back( AimsVector<uint32_t,2>( 15, 31 ) );
  }


  rc_ptr<AObject> zoomStateMarkers( const Point3df & p0, float radius,
                                    float zoom, float colr, float colg,
                                    float colb, float cola )
  {
    AimsTimeSurface<2, Void> *mesh = new AimsTimeSurface<2, Void>;
    updateZoomStateMarkers( *mesh, p0, radius, zoom );

    rc_ptr<AObject> rmesh = ameshFromMesh( mesh, "zoomStateMarkers", colr,
                                           colg, colb, cola, 3. );
    return rmesh;
  }


  void initGVItems( QGraphicsView* gv, Action* action,
                    Transformer::Private *d )
  {
    if( !gv )
      return;
    Action *trac = action->view()->controlSwitch()->getAction( "Transformer" );
    if( ( trac && trac != action ) || !d->gvitems.empty() )
      return;
    Ui::transform_feedback* tui = new Ui::transform_feedback;
    QWidget *wid = new QWidget( 0 );
    QObject *ob = dynamic_cast<QObject *>( action );
    tui->setupUi( wid );
    tui->matrix_tab->setLayout( tui->matrix_layout );
    tui->euler_tab->setLayout( tui->euler_layout );
    tui->transform_tabWidget->removeTab( 1 ); // disable euler for now.
    tui->rotation_tab->setLayout( tui->rotation_layout );
    if( ob )
    {
      ob->connect( tui->reset_pushButton, SIGNAL( pressed() ),
                   ob, SLOT( resetTransform() ) );
      ob->connect( tui->resetR_pushButton, SIGNAL( pressed() ),
                   ob, SLOT( resetRotation() ) );
      ob->connect( tui->matrix_tableWidget, SIGNAL( cellChanged( int, int ) ),
                   ob, SLOT( matrixCellChanged( int, int ) ) );
      ob->connect( tui->rotation_axis_tableWidget,
                   SIGNAL( cellChanged( int, int ) ),
                   ob, SLOT( axisCellChanged( int, int ) ) );
      ob->connect( tui->rotation_center_tableWidget,
                   SIGNAL( cellChanged( int, int ) ),
                   ob, SLOT( centerCellChanged( int, int ) ) );
      ob->connect( tui->rotation_angle_lineEdit,
                   SIGNAL( textEdited( const QString &) ),
                   ob, SLOT( rotationAngleEdited( const QString & ) ) );
      ob->connect( tui->rotation_angle_lineEdit, SIGNAL( editingFinished() ),
                   ob, SLOT( rotationAngleChanged() ) );
      ob->connect( tui->rotation_scaling_lineEdit,
                   SIGNAL( textEdited( const QString & ) ),
                   ob, SLOT( rotationScaleEdited( const QString & ) ) );
      ob->connect( tui->rotation_scaling_lineEdit, SIGNAL( editingFinished() ),
                   ob, SLOT( rotationScaleChanged() ) );
      ob->connect( tui->from_ref_button, SIGNAL( clicked() ),
                   ob, SLOT( fromRefButtonClicked() ) );
      ob->connect( tui->to_ref_button, SIGNAL( clicked() ),
                   ob, SLOT( toRefButtonClicked() ) );
      ob->connect( tui->inv_trans_button, SIGNAL( clicked() ),
                   ob, SLOT( invertTransformationClicked() ) );
      ob->connect( tui->center_on_objects, SIGNAL( stateChanged( int ) ),
                   ob, SLOT( centerOnObjectsToggled( int ) ) );
    }

    QGraphicsScene *scene = gv->scene();
    if( !scene )
    {
      scene = new QGraphicsScene( gv );
      gv->setScene( scene );
    }
    QGraphicsProxyWidget *item = scene->addWidget(
      wid, Qt::Window | Qt::FramelessWindowHint );
    if( !d->show_info )
      item->hide();
    QTransform tr = item->transform();
    tr.translate( 10, 10 );
    item->setTransform( tr );
    d->gvitems.push_back( item );
    d->trans_ui = tui;
  }


  void removeGVItems( QGraphicsView* gv, Transformer::Private *d )
  {
    if( !gv )
      return;
    QGraphicsScene *scene = gv->scene();
    if( !scene )
      return;
    list<QGraphicsItem *>::iterator i, e = d->gvitems.end();
    for( i=d->gvitems.begin(); i!=e; ++i )
    {
      scene->removeItem( *i );
      delete *i;
    }
    d->gvitems.clear();
  }


  void showGvItems( Transformer::Private *d, bool x )
  {

    if( x == d->show_info )
      return;

    d->show_info = x;
    list<QGraphicsItem *>::iterator i, e = d->gvitems.end();
    for( i=d->gvitems.begin(); i!=e; ++i )
      if( x )
        (*i)->show();
      else
        (*i)->hide();
  }


  void updateGVInfo( Transformer::Private *d, anatomist::Transformation * tr,
                     Action* action, const Quaternion & q,
                     int centerOnObjects, float scale=1. )
  {
    Action * ac = action->view()->controlSwitch()->getAction( "Transformer" );
    if( !ac )
      return;
    Transformer *trac = dynamic_cast<Transformer *>( ac );
    if( !trac )
      return;
    d = trac->data();
    if( d->gvitems.empty() )
      return;
    QGraphicsProxyWidget* gw
      = dynamic_cast<QGraphicsProxyWidget *>( *d->gvitems.begin() );
    if( !gw )
      return; // not normal...
    QWidget *wid = gw->widget();
    if( !tr )
    {
      // clear all
      return;
    }
    // tr is not null
    trac->clearEditionFlags();
    AimsRGB col = tr->source()->Color();
    QPixmap pix( d->trans_ui->from_ref_button->size() );
    pix.fill( QColor( col[0], col[1], col[2] ) );
    d->trans_ui->from_ref_button->setIcon( pix );
    col = tr->destination()->Color();
    pix.fill( QColor( col[0], col[1], col[2] ) );
    d->trans_ui->to_ref_button->setIcon( pix );
    const AffineTransformation3d & atr = tr->motion();
    const AimsData<float> & rot = atr.rotation();
    const Point3df & tra = atr.translation();
    int i, j;

    d->trans_ui->matrix_tableWidget->blockSignals( true );
    for( i=0; i<3; ++i )
      for( j=0; j<3; ++j )
      d->trans_ui->matrix_tableWidget->setItem( i, j,
        new QTableWidgetItem( QString::number( rot( i, j ), 'f', 2 ) ) );
    d->trans_ui->matrix_tableWidget->setItem( 0, 3,
         new QTableWidgetItem( QString::number( tra[0], 'f', 2 ) ) );
    d->trans_ui->matrix_tableWidget->setItem( 1, 3,
         new QTableWidgetItem( QString::number( tra[1], 'f', 2 ) ) );
    d->trans_ui->matrix_tableWidget->setItem( 2, 3,
         new QTableWidgetItem( QString::number( tra[2], 'f', 2 ) ) );
    d->trans_ui->matrix_tableWidget->blockSignals( false );

    GLWidgetManager* w = dynamic_cast<GLWidgetManager *>( action->view() );
    Point3df cent( 0, 0, 0 );
    if( w )
      cent = w->rotationCenter();

    d->trans_ui->euler_center_tableWidget->blockSignals( true );
    d->trans_ui->euler_center_tableWidget->setItem( 0, 0,
        new QTableWidgetItem( QString::number( cent[0], 'f', 2 ) ) );
    d->trans_ui->euler_center_tableWidget->setItem( 0, 1,
        new QTableWidgetItem( QString::number( cent[1], 'f', 2 ) ) );
    d->trans_ui->euler_center_tableWidget->setItem( 0, 2,
        new QTableWidgetItem( QString::number( cent[2], 'f', 2 ) ) );
    d->trans_ui->euler_center_tableWidget->blockSignals( false );

    d->trans_ui->rotation_center_tableWidget->blockSignals( true );
    d->trans_ui->rotation_center_tableWidget->setItem( 0, 0,
        new QTableWidgetItem( QString::number( cent[0], 'f', 2 ) ) );
    d->trans_ui->rotation_center_tableWidget->setItem( 0, 1,
        new QTableWidgetItem( QString::number( cent[1], 'f', 2 ) ) );
    d->trans_ui->rotation_center_tableWidget->setItem( 0, 2,
        new QTableWidgetItem( QString::number( cent[2], 'f', 2 ) ) );
    d->trans_ui->rotation_center_tableWidget->blockSignals( false );

    d->trans_ui->euler_scaling_lineEdit->blockSignals( true );
    d->trans_ui->euler_scaling_lineEdit->setText(
      QString::number( scale, 'f', 3 ) );
    d->trans_ui->euler_scaling_lineEdit->blockSignals( false );

    Point3df axis = q.axis();
    d->trans_ui->rotation_axis_tableWidget->blockSignals( true );
    d->trans_ui->rotation_axis_tableWidget->setItem( 0, 0,
       new QTableWidgetItem( QString::number( axis[0], 'f', 2 ) ) );
    d->trans_ui->rotation_axis_tableWidget->setItem( 0, 1,
       new QTableWidgetItem( QString::number( axis[1], 'f', 2 ) ) );
    d->trans_ui->rotation_axis_tableWidget->setItem( 0, 2,
       new QTableWidgetItem( QString::number( axis[2], 'f', 2 ) ) );
    d->trans_ui->rotation_axis_tableWidget->blockSignals( false );

    d->trans_ui->rotation_angle_lineEdit->blockSignals( true );
    float angle = q.angle();
    if( d->trans_ui->rotation_angle_unit_comboBox->currentIndex() == 1 )
      angle *= 180. / M_PI;
    d->trans_ui->rotation_angle_lineEdit->setText(
      QString::number( angle, 'f', 2 ) );

    d->trans_ui->center_on_objects->setChecked( centerOnObjects );

    d->trans_ui->rotation_angle_lineEdit->blockSignals( false );
    d->trans_ui->rotation_scaling_lineEdit->blockSignals( true );
    d->trans_ui->rotation_scaling_lineEdit->setText(
      QString::number( scale, 'f', 3 ) );
    d->trans_ui->rotation_scaling_lineEdit->blockSignals( false );

//     GLWidgetManager *glw = dynamic_cast<GLWidgetManager *>( action->view() );
//     if( glw )
//     {
//       Point3df center = glw->rotationCenter();
//     }

  }

}


Transformer::Transformer()
  : TransformerActionData(), Trackball(), d( new Private )
{
  d->box1.reset( new BoxViewSlice( this ) );
  d->box2.reset( new BoxViewSlice( this ) );
  initBoxes( *d->box1, *d->box2 );
}


Transformer::Transformer( const Transformer & a )
  : TransformerActionData( a ), Trackball( a )
{
}


Transformer::~Transformer()
{
  delete d;
}


Action* Transformer::creator()
{
  return( new Transformer );
}


string Transformer::name() const
{
  return( "Transformer" );
}


Quaternion Transformer::initialQuaternion()
{
  return Quaternion();
}


namespace
{

  Point3df getRotationCenter( GLWidgetManager *w, bool centerOnObjects )
  {
    Point3df center = w->rotationCenter();
    if( centerOnObjects )
    {
      AWindow* w3 = w->aWindow();
      const std::map<unsigned, set<AObject *> >
        & sel = SelectFactory::factory()->selected();
      map<unsigned, set<AObject *> >::const_iterator
        is = sel.find( w3->Group() );
      if( is != sel.end() )
      {
        int n = 0;
        center = Point3df( 0, 0, 0 );
        set<AObject *>::const_iterator io, eo = is->second.end();
        for( io=is->second.begin(); io!=eo; ++io, ++n )
        {
          Point3df bbmin, bbmax;
          (*io)->boundingBox( bbmin, bbmax );
          bbmax += bbmin;
          bbmax /= 2;
          if( (*io)->getReferential() )
          {
            anatomist::Transformation *tr
              = theAnatomist->getTransformation( (*io)->getReferential(),
                                                 w3->getReferential() );
            if( tr )
              bbmax = tr->transform( bbmax );
          }
          center += bbmax;
        }
        if( n == 0 )
          center = w->rotationCenter();
        else
          center /= n;
      }
    }

    return center;
  }

}


void Transformer::beginTrackball( int x, int y, int globalX, int globalY )
{
  selectTransformations( view()->aWindow() );
  if( !_maintrans )
    return;

  GLWidgetManager * w = dynamic_cast<GLWidgetManager *>( view() );
  Point3df center = getRotationCenter( w, _centerOnObjects );
  Point3df old_center = w->rotationCenter();
  if( old_center != center )
    w->setRotationCenter( center );

  Trackball::beginTrackball( x, y, globalX, globalY );

  Referential *ref, *cref;
  if( isMainTransDirect() )
  {
    ref = _maintrans->source();
    cref = _maintrans->destination();
  }
  else
  {
    ref = _maintrans->destination();
    cref = _maintrans->source();
  }

  d->box1->setObjectsReferential( ref );
  d->box2->setObjectsReferential( cref );

  AWindow* w3 = w->aWindow();

  if( w && d->box2->additionalObjects().size() <= 1 )
  {
    rc_ptr<AObject> axis = axisWithCircles( w->rotationCenter(),
                                            initialQuaternion(),
                                            70, 100, 0.8, 0.3, 0.2, 1. );
    axis->setReferential( w3->getReferential() );
    d->box2->addObject( axis );
    rc_ptr<AObject> circles = circlesWithAngle( w->rotationCenter(),
                                                initialQuaternion(), 70, 100,
                                                0.3, 0., 0.8, 1. );
    circles->setReferential( w3->getReferential() );
    d->box2->addObject( circles );
  }
  else
    updateTemporaryObjects( initialQuaternion() );

  initGVItems( d->box1->graphicsView(), this, d );
  ::updateGVInfo( d, _maintrans, this, initialQuaternion(), _centerOnObjects );

  d->box1->beginTrackball( x, y );
  d->box2->beginTrackball( x, y );

  if( old_center != center )
    w->setRotationCenter( old_center );
}


Quaternion Transformer::rotation( int x, int y )
{
  if( _beginx < 0 || _beginy < 0 )
    return( Quaternion( 0, 0, 0, 1 ) );

  GLWidgetManager * w = dynamic_cast<GLWidgetManager *>( view() );

  if( !w )
    {
      cerr << "Transformer operating on wrong view type -- error\n";
      return( Quaternion( 0, 0, 0, 1 ) );
    }

  float	dimx = w->width();
  float	dimy = w->height();

  Quaternion	q 
    = w->quaternion().inverse() * 
    initQuaternion( ( 2. * _beginx - dimx ) / dimx,
		    ( dimy - 2. * _beginy ) / dimy,
		    ( 2. * x - dimx ) / dimx,
		    ( dimy - 2. * y ) / dimy ).inverse() 
    * w->quaternion();

  Point4df	vec = q.vector();
  // we must invert Z since we're transforming to an indirect referential
  float zfac = w->invertedZ() ? 1 : -1;
  q = Quaternion( zfac * vec[0], zfac * vec[1], -vec[2], vec[3] );

  //cout << "rotation axis : " << q.axis() << ", angle : " 
  //     << q.angle() * 180. / M_PI << endl;
  return q;
}


void Transformer::updateTemporaryObjects( const Quaternion & rotation )
{
  GLWidgetManager * w = dynamic_cast<GLWidgetManager *>( view() );
  if( !w )
    return;
  list<rc_ptr<AObject> > & addobj = d->box2->additionalObjects();
  list<rc_ptr<AObject> >::reverse_iterator io = addobj.rbegin();
  if( io != addobj.rend() )
  {
    ASurface<2> *circles = dynamic_cast<ASurface<2> *>( io->get() );
    if( circles )
    {
      updateCirclesAngles( *circles->surface(), w->rotationCenter(), rotation,
                          70, 100 );
      circles->setReferential( w->aWindow()->getReferential() );
      circles->glSetChanged( GLComponent::glGEOMETRY );
    }
    ++io;
    if( io != addobj.rend() )
    {
      ASurface<2> *axis = dynamic_cast<ASurface<2> *>( io->get() );
      if( axis )
      {
        updateAxisWithCircles( *axis->surface(), w->rotationCenter(), rotation,
                              70, 100 );
        axis->setReferential( w->aWindow()->getReferential() );
        axis->glSetChanged( GLComponent::glGEOMETRY );
      }
    }
  }
}


void Transformer::moveTrackball( int x, int y, int, int )
{
  if( !_maintrans )
    return;
  GLWidgetManager * w = dynamic_cast<GLWidgetManager *>( view() );
  if( !w )
  {
    cerr << "Transformer operating on wrong view type -- error\n";
    return;
  }

  Point3df center = getRotationCenter( w, _centerOnObjects );
  Point3df old_center = w->rotationCenter();
  if( old_center != center )
    w->setRotationCenter( center );

  Quaternion	q = rotation( x, y );
  q.norm();

  Transformation	t( 0, 0 );
  t.setQuaternion( q.inverse() );
  Point3df t0 = center;
  t0 -= t.transform( t0 ); // (I-R) t0
  t.SetTranslation( 0, t0[0] );
  t.SetTranslation( 1, t0[1] );
  t.SetTranslation( 2, t0[2] );

  rotationAxis = q.axis();
  updatePendingMotion(t.motion());
  setTransformData( t );
  updateTemporaryObjects( q );
  ::updateGVInfo( d, _maintrans /*_trans.begin()->first*/, this, q,
                  _centerOnObjects );
//   d->box1->moveTrackball( x, y );
//   d->box2->moveTrackball( x, y );

  if( center != old_center )
    w->setRotationCenter( old_center );

  AWindow3D    *w3 = dynamic_cast<AWindow3D *>( view()->aWindow() );
  if( w3 )
    w3->refreshNow();
}


void Transformer::endTrackball( int x, int y, int globx, int globy )
{
  d->box1->endTrackball( x, y );
  d->box2->endTrackball( x, y );
  Trackball::endTrackball( x, y, globx, globy );

  transmitValidatedMotion(pendingMotion(), true);
}


void Transformer::showGraphicsView()
{
  initGVItems( d->box1->graphicsView(), this, d );
  updateGVInfo( initialQuaternion() );
}


void Transformer::clearGraphicsView()
{
  removeGVItems( d->box1->graphicsView(), d );
}


void Transformer::toggleDisplayInfo()
{
  showGvItems( d, !d->show_info );
}


Transformer::Private *Transformer::data()
{
  return d;
}


void Transformer::matrixCellChanged( int row, int col )
{
  TransformerActionData::matrixCellChanged( row, col,
                                            d->trans_ui->matrix_tableWidget );
}


void Transformer::axisCellChanged( int row, int col )
{
  TransformerActionData::axisCellChanged( row, col,
                                          d->trans_ui->matrix_tableWidget );
}


void Transformer::centerCellChanged( int row, int col )
{
  TransformerActionData::centerCellChanged(
    row, col, d->trans_ui->rotation_center_tableWidget );
}


void Transformer::rotationAngleChanged()
{
  TransformerActionData::rotationAngleChanged(
    d->trans_ui->rotation_angle_lineEdit,
    d->trans_ui->rotation_angle_unit_comboBox );
}


void Transformer::rotationScaleChanged()
{
  TransformerActionData::rotationScaleChanged(
    d->trans_ui->rotation_scaling_lineEdit );
}


void Transformer::updateGVInfo( const Quaternion & q )
{
  ::updateGVInfo( d, _maintrans, this, q, _centerOnObjects );
}

// ------------------


TranslaterAction::TranslaterAction()
  : TransformerActionData(), Action(), d( new Private )
{
  d->box1.reset( new BoxViewSlice( this ) );
  d->box2.reset( new BoxViewSlice( this ) );
  initBoxes( *d->box1, *d->box2 );
}


TranslaterAction::~TranslaterAction()
{
  delete d;
}


TranslaterAction::TranslaterAction( const TranslaterAction & a ) 
  : TransformerActionData( a ), Action( a )
{
}


Action* TranslaterAction::creator()
{
  return( new TranslaterAction );
}


string TranslaterAction::name() const
{
  return( "TranslaterAction" );
}


void TranslaterAction::begin( int x, int y, int, int )
{
  selectTransformations( view()->aWindow() );
  if( !_maintrans )
    return;

  Referential *ref, *cref;
  if( isMainTransDirect() )
  {
    ref = _maintrans->source();
    cref = _maintrans->destination();
  }
  else
  {
    ref = _maintrans->destination();
    cref = _maintrans->source();
  }

  _started = true;
  _beginx = x;
  _beginy = y;

  d->box1->setObjectsReferential( ref );
  d->box2->setObjectsReferential( cref );
  d->box1->beginTrackball( x, y );
  d->box2->beginTrackball( x, y );

  initGVItems( d->box1->graphicsView(), this, d );
  ::updateGVInfo( d, _maintrans, this, Quaternion(), _centerOnObjects );
}


void TranslaterAction::move( int x, int y, int, int )
{
  if( !_maintrans )
    return;
  if( !_started )
  {
    cerr << "error: translation not started (BUG)\n";
    return;
  }

  GLWidgetManager * w = dynamic_cast<GLWidgetManager *>( view() );
  if( !w )
  {
    cerr << "Translate3DAction operating on wrong view type -- error\n";
    return;
  }

  // cout << "translater move\n";
  float	mx = _beginx - x;
  float	my = y - _beginy;
  Point3df	sz = w->windowBoundingMax() - w->windowBoundingMin();
  float oratio = float(w->width()) / w->height() / sz[0] * sz[1];
  if( oratio <= 1 )
  {
    mx *= sz[0] / w->width();
    my *= sz[1] / w->height() / oratio;
  }
  else
  {
    mx *= sz[0] / w->width() * oratio;
    my *= sz[1] / w->height();
  }
  mx /= w->zoom();
  my /= w->zoom();
  Point3df	p = w->quaternion().transform( Point3df( mx, my, 0 ) );
  Transformation	t( 0, 0 );
  float zfac = w->invertedZ() ? -1 : 1;
  t.SetTranslation( 0, zfac * p[0] );
  t.SetTranslation( 1, zfac * p[1] );
  t.SetTranslation( 2, p[2] );

  updatePendingMotion(t.motion());
  setTransformData( t );

  if( !_trans.empty() )
    ::updateGVInfo( d, _maintrans /*_trans.begin()->first*/, this,
                    Quaternion(), _centerOnObjects );
//   d->box1->moveTrackball( x, y );
//   d->box2->moveTrackball( x, y );
  AWindow3D    *w3 = dynamic_cast<AWindow3D *>( view()->aWindow() );
  if( w3 )
    w3->refreshNow();
  // cout << "translation done\n";
}


void TranslaterAction::end( int x, int y, int, int )
{
  d->box1->endTrackball( x, y );
  d->box2->endTrackball( x, y );
  _started = false;

  transmitValidatedMotion(pendingMotion(), true);
}


Quaternion TranslaterAction::initialQuaternion()
{
  return Quaternion();
}


void TranslaterAction::updateTemporaryObjects( const aims::Quaternion & )
{
}


void TranslaterAction::updateGVInfo( const Quaternion & q )
{
  ::updateGVInfo( d, _maintrans, this, q, _centerOnObjects );
}


// ----------------------


PlanarTransformer::PlanarTransformer() : Transformer()
{
}


PlanarTransformer::PlanarTransformer( const PlanarTransformer & a ) 
  : Transformer( a )
{
}


PlanarTransformer::~PlanarTransformer()
{
}


Action* PlanarTransformer::creator()
{
  return( new PlanarTransformer );
}


string PlanarTransformer::name() const
{
  return( "PlanarTransformer" );
}


Quaternion PlanarTransformer::initialQuaternion()
{
  AWindow3D     *w3 = dynamic_cast<AWindow3D *>( view()->aWindow() );
  if( !w3 )
    return Quaternion( 0, 0, 0, 1 );
  Point3df      axis = w3->sliceQuaternion().transformInverse(
    Point3df( 0, 0, -1 ) );
  Quaternion q( axis[0], axis[1], axis[2], 1. );
  return q;
}

Quaternion PlanarTransformer::rotation( int x, int y )
{


  GLWidgetManager * w = dynamic_cast<GLWidgetManager *>( view() );

  if( !w )
    {
      cerr << "PlanarTransformer operating on wrong view type -- error\n";
      return( Quaternion( 0, 0, 0, 1 ) );
    }

  AWindow3D	*w3 = dynamic_cast<AWindow3D *>( view()->aWindow() );
  if( !w3 )
    return Quaternion( 0, 0, 0, 1 );

  Point3df	axis = w3->sliceQuaternion().transformInverse(
    Point3df( 0, 0, -1 ) );

  float	dimx = w->width();
  float	dimy = w->height();

  // compute an intuitive trackball-compatible rotation angle
  Point3df	axis2 = axis;
  axis2[2] *= -1;
  axis2 = w->quaternion().transformInverse( axis2 ).normalize();
  float x1 = ( 2. * _beginx - dimx ) / dimx;
  float y1 = ( dimy - 2. * _beginy ) / dimy;
  float x2 = ( 2. * x - dimx ) / dimx;
  float y2 = ( dimy - 2. * y ) / dimy;
  //    coords on a sphere (initial, current)
  Point3df	a( x1, y1, tbProj2Sphere( 0.8, x1, y1 ) );
  Point3df	b( x2, y2, tbProj2Sphere( 0.8, x2, y2 ) );
  Point3df	c(0,0,0);// = w->rotationCenter();
  //    referential with 1st point (a-c, v2, axis2)
  Point3df	v3 = crossed( a - c, axis2 ).normalize();
  a = crossed( axis2, v3 ).normalize();
  //    b in this ref (only 2D coords are needed)
  b = Point3df( b.dot( a ), b.dot( v3 ), 0 );
  //    get the angle
  float	r = sqrt( b[0] * b[0] + b[1] * b[1] );
  float	angle = acos( b[0] / r );
  if( b[1] > 0 )
    angle *= -1;

  Quaternion	q;
  q.fromAxis( axis, angle );

  return q;
}

// -------------------


ResizerAction::ResizerAction() : TranslaterAction()
{
}


ResizerAction::~ResizerAction()
{
}


ResizerAction::ResizerAction( const ResizerAction & a ) 
  : TranslaterAction( a )
{
}


Action* ResizerAction::creator()
{
  return( new ResizerAction );
}


string ResizerAction::name() const
{
  return( "ResizerAction" );
}


void ResizerAction::begin( int x, int y, int globalX, int globalY )
{
  selectTransformations( view()->aWindow() );
  if( !_maintrans )
    return;

  GLWidgetManager * w = dynamic_cast<GLWidgetManager *>( view() );
  Point3df center = getRotationCenter( w, _centerOnObjects );
  Point3df old_center = w->rotationCenter();
  if( old_center != center )
    w->setRotationCenter( center );

  if( w && d->box2->additionalObjects().size() <= 1 )
  {
    AWindow* w3 = w->aWindow();
    rc_ptr<AObject> zoomm = zoomMarkers( w->rotationCenter(),
                                         130, 0.8, 0.3, 0.2, 1. );
    zoomm->setReferential( w3->getReferential() );
    d->box2->addObject( zoomm );
    rc_ptr<AObject> zooms = zoomStateMarkers( w->rotationCenter(),
                                              130, 1., 0.3, 0., 0.8, 1. );
    zooms->setReferential( w3->getReferential() );
    d->box2->addObject( zooms );
  }
  else
    updateTemporaryObjects( 1. );
  TranslaterAction::begin( x, y, globalX, globalY );

  if( center != old_center )
    w->setRotationCenter( old_center );
}


void ResizerAction::move( int /* x */, int y, int, int )
{
  if( !_maintrans )
    return;
  if( !_started )
    {
      cerr << "error: resize not started (BUG)\n";
      return;
    }

  GLWidgetManager * w = dynamic_cast<GLWidgetManager *>( view() );
  Point3df center = getRotationCenter( w, _centerOnObjects );
  Point3df old_center = w->rotationCenter();
  if( old_center != center )
    w->setRotationCenter( center );

  if( !w )
    {
      cerr << "ResizeAction operating on wrong view type -- error\n";
      return;
    }

  int	m = _beginy - y;
  float	zfac = exp( 0.01 * m );
  Point3df t0 = w->rotationCenter();

  Transformation	t( 0, 0 );
  t.SetRotation( 0, 0, zfac );
  t.SetRotation( 1, 1, zfac );
  t.SetRotation( 2, 2, zfac );
  t.SetTranslation( 0, t0[0] * ( 1 - zfac ) );
  t.SetTranslation( 1, t0[1] * ( 1 - zfac ) );
  t.SetTranslation( 2, t0[2] * ( 1 - zfac ) );

  updatePendingMotion(t.motion());
  setTransformData( t );
  updateTemporaryObjects( zfac );
  if( !_trans.empty() )
    ::updateGVInfo( d, _maintrans /*_trans.begin()->first*/, this,
                    Quaternion(), _centerOnObjects, zfac );
//   d->box1->moveTrackball( x, y );
//   d->box2->moveTrackball( x, y );

  if( old_center != center )
    w->setRotationCenter( old_center );

  AWindow3D    *w3 = dynamic_cast<AWindow3D *>( view()->aWindow() );
  if( w3 )
    w3->refreshNow();
  // cout << "scaling done\n";
}


void ResizerAction::updateTemporaryObjects( float zoom )
{
  GLWidgetManager * w = dynamic_cast<GLWidgetManager *>( view() );
  if( !w )
    return;
  list<rc_ptr<AObject> > & addobj = d->box2->additionalObjects();
  list<rc_ptr<AObject> >::reverse_iterator io = addobj.rbegin();
  ASurface<2> *zooms = dynamic_cast<ASurface<2> *>( io->get() );
  if( zooms )
  {
    updateZoomStateMarkers( *zooms->surface(), w->rotationCenter(), 130,
                            zoom );
    zooms->setReferential( w->aWindow()->getReferential() );
    zooms->glSetChanged( GLComponent::glGEOMETRY );
  }
}


