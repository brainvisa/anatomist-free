/* Copyright (c) 1995-2005 CEA
 *
 *  This software and supporting documentation were developed by
 *      CEA/DSV/SHFJ
 *      4 place du General Leclerc
 *      91401 Orsay cedex
 *      France
 *
 * This software is governed by the CeCILL license version 2 under 
 * French law and abiding by the rules of distribution of free software.
 * You can  use, modify and/or redistribute the software under the 
 * terms of the CeCILL license version 2 as circulated by CEA, CNRS
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
 * knowledge of the CeCILL license version 2 and that you accept its terms.
 */


#include <cstdlib>
#include <anatomist/action/paintaction.h>
#include <anatomist/application/roibasemodule.h>
#include <anatomist/object/Object.h>
#include <aims/bucket/bucket.h>
#include <aims/data/data.h>
#include <aims/vector/vector.h>
#include <anatomist/reference/Referential.h>
#include <anatomist/reference/Transformation.h>
#include <anatomist/window3D/window3D.h>
#include <anatomist/window/Window.h>
#include <anatomist/window/glwidget.h>
#include <anatomist/selection/selectFactory.h>
#include <anatomist/bucket/Bucket.h>
#include <anatomist/graph/Graph.h>
#include <anatomist/graph/GraphObject.h>
#include <anatomist/controler/view.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/misc/error.h>
#include <anatomist/commands/cLinkedCursor.h>
#include <anatomist/processor/Processor.h>

#include <aims/resampling/quaternion.h>
#include <aims/qtcompat/qvbox.h>
#include <aims/qtcompat/qhbox.h>
#include <aims/qtcompat/qvgroupbox.h>
#include <aims/qtcompat/qhgroupbox.h>
#include <aims/qtcompat/qhbuttongroup.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qslider.h>
#include <qcursor.h>

#include <algorithm>

using namespace std;

const unsigned short ACTION_ROI_PAINT = 0x0001;

namespace anatomist
{
  struct PaintActionView_Private {
    PaintAction * myPaintAction ;
    
    QHBox * myUpperPart ;
    
    QVGroupBox * myBrushParameters ;
    QHButtonGroup * myBrushes ;
    QPushButton * myPointBrush ;
    QPushButton * myDiskBrush ;
    QPushButton * myBallBrush ;
    
    QHGroupBox * myBrushSizeBox ;
    QSlider * myBrushSize ;
    QLabel * myBrushSizeLabel ;

    QHGroupBox * myRegionTransparencyBox ;
    QSlider * myRegionTransparency ;
    QLabel * myRegionTransparencyLabel ;
    
    QVGroupBox * myModes ;
    
    QHButtonGroup * myLineMode ;
    QRadioButton * myLineModeOn ;
    QRadioButton * myLineModeOff ;
    QHButtonGroup * myReplaceMode ;
    QRadioButton * myReplaceModeOn ;
    QRadioButton * myReplaceModeOff ;
    
    QHButtonGroup * myLinkedCursorMode ;
    QRadioButton * myLinkedCursorModeOn ;
    QRadioButton * myLinkedCursorModeOff ;
    QHButtonGroup * myMmMode ;
    QRadioButton * myMmModeOn ;
    QRadioButton * myVoxelModeOn ;
    
    
    QVGroupBox * myDirectActions ;
    QHBox * myLeftDirectActions ;
    QPushButton * myUndoButton ;
    QPushButton * myRedoButton ;
    
    QHBox * myRightDirectActions ;  
    QPushButton * myFillButton ;
    QPushButton * myClearRegionButton ;
  } ;
}

using namespace anatomist ;
using namespace aims;


PaintActionView::PaintActionView( PaintAction * paintAction, QWidget * parent)
  : QVBox(parent), Observer(), myUpdatingFlag(false)
{
  _private = new PaintActionView_Private ;

  _private->myPaintAction = paintAction ;
  paintAction->addObserver(this) ;
  RoiChangeProcessor::instance()->addObserver(this) ;
  
  _private->myUpperPart = new QHBox( this ) ;

  _private->myBrushParameters = new QVGroupBox( tr("Brush"), 
						_private->myUpperPart ) ;
  _private->myBrushes = new QHButtonGroup( tr("Brushes"), 
					   _private->myBrushParameters ) ;
  _private->myBrushes->setExclusive(true) ;
  _private->myPointBrush = new QPushButton(tr("Point"), _private->myBrushes) ;
  _private->myPointBrush->setToggleButton(true) ;
  _private->myDiskBrush = new QPushButton(tr("Disk"), _private->myBrushes) ;
  _private->myDiskBrush->setToggleButton(true) ;
  _private->myBallBrush = new QPushButton(tr("Ball"), _private->myBrushes) ;
  _private->myBallBrush->setToggleButton(true) ;  
  
  if( _private->myPaintAction->paintType() == PaintStrategy::POINT)
    _private->myBrushes->setButton(0) ;
  else if( _private->myPaintAction->paintType() == PaintStrategy::DISK)
    _private->myBrushes->setButton(1) ;
  else if( _private->myPaintAction->paintType() == PaintStrategy::BALL)
    _private->myBrushes->setButton(2) ;
  
  _private->myBrushSizeBox = new QHGroupBox( tr("Brush Size"), 
						_private->myBrushParameters ) ;
  _private->myBrushSize = new QSlider( 1, 500, 10, int(rint(_private->myPaintAction->brushSize()*10.)),
		     Qt::Horizontal, _private->myBrushSizeBox ) ;
  _private->myBrushSize->setMinimumSize( 50, 
					 _private->myBrushSize->sizeHint().height() );
  _private->myBrushSizeLabel = 
    new QLabel( QString::number(_private->myPaintAction->brushSize()), 
		_private->myBrushSizeBox ) ;
  _private->myBrushSizeLabel
    ->setMinimumSize( 25, _private->myBrushSizeLabel->sizeHint().height() );

  
  _private->myRegionTransparencyBox = new QHGroupBox( tr("Transparency"), 
						      _private->myBrushParameters ) ;
  if( SelectFactory::selectColor().a == 1 )
    SelectFactory::selectColor().a = 0.99 ;
  
  _private->myRegionTransparency = 
    new QSlider( 0, 100, 20, 
		 int(SelectFactory::selectColor().a * 100),
		 Qt::Horizontal, _private->myRegionTransparencyBox ) ;
  _private->myRegionTransparency
    ->setMinimumSize( 50, _private->myBrushSize->sizeHint().height() );
  _private->myRegionTransparency->setTracking(true) ;

  _private->myRegionTransparencyLabel = 
    new QLabel( QString::number(int(SelectFactory::selectColor().a * 100)), 
		_private->myRegionTransparencyBox ) ;
  _private->myBrushSizeLabel
    ->setMinimumSize( 25, _private->myBrushSizeLabel->sizeHint().height() );
  

  _private->myModes = new QVGroupBox( tr("Modes"), _private->myUpperPart ) ;

  _private->myLineMode = new QHButtonGroup( tr("Line"), _private->myModes ) ;
  _private->myLineMode->setExclusive(true) ;
  _private->myLineModeOn = new QRadioButton(tr("On"), _private->myLineMode ) ;
  _private->myLineModeOff = new QRadioButton( tr("Off"), 
					      _private->myLineMode ) ;
  _private->myLineMode->setButton( 0 ) ;
  
  _private->myReplaceMode = new QHButtonGroup( tr("Replace"), 
					       _private->myModes ) ;
  _private->myReplaceMode->setExclusive(true) ;
  _private->myReplaceModeOn = new QRadioButton( tr("On"), 
						_private->myReplaceMode ) ;
  _private->myReplaceModeOff = new QRadioButton( tr("Off"), 
						 _private->myReplaceMode ) ;
  _private->myReplaceMode->setButton( 1 ) ;
  
  _private->myLinkedCursorMode = 
    new QHButtonGroup( tr("LinkedCursor"), _private->myModes ) ;
  _private->myLinkedCursorMode->setExclusive(true) ;
  _private->myLinkedCursorModeOn = 
    new QRadioButton(tr("On"), _private->myLinkedCursorMode ) ;
  _private->myLinkedCursorModeOff = 
    new QRadioButton(tr("Off"), _private->myLinkedCursorMode ) ;
  _private->myLinkedCursorMode->setButton( 1 ) ;

  _private->myMmMode = 
    new QHButtonGroup( tr("BrushUnit"), _private->myModes ) ;
  _private->myMmMode->setExclusive(true) ;
  _private->myVoxelModeOn = 
    new QRadioButton(tr("Voxel"), _private->myMmMode ) ;
  _private->myMmModeOn = 
    new QRadioButton(tr("Mm"), _private->myMmMode ) ;
  _private->myMmMode->setButton( ( _private->myPaintAction->mmMode() ? 1 : 0 ) ) ;
  
  _private->myDirectActions = 
    new QVGroupBox( tr("Actions"), this ) ;
  _private->myLeftDirectActions = new QHBox( _private->myDirectActions ) ;
  
  _private->myUndoButton = new QPushButton(tr("Undo"), 
					   _private->myLeftDirectActions ) ;
  if( !_private->myPaintAction->undoable() )
    _private->myUndoButton->setEnabled( false ) ;
  
  _private->myRedoButton = new QPushButton( tr("Redo"), 
					    _private->myLeftDirectActions ) ;
  if( !_private->myPaintAction->redoable() )
    _private->myRedoButton->setEnabled( false ) ;
  
  _private->myRightDirectActions = new QHBox( _private->myDirectActions ) ;
  _private->myFillButton = 
    new QPushButton(tr("Fill Region"), _private->myRightDirectActions ) ;
  _private->myFillButton->setEnabled(false) ;
    
  _private->myClearRegionButton = 
    new QPushButton(tr("Clear Region"), _private->myRightDirectActions ) ;

  connect( _private->myBrushes, SIGNAL(clicked(int)), 
	  this, SLOT(brushSelection(int) ) ) ;

  connect( _private->myBrushSize, SIGNAL(valueChanged(int)), 
	   this, SLOT(brushSizeChange(int) ) ) ;
  
  connect( _private->myRegionTransparency, 
	   SIGNAL(valueChanged(int) ),
	   this, SLOT(regionTransparencyChange( int ) ) ) ;
  
  connect( _private->myLineModeOn, SIGNAL(clicked()), 
	   this, SLOT(lineModeOn()) ) ;

  connect( _private->myLineModeOff, SIGNAL(clicked()), 
	   this, SLOT(lineModeOff()) ) ;

  connect( _private->myReplaceModeOn, SIGNAL(clicked()), 
	  this, SLOT(replaceModeOn()) ) ;

  connect( _private->myReplaceModeOff, SIGNAL(clicked()), 
	  this, SLOT(replaceModeOff()) ) ;

  connect( _private->myLinkedCursorModeOn, SIGNAL(clicked()), 
	  this, SLOT(linkedCursorModeOn()) ) ;

  connect( _private->myLinkedCursorModeOff, SIGNAL(clicked()), 
	  this, SLOT(linkedCursorModeOff()) ) ;

  connect( _private->myMmModeOn, SIGNAL(clicked()), 
	  this, SLOT(mmModeOn()) ) ;

  connect( _private->myVoxelModeOn, SIGNAL(clicked()), 
	  this, SLOT(voxelModeOn()) ) ;

  connect( _private->myUndoButton, SIGNAL(clicked()), 
	   this, SLOT(undoAction()) ) ;

  connect( _private->myRedoButton, SIGNAL(clicked()), 
	   this, SLOT(redoAction()) ) ;

  connect( _private->myFillButton, SIGNAL(clicked()), 
	   this, SLOT(fillAction()) ) ;

  connect( _private->myClearRegionButton, SIGNAL(clicked()), 
	   this, SLOT(clearRegionAction()) ) ;  
}


PaintActionView::~PaintActionView()
{
  _private->myPaintAction->deleteObserver( this ) ;
  RoiChangeProcessor::instance()->deleteObserver(this) ;
}

void 
PaintActionView::brushSelection( int id ) 
{
  //cout <<  "BRUSH SELECTION" << endl ;
  if( myUpdatingFlag)
    return ;
  
  switch( id) {
  case 0:
    _private->myPaintAction->brushToPoint() ;
    //cout << "Brush : Point" << endl ;
    break ;
  case 1:
    _private->myPaintAction->brushToDisk() ;
    //cout << "Brush : Disk" << endl ;
    break;
  case 2:
    _private->myPaintAction->brushToBall() ;
    //cout << "Brush : Ball" << endl ;
    break ;
  default:
    break;
  }
}

void 
PaintActionView::brushSizeChange( int size ) 
{
  if( myUpdatingFlag)
    return ;
  
  _private->myPaintAction->setSize( (  _private->myPaintAction->mmMode() ? size / 10. : std::max( int(rint(size / 10.)), int(1)) ) ) ;

  _private->myBrushSizeLabel->setText( QString::number( ( _private->myPaintAction->mmMode() ? size / 10. :  std::max( int( rint(size / 10.) ), int(1) ) ) ) ) ;
}

void 
PaintActionView::regionTransparencyChange( int alpha ) 
{
  if( myUpdatingFlag)
    return ;
  
  _private->myPaintAction->changeRegionTransparency( float(alpha) / 100. ) ;
  _private->myRegionTransparencyLabel->setText( QString::number( alpha ) ) ;
}

void 
PaintActionView::lineModeOn() 
{
  if( myUpdatingFlag)
    return ;
  
  _private->myPaintAction->lineOn() ;
}

void 
PaintActionView::lineModeOff() 
{
  if( myUpdatingFlag)
    return ;
  
  _private->myPaintAction->lineOff() ;
}

void 
PaintActionView::replaceModeOn() 
{
  if( myUpdatingFlag)
    return ;
  
  _private->myPaintAction->replaceOn() ;
}

void 
PaintActionView::replaceModeOff() 
{
  if( myUpdatingFlag)
    return ;
  
  _private->myPaintAction->replaceOff() ;
}

void 
PaintActionView::linkedCursorModeOn() 
{
  if( myUpdatingFlag)
    return ;
  
  _private->myPaintAction->followingLinkedCursorOn() ;
}

void 
PaintActionView::linkedCursorModeOff() 
{
  if( myUpdatingFlag)
    return ;
  
  _private->myPaintAction->followingLinkedCursorOff() ;
}

void 
PaintActionView::mmModeOn() 
{
  if( myUpdatingFlag)
    return ;
  
  _private->myPaintAction->brushToMm() ;
}

void 
PaintActionView::voxelModeOn() 
{
  if( myUpdatingFlag)
    return ;
  
  _private->myPaintAction->brushToVoxel() ;
}

void 
PaintActionView::undoAction() 
{
  _private->myPaintAction->undo() ;
}

void 
PaintActionView::redoAction() 
{
  _private->myPaintAction->redo() ;
}

void 
PaintActionView::fillAction() 
{
  //...
}

void 
PaintActionView::clearRegionAction() 
{
  _private->myPaintAction->clearRegion() ;
}

void 
PaintActionView::update( const Observable *, void * )
{
  //cout << "PaintActionView::update" << endl ;
  if( myUpdatingFlag )
    return ;
  myUpdatingFlag = true ;

  // Updating mode parameters
  if ( _private->myPaintAction->lineMode() )
    _private->myLineMode->setButton(0) ;
  else
    _private->myLineMode->setButton(1) ;
  
  if ( _private->myPaintAction->replaceMode() )
    _private->myReplaceMode->setButton(0) ;
  else
    _private->myReplaceMode->setButton(1) ;
  
  if ( _private->myPaintAction->followingLinkedCursorMode() )
    _private->myLinkedCursorMode->setButton(0) ;
  else
    _private->myLinkedCursorMode->setButton(1) ;

  // Updating BrushParameters
  if( _private->myPaintAction->paintType() == PaintStrategy::POINT)
    _private->myBrushes->setButton(0) ;
  else if( _private->myPaintAction->paintType() == PaintStrategy::DISK)
    _private->myBrushes->setButton(1) ;
  else if( _private->myPaintAction->paintType() == PaintStrategy::BALL)
    _private->myBrushes->setButton(2) ;
  

  float size = _private->myPaintAction->brushSize() ;
  _private->myBrushSize->setValue( int(size * 10) ) ;
  _private->myBrushSizeLabel->setText( QString::number( size ) ) ;
  
  int transparency =  int(SelectFactory::selectColor().a * 100) ;
  _private->myRegionTransparency->setValue( transparency );
  _private->myRegionTransparencyLabel->setText( QString::number( transparency ) ) ;

  // Refresh action buttons
  if( _private->myPaintAction->redoable() )
    _private->myRedoButton->setEnabled( true ) ;
  else
    _private->myRedoButton->setEnabled( false ) ;
  
  if( _private->myPaintAction->undoable() )
    _private->myUndoButton->setEnabled( true ) ;
  else
    _private->myUndoButton->setEnabled( false ) ;

  myUpdatingFlag = false ;
}


PaintActionSharedData* PaintActionSharedData::_instance = 0 ;

PaintActionSharedData* PaintActionSharedData::instance()
{
  if( _instance == 0 )
    _instance = new PaintActionSharedData ;
  return _instance ;
}

void 
PaintActionSharedData::update (const Observable *, void *)
{
  //cout << "PaintActionSharedData::update" << endl ;
  setChanged() ;
  notifyObservers(this) ;
}

// Paint Action class

PaintAction::PaintAction() : Action()
{
  _sharedData = PaintActionSharedData::instance() ;
} 

PaintAction::~PaintAction( ) 
{
}

string PaintAction::name() const
{
  return QT_TRANSLATE_NOOP( "ControlSwitch", "PaintAction" );
} 

// void 
// PaintAction::refreshWindow( ) 
// {  
//   AGraph * g = RoiChangeProcessor::instance()->getGraph( view()->window() ) ;
//   if (g)
//     g->updateAll() ;
// }


void 
PaintAction::increaseBrushSize( ) 
{  
  cout << "PaintAction::increaseBrushSize" << endl ;
  
  if ( _sharedData->myBrushSize >= 50. )
    return ;
  _sharedData->myBrushSize += 1. ;
  
  setChanged() ;
  notifyObservers();
  _sharedData->myCursorShapeChanged = true;
  updateCursor();
}

void 
PaintAction::decreaseBrushSize( )
{
  cout << "PaintAction::decreaseBrushSize" << endl ;
  if ( _sharedData->myBrushSize <= 1. )
    return ;
  _sharedData->myBrushSize -= 1. ;

  setChanged() ;
  notifyObservers() ;
  _sharedData->myCursorShapeChanged = true;
  updateCursor();
}

void 
PaintAction::setSize( float size)
{
  _sharedData->myBrushSize = size ;
  
  if( size < 1. )
    _sharedData->myBrushSize = 1. ;
  
  if( size > 50. )
    _sharedData->myBrushSize = 50. ;

  setChanged() ;
  notifyObservers() ;
  _sharedData->myCursorShapeChanged = true;
  updateCursor();
}

float
PaintAction::brushSize() 
{
  return _sharedData->myBrushSize ;
}

PaintStrategy::PaintType 
PaintAction::paintType()
{
  return _sharedData->myPainter->paintType() ;
}


namespace
{

  AObject* renderBeforeObject( const AWindow3D* win, AGraph* gr )
  {
    list<AObject *> objs;
    list<AObject *>::iterator i, e = objs.end();
    win->processRenderingOrder( objs );
    for( i=objs.begin(); i!=e; ++i )
    {
      const AObject::ParentList & pl = (*i)->parents();
      if( pl.find( gr ) != pl.end() )
        return *i;
    }
    if( objs.empty() )
      return 0;
    return objs.front();
  }

}


void 
PaintAction::paintStart( int x, int y, int globalX, int globalY )
{
  hideCursor();
  // We've first to get the selected bucket.
  if( !( _sharedData->myCurrentModifiedRegion =
        RoiChangeProcessor::instance()->getCurrentRegion(view()->window() ) ) )
    {
      _sharedData->myValidRegion = false ;
      return ;
    }
  _sharedData->myValidRegion = true ;

  _sharedData->myCurrentChanges = new list< pair< Point3d, ChangesItem> > ;

  _sharedData->myDeltaModifications
    ->setVoxelSize( _sharedData->myCurrentModifiedRegion->VoxelSize() );
  _sharedData->myDeltaModifications
    ->setReferential( _sharedData->myCurrentModifiedRegion->getReferential() );
  _sharedData->myDeltaModifications->GetMaterial().SetDiffuse
    ( 0.0, 0.8, 0.2,
      _sharedData->myCurrentModifiedRegion->GetMaterial().Diffuse(3) );

  _sharedData->myPainting = true ;
  
  RoiChangeProcessor::instance()->setRedoable( false ) ;
  
  setChanged() ;
  
  AGraph * g = RoiChangeProcessor::instance()->getGraph( view()->window() ) ;
  if (!g) return ;
  
  AimsData<AObject*>& labels = g->volumeOfLabels( 0 ) ;
  if( labels.dimX() != ( g->MaxX2D() - g->MinX2D() + 1 ) || 
      labels.dimY() != ( g->MaxY2D() - g->MinY2D() + 1 ) ||
      labels.dimZ() != ( g->MaxZ2D() - g->MinZ2D() + 1 ) )
    {
      g->clearLabelsVolume() ;
      g->setLabelsVolumeDimension( static_cast<int>( g->MaxX2D() 
						     - g->MinX2D() ) + 1, 
				   static_cast<int>( g->MaxY2D() 
						     - g->MinY2D() ) + 1,
				   static_cast<int>( g->MaxZ2D() 
						     - g->MinZ2D() ) + 1 ) ;
    }
  
  RoiChangeProcessor::instance()->getGraphObject( view()->window() )
    ->attributed()->setProperty("modified", true) ;

  myLinkedWindows.clear() ;
  set<AWindow*> group = g->WinList() ;
  set<AWindow*>::iterator gbegin = group.begin();
  set<AWindow*>::iterator gend = group.end();
  for( set<AWindow*>::iterator i = gbegin; i != gend; ++i ){
    AWindow3D * win3d = dynamic_cast<AWindow3D*>(*i) ;
    if ( win3d )
    {
      myLinkedWindows.push_back( win3d ) ;
      // _sharedData->myDeltaModifications->setName( "tmp modifications" );
      win3d->registerObject( _sharedData->myDeltaModifications, true );
      win3d->renderBefore( _sharedData->myDeltaModifications,
        renderBeforeObject( win3d, g ) );
    }
  }

  paint( x, y, globalX, globalY ) ;
//   notifyObservers() ;
}

void 
PaintAction::paint( int x, int y, int, int )
{
  // cerr << "PaintAction::paint : entering" << endl ;

  // Now, we've to find the roi graph itself, which is a parent of the bucket.
  AWindow3D * win = dynamic_cast<AWindow3D*>( view()->window() ) ;
  if( !win )
    {
      cerr << "warning: PaintAction operating on wrong view type\n";
      return;
    }

  if ( !_sharedData->myValidRegion )
    return ;

  Referential* winRef = win->getReferential() ;
  Referential* buckRef = _sharedData->myCurrentModifiedRegion->getReferential() ;
  Point3df pos ;
  if( win->positionFromCursor( x, y, pos ) )
    {
      _sharedData->myCursorPos = pos;
      AGraph	*g = RoiChangeProcessor::instance()->getGraph( view()->window() );

      _sharedData->myPainter->paint( win,
		        theAnatomist->getTransformation(winRef, buckRef), 
			pos,
			(AGraphObject *)0, RoiChangeProcessor::instance()->getGraphObject( view()->window() ), 
			_sharedData->myBrushSize, _sharedData->myLineMode,
			/* BEWARE, WE MIGHT NEED TO DRAW ROI OVER TIME*/
			&g->volumeOfLabels( 0 ),
			Point3df( g->MinX2D(), g->MinY2D(), g->MinZ2D() ), 
			/* BEWARE, WE MIGHT NEED TO DRAW ROI OVER TIME*/
			(_sharedData->myDeltaModifications->bucket())[0],
			(*_sharedData->myCurrentChanges),
			_sharedData->myCurrentModifiedRegion->VoxelSize(),
			_sharedData->myLineMode,
			_sharedData->myReplaceMode, _sharedData->myMmMode ) ;
      _sharedData->myIsChangeValidated = false ;

      _sharedData->myDeltaModifications->setBucketChanged() ;
      _sharedData->myDeltaModifications->setGeomExtrema();	// not optimal
//       _sharedData->myDeltaModifications
// 	->setSubBucketGeomExtrema(Point3df( _sharedData->myDeltaModifications->MinX2D(), 
// 					    _sharedData->myDeltaModifications->MinY2D(),
// 					    _sharedData->myDeltaModifications->MinZ2D() ),
// 				  Point3df( _sharedData->myDeltaModifications->MaxX2D(), 
// 					    _sharedData->myDeltaModifications->MaxY2D(),
// 					    _sharedData->myDeltaModifications->MaxZ2D()+1.) );

      if( _sharedData->myFollowingLinkedCursor )
	{
	  vector<float>	vp;
	  vp.push_back( pos[0] );
	  vp.push_back( pos[1] );
	  vp.push_back( pos[2] );
	  LinkedCursorCommand	*c 
	    = new LinkedCursorCommand( win, vp );
	  theProcessor->execute( c );    
	}

      list<AWindow3D*>::iterator iter( myLinkedWindows.begin() ), 
	last( myLinkedWindows.end() ) ;

      while ( iter != last ){
	(*iter)->refreshTemp() ;
	++iter ; 
      }

    }
}



void 
PaintAction::eraseStart( int x, int y, int globalX, int globalY )
{
  hideCursor();
  //cerr << "PaintAction::eraseStart : entering" << endl ;

  // We got first to get the selected bucket.
  if ( ! ( _sharedData->myCurrentModifiedRegion = 
	   RoiChangeProcessor::instance()->getCurrentRegion(view()->window() ) ) ) {
    _sharedData->myValidRegion = false ;
    return ;
  }
  _sharedData->myValidRegion = true ;

  // Actions are always responsible of myCurrentChanges creation and 
  // RoiChangeProcessor of it's destruction
  _sharedData->myCurrentChanges = new list< pair< Point3d, ChangesItem> > ;
  
  _sharedData->myDeltaModifications->bucket().clear() ;
  _sharedData->myDeltaModifications
    ->setVoxelSize( _sharedData->myCurrentModifiedRegion->VoxelSize() );
  _sharedData->myDeltaModifications->GetMaterial().SetDiffuse
    ( 1., 1., 1., 0.3 );

  _sharedData->myPainting = false ;
  RoiChangeProcessor::instance()->setRedoable( false ) ;

  setChanged() ;

  AGraph * g = RoiChangeProcessor::instance()->getGraph( view()->window() ) ;
  if( !g ) return ;
  AimsData<AObject*>& labels = g->volumeOfLabels( 0 ) ;
  if( labels.dimX() != ( g->MaxX2D() - g->MinX2D() + 1 ) || 
      labels.dimY() != ( g->MaxY2D() - g->MinY2D() + 1 ) ||
      labels.dimZ() != ( g->MaxZ2D() - g->MinZ2D() + 1 ) ){
    g->clearLabelsVolume() ;
    g->setLabelsVolumeDimension( static_cast<int>( g->MaxX2D() - g->MinX2D() ) + 1, 
				 static_cast<int>( g->MaxY2D() - g->MinY2D() ) + 1,
				 static_cast<int>( g->MaxZ2D() - g->MinZ2D() ) + 1 ) ;
  }
  
  RoiChangeProcessor::instance()->getGraphObject( view()->window() )
    ->attributed()->setProperty("modified", true) ;

  myLinkedWindows.clear() ;
  set<AWindow*> group = g->WinList() ;
  set<AWindow*>::iterator gbegin = group.begin();
  set<AWindow*>::iterator gend = group.end();
  for( set<AWindow*>::iterator i = gbegin; i != gend; ++i ){
    AWindow3D * win3d = dynamic_cast<AWindow3D*>(*i) ;
    if (win3d)
    {
      myLinkedWindows.push_back( win3d ) ;
      win3d->registerObject( _sharedData->myDeltaModifications, true );
      win3d->renderBefore( _sharedData->myDeltaModifications,
        renderBeforeObject( win3d, g ) );
    }
  }

  erase( x, y, globalX, globalY ) ;
//   notifyObservers() ;
  //cerr << "PaintAction::eraseStart : exiting" << endl ;
}

void 
PaintAction::erase( int x, int y, int, int ) 
{
  //cerr << "PaintAction::erase : entering" << endl ;

  AWindow3D * win = dynamic_cast<AWindow3D*>( view()->window() ) ;
  if( !win )
    {
      cerr << "warning: PaintAction operating on wrong view type\n";
      return;
    }

  if( !_sharedData->myValidRegion )
    return ;

  Referential* winRef = win->getReferential() ;
  Referential* buckRef = _sharedData->myCurrentModifiedRegion->getReferential() ;

  Point3df pos ;
  if( win->positionFromCursor( x, y, pos ) )
    {
      _sharedData->myCursorPos = pos;
      Point3df normalVector( win->sliceQuaternion().
			     apply(Point3df(0., 0., 1.) ) ) ;

      normalVector *= normalVector.dot( pos - win->GetPosition() ) ;
      pos = pos - normalVector ;

      //Bucket * temp = new Bucket() ;
      //temp->setVoxelSize( _sharedData->myCurrentModifiedRegion->VoxelSize() ) ;
      AGraph	*g = RoiChangeProcessor::instance()->getGraph( view()->window() );
      _sharedData->myPainter-> paint( win,
			 theAnatomist->getTransformation(winRef, buckRef), pos,
			 RoiChangeProcessor::instance()->getGraphObject( view()->window() ), 
			 0, _sharedData->myBrushSize, 
			 _sharedData->myLineMode,
			 /* BEWARE, WE MIGHT NEED TO DRAW ROI OVER TIME*/
			 &g->volumeOfLabels( 0 ),
			 /* BEWARE, WE MIGHT NEED TO DRAW ROI OVER TIME*/
			 Point3df( g->MinX2D(), g->MinY2D(), g->MinZ2D() ), 
			 //(temp->bucket())[0],
                         (_sharedData->myDeltaModifications->bucket())[0],
			 (*_sharedData->myCurrentChanges),
			 _sharedData->myCurrentModifiedRegion->VoxelSize(),
			 _sharedData->myLineMode,
			 _sharedData->myReplaceMode, _sharedData->myMmMode) ;
      _sharedData->myIsChangeValidated = false ;
      // _sharedData->myCurrentModifiedRegion->erase( temp->bucket() ) ;
      //_sharedData->myCurrentModifiedRegion->setBucketChanged() ;
      _sharedData->myDeltaModifications->setBucketChanged() ;
      _sharedData->myDeltaModifications->setGeomExtrema();      // not optimal

      if( _sharedData->myFollowingLinkedCursor )
      {
        vector<float>	vp;
        vp.push_back( pos[0] );
        vp.push_back( pos[1] );
        vp.push_back( pos[2] );
        LinkedCursorCommand	*c
          = new LinkedCursorCommand( win, vp );
        theProcessor->execute( c );
      }

      list<AWindow3D*>::iterator iter( myLinkedWindows.begin() ),
        last( myLinkedWindows.end() ) ;

      while ( iter != last )
      {
        (*iter)->refreshTemp();
        ++iter ;
      }
    }
  // cerr << "PaintAction::erase : exiting" << endl ;
}




void 
PaintAction::clearRegion()
{
  if ( ! ( _sharedData->myCurrentModifiedRegion = 
	   RoiChangeProcessor::instance()->getCurrentRegion( 0/*view()->window()*/ ) ) ) {
    _sharedData->myValidRegion = false ;
    return ;
  }
  _sharedData->myValidRegion = true ;

  _sharedData->myCurrentChanges = new list< pair< Point3d, ChangesItem> > ;
  
  _sharedData->myDeltaModifications->bucket()[0].clear() ;
  _sharedData->myDeltaModifications
    ->setVoxelSize( _sharedData->myCurrentModifiedRegion->VoxelSize() );
  _sharedData->myDeltaModifications
    ->setReferential( _sharedData->myCurrentModifiedRegion->getReferential() );
  
  AGraph * g = RoiChangeProcessor::instance()->getGraph( 0 ) ;
  AGraphObject * go = RoiChangeProcessor::instance()->getGraphObject( 0 ) ;

  if( go )
    go->attributed()->setProperty("modified", true) ;
  
  
  if (!g) return ;
  AimsData<AObject*>& label = g->volumeOfLabels( 0 ) ;
  if( label.dimX() != ( g->MaxX2D() - g->MinX2D() + 1 ) || 
      label.dimY() != ( g->MaxY2D() - g->MinY2D() + 1 ) ||
      label.dimZ() != ( g->MaxZ2D() - g->MinZ2D() + 1 ) ){
    g->clearLabelsVolume() ;
    g->setLabelsVolumeDimension( static_cast<int>( g->MaxX2D() - g->MinX2D() ) + 1, 
				 static_cast<int>( g->MaxY2D() - g->MinY2D() ) + 1,
				 static_cast<int>( g->MaxZ2D() - g->MinZ2D() ) + 1 ) ;
  }
  
  AimsData<AObject*>& labels = g->volumeOfLabels( 0 ) ;
  
  BucketMap<Void>::Bucket::iterator 
    iter( _sharedData->myCurrentModifiedRegion->bucket()[0].begin() ), 
    last( _sharedData->myCurrentModifiedRegion->bucket()[0].end() ) ;
  
  while ( iter != last){
    ChangesItem item ;
    item.after = 0 ;
    item.before = go ;

    _sharedData->myCurrentChanges->push_back( pair<Point3d, ChangesItem>(iter->first, item) ) ;
    
    labels( iter->first ) = 0  ;
    ++iter ;
  }
  
  if ( ! (*_sharedData->myCurrentChanges).empty() )
    RoiChangeProcessor::instance()->applyChange( _sharedData->myCurrentChanges ) ;
  
  _sharedData->myCurrentModifiedRegion->setBucketChanged() ;
    
  _sharedData->myIsChangeValidated = true ;
  _sharedData->myPainting = false ;
  
  setChanged() ;
  notifyObservers() ;
}


void
PaintAction::changeRegionTransparency( float alpha )
{
  SelectFactory::selectColor().a = alpha ;
  SelectFactory::selectColor().na = false ;

  map< unsigned, set< AObject *> > sel = SelectFactory::factory()->selected ();
  
  
  map< unsigned, set< AObject *> >::iterator iter( sel.begin( ) ), 
    last( sel.end( ) ) ;
  
  while( iter != last ){
    SelectFactory::factory()->unselectAll( iter->first ) ;
    SelectFactory::factory()->select( iter->first, iter->second ) ;
    for( set<AObject*>::iterator it = iter->second.begin() ; it != iter->second.end() ; ++it )
      (*it)->notifyObservers() ;
    ++iter ;
  }
  
  setChanged() ;
  notifyObservers() ;
}

void 
PaintAction::lineOn( ) 
{
  //cout << "Line mode set to ON" << endl ;
  _sharedData->myLineMode = true ;

  setChanged() ;
  notifyObservers() ;
}


void 
PaintAction::lineOff( ) 
{
  //cout << "Line mode set to OFF" << endl ;
  _sharedData->myLineMode = false ; 

  setChanged() ;
  notifyObservers() ;
}

void 
PaintAction::replaceOn( ) 
{
  //cout << "Replace mode set to ON" << endl ;
  _sharedData->myReplaceMode = true ;

  setChanged() ;
  notifyObservers() ;
}


void 
PaintAction::replaceOff( ) 
{
  //cout << "Replace mode set to OFF" << endl ;
  _sharedData->myReplaceMode = false ; 

  setChanged() ;
  notifyObservers() ;
}


void 
PaintAction::followingLinkedCursorOn( )
{
  //cout << "Following linked cursor mode set to ON" << endl ;
  _sharedData->myFollowingLinkedCursor = true ;

  setChanged() ;
  notifyObservers() ;
}
 
void 
PaintAction::followingLinkedCursorOff( )
{
  //cout << "Following linked cursor mode set to Off" << endl ;
  _sharedData->myFollowingLinkedCursor = false ;  

  setChanged() ;
  notifyObservers() ;
}

void 
PaintAction::brushToSquare( ) 
{
  //cerr << "PaintAction::brushToSquare : entering" << endl ;

  if ( _sharedData->myPainter->paintType() == PaintStrategy::SQUARE )
    return ;
  if(_sharedData->myPainter) 
    delete _sharedData->myPainter ;
  _sharedData->myPainter = new SquarePaintStrategy ;

  //cerr << "PaintAction::brushToSquare : exiting" << endl ;

  setChanged() ;
  notifyObservers() ;
  _sharedData->myCursorShapeChanged = true;
  updateCursor();
}



void 
PaintAction::brushToDisk( ) 
{
  if ( _sharedData->myPainter->paintType() == PaintStrategy::DISK )
    return ;
  if(_sharedData->myPainter) 
    delete _sharedData->myPainter ;
  _sharedData->myPainter = new DiskPaintStrategy ;

  setChanged() ;
  notifyObservers() ;
  _sharedData->myCursorShapeChanged = true;
  updateCursor();
}

void 
PaintAction::brushToBall() 
{
  if ( _sharedData->myPainter->paintType() == PaintStrategy::BALL )
    return ;
  if(_sharedData->myPainter) 
    delete _sharedData->myPainter ;
  _sharedData->myPainter = new BallPaintStrategy ;

  setChanged() ;
  notifyObservers() ;
  _sharedData->myCursorShapeChanged = true;
  updateCursor();
}


// Beware, size doesn't matter in this mode.
void 
PaintAction::brushToPoint( ) 
{
  if ( _sharedData->myPainter->paintType() == PaintStrategy::POINT )
    return ;
  if(_sharedData->myPainter) 
    delete _sharedData->myPainter ;
  _sharedData->myPainter = new PointPaintStrategy ;

  setChanged() ;
  notifyObservers() ;
  _sharedData->myCursorShapeChanged = true;
  updateCursor();
}


void 
PaintAction::brushToMm( ) 
{
  _sharedData->myMmMode = true ;

  setChanged() ;
  notifyObservers() ;
  _sharedData->myCursorShapeChanged = true;
  updateCursor();
}

void 
PaintAction::brushToVoxel( ) 
{
  _sharedData->myMmMode = false ;

  setChanged() ;
  notifyObservers() ;
  _sharedData->myCursorShapeChanged = true;
  updateCursor();
}

void 
PaintAction::validateChange( int, int, int, int )
{
  //cerr << "PaintAction::validateChange : entering" << endl ;
  _sharedData->myPainter->reset() ;
  
  if( _sharedData->myIsChangeValidated || ( !_sharedData->myValidRegion )) 
    return ;
  
  RoiChangeProcessor::instance()->applyChange( _sharedData->myCurrentChanges ) ;

  bool mustBeUnregistered = false ;
  if( !_sharedData->myDeltaModifications->bucket()[0].empty() ){
    mustBeUnregistered = true ;
    _sharedData->myDeltaModifications->bucket()[0].clear() ;
  }

  list<AWindow3D*>::iterator iter( myLinkedWindows.begin() ), 
    last( myLinkedWindows.end() ) ;

  while ( iter != last ){
    if( mustBeUnregistered )
      (*iter)->unregisterObject( _sharedData->myDeltaModifications ) ;
    ++iter ;
  }

  _sharedData->myIsChangeValidated = true ;

  setChanged() ;
  notifyObservers() ;
  updateCursor();
  //cerr << "PaintAction::validateChange : exiting" << endl ;
}

void 
PaintAction::undo()
{
  RoiChangeProcessor::instance()->undo() ;
}

void
PaintAction::redo()
{
  RoiChangeProcessor::instance()->redo() ;
  
//   setChanged() ;
//   notifyObservers() ;
}

void
PaintAction::copyPreviousSliceWholeSession()
{
  copySlice( true, -1 ) ;
}

void
PaintAction::copyNextSliceWholeSession()
{
  copySlice( true, 1 ) ;
}

void
PaintAction::copyPreviousSliceCurrentRegion()
{
  copySlice( false, -1 ) ;
}

void
PaintAction::copyNextSliceCurrentRegion()
{
  copySlice( false, 1 ) ;
}

void
PaintAction::fill(int x, int y, int, int )
{
  AWindow3D * win = dynamic_cast<AWindow3D*>( view()->window() ) ;
  if( !win )
    {
      cerr << "warning: PaintAction operating on wrong view type\n";
      return;
    }

  Point3df normalVector( win->sliceQuaternion().
			 apply(Point3df(0., 0., 1.) ) ) ;
  
  if( normalVector[0] <= 0.99 && normalVector[1] <= 0.99 && normalVector[2] <= 0.99 )
    return ;
  
  AGraph * g = RoiChangeProcessor::instance()->getGraph( view()->window() ) ;
  if (!g) return ;
  
  _sharedData->myCurrentChanges = new list< pair< Point3d, ChangesItem> > ;

  AimsData<AObject*>& labels = g->volumeOfLabels( 0 ) ;
  if( labels.dimX() != ( g->MaxX2D() - g->MinX2D() + 1 ) || 
      labels.dimY() != ( g->MaxY2D() - g->MinY2D() + 1 ) ||
      labels.dimZ() != ( g->MaxZ2D() - g->MinZ2D() + 1 ) )
    {
      g->clearLabelsVolume() ;
      g->setLabelsVolumeDimension( static_cast<int>( g->MaxX2D() 
						     - g->MinX2D() ) + 1, 
				   static_cast<int>( g->MaxY2D() 
						     - g->MinY2D() ) + 1,
				   static_cast<int>( g->MaxZ2D() 
						     - g->MinZ2D() ) + 1 ) ;
    }
  
  AGraphObject * graphObject = RoiChangeProcessor::instance()->getGraphObject( view()->window()) ;
  graphObject->attributed()->setProperty("modified", true) ;

  myLinkedWindows.clear() ;
  set<AWindow*> group = g->WinList() ;
  set<AWindow*>::iterator gbegin = group.begin();
  set<AWindow*>::iterator gend = group.end();
  for( set<AWindow*>::iterator i = gbegin; i != gend; ++i ){
    AWindow3D * win3d = dynamic_cast<AWindow3D*>(*i) ;
    if ( win3d )
      myLinkedWindows.push_back( win3d ) ;
  }
  
  if ( !_sharedData->myValidRegion )
    return ;
  
  Referential* winRef = win->getReferential() ;
  Referential* buckRef = _sharedData->myCurrentModifiedRegion->getReferential() ;
  Point3df pos ;
  if( win->positionFromCursor( x, y, pos ) )
    {
      Point3df n( normalVector * normalVector.dot( pos - win->GetPosition() ) ) ;
      pos = pos - n ;
      AGraph	*g = RoiChangeProcessor::instance()->getGraph( view()->window() );
      
      Point3df voxelSize ( _sharedData->myCurrentModifiedRegion->VoxelSize() ) ;
      Transformation * transf = theAnatomist->getTransformation(winRef, buckRef) ;
      if ( transf )
	pos = Transformation::transform( pos, transf, voxelSize ) ;
      else {
	pos[0] /= voxelSize[0] - g->MinX2D() ; 
	pos[1] /= voxelSize[1] - g->MinY2D() ;
	pos[2] /= voxelSize[2] - g->MinZ2D() ;
      }
      
      Point3d neighbourhood[4] ;
      
      if( normalVector[0] > 0.99 ){
	neighbourhood[0] = Point3d(0, -1, 0 ) ; neighbourhood[1] = Point3d(0, 1, 0 ) ; 
	neighbourhood[2] = Point3d(0, 0, -1 ) ; neighbourhood[3] = Point3d(0, 0, 1 ) ; 
      }
      if( normalVector[1] > 0.99 ){
	neighbourhood[0] = Point3d(-1, 0, 0 ) ; neighbourhood[1] = Point3d(1, 0, 0 ) ; 
	neighbourhood[2] = Point3d(0, 0, -1 ) ; neighbourhood[3] = Point3d(0, 0, 1 ) ; 
      }
      if( normalVector[2] > 0.99 ){
	neighbourhood[0] = Point3d(-1, 0, 0 ) ; neighbourhood[1] = Point3d(1, 0, 0 ) ; 
	neighbourhood[2] = Point3d(0, -1, 0 ) ; neighbourhood[3] = Point3d(0, 1, 0 ) ; 
      }
      
      Point3d pt( (short) rint( pos[0] ), 
		  (short) rint( pos[1] ), 
		  (short) rint( pos[2] ) ) ;
      
      fillRegion2D( pt, neighbourhood, 
		    g->volumeOfLabels(), graphObject,
		    (*_sharedData->myCurrentChanges) ) ;
      _sharedData->myIsChangeValidated = false ;
      validateChange() ;
    }
}

void
PaintAction::fillRegion2D( const Point3d& seed, Point3d neighbourhood[],
			   AimsData<AObject*>& volumeOfLabels, AObject * final,
			   list< pair< Point3d, ChangesItem> > & changes )
{
  if( _sharedData->myPainter->in( &volumeOfLabels, 
				  Point3df( (float)seed[0], (float)seed[1], (float)seed[2]),
				  Point3df(0., 0., 0.) ) ){
    AObject *& value = volumeOfLabels(seed[0], seed[1], seed[2]) ;
    
    if ( ( !_sharedData->myReplaceMode && (value != 0) ) || (value == final) )
      return ;
    ChangesItem item ;
    item.before = value ;
    item.after = final ;
    changes.push_back( pair<Point3d, ChangesItem>(seed, item) ) ;
    
    value = final ;
  }

  queue<Point3d> trialPoints ;
  trialPoints.push(seed) ;
  
  Point3d p ;
  while( !trialPoints.empty( ) ){
    p = trialPoints.front() ;
    trialPoints.pop() ;
    
    for(int i = 0 ; i < 4 ; ++i ){
      Point3d neighbour = p + neighbourhood[i] ;
      if( _sharedData->myPainter->in( &volumeOfLabels,
				      Point3df( (float)neighbour[0], 
						(float)neighbour[1], (float)neighbour[2]),
				      Point3df(0., 0., 0.) ) ){
	AObject *& val = volumeOfLabels(neighbour[0], neighbour[1], neighbour[2]) ;
	if ( ( _sharedData->myReplaceMode || (val == 0) ) && (val != final) ){
	  trialPoints.push(neighbour) ;
	  ChangesItem item ;
	  item.before = val ;
	  item.after = final ;
	  changes.push_back( pair<Point3d, ChangesItem>(neighbour, item) ) ;
	  
	  val = final ;
	}
      }
    }
  }
}

void 
PaintActionSharedData::noMoreUndoable()
{
  RoiChangeProcessor::instance()->noMoreUndoable() ;

  setChanged() ;
  notifyObservers() ;
}

Action*
PaintAction::creator()
{
  return  new PaintAction( ) ;
}


PaintStrategy::PaintStrategy( ) : myPreviousPoint(Point3df(0,0,0)), 
  myPreviousPointExists(false)
{}

PaintStrategy::~PaintStrategy() {}

void
PaintStrategy::reset()
{
  myPreviousPointExists = false ;
}

list< Point3d >
PaintStrategy::drawFastLine( const Point3d& from, const Point3d& dep )
{
  Point3d p = from ;

  int sx = (dep[0] > 0 ? 1 : -1), 
    sy = ( dep[1] > 0 ? 1 : -1), 
    sz = ( dep[2] > 0 ? 1 : -1), 
    ax = abs(dep[0]), ay = abs(dep[1]), az = abs(dep[2]), 
    bx = 2*ax, by = 2*ay, bz = 2*az, 
    exy = ay - ax, exz = az - ax, ezy = ay - az,
    n = ax + ay + az ;
  
  list< Point3d > line ;
  
  while( n-- ){
    if( exy < 0 ){
      if( exz < 0 ){
	p[0] += sx ;
	exy += by ;
	exz += bz ;
      } else {
	p[2] += sz ;
	exz -= bx ;
	exz += by ;
      }
    } else {
      if( ezy < 0 ){
	p[2] += sz ;
	exz -= bx ;
	ezy += by ;
      } else {
	p[1] += sy ;
	exy -= bx ;
	ezy -= bz ;
      }
    }
    
    line.push_back( p ) ;
  }
  return line ;
}

list< Point3df >
PaintStrategy::drawLine( const Point3df& from, const Point3df& dep )
{
  
  list< Point3df > line ;
  
  Point3df p( from ) ;
  if( p[0] < 0. && fabs(p[0]) < 0.001 )
    p[0] = 0. ;
  else if( p[1] < 0. && fabs(p[1]) < 0.001 )
    p[1] = 0. ;
  else if( p[2] < 0. && fabs(p[2]) < 0.001 ){
    p[2] = 0. ;
  }
  line.push_back( p ) ;
  
  float sx = ( dep[0] >= 0 ? 1. : -1. ), sy = ( dep[1] >= 0 ? 1. : -1. ), sz = (dep[2] >= 0 ? 1. : -1. ) ;

  float dx = ( sx*dep[0]  >= 1 ? sx / dep[0] : 2 ),
    dy = ( sy*dep[1] >= 1 ? sy / dep[1] : 2 ), 
    dz = ( sz*dep[2] >= 1 ? sz / dep[2] : 2 ),
    tx = dx,
    ty = dy,
    tz = dz ;
  
  while ( tx < 1.0001 || ty < 1.0001 || tz < 1.0001 ){
    if( p[0] < 0. && fabs(p[0]) < 0.001 )
      p[0] = 0. ;
    else if( p[1] < 0. && fabs(p[1]) < 0.001 )
      p[1] = 0. ;
    else if( p[2] < 0. && fabs(p[2]) < 0.001 ){
      p[2] = 0. ;
    }
    
    if( tx <= ty && tx <= tz )
      {
	p[0] += sx ;
	line.push_back( p ) ;
	tx += dx ;
      } 
    else if ( ty <= tx && ty <= tz )
      {
	p[1] += sy;
	line.push_back( p ) ;
	ty += dy ;	  
      }
    else
      {
	p[2] += sz ;
	line.push_back( p ) ;
	tz += dz ;	  	
      }
  }
  return line ;
}


PointPaintStrategy::PointPaintStrategy() {}

PointPaintStrategy::~PointPaintStrategy() {}

PaintStrategy::PaintType
PointPaintStrategy::paintType()
{
  return PaintStrategy::POINT ;
}

void 
PointPaintStrategy::paint( AWindow3D * /*win*/,
			   Transformation * transf, const Point3df& point,
			   const AObject * originalLabel, AObject * finalLabel,
			   float /*brushSize*/, bool /*lineMode*/,
			   AimsData<AObject*> *volumeOfLabels,
			   const Point3df & vlOffset, 
			   BucketMap<Void>::Bucket & deltaModifications,
			   list< pair< Point3d, ChangesItem> > & changes,
			   const Point3df& voxelSize,
			   bool line,
			   bool replace, bool /*mm*/ )
{
  //cerr << "PointPaintStrategy::paint : entering" << endl ;
  
  Point3df p ;
  if ( transf )
    p = Transformation::transform( point, transf, voxelSize ) ;
  else
    {
      p = point ;
      p[0] /= voxelSize[0] ; 
      p[1] /= voxelSize[1] ;
      p[2] /= voxelSize[2] ;
    }
  
  Point3d pToInt( static_cast<int> ( p[0] +.5 ), 
		  static_cast<int> ( p[1] +.5 ), 
		  static_cast<int> ( p[2] +.5 ) ) ;
  Point3d pVL( static_cast<int> ( p[0] - vlOffset[0] +.5 ), 
	       static_cast<int> ( p[1] - vlOffset[1] +.5 ), 
	       static_cast<int> ( p[2] - vlOffset[2] +.5 ) );

  if( line && myPreviousPointExists )
    {
      list< Point3df > line = drawLine( myPreviousPoint, 
					p - myPreviousPoint ) ;
      list< Point3df >::iterator iter( line.begin() ), last( line.end() ) ;
      while( iter != last )
        {
          if ( in( volumeOfLabels, *iter, vlOffset ) )
            {
              Point3d pt( (short) rint( (*iter)[0] ),
                          (short) rint( (*iter)[1] ),
                          (short) rint( (*iter)[2] ) ) ;
              if( volumeOfLabels )
              {
                AObject *& value
                  = (*volumeOfLabels)
                  ( (unsigned) rint( (*iter)[0] - vlOffset[0] ),
                    (unsigned) rint( (*iter)[1] - vlOffset[1] ),
                    (unsigned) rint( (*iter)[2] - vlOffset[2] ) );
                if( ( value != originalLabel && !replace ) ||
                    value == finalLabel )
                {
                  ++iter ;
                  continue ;
                }

                ChangesItem item ;
                item.before = value ;
                item.after = finalLabel ;
                changes.push_back( pair<Point3d, ChangesItem>(pt, item) ) ;

                value = finalLabel ;
              }

              deltaModifications[pt] ;
            }

          ++iter ;
        }

    }
  else if ( in( volumeOfLabels, Point3df((float)pToInt[0], (float)pToInt[1], 
					 (float)pToInt[2] ), vlOffset ) )
    {
      if( volumeOfLabels )
      {
        AObject *& value = (*volumeOfLabels)( pVL );

        if( ( value != originalLabel &&  !replace )
            || value == finalLabel )
          return ;

        ChangesItem item ;
        item.before = value ;
        item.after = finalLabel ;
        changes.push_back( pair<Point3d, ChangesItem>(pToInt, item) ) ;
        value = finalLabel ;
      }

      deltaModifications[ pToInt ] ;
    }
  else
    cerr << "point " << pToInt << " not in vol of labels\n";

  myPreviousPointExists = true ;
  myPreviousPoint = p ;

  //cerr << "PointPaintStrategy::Paint : exiting" << endl ;
}

SquarePaintStrategy::SquarePaintStrategy() {}

SquarePaintStrategy::~SquarePaintStrategy() {}

PaintStrategy::PaintType
SquarePaintStrategy::paintType()
{
  return PaintStrategy::SQUARE ;
}

void 
SquarePaintStrategy::paint( AWindow3D */* win*/,
			   Transformation * /*transf*/, 
			    const Point3df& /*point*/,
			    const AObject * /*originalLabel*/, 
			    AObject * /*finalLabel*/, 
			    float /*brushSize*/, bool /*lineMode*/,
			    AimsData<AObject*>* /*volumeOfLabels*/,
			    const Point3df & /*vlOffset*/, 
			    BucketMap<Void>::Bucket & /*deltaModifications*/,
			    list< pair< Point3d, ChangesItem> > & /*changes*/,
			    const Point3df& /*voxelSize*/,
			    bool /*line*/,
			    bool /*replace*/,
			    bool /*mm*/ ) 
{
}

DiskPaintStrategy::DiskPaintStrategy() 
{
  /* Denis
     Aaaaaahh ? Tu prends des buckets Anatomist pour stocker tes brosses ????
     Si c'est pas tuer des mouches a coups de canon, ca..... !
   */
}

DiskPaintStrategy::~DiskPaintStrategy() 
{
}

PaintStrategy::PaintType
DiskPaintStrategy::paintType()
{
  return PaintStrategy::DISK ;
}

void 
DiskPaintStrategy::brushPainter( const Point3df& diskCenter,
				 const Point3df& n,
				 const AObject * originalLabel, 
				 AObject * finalLabel,
				 float brushSize, 
				 AimsData<AObject*> *volumeOfLabels,
				 const Point3df & voxelSize, 
				 const Point3df & vlOffset, 
				 BucketMap<Void>::Bucket & deltaModifications,
				 list< pair< Point3d, ChangesItem> > & changes,
				 bool replace,
				 bool mm )
{
  Point3df point ;
  Point3d realPoint ;
  int i, j, k ;
  float brush = brushSize ;
  float brush2 = brush * brush ;
  
  if ( n[2] >= n[1] && n[2] >= n[0] )
    {
      i = 0, j = 1, k = 2 ;
    }
  else if ( n[1] > n[0] && n[1] > n[2] )
    {
      i = 0, j = 2, k = 1 ;
    } 
  else
    i = 1, j = 2, k = 0 ;
  
  Point3df distanceToCenter ;
  float minVS = voxelSize[0] ;
  if( voxelSize[1] < minVS )
    minVS = voxelSize[1] ;
  if( voxelSize[2] < minVS )
    minVS = voxelSize[2] ;
  float invnk = 1 / n[k] ;
  float maxOnI, maxOnJ, minOnI, minOnJ ;
  
  if( mm ){
    maxOnI = (float)(int)( diskCenter[i] + brush/minVS + 1) ;
    maxOnJ = (float)(int)( diskCenter[j] + brush/minVS + 1 ) ;
    minOnI = (float)(int)( diskCenter[i] - brush/minVS - 1 ) ; 
    minOnJ = (float)(int)( diskCenter[j] - brush/minVS - 1 ) ;
  } else {
    maxOnI = (float)(int)( diskCenter[i] ) + brush + 1 ;
    maxOnJ = (float)(int)( diskCenter[j] ) + brush + 1 ;
    minOnI = (float)(int)( diskCenter[i] ) - brush - 1 ; 
    minOnJ = (float)(int)( diskCenter[j] ) - brush - 1 ;
  }
  for ( point[i] = minOnI ; point[i] <= maxOnI ; point[i] += 1. )
    for ( point[j] = minOnJ ; point[j] <= maxOnJ ; point[j] += 1. )
      {
        point[k] =  ( diskCenter.dot(n) - n[i] * point[i] - n[j] * point[j] )
          * invnk ;

        distanceToCenter = point - diskCenter ;



        if( mm ){
          distanceToCenter[0] *= voxelSize[0] ;
          distanceToCenter[1] *= voxelSize[1] ;
          distanceToCenter[2] *= voxelSize[2] ;
        }

        if ( /*distanceToCenter.dot(distanceToCenter)*/
              distanceToCenter[0]*distanceToCenter[0]
              + distanceToCenter[1]*distanceToCenter[1]
              + distanceToCenter[2]*distanceToCenter[2] < brush2 &&
              in( volumeOfLabels, point, vlOffset ) )
          {
            realPoint[0] = (int) ( point[0] + 0.5 ) ;
            realPoint[1] = (int) ( point[1] + 0.5 ) ;
            realPoint[2] = (int) ( point[2] + 0.5 ) ;
            Point3d	VL( (int) rint( vlOffset[0] ),
                            (int) rint( vlOffset[1] ),
                            (int) rint( vlOffset[2] ));

            if( volumeOfLabels )
            {
              AObject *& value = (*volumeOfLabels)( realPoint - VL ) ;

              if( ( value != originalLabel && !replace ) ||
                  value == finalLabel )
                continue ;

              ChangesItem item ;
              item.before = value ;
              item.after = finalLabel ;
              changes.push_back( pair<Point3d, ChangesItem>(realPoint, item) ) ;
              value = finalLabel ;
            }

            deltaModifications[ realPoint ] ;

          }
      }
}


void 
DiskPaintStrategy::paint( AWindow3D * win,
			  Transformation * transf, const Point3df& point,
			  const AObject * originalLabel, AObject * finalLabel,
			  float brushSize, bool /*lineMode*/,
			  AimsData<AObject*> *volumeOfLabels,
			  const Point3df & vlOffset, 
			  BucketMap<Void>::Bucket & deltaModifications,
			  list< pair< Point3d, ChangesItem> > & changes,
			  const Point3df& voxelSize,
			  bool lineMode,
			  bool replace,
			  bool mm )
{
  //cerr << "BallPaintStrategy::paint : entering" << endl ;
  
  Point3df p ;
  if ( transf )
    p = Transformation::transform( point, transf, voxelSize ) ;
  else {
    p = point ;
    p[0] /= voxelSize[0] ; 
    p[1] /= voxelSize[1] ;
    p[2] /= voxelSize[2] ;
  }
    
  Point3df normal(win->sliceQuaternion().apply(Point3df(0., 0., 1.) ) ) ;
  
  if( lineMode && myPreviousPointExists )
    {
      list< Point3df > line = drawLine( myPreviousPoint, 
					p - myPreviousPoint ) ;
      list< Point3df >::iterator iter( line.begin() ), last( line.end() ) ;
    
      while( iter != last ) {
	if( normal[0] > 0.999 || normal[0] < -0.999 || 
	    normal[1] > 0.999 || normal[1] < -0.999 || 
	    normal[2] > 0.999 || normal[2] < -0.999 )
	  brushPainter( Point3df( (float)(int)((*iter)[0] + 0.5 ),
				  (float)(int)((*iter)[1] + 0.5 ),
				  (float)(int)((*iter)[2] + 0.5 ) ),
			normal,
			originalLabel, finalLabel, brushSize, 
			volumeOfLabels, voxelSize, vlOffset, deltaModifications, 
			changes, replace, mm ) ;
	else
	  brushPainter( *iter, normal,
			originalLabel, finalLabel, brushSize, 
			volumeOfLabels, voxelSize, vlOffset, deltaModifications, 
			changes, replace, mm ) ;
	++iter ;
      }
    
    } else
      if( normal[0] > 0.999 || normal[0] < -0.999 || 
	  normal[1] > 0.999 || normal[1] < -0.999 || 
	  normal[2] > 0.999 || normal[2] < -0.999 )
	brushPainter( Point3df( (float)(int)(p[0] + 0.5 ),
				(float)(int)(p[1] + 0.5 ),
				(float)(int)(p[2] + 0.5 ) ),
		      normal,
		      originalLabel, finalLabel, brushSize, 
		      volumeOfLabels, voxelSize, vlOffset, deltaModifications, 
		      changes, replace, mm ) ;
  
    else
      brushPainter( p, normal,
		    originalLabel, finalLabel, brushSize, 
		    volumeOfLabels, voxelSize, vlOffset, deltaModifications, 
		    changes, replace, mm ) ;
  
  myPreviousPointExists = true ;
  myPreviousPoint = p ;
  
  //cerr << "BallPaintStrategy::Paint : exiting" << endl ;
}



BallPaintStrategy::BallPaintStrategy() 
{
}

BallPaintStrategy::~BallPaintStrategy() {
  //delete [] myBrushes ;
}

const vector<Point3d>&
BallPaintStrategy::brush( int size )
{
  static vector< vector<Point3d> > myBrush(51) ;
  static vector<bool> firstTime( 51, true) ;

  if ( firstTime[size] )
    {
      firstTime[size] = false ;
      myBrush[size].reserve( (2*size-1)*(2*size-1)*(2*size-1) ) ;
      for( int i = -size+1 ; i <= size-1 ; ++i )
	for( int j = -size+1 ; j <= size-1 ; ++j )
	  for( int k = -size+1 ; k <= size-1 ; ++k )
	    if( ::sqrt( i*i + j*j + k*k ) - 0.001 <= float(size-1) ) 
	      myBrush[size].push_back( Point3d(i, j, k) ) ;

//       float sq ;
      
//       for( int i = -49 ; i <= 49 ; i++ )
// 	for( int j = -49 ; j <= 49 ; j++ ) 
// 	  for( int k = -49 ; k <= 49 ; k++ )
// 	    {
// 	      sq = ::sqrt( i*i + j*j + k*k ) ;
// 	      for( int l = static_cast<int>( sq + .0001 ) ; l <= 49 ; l++ )
// 		{
// 		  myBrush[l+1].bucket()[0][ Point3d(i, j, k) ] ;
// 		}
// 	    }
    }  
  return myBrush[size] ;
}

PaintStrategy::PaintType
BallPaintStrategy::paintType()
{
  return PaintStrategy::BALL ;
}

void 
BallPaintStrategy::brushPainter( const Point3d& pToInt, 
				 const AObject * originalLabel, 
				 AObject * finalLabel,
				 float brushSize, 
				 AimsData<AObject*> *volumeOfLabels,
				 const Point3df & voxelSize, 
				 const Point3df & vlOffset, 
				 BucketMap<Void>::Bucket & deltaModifications,
				 list< pair< Point3d, ChangesItem> > & changes,
				 bool replace, bool mm )
{

  if( mm ){
    for( int i = -int(brushSize/voxelSize[0]+1) ;
         i <= int(brushSize/voxelSize[0]+1) ; ++i )
      for( int j = -int(brushSize/voxelSize[1]+1) ;
           j <= int(brushSize/voxelSize[1]+1) ; ++j )
        for( int k = -int(brushSize/voxelSize[2]+1) ;
             k <= int(brushSize/voxelSize[2]+1) ; ++k )
          if( i*i*voxelSize[0]*voxelSize[0] + j*j*voxelSize[1]*voxelSize[1]
              + k*k*voxelSize[2]*voxelSize[2] < brushSize * brushSize ){
            Point3d realPosition ( pToInt + Point3d(i, j, k) ) ;
            if( in( volumeOfLabels, Point3df(realPosition[0],
                                              realPosition[1],
                                              realPosition[2] ), vlOffset ) )
              {
                Point3d	VL( (int) rint( vlOffset[0] ),
                            (int) rint( vlOffset[1] ),
                            (int) rint( vlOffset[2] ) );
                if( volumeOfLabels )
                {
                  AObject *& value = (*volumeOfLabels)( realPosition - VL ) ;

                  if( ( value != originalLabel  && !replace ) ||
                      value == finalLabel )
                    continue ;

                  ChangesItem item ;
                  item.before = value ;
                  item.after = finalLabel ;
                  changes.push_back( pair<Point3d, ChangesItem>( realPosition,
                                                                  item) ) ;
                  value = finalLabel ;
                }

                deltaModifications[ realPosition ] ;

              }
          }
  }
  else
  {
    vector<Point3d>::const_iterator 
      iter( brush(int(rint(brushSize))).begin() ), 
      last( brush(int(rint(brushSize))).end() ) ;

    while( iter != last )
    {
      Point3d realPosition ( pToInt + *iter ) ;

      if( in( volumeOfLabels, Point3df(realPosition[0],
                                        realPosition[1],
                                        realPosition[2] ), vlOffset ) )
      {
        Point3d	VL( (int) rint( vlOffset[0] ),
                        (int) rint( vlOffset[1] ),
                        (int) rint( vlOffset[2] ) );
        if( volumeOfLabels )
        {
          AObject *& value = (*volumeOfLabels)( realPosition - VL ) ;

          if( ( value != originalLabel  && !replace ) ||
              value == finalLabel )
          {
            ++iter ;
            continue ;
          }

          ChangesItem item ;
          item.before = value ;
          item.after = finalLabel ;
          changes.push_back( pair<Point3d, ChangesItem>(realPosition, item) ) ;
          value = finalLabel ;
        }

        deltaModifications[ realPosition ] ;
      }
      ++iter ;
    }
  }
}


void 
BallPaintStrategy::paint( AWindow3D * /*win*/,
			  Transformation * transf, const Point3df& point,
			  const AObject * originalLabel, AObject * finalLabel,
			  float brushSize, bool /*lineMode*/,
			  AimsData<AObject*> *volumeOfLabels,
			  const Point3df & vlOffset, 
			  BucketMap<Void>::Bucket & deltaModifications,
			  list< pair< Point3d, ChangesItem> > & changes,
			  const Point3df& voxelSize,
			  bool line,
			  bool replace, bool mm )
{
  //cerr << "BallPaintStrategy::paint : entering" << endl ;
  
  Point3df p ;
  if ( transf )
    p = Transformation::transform( point, transf, voxelSize ) ;
  else {
    p = point ;
    p[0] /= voxelSize[0] ; 
    p[1] /= voxelSize[1] ;
    p[2] /= voxelSize[2] ;
  }
  
  Point3d pToInt( static_cast<int> ( p[0] +.5 ), 
		  static_cast<int> ( p[1] +.5 ), 
		  static_cast<int> ( p[2] +.5 ) ) ;
  
  if( line && myPreviousPointExists )
    {
      list< Point3df > line = drawLine( myPreviousPoint, 
					p - myPreviousPoint ) ;
      list< Point3df >::iterator iter( line.begin() ), last( line.end() ) ;

      while( iter != last )
	{
	  brushPainter( Point3d( (short) rint( (*iter)[0] ), 
				 (short) rint( (*iter)[1] ), 
				 (short) rint( (*iter)[2] ) ), originalLabel, 
			finalLabel, brushSize, volumeOfLabels, voxelSize, vlOffset, 
			deltaModifications, 
			changes, replace, mm ) ;
	  ++iter ;
	}
    
    } else
    brushPainter( pToInt, originalLabel, finalLabel, brushSize, 
		  volumeOfLabels, voxelSize, vlOffset, deltaModifications, 
		  changes, replace, mm ) ;
  
  myPreviousPointExists = true ;
  myPreviousPoint = p ;
}

QWidget *
PaintAction::actionView( QWidget * parent ) 
{ 
  PaintActionView * obs = new PaintActionView( this, parent ) ;
//   myObserver = new PaintActionView( this, parent ) ;

  return obs ;
}
  
bool 
PaintAction::viewableAction( ) const 
{ 
  return true ; 
}


PaintActionSharedData::PaintActionSharedData() : Observable(),
  myBrushSize(1.), myLineMode(true), myReplaceMode(false),
  myFollowingLinkedCursor(false), myMmMode(false), myIsChangeValidated(true), 
  myPainting(true), myCurrentModifiedRegion(0), myCursor(0),
  myCursorPos( 0, 0, 0 ), myCursorShapeChanged( true ), myCursorRef( 0 )
{
  myPainter = new PointPaintStrategy() ;
  myDeltaModifications = new Bucket() ;
  myCurrentChanges = new list< pair< Point3d, ChangesItem> >() ;
}

PaintActionSharedData::~PaintActionSharedData()
{
  if( myPainter )
    delete myPainter ;
  if( myDeltaModifications ) 
    delete myDeltaModifications ;
  if( myCursor )
  {
    delete myCursorRef;
    delete myCursor;
  }

  notifyUnregisterObservers() ;
}



void 
PaintAction::changeCursor( bool cross )
{
  AWindow3D * win = dynamic_cast<AWindow3D*>( view()->window() ) ;
  if (win)
  {
    if ( cross ) 
      win->setCursor(Qt::crossCursor) ;
    else
      win->setCursor(Qt::arrowCursor) ;
  }
}


void 
PaintAction::copySlice( bool wholeSession, int sliceIncrement )
{
  if ( ! ( _sharedData->myCurrentModifiedRegion = 
	   RoiChangeProcessor::instance()->getCurrentRegion(view()->window() ) ) )
    {
      _sharedData->myValidRegion = false ;
      return ;
    }
  _sharedData->myValidRegion = true ;

  _sharedData->myCurrentChanges = new list< pair< Point3d, ChangesItem> > ;
  
  Point3df voxelSize = _sharedData->myCurrentModifiedRegion->VoxelSize() ;
  
  RoiChangeProcessor::instance()->setRedoable( false ) ;
  
  setChanged() ;
  
  AGraph * g = RoiChangeProcessor::instance()->getGraph( view()->window() ) ;
  if (!g) return ;
  
  AimsData<AObject*>& labels = g->volumeOfLabels( 0 ) ;
  if( labels.dimX() != ( g->MaxX2D() - g->MinX2D() + 1 ) || 
      labels.dimY() != ( g->MaxY2D() - g->MinY2D() + 1 ) ||
      labels.dimZ() != ( g->MaxZ2D() - g->MinZ2D() + 1 ) )
    {
      g->clearLabelsVolume() ;
      g->setLabelsVolumeDimension( static_cast<int>( g->MaxX2D() 
						     - g->MinX2D() ) + 1, 
				   static_cast<int>( g->MaxY2D() 
						     - g->MinY2D() ) + 1,
				   static_cast<int>( g->MaxZ2D() 
						     - g->MinZ2D() ) + 1 ) ;
    }
  
  AimsData<AObject*>& volumeOfLabels = g->volumeOfLabels( 0 ) ;
  AGraphObject * grao = RoiChangeProcessor::instance()->getGraphObject( view()->window() ) ;
  grao->attributed()->setProperty("modified", true) ;
  
  myLinkedWindows.clear() ;
  set<AWindow*> group = g->WinList() ;
  set<AWindow*>::iterator gbegin = group.begin();
  set<AWindow*>::iterator gend = group.end();
  for( set<AWindow*>::iterator i = gbegin; i != gend; ++i ){
    AWindow3D * win3d = dynamic_cast<AWindow3D*>(*i) ;
    if ( win3d ){
      myLinkedWindows.push_back( win3d ) ;
    }
  }
  
  AWindow3D * win = dynamic_cast<AWindow3D*>( view()->window() ) ;
  if( !win )
    {
      cerr << "warning: PaintAction operating on wrong view type\n";
      return;
    }
  
  if ( !_sharedData->myValidRegion )
    return ;
  
  Referential* winRef = win->getReferential() ;
  Referential* buckRef = _sharedData->myCurrentModifiedRegion->getReferential() ;

  Point3df normalVector( win->sliceQuaternion().
			 apply(Point3df(0., 0., 1.) ) ) ;
  
  Point3df cursorPosition( win->GetPosition() ) ;
  
  Transformation * transf = theAnatomist->getTransformation(winRef, buckRef) ;
  
  Point3df cp ;
  if ( transf )
    cp = Transformation::transform( cursorPosition, transf, voxelSize ) ;
  else {
    cp = cursorPosition ;
    cp[0] /= voxelSize[0] ; 
    cp[1] /= voxelSize[1] ;
    cp[2] /= voxelSize[2] ;
  }
  
  Point3df vlOffset( g->MinX2D(), g->MinY2D(), g->MinZ2D() ) ;
  
  Point3d pVL( static_cast<int> ( cp[0] - vlOffset[0] +.5 ), 
	       static_cast<int> ( cp[1] - vlOffset[1] +.5 ), 
	       static_cast<int> ( cp[2] - vlOffset[2] +.5 ) );

  if( normalVector[0] > 0.99 || normalVector[1] > 0.99 || normalVector[2] > 0.99 )
    {
      Point3d redirect ;
      if( normalVector[0] > 0.99 )
	{
	  redirect[0] = 1 ;
	  redirect[1] = 2 ;
	  redirect[2] = 0 ;
	}
      else if( normalVector[1] > 0.99 )
	{
	  redirect[0] = 0 ;
	  redirect[1] = 2 ;
	  redirect[2] = 1 ;
	}
      else
	{
	  redirect[0] = 0 ;
	  redirect[1] = 1 ;
	  redirect[2] = 2 ;
	}
      
      Point3d dims(volumeOfLabels.dimX(), volumeOfLabels.dimY(),  volumeOfLabels.dimZ() ) ;
      Point3d p ;
      p[ redirect[2] ] = pVL[redirect[2]]  ;
      Point3d normal(0, 0, 0) ;
      normal[redirect[2]] = 1 ;
      Point3d neighbor ;
      int neighbourSlice = p[ redirect[2] ] + normal[redirect[2]] * sliceIncrement ;
      if( neighbourSlice < 0 && neighbourSlice > dims[redirect[2]] - 1 )
	return ;
      
      for( p[redirect[0]] = 0 ; p[redirect[0]] < dims[redirect[0]] ; ++p[redirect[0]] )
	for( p[redirect[1]] = 0 ; p[redirect[1]] < dims[redirect[1]] ; ++p[redirect[1]] ){
	  neighbor = p + (sliceIncrement > 0 ? normal : -normal)  ;
	  // Si les conditions sont respect�es, ajouter ce voxel dans le bucket courant.
	  if( (wholeSession && _sharedData->myReplaceMode ) || 
	      (wholeSession && !(_sharedData->myReplaceMode) && (volumeOfLabels(p)==0) ) ||
	      ((!wholeSession) && _sharedData->myReplaceMode && (volumeOfLabels(neighbor)==grao) ) || 
	      ((!wholeSession) && (!(_sharedData->myReplaceMode)) && (volumeOfLabels(neighbor)==grao) &&
	       (volumeOfLabels(p)==0) ) )
	    {
	      // Delta
	      //_sharedData->myDeltaModifications->bucket()[0][p] ;
	      
	      //Changements pour le processeur
	      ChangesItem item ;
	      item.before = volumeOfLabels(p) ;
	      item.after = volumeOfLabels(neighbor) ;
	      _sharedData->myCurrentChanges->push_back( pair<Point3d, ChangesItem>( p, 
										    item ) ) ;
	      
	      //Mise a jour du volume de labels
	      volumeOfLabels(p) = volumeOfLabels(neighbor) ;
	    }
	} 
    } 
  else 
    {
      cerr << "Not implemented yet" << endl ;
    }

  
  _sharedData->myIsChangeValidated = false ;
  
  RoiChangeProcessor::instance()->applyChange( _sharedData->myCurrentChanges ) ;

  
  list<AWindow3D*>::iterator iter( myLinkedWindows.begin() ), 
    last( myLinkedWindows.end() ) ;
  
  while ( iter != last ){
    (*iter)->refreshTemp() ;
    ++iter ; 
  }
  
  
  notifyObservers() ;
  
}


void PaintAction::updateCursor()
{
  // cout << "PaintAction::updateCursor\n" << flush;
  AWindow3D *win = dynamic_cast<AWindow3D *>( view()->window() );
  if( !win )
    return;
  AObject *region = RoiChangeProcessor::instance()->getCurrentRegion(
  view()->window() );
  if( !region )
    return;

  if( _sharedData->myCursor && _sharedData->myCursorShapeChanged )
  {
    delete _sharedData->myCursorRef;
    delete _sharedData->myCursor;
    _sharedData->myCursor = 0;
  }

  if( !_sharedData->myCursor )
  {
    _sharedData->myCursor = new Bucket;
    Material & mat = _sharedData->myCursor->GetMaterial();
    // mat.setRenderProperty( Material::RenderMode, Material::Wireframe );
    mat.setRenderProperty( Material::Ghost, 1 );
    // mat.setLineWidth( 2. );
    mat.SetDiffuse( 1., 0.5, 0.2, 0.5 );
    _sharedData->myCursor->SetMaterial( mat );
    _sharedData->myCursor->setAllow2DRendering( false );
    _sharedData->myCursorRef = new Referential;
    _sharedData->myCursorRef->header().setProperty( "hidden", true );
    _sharedData->myCursor->setReferential( _sharedData->myCursorRef );
    _sharedData->myCursorShapeChanged = true;
  }

  Referential* buckRef = region->getReferential();
  Referential* ref = _sharedData->myCursorRef;
  if( !ref || !theAnatomist->hasReferential( ref ) )
  {
    ref = new Referential;
    ref->header().setProperty( "hidden", true );
    _sharedData->myCursorRef = ref;
  }
  if( ref != _sharedData->myCursor->getReferential() )
    _sharedData->myCursor->setReferential( ref );
  Referential* winRef = win->getReferential() ;
  AGraph    *g = RoiChangeProcessor::instance()->getGraph( win );
  if( _sharedData->myCursorShapeChanged )
  {
    _sharedData->myCursorShapeChanged = false;
    _sharedData->myCursor->setVoxelSize( region->VoxelSize() );
    _sharedData->myCursor->bucket().clear();

    list< pair< Point3d, ChangesItem> > changes;

    _sharedData->myPainter->reset();
    _sharedData->myPainter->paint
      ( win, theAnatomist->getTransformation(winRef, buckRef),
        Point3df( 0, 0, 0 ), //_sharedData->myCursorPos,
        0, 0, _sharedData->myBrushSize, false, 0,
        Point3df( g->MinX2D(), g->MinY2D(), g->MinZ2D() ),
        (_sharedData->myCursor->bucket())[0],
        changes, region->VoxelSize(),
        false, true, _sharedData->myMmMode );
    _sharedData->myCursor->setBucketChanged();
    _sharedData->myCursor->setGeomExtrema();
    _sharedData->myPainter->reset();
  }

  Transformation *tr = theAnatomist->getTransformation( buckRef, ref );
  if( !tr )
  {
    tr = new Transformation( buckRef, ref /*, true ? */ );
  }
  Point3df vs = region->VoxelSize();
  Point3df pos( _sharedData->myCursorPos );
  Transformation *tr2 = theAnatomist->getTransformation( winRef, buckRef );
  if( tr2 )
    pos = tr2->transform( pos );
  pos[0] = round( pos[0] / vs[0] ) * vs[0];
  pos[1] = round( pos[1] / vs[1] ) * vs[1];
  pos[2] = round( pos[2] / vs[2] ) * vs[2];
  pos *= -1;
  tr->setTranslation( &pos[0] );
  tr->unregisterTrans();
  tr->registerTrans();

  //win->unregisterObject( _sharedData->myCursor );
  if( !win->hasObject( _sharedData->myCursor ) )
  {
    win->registerObject( _sharedData->myCursor, true );
    win->renderBefore( _sharedData->myCursor, renderBeforeObject( win, g ) );
  }
  win->refreshTemp();
}


void PaintAction::moveCursor( int x, int y, int, int )
{
  AWindow3D *win = dynamic_cast<AWindow3D *>( view()->window() );
  if( !win )
    return;
  Point3df pos;
  if( !win || !win->positionFromCursor( x, y, pos ) )
    return;
  _sharedData->myCursorPos = pos;

  updateCursor();
}


void PaintAction::hideCursor()
{
  if( _sharedData->myCursor )
  {
    view()->window()->unregisterObject( _sharedData->myCursor );
    ((AWindow3D *) view()->window())->refreshTemp();
    _sharedData->myCursorShapeChanged = true;
  }
}

