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


#include <cstdlib>
#include <cartobase/stream/fileutil.h>
#include <anatomist/action/roimanagementaction.h>
#include <anatomist/action/paintaction.h>
#include <anatomist/action/roichangeprocessor.h>
#include <anatomist/commands/cAddNode.h>
#include <anatomist/commands/cCreateGraph.h>
#include <anatomist/commands/cSaveObject.h>

#include <anatomist/application/Anatomist.h>
#include <anatomist/hierarchy/hierarchy.h>
#include <anatomist/graph/Graph.h>
#include <anatomist/graph/GraphObject.h>
#include <anatomist/selection/selectFactory.h>
#include <anatomist/window/Window.h>
#include <anatomist/control/wControl.h>
#include <anatomist/control/graphParams.h>
#include <anatomist/commands/cSelect.h>
#include <anatomist/commands/cCreateWindow.h>
#include <anatomist/commands/cLoadObject.h>
#include <anatomist/commands/cDeleteObject.h>
#include <anatomist/commands/cAddObject.h>
#include <anatomist/commands/cSetControl.h>
#include <anatomist/processor/Processor.h>
#include <anatomist/misc/error.h>
#include <anatomist/object/actions.h>
#include <anatomist/bucket/Bucket.h>
#include <anatomist/volume/Volume.h>
#include <anatomist/controler/view.h>
#include <anatomist/controler/icondictionary.h>
#include <anatomist/commands/cLinkWindows.h>
#include <cartobase/object/attributed.h>
#include <cartobase/type/string_conversion.h>
#include <graph/tree/tree.h>
#include <aims/utility/converter_bucket.h>
#include <aims/def/path.h>
#include <cartobase/object/object.h>
#include <anatomist/application/settings.h>
#include <graph/tree/tfactory.h>

#include <QListWidget>
#include <qcombobox.h>
#include <qcolordialog.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qslider.h>
#include <qlineedit.h>
#include <qdialog.h>
#include <qmenubar.h>
#include <qstringlist.h>
#include <QGroupBox>
#include <anatomist/application/fileDialog.h>

using namespace anatomist;
using namespace carto;
using namespace std;


RegionsFusionWindow::RegionsFusionWindow( QWidget * parent,
					  const QStringList& regions ) :
  QDialog( parent, Qt::WindowTitleHint ), myNewRegionName("")
{
  setModal( true );
  setWindowTitle( tr( "Regions Fusion" ) );

  QVBoxLayout *l = new QVBoxLayout( this ) ;

  selectRegions = new QListWidget( this ) ;
  l->addWidget( selectRegions );
  selectRegions->setMinimumHeight( 100 );
  selectRegions->setSizePolicy( QSizePolicy( QSizePolicy::Expanding,
                                             QSizePolicy::Expanding ) );
  selectRegions->addItems( regions ) ;
  selectRegions->setSelectionMode( QListWidget::ExtendedSelection ) ;

  selectRegionName = new QComboBox( this );
  l->addWidget( selectRegionName );
  selectRegionName->setCurrentIndex(0);

  QWidget *buttons = new QWidget( this );
  l->addWidget( buttons );
  QHBoxLayout *blay = new QHBoxLayout( buttons );

  QPushButton *okButton = new QPushButton( tr( "Ok" ), buttons );
  blay->addWidget( okButton );
  okButton->setDefault( true );
  okButton->setSizePolicy( QSizePolicy( QSizePolicy::Fixed,
                                        QSizePolicy::Fixed ) );
  QPushButton *cancelButton = new QPushButton( tr( "Cancel" ), buttons );
  blay->addWidget( cancelButton );
  cancelButton->setSizePolicy( QSizePolicy( QSizePolicy::Fixed,
                                            QSizePolicy::Fixed ) );

  QObject::connect( okButton , SIGNAL( clicked() ), this, SLOT( accept () ) );
  QObject::connect( cancelButton , SIGNAL( clicked() ),
                    this, SLOT( reject() ) );
  QObject::connect( selectRegions , SIGNAL( itemSelectionChanged() ),
                    this, SLOT( selectedRegionsChanged() ) );
}

// void
// NewUDRegion::redSliderChange( int value )
// {
//   _redLabel->setText( QString::number(value) ) ;
// }

// void
// NewUDRegion::greenSliderChange( int value )
// {
//   _greenLabel->setText( QString::number(value) ) ;
// }

// void
// NewUDRegion::blueSliderChange( int value )
// {
//   _blueLabel->setText( QString::number(value) ) ;
// }


set<string>
RegionsFusionWindow::regionsToBeFusioned()
{
  set<string> result ;
  QStringList::ConstIterator iter( mySelectedRegions.begin() ),
    last( mySelectedRegions.end() ) ;


  while( iter != last )
    {
      result.insert( iter->toStdString() );
      ++iter ;
    }
  return result ;
}

string
RegionsFusionWindow::newRegionName()
{
  return selectRegionName->currentText().toStdString();
}

void
RegionsFusionWindow::selectedRegionsChanged()
{
  mySelectedRegions.clear();
  for( unsigned int i = 0 ; i < selectRegions->count() ; ++i )
  {
    if( selectRegions->item( i )->isSelected() )
      mySelectedRegions.append( selectRegions->item(i)->text() );
  }
  selectRegionName->clear();
  selectRegionName->addItems( mySelectedRegions );
  selectRegionName->setCurrentIndex( 0 );
}


namespace anatomist
{
  struct RoiManagementActionView_Private
  {
    enum FrameWork
    {
      NEURO = 301,
      LATERALNEURO = 302,
      SULCI = 303,
      RAT_WB,
      USERDEFINED,
      FREE
    };


    RoiManagementAction * myRoiManagementAction ;

    QMenuBar 	* myMainMenu ;
    QMenu 	* mySessionMenu ;
    QMenu 	* myRegionMenu ;
    QMenu 	* myFrameWorkMenu ;
    QMenu 	* myUserDefinedFrameWorkMenu; // 305
    QMenu 	* myWindowMenu ;
    QAction *mySaveGraphAction;
    QAction *myNeuroAction; // 301
    QAction *myLateralNeuroAction; // 302
    QAction *mySulciAction; // 303
    QAction *myRatWbAction; // 304
    QAction *myNewRegionAction; // 314
    QAction *myModifyRegionNameAction; // 315
    QAction *myModifyRegionColorAction; // 316
    QAction *myDeleteRegionAction; // 317
    QAction *myFreeFrameworkAction; // 306

    QListWidget	* mySelectGraph ;

    QListWidget	* mySelectImage ;

    QListWidget	* mySelectRegion ;

    QPushButton * myAxial ;
    QPushButton * myCoronal ;
    QPushButton * mySagittal ;
    //     QPushButton * myOblique ;
    QPushButton * my3D ;
    // QPushButton	* myRefresh ;

    QSlider * myRegionTransparency ;
    QLabel * myRegionTransparencyLabel ;

    QSlider * myGraphTransparency ;
    QLabel * myGraphTransparencyLabel;
  };
}

using namespace anatomist ;
using namespace aims;

namespace
{

  set<RoiManagementActionView*> & roiManagementViews()
  {
    static set<RoiManagementActionView*>	rv;
    return rv;
  }

}

RoiManagementActionView::RoiManagementActionView( RoiManagementAction * action,
                                                  QWidget * parent )
  : QWidget(parent), Observer(), myUpdatingFlag(false),
    mySelectingImage(false),
    mySelectingGraph(false), mySelectingRegion(false),
    myChangingGraphTransparency(false), myChangingRegionTransparency(false),
    myGettingImageNames(false), myGettingGraphNames(false), myGettingRegionNames(false),
    myGettingHierarchyNames(false), myGettingHierarchyRegionNames("")
{
#ifdef ANA_DEBUG
  cout << "RoiManagementActionView::RoiManagementActionView" << endl ;
#endif

  _private = new RoiManagementActionView_Private ;
  _private->myRoiManagementAction = action ;
  action->addObserver(this) ;

  QVBoxLayout *lay1 = new QVBoxLayout( this );

  _private->myMainMenu = new QMenuBar(this) ;
  _private->mySessionMenu = _private->myMainMenu->addMenu( tr( "Session" ) );
  lay1->addWidget( _private->myMainMenu );

  _private->mySessionMenu->addAction(
    tr("New"), this, SLOT( newGraph() ), Qt::CTRL + Qt::ALT + Qt::Key_N );

  _private->mySessionMenu->addAction(
    tr("Open"), this, SLOT( loadGraph() ), Qt::CTRL + Qt::Key_O );

//   _private->mySessionMenu->addAction( tr("Reload"), this,
//                                     SLOT( reloadGraph() ) ) ;

  _private->mySessionMenu->addAction(
    tr("Close"), this, SLOT( deleteGraph() ) );

  _private->mySessionMenu->addSeparator() ;
  _private->mySaveGraphAction = _private->mySessionMenu->addAction(
    tr("Save"), this, SLOT( saveGraph() ), Qt::CTRL + Qt::Key_S );

  _private->mySessionMenu->addAction(
    tr("Save As"), this, SLOT( saveGraphAs() ),
    Qt::CTRL + Qt::SHIFT + Qt::Key_S );

  _private->mySessionMenu->addAction(
    tr("Clean"), this, SLOT( cleanSession() ),
    Qt::CTRL + Qt::SHIFT + Qt::Key_C );

  _private->mySaveGraphAction->setEnabled(
    _private->myRoiManagementAction->savableGraph() );

  _private->myRegionMenu = _private->myMainMenu->addMenu( tr( "Region" ) );
  _private->myRegionMenu->addAction(
    tr("New"), this, SLOT( newRegion() ), Qt::CTRL + Qt::Key_N );

  _private->myRegionMenu->addAction(
    tr("Delete"), this, SLOT( deleteRegion() ), Qt::CTRL + Qt::Key_D );
  _private->myRegionMenu->addAction(
    tr("Fusion"), this, SLOT( regionsFusion() ) );

  _private->myRegionMenu->addSeparator() ;
  _private->myRegionMenu->addAction(
    tr("Export as mask"), this, SLOT( exportAsMask() ) );
  _private->myRegionMenu->addAction(
    tr("Morpho Stats"), this, SLOT( regionStats() ), Qt::CTRL + Qt::Key_M );

  _private->myFrameWorkMenu
    = _private->myMainMenu->addMenu( tr( "FrameWork" ) );
//   _private->myFrameWorkMenu->setCheckable(true) ; // ??
  _private->myNeuroAction = _private->myFrameWorkMenu->addAction(
    tr("Neuro"), this, SLOT( neuroFrameWork() ) );
  _private->myNeuroAction->setCheckable( true );
  _private->myLateralNeuroAction = _private->myFrameWorkMenu->addAction(
    tr("Lateral Neuro"), this, SLOT( lateralNeuroFrameWork() ) );
  _private->myLateralNeuroAction->setCheckable( true );
  _private->mySulciAction = _private->myFrameWorkMenu->addAction(
    tr("Sulci"), this, SLOT( sulciFrameWork() ) );
  _private->mySulciAction->setCheckable( true );
  _private->myRatWbAction = _private->myFrameWorkMenu->addAction(
    tr("Rat_wb"), this, SLOT( ratFrameWork() ) );
  _private->myRatWbAction->setCheckable( true );

  _private->myUserDefinedFrameWorkMenu = _private->myFrameWorkMenu->addMenu(
    tr("Personal") );
  _private->myUserDefinedFrameWorkMenu->addAction(
    tr("New"), this, SLOT(newUserDefinedFrameWork()) );
  _private->myUserDefinedFrameWorkMenu->addAction(
    tr("Load"), this, SLOT(loadUserDefinedFrameWork()) );
  _private->myUserDefinedFrameWorkMenu->addAction(
    tr("Save"), this, SLOT(saveUserDefinedFrameWork()) );
  _private->myUserDefinedFrameWorkMenu->addSeparator();
  _private->myNewRegionAction
    = _private->myUserDefinedFrameWorkMenu->addAction(
      tr("Define new region"), this, SLOT(defineNewFWRegionName()) );
  _private->myNewRegionAction->setVisible(
    _private->myRoiManagementAction->selectedHierarchy()
      == _private->myRoiManagementAction->userDefinedHierarchy() );
  _private->myModifyRegionNameAction
    = _private->myUserDefinedFrameWorkMenu->addAction(
      tr("Modify region name"), this, SLOT(modifyFWRegionName()) );
  _private->myModifyRegionNameAction->setVisible(
    _private->myRoiManagementAction->selectedHierarchy()
      == _private->myRoiManagementAction->userDefinedHierarchy() );
  _private->myModifyRegionColorAction
    = _private->myUserDefinedFrameWorkMenu->addAction(
      tr("Modify region color"), this, SLOT(modifyFWRegionColor()) );
  _private->myModifyRegionColorAction->setVisible(
    _private->myRoiManagementAction->selectedHierarchy()
        == _private->myRoiManagementAction->userDefinedHierarchy() );
  _private->myDeleteRegionAction =
    _private->myUserDefinedFrameWorkMenu->addAction(
      tr("Delete region name"), this, SLOT(deleteFWRegionName()) );
  _private->myDeleteRegionAction->setVisible(
    _private->myRoiManagementAction->selectedHierarchy()
      == _private->myRoiManagementAction->userDefinedHierarchy() );
//   _private->myUserDefinedFrameWorkMenu->addAction( tr("SaveAs"), this,
//      SLOT(saveUserDefinedFrameWorkAs()) );

  _private->myFreeFrameworkAction = _private->myFrameWorkMenu->addAction(
    tr("Free"), this, SLOT(freeFrameWork()) );
  _private->myFreeFrameworkAction->setCheckable( true );

  _private->myWindowMenu = _private->myMainMenu->addMenu( tr( "Windows" ) );
  _private->myWindowMenu->addAction(
    tr("Axial"), this, SLOT( createAxialWindow() ) );
  _private->myWindowMenu->addAction(
    tr("Sagittal"), this, SLOT( createSagittalWindow() ) );
  _private->myWindowMenu->addAction(
    tr("Coronal"), this, SLOT( createCoronalWindow() ) );
  _private->myWindowMenu->addAction(
    tr("3D"), this, SLOT( create3DWindow() ) );


  QGridLayout *lay = new QGridLayout;
  lay1->addLayout( lay );
  QGroupBox *myGraphSelectBox = new QGroupBox( tr("Session"), this ) ;
  lay->addWidget( myGraphSelectBox, 0, 0 );
  QVBoxLayout *gsblay = new QVBoxLayout( myGraphSelectBox );
  _private->mySelectGraph = new QListWidget( myGraphSelectBox );
  gsblay->addWidget( _private->mySelectGraph );
  _private->mySelectGraph->setMinimumHeight( 100 );
  _private->mySelectGraph->setSizePolicy(
    QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );

  QGroupBox *myRegionSelectBox = new QGroupBox( tr("Region"), this ) ;
  lay->addWidget( myRegionSelectBox, 0, 1 );
  QVBoxLayout *rsblay = new QVBoxLayout( myRegionSelectBox );
  _private->mySelectRegion = new QListWidget( myRegionSelectBox );
  rsblay->addWidget( _private->mySelectRegion );
  _private->mySelectRegion->setMinimumHeight( 100 ) ;
  _private->mySelectRegion->setSizePolicy(
    QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );

  QGroupBox *myImageBox = new QGroupBox( tr("Image"), this );
  lay->addWidget( myImageBox, 1, 0 );
  QVBoxLayout *iblay = new QVBoxLayout( myImageBox );
  _private->mySelectImage = new QListWidget( myImageBox );
  iblay->addWidget( _private->mySelectImage );
  _private->mySelectImage->setMinimumHeight( 100 ) ;
  _private->mySelectImage->setSizePolicy(
    QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );


  const QPixmap		*p;
  QGroupBox *myWindowPart = new QGroupBox( tr("Painting Views"), this );
  lay->addWidget( myWindowPart, 1, 1 );
  QVBoxLayout *wplay = new QVBoxLayout( myWindowPart );
  QWidget *myWindowGrid = new QWidget( myWindowPart );
  wplay->addWidget( myWindowGrid );
  QGridLayout *wglay = new QGridLayout( myWindowGrid );
  p = IconDictionary::instance()->getIconInstance( "axial" );
  _private->myAxial = new QPushButton( *p, tr("Axial"), myWindowGrid );
  wglay->addWidget( _private->myAxial, 0, 0 );

  p = IconDictionary::instance()->getIconInstance( "sagittal" );
  _private->mySagittal = new QPushButton( *p, tr("Sagittal"), myWindowGrid );
  wglay->addWidget( _private->mySagittal, 0, 1 );

  p = IconDictionary::instance()->getIconInstance( "coronal" );
  _private->myCoronal = new QPushButton( *p, tr("Coronal"), myWindowGrid );
  wglay->addWidget( _private->myCoronal, 1, 0 );

  p = IconDictionary::instance()->getIconInstance( "3D" );
  _private->my3D = new QPushButton( *p, tr("3D"), myWindowGrid );
  wglay->addWidget( _private->my3D );

  QGroupBox *myRegionTransparencyBox = new QGroupBox(
    tr("Region Transparency"),myWindowPart );
  wplay->addWidget( myRegionTransparencyBox );
  QHBoxLayout *rtlay = new QHBoxLayout( myRegionTransparencyBox );

  _private->myRegionTransparency =
    new QSlider( Qt::Horizontal, myRegionTransparencyBox );
  _private->myRegionTransparency->setRange( 0, 100 );
  _private->myRegionTransparency->setPageStep( 20 );
  _private->myRegionTransparency->setValue(
    int( SelectFactory::selectColor().a * 100 ) );
  rtlay->addWidget( _private->myRegionTransparency );
  _private->myRegionTransparency
    ->setMinimumSize( 50, _private->myRegionTransparency->sizeHint().height() );
  _private->myRegionTransparency->setTracking(true) ;

  _private->myRegionTransparencyLabel =
    new QLabel( QString::number(int(SelectFactory::selectColor().a * 100)),
                myRegionTransparencyBox );
  rtlay->addWidget( _private->myRegionTransparencyLabel );

  QGroupBox *myGraphTransparencyBox = new QGroupBox(
    tr("Graph Transparency"), myWindowPart );
  wplay->addWidget( myGraphTransparencyBox );
  QHBoxLayout *gtblay = new QHBoxLayout( myGraphTransparencyBox );
  _private->myGraphTransparency =
    new QSlider( Qt::Horizontal, myGraphTransparencyBox );
  _private->myGraphTransparency->setRange( 0, 100 );
  _private->myGraphTransparency->setPageStep( 20 );
  _private->myGraphTransparency->setValue(
    int( SelectFactory::selectColor().a * 100 ) );
  gtblay->addWidget( _private->myGraphTransparency );
  _private->myGraphTransparency
    ->setMinimumSize( 50, _private->myGraphTransparency->sizeHint().height() );
  _private->myGraphTransparency->setTracking(true) ;

  _private->myGraphTransparencyLabel =
    new QLabel( QString::number(int(
        _private->myRoiManagementAction->graphTransparency() * 100)),
      myGraphTransparencyBox );
  gtblay->addWidget( _private->myGraphTransparencyLabel );

  set<AObject*> objs = theAnatomist->getObjects() ;

  set<AObject*>::iterator iter( objs.begin() ), last( objs.end() ) ;
  string name ;
  bool found = false ;
  while (iter != last)
    {
      if( (*iter)->type( ) == Hierarchy::classType( ) )
	found = true ;
      ++iter ;
    }

  string selectHie = _private->myRoiManagementAction->getCurrentHierarchy() ;
  if( selectHie == "" && (!found) )
    freeFrameWork() ;

  _private->mySelectGraph->setContextMenuPolicy( Qt::CustomContextMenu );
  _private->mySelectRegion->setContextMenuPolicy( Qt::CustomContextMenu );
  _private->mySelectImage->setContextMenuPolicy( Qt::CustomContextMenu );

  connect( _private->mySelectGraph, SIGNAL( currentRowChanged( int ) ),
           this, SLOT( selectGraph( int ) ) );
  connect( _private->mySelectGraph,
           SIGNAL( itemDoubleClicked( QListWidgetItem * ) ),
           this, SLOT( renameGraph( QListWidgetItem * ) ) ) ;
  connect( _private->mySelectImage,
           SIGNAL( currentRowChanged( int ) ),
           this, SLOT( selectImage( int ) ) ) ;
  connect( _private->mySelectRegion,
           SIGNAL( currentRowChanged( int ) ),
           this, SLOT( selectRegion( int ) ) ) ;
  connect( _private->mySelectRegion,
           SIGNAL( itemDoubleClicked( QListWidgetItem * ) ),
           this, SLOT( renameRegion( QListWidgetItem * ) ) ) ;
  connect( _private->mySelectGraph,
           SIGNAL( customContextMenuRequested( const QPoint & ) ),
           this, SLOT( selectGraphContextMenu( const QPoint & ) ) ) ;
  connect( _private->mySelectRegion,
           SIGNAL( customContextMenuRequested( const QPoint & ) ),
           this, SLOT( selectRegionContextMenu( const QPoint & ) ) ) ;
  connect( _private->mySelectImage,
           SIGNAL( customContextMenuRequested( const QPoint & ) ),
           this, SLOT( selectImageContextMenu( const QPoint & ) ) ) ;

  connect( _private->myAxial, SIGNAL( clicked( ) ),
	   this, SLOT(createAxialWindow( ) ) ) ;
  connect( _private->myCoronal, SIGNAL( clicked( ) ),
	   this, SLOT(createCoronalWindow( ) ) ) ;
  connect( _private->mySagittal, SIGNAL( clicked( ) ),
	   this, SLOT(createSagittalWindow( ) ) ) ;
  connect( _private->my3D, SIGNAL( clicked( ) ),
	   this, SLOT(create3DWindow( ) ) ) ;

  connect( _private->myRegionTransparency,
	   SIGNAL(valueChanged(int) ),
	   this, SLOT(regionTransparencyChange( int ) ) ) ;
  connect( _private->myGraphTransparency,
	   SIGNAL(valueChanged(int) ),
	   this, SLOT(graphTransparencyChange( int ) ) ) ;

  update( 0, 0 ) ;

  // maintain a global list of RoiManagementActionView instances
  roiManagementViews().insert( this );
}

RoiManagementActionView::~RoiManagementActionView()
{
#ifdef ANA_DEBUG
  cout << "RoiManagementActionView::~RoiManagementActionView"<< endl ;
#endif
  roiManagementViews().erase( this );
  _private->myRoiManagementAction->deleteObserver(this) ;
}

QStringList
RoiManagementActionView::getHierarchyNames()
{
  myGettingHierarchyNames = true ;

#ifdef ANA_DEBUG
  cout << "RoiManagementActionView::getHierarchyNames" << endl ;
#endif
  set<string> hieList( _private->myRoiManagementAction->getHierarchyNames() ) ;
  QStringList output ;
  set<string>::iterator iter( hieList.begin() ), last( hieList.end() ) ;

  while (iter != last)
    {
      output.append( QString( iter->c_str() ) ) ;

      ++iter ;
    }
  output.append( QString( "UserDefined" ) ) ;
  myGettingHierarchyNames = false ;
  return output ;
}

QStringList
RoiManagementActionView::getGraphNames()
{
  myGettingGraphNames = true ;
#ifdef ANA_DEBUG
  cout << "RoiManagementActionView::getGraphNames" << endl ;
#endif
  set<string> graphNamesList( _private->myRoiManagementAction->getGraphNames() ) ;
  QStringList output ;
  set<string>::iterator iter( graphNamesList.begin() ), last( graphNamesList.end() ) ;

  while (iter != last)
    {
      output.append( QString( iter->c_str() ) ) ;

      ++iter ;
    }
  myGettingGraphNames = false ;
  return output ;
}

QStringList
RoiManagementActionView::getImageNames()
{
#ifdef ANA_DEBUG
  cout << "RoiManagementActionView::getImageNames" << endl ;
#endif
  myGettingImageNames = true ;
  set<string> imageNamesList( _private->myRoiManagementAction->getImageNames() ) ;
  QStringList output ;
  set<string>::iterator iter( imageNamesList.begin() ), last( imageNamesList.end() ) ;

  while (iter != last)
    {
      output.append( QString( iter->c_str() ) ) ;

      ++iter ;
    }
  myGettingImageNames = false ;
  return output ;
}

QStringList
RoiManagementActionView::getCurrentGraphRegions()
{
  #ifdef ANA_DEBUG
  cout << "RoiManagementActionView::getCurrentGraphRegions" << endl ;
#endif
  myGettingRegionNames = true ;
  QStringList output ;
  set<string> graphRegions = _private->myRoiManagementAction->getCurrentGraphRegions( ) ;

  set<string>::iterator iter( graphRegions.begin() ), last( graphRegions.end() ) ;

  while (iter != last)
    {
      output.append( QString( iter->c_str() ) ) ;

      ++iter ;
    }
  myGettingRegionNames = false ;
  return output ;
}

string
RoiManagementActionView::getSelectedGraphName( )
{
  #ifdef ANA_DEBUG
  cout << "RoiManagementActionView::getSelectedGraphName" << endl ;
#endif
  string name ;
  if (_private->myRoiManagementAction->getSelectedGraphName( name ) )
    return name ;
  return "Roi" ;
}

QStringList
RoiManagementActionView::getCurrentHierarchyRoiNames()
{
  myGettingHierarchyNames = true ;
  #ifdef ANA_DEBUG
  cout << "RoiManagementActionView::getCurrentHierarchyRoiNames" << endl ;
#endif
  QStringList output ;
  set<string> roiNames = _private->myRoiManagementAction->getCurrentHierarchyRoiNames( ) ;

  set<string>::iterator iter( roiNames.begin() ), last( roiNames.end() ) ;

  while (iter != last)
    {
      output.append( QString( iter->c_str() ) ) ;

      ++iter ;
    }
  myGettingHierarchyNames = false ;
  return output ;
}

void
RoiManagementActionView::selectGraph( int row )
{
#ifdef ANA_DEBUG
  cout << "RoiManagementActionView::selectGraph" << endl ;
#endif
  if ( myUpdatingFlag || row < 0 )
    return ;

  cout << "row: " << row << endl;
  QListWidgetItem *graph = _private->mySelectGraph->item( row );

  mySelectingGraph = true ;
  _private->myRoiManagementAction->selectGraph(
    graph->text().toStdString(),
    _private->mySelectGraph->currentRow() );

//   if( _private->myRoiManagementAction->savableGraph() )
//     _private->mySaveGraphAction->setEnabled( true ) ;
//   else
//     _private->mySaveGraphAction->setEnabled( false ) ;

  mySelectingGraph = false ;
}


// void
// RoiManagementActionView::graphNameChanged( const QString & graphName )
// {
//   _private->myRoiManagementAction->setGraphName( string ( ( const char * ) graphName ) ) ;
// }

void
RoiManagementActionView::selectImage( int row )
{
  #ifdef ANA_DEBUG
  cout << "RoiManagementActionView::selectImage" << endl ;
#endif
  if ( myUpdatingFlag || row < 0 )
    return ;

  QListWidgetItem *image = _private->mySelectImage->item( row );

  mySelectingImage = true ;
  _private->myRoiManagementAction->selectImage(
    image->text().toStdString(), _private->mySelectImage->currentRow() );
  mySelectingImage = false ;
}


string
RoiManagementActionView::askName (const string& type,
                                  const string& originalName,
                                  const string& message, bool noHierarchy )
{
  QDialog * nameSetter = new QDialog( this );
  nameSetter->setModal( true );
  nameSetter->setWindowTitle( message.c_str() );

  QVBoxLayout * l = new QVBoxLayout( nameSetter );
  QLineEdit * lineEdition = 0;
  QComboBox * selectRegionName = 0;

  if( type == "FrameWork" )
  {
    //cout<< "No Hierarchy" << endl ;
    lineEdition = new QLineEdit( QString( originalName.c_str() ), this );
    l->addWidget( lineEdition );
    lineEdition->setMinimumWidth(250) ;
  }
  else if( type == "session" || noHierarchy)
  {
    //cout<< "No Hierarchy" << endl ;
    lineEdition = new QLineEdit( QString( originalName.c_str() ), this );
    l->addWidget( lineEdition );
    lineEdition->setMinimumWidth(250) ;
  } else if( type == "region" && (!noHierarchy) )
  {
    //cout<< "Hierarchy" << endl ;
    selectRegionName = new QComboBox( this );
    l->addWidget( selectRegionName );
    selectRegionName->addItems( getCurrentHierarchyRoiNames() );
    selectRegionName->setCurrentIndex(0) ;
  } else {
    ASSERT(false);  // wrong value for type argument
  }

  QWidget * buttons = new QWidget( this );
  l->addWidget( buttons );
  QHBoxLayout *blay = new QHBoxLayout( buttons );

  QPushButton * okButton = new QPushButton( "Ok", buttons );
  blay->addWidget( okButton );
  okButton->setDefault( true );
  okButton->setSizePolicy( QSizePolicy( QSizePolicy::Fixed,
                                        QSizePolicy::Fixed ) );
  QPushButton * cancelButton = new QPushButton( "Cancel", buttons );
  blay->addWidget( cancelButton );
  cancelButton->setSizePolicy( QSizePolicy( QSizePolicy::Fixed,
                                            QSizePolicy::Fixed ) );

  QObject::connect( okButton , SIGNAL( clicked() ),
                    nameSetter, SLOT( accept() ) );
  QObject::connect( cancelButton , SIGNAL( clicked() ),
                    nameSetter, SLOT( reject() ) );

  string result = "" ;
  if( nameSetter->exec() )
  {
    if( type == "session" || noHierarchy)
      result = lineEdition->text().toStdString();
    else if(selectRegionName)
      result = selectRegionName->currentText().toStdString();
    if( result == "" )
      result = "Unknown";
  }

  #ifdef ANA_DEBUG
  cout << "Result : " << result << endl ;
#endif

  delete nameSetter ;
  return result ;
}

void
RoiManagementActionView::newGraph()
{
  string name = "" ;
  _private->myRoiManagementAction->newGraph( name );
}

void
RoiManagementActionView::regionStats()
{
  _private->myRoiManagementAction->regionStats( ) ;
}

void
RoiManagementActionView::renameGraph( QListWidgetItem * graph )
{
  _private->myRoiManagementAction->renameGraph(
    askName( "session", graph->text().toStdString(),
              "Please enter new name for selected session" ),
    _private->mySelectGraph->currentRow() );
}

void
RoiManagementActionView::deleteGraph()
{
  QMessageBox::StandardButton result = QMessageBox::warning(
    this, tr( "Delete ROI session" ),
    tr( "If you close this session, all modifications since your last save will be lost. \nDo you still want to proceed ?" ),
    QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel );
  if( result != QMessageBox::Ok )
    return;

  _private->myRoiManagementAction->deleteGraph( ) ;
}

void
RoiManagementActionView::loadGraph( )
{
  QString filt = ( ControlWindow::tr( "Graphs" )
                   + " (*.arg);;"
                   // + ControlWindow::tr( "ROIs" )
                   // + " (*.bck);;"
                   + ControlWindow::tr( "Label volumes" )
                   + " (*.ima *.img *.v *.vimg *.mnc)" ) ;

  QString capt = RoiManagementActionView::tr( "Load ROI Graph" ) ;

  if ( filt == "" )
    filt = theAnatomist->objectsFileFilter().c_str();

  if ( capt == "" )
    capt = tr( "Load ROI session" );

  QFileDialog	& fd = anatomist::fileDialog();
  fd.setNameFilter( filt );
  fd.setWindowTitle( capt );
  fd.setFileMode( QFileDialog::ExistingFiles );
  if( !fd.exec() )
    return;

  QStringList filenames = fd.selectedFiles();
  _private->myRoiManagementAction->loadGraph( filenames ) ;

//   if( _private->myRoiManagementAction->savableGraph() )
//     _private->mySaveGraphAction->setEnabled( true ) ;
//   else
//     _private->mySaveGraphAction->setEnabled( false ) ;

//   selectGraph( graph ) ;
//   selectRegion( region ) ;
}

void
RoiManagementActionView::saveGraphAs( )
{
  _private->myRoiManagementAction->saveGraphAs( ) ;

  if( _private->myRoiManagementAction->savableGraph() )
    _private->mySaveGraphAction->setEnabled( true );
  else
    _private->mySaveGraphAction->setEnabled( false );
}

void
RoiManagementActionView::reloadGraph( )
{
    _private->myRoiManagementAction->reloadGraph( );
}

void
RoiManagementActionView::saveGraph( )
{
  _private->myRoiManagementAction->saveGraph( ) ;
}

void
RoiManagementActionView::selectRegion( int row )
{
  if ( myUpdatingFlag || row < 0 )
    return ;

  if( _private->mySelectRegion->count() <= row )
    return;
  QListWidgetItem *region = _private->mySelectRegion->item( row );

  mySelectingRegion = true ;
  _private->myRoiManagementAction->selectRegion(
    region->text().toStdString(), _private->mySelectRegion->currentRow() );
  mySelectingRegion = false ;
}

void
RoiManagementActionView::newRegion( )
{
  string regionName = askName(
    "region", "", "Please enter new region's name",
    _private->myFreeFrameworkAction->isChecked() );
  if( regionName != "" )
    _private->myRoiManagementAction->newRegion( regionName ) ;

//   if( _private->myRoiManagementAction->savableGraph() )
//     _private->mySaveGraphAction->setEnabled( true );
//   else
//     _private->mySaveGraphAction->setEnabled( false );
}

void
RoiManagementActionView::renameRegion( QListWidgetItem * region )
{
  _private->myRoiManagementAction->renameRegion(
    askName(
      "region",
      region->text().toStdString(),
      "Please enter selected region's new name",
      _private->myFreeFrameworkAction->isChecked() ),
    _private->mySelectRegion->currentRow() ) ;
}

void
RoiManagementActionView::deleteRegion( )
{
  _private->myRoiManagementAction->deleteRegion( ) ;
}

void
RoiManagementActionView::exportAsMask( )
{
  _private->myRoiManagementAction->exportAsMask( ) ;
}

void
RoiManagementActionView::regionsFusion( )
{
  RegionsFusionWindow * fusionWindow = new RegionsFusionWindow( this,
								getCurrentGraphRegions() ) ;
  fusionWindow->show() ;
  if( fusionWindow->exec() )
    {
      _private->myRoiManagementAction->regionsFusion( fusionWindow->regionsToBeFusioned(),
						      fusionWindow->newRegionName() ) ;
    }
  delete fusionWindow ;
}

void
RoiManagementActionView::cleanSession()
{
  _private->myRoiManagementAction->cleanSession() ;
}

void
RoiManagementActionView::cleanRegion()
{
  _private->myRoiManagementAction->cleanRegion(0) ;
}


void
RoiManagementActionView::neuroFrameWork()
{
  cout << "Neuro Framework" << endl ;
  _private->myNeuroAction->setChecked( true );
  _private->myLateralNeuroAction->setChecked( false );
  _private->mySulciAction->setChecked( false );
  _private->myRatWbAction->setChecked( false );
//   _private->myUserDefinedFrameWorkMenu->setChecked( false );
  _private->myUserDefinedFrameWorkMenu->setTitle( tr( "User Defined" ) );
  _private->myFreeFrameworkAction->setChecked( false );
  _private->myNewRegionAction->setVisible( false );
  _private->myModifyRegionNameAction->setVisible( false );
  _private->myModifyRegionColorAction->setVisible( false );
  _private->myDeleteRegionAction->setVisible( false );
  // FIXME: remove this hieId 301
  _private->myRoiManagementAction->selectHierarchy(
    "neuronames.hie", RoiManagementActionView_Private::NEURO ) ;
}

void
RoiManagementActionView::lateralNeuroFrameWork()
{
  cout << "Lateral Neuro Framework" << endl ;
  _private->myNeuroAction->setChecked( false );
  _private->myLateralNeuroAction->setChecked( true );
  _private->mySulciAction->setChecked( false );
  _private->myRatWbAction->setChecked( false );
//   _private->myUserDefinedFrameWorkMenu->setChecked( false );
  _private->myUserDefinedFrameWorkMenu->setTitle( tr( "User Defined" ) );
  _private->myFreeFrameworkAction->setChecked( false );
  _private->myNewRegionAction->setVisible( false );
  _private->myModifyRegionNameAction->setVisible( false );
  _private->myModifyRegionColorAction->setVisible( false );
  _private->myDeleteRegionAction->setVisible( false );

  _private->myRoiManagementAction->selectHierarchy(
    "lateral_neuronames.hie", RoiManagementActionView_Private::LATERALNEURO );
}

void
RoiManagementActionView::sulciFrameWork()
{
  cout << "Sulci Framework" << endl;
  _private->myNeuroAction->setChecked( false );
  _private->myLateralNeuroAction->setChecked( false );
  _private->mySulciAction->setChecked( true );
  _private->myRatWbAction->setChecked( false );
//   _private->myUserDefinedFrameWorkMenu->setChecked( false ) ;
  _private->myUserDefinedFrameWorkMenu->setTitle( tr( "User Defined" ) );
  _private->myFreeFrameworkAction->setChecked( false );
  _private->myNewRegionAction->setVisible( false );
  _private->myModifyRegionNameAction->setVisible( false );
  _private->myModifyRegionColorAction->setVisible( false );
  _private->myDeleteRegionAction->setVisible( false );

  _private->myRoiManagementAction->selectHierarchy(
    "sulcal_root_colors.hie", RoiManagementActionView_Private::SULCI );
}

void
RoiManagementActionView::ratFrameWork()
{
  cout << "Rat Framework" << endl ;
  _private->myNeuroAction->setChecked( false );
  _private->myLateralNeuroAction->setChecked( false );
  _private->mySulciAction->setChecked( false );
  _private->myRatWbAction->setChecked( true );
//   _private->myUserDefinedFrameWorkMenu->setChecked( false );
  _private->myUserDefinedFrameWorkMenu->setTitle( tr( "User Defined" ) );
  _private->myFreeFrameworkAction->setChecked( false );
  _private->myNewRegionAction->setVisible( false );
  _private->myModifyRegionNameAction->setVisible( false );
  _private->myModifyRegionColorAction->setVisible( false );
  _private->myDeleteRegionAction->setVisible( false );

  _private->myRoiManagementAction->selectHierarchy(
    "rat_wb.hie", RoiManagementActionView_Private::RAT_WB ) ;
}

void
RoiManagementActionView::freeFrameWork()
{
  cout << "Free Framework" << endl ;
  _private->myNeuroAction->setChecked( false );
  _private->myLateralNeuroAction->setChecked( false );
  _private->mySulciAction->setChecked( false );
  _private->myRatWbAction->setChecked( false );
//   _private->myUserDefinedFrameWorkMenu->setChecked( false );
  _private->myUserDefinedFrameWorkMenu->setTitle( tr( "User Defined" ) );
  _private->myFreeFrameworkAction->setChecked( true );
  _private->myNewRegionAction->setVisible( false );
  _private->myModifyRegionNameAction->setVisible( false );
  _private->myModifyRegionColorAction->setVisible( false );
  _private->myDeleteRegionAction->setVisible( false );

  _private->myRoiManagementAction->selectHierarchy(
    "Free", RoiManagementActionView_Private::FREE ) ;

}

void
RoiManagementActionView::newUserDefinedFrameWork()
{
  cout << "New Personal Framework" << endl ;
  _private->myNeuroAction->setChecked( false );
  _private->myLateralNeuroAction->setChecked( false );
  _private->mySulciAction->setChecked( false );
  _private->myRatWbAction->setChecked( false );
//   _private->myUserDefinedFrameWorkMenu->setChecked( true );
  _private->myUserDefinedFrameWorkMenu->setTitle(
    QString::fromUtf8( "\xe2\x9c\x93  " ) + tr( "User Defined" ) );
  _private->myFreeFrameworkAction->setChecked( false ) ;
  _private->myNewRegionAction->setVisible( true );
  _private->myModifyRegionNameAction->setVisible( true );
  _private->myModifyRegionColorAction->setVisible( true );
  _private->myDeleteRegionAction->setVisible( false );
  string newFName ;
  newFName = askName( "FrameWork", "",
		      tr("Please enter new framework's name").toStdString(),
		      true ) ;
  if( newFName != "" )
    _private->myRoiManagementAction->newUDHierarchy( newFName ) ;
  //updateRecentFrameWorksMenu( newFName ) ;
}

void
RoiManagementActionView::loadUserDefinedFrameWork()
{
#ifdef ANA_DEBUG
  cout << "Load Personal Framework" << endl ;
#endif

  QString filt = ( ControlWindow::tr( "Hierarchies" ) + " (*.hie)" ) ;

  QString capt = RoiManagementActionView::tr( "Load User Defined Framework" ) ;

  if ( filt == "" )
    filt = theAnatomist->objectsFileFilter().c_str();

  if ( capt == "" )
    capt = tr( "Load Anatomist objects" );

  QFileDialog	& fd = anatomist::fileDialog();
  fd.setNameFilter( filt );
  fd.setWindowTitle( capt );
  fd.setFileMode( QFileDialog::ExistingFiles );
  fd.setDirectory(QDir( (Settings::localPath() + FileUtil::separator()
                        + "frameworks/").c_str(),
                        QString("*.hie") ) );
  if( !fd.exec() )
    return;

  QStringList filenames = fd.selectedFiles();
  string udhiename
    = _private->myRoiManagementAction->loadUDHierarchy(
      filenames[0].toStdString());
  _private->myUserDefinedFrameWorkMenu->setTitle( QString(udhiename.c_str()) );

  if( udhiename != "" )
  {
    _private->myNeuroAction->setChecked( false );
    _private->myLateralNeuroAction->setChecked( false );
    _private->mySulciAction->setChecked( false );
    _private->myRatWbAction->setChecked( false );
//     _private->myUserDefinedFrameWorkMenu->setChecked( true );
    _private->myUserDefinedFrameWorkMenu->setTitle(
      QString::fromUtf8( "\xe2\x9c\x93  " ) + tr( "User defined" ) );
    _private->myFreeFrameworkAction->setChecked( false ) ;
    _private->myNewRegionAction->setVisible( true );
    _private->myModifyRegionNameAction->setVisible( true );
    _private->myModifyRegionColorAction->setVisible( true );
    _private->myDeleteRegionAction->setVisible(
      !_private->myRoiManagementAction->getHierarchyNames().empty() );
  }
}

void
RoiManagementActionView::saveUserDefinedFrameWork()
{
  _private->myRoiManagementAction->saveUDHierarchy() ;
}

void
RoiManagementActionView::defineNewFWRegionName()
{
  string name = askName("region",
			"",
			"New region name for framework", true) ;

  if( name != "" && name != "Unknown" ) {
    QColor col = QColorDialog::getColor() ;

    if( col.isValid() ){
      _private->myRoiManagementAction->defineNewFWRegionName(
        name, col.red(), col.green(), col.blue() );
      _private->myDeleteRegionAction->setVisible( true );
    }
  }
}

void
RoiManagementActionView::deleteFWRegionName()
{
  string name = askName("region",
			"",
			"Framework region name to delete", false) ;

  if( name != "" && name != "Unknown" )
      _private->myRoiManagementAction->deleteFWRegionName( name ) ;

  if( _private->myRoiManagementAction->getHierarchyNames().empty() )
    _private->myDeleteRegionAction->setVisible( false );
}

void
RoiManagementActionView::modifyFWRegionName()
{
  QDialog * nameSetter = new QDialog( this );
  nameSetter->setModal( true );
  nameSetter->setWindowTitle( tr( "Modify Frame Work Region Name" ) );

  QVBoxLayout * l = new QVBoxLayout( nameSetter );
  l->setMargin( 5 );
  l->setSpacing( 5 );
  QLineEdit * lineEdition = 0 ;
  QComboBox * selectRegionName = 0;

  selectRegionName = new QComboBox( nameSetter );
  l->addWidget( selectRegionName );
  selectRegionName->addItems( getCurrentHierarchyRoiNames() );
  selectRegionName->setCurrentIndex(0);

  lineEdition = new QLineEdit( selectRegionName->currentText(), nameSetter );
  l->addWidget( lineEdition );
//   lineEdition->setMinimumWidth(250);
  lineEdition->setSizePolicy( QSizePolicy( QSizePolicy::Expanding,
                                           QSizePolicy::Fixed ) );

  connect( selectRegionName, SIGNAL( textChanged( const QString & ) ),
	   lineEdition, SLOT(setText(const QString&) ) ) ;

  QWidget * buttons = new QWidget( nameSetter );
  l->addWidget( buttons );
  QHBoxLayout *blay = new QHBoxLayout( buttons );

  QPushButton * okButton = new QPushButton( "Ok", buttons );
  blay->addWidget( okButton );
  okButton->setDefault( true );
  okButton->setSizePolicy( QSizePolicy( QSizePolicy::Fixed,
                                        QSizePolicy::Fixed ) );
  QPushButton * cancelButton = new QPushButton( "Cancel", buttons );
  blay->addWidget( cancelButton );
  cancelButton->setSizePolicy( QSizePolicy( QSizePolicy::Fixed,
                                            QSizePolicy::Fixed ) );

  QObject::connect( okButton , SIGNAL( clicked() ),
                    nameSetter, SLOT( accept() ) );
  QObject::connect( cancelButton , SIGNAL( clicked() ),
                    nameSetter, SLOT( reject() ) );

  string newName, oldName ;
  if( nameSetter->exec() )
  {
    newName = lineEdition->text().toStdString();
    oldName = selectRegionName->currentText().toStdString();
    if( newName != oldName && newName != "" )
      _private->myRoiManagementAction->modifyUDFWRegionName(
        oldName, newName );
  }
  delete nameSetter ;
}

void
RoiManagementActionView::modifyFWRegionColor()
{
  string name = askName("region", "", tr("Choose the name of the region\n"
			"whose color you want to change").toStdString(), false ) ;
  QColor col = QColorDialog::getColor() ;
  if( col.isValid() )
    _private->myRoiManagementAction->modifyUDFWRegionColor( name, col.red(), col.green(),
							    col.blue() ) ;
}

// void
// RoiManagementActionView::saveUserDefinedFrameWorkAs()
// {
//   _private->myRoiManagementAction->saveUDHierarchyAs() ;
// }

// void
// RoiManagementActionView::updateRecentFrameWorksMenu( const string& newHie )
// {
//   NEWNEWNEW

//   ifstream is(string(Settings::localPath() + FileUtil::separator() + "recentFrameWorks").c_str() ) ;
//   string recent ;
//   while( is.eof() ){
//     is >> recent ;
//     myRecentFrameWorks.append( recent ) ;
//   }
//   is.close() ;

//   for( int i = 0 ; i < (int)myRecentFrameWorks.size() ; ++i ){
//     QString text = tr( "%1 %2" ).arg(i+1).arg( strippedName( myRecentFrameWorks[i] ) ) ;
//     if( myRecentFrameWorksIds[i] == -1 ){
//       myRecentFrameWorksIds[i] = pop->insertItem( text, this, SLOT( recentFrameWork(int) ), 0, -1,
// 						  pop->count() ) ;
//       pop->setItemParameter( myRecentFrameWorksIds[i], i ) ;
//     }else
//       pop->chanteItem(recentFileIds[i], text) ;
//   }
// }


void
RoiManagementActionView::createAxialWindow()
{
  _private->myRoiManagementAction->createWindow( "Axial" ) ;
}

void
RoiManagementActionView::createCoronalWindow()
{
  _private->myRoiManagementAction->createWindow( "Coronal" ) ;
}

void
RoiManagementActionView::createSagittalWindow()
{
  _private->myRoiManagementAction->createWindow( "Sagittal" ) ;
}

// void
// RoiManagementActionView::createObliqueWindow()
// {
//   _private->myRoiManagementAction->createWindow( "Oblique" ) ;
// }

void
RoiManagementActionView::create3DWindow()
{
  _private->myRoiManagementAction->createWindow( "3D" ) ;
}


void
RoiManagementActionView::update( const anatomist::Observable *,
                                 void * /*realOrig*/ )
{
#ifdef ANA_DEBUG
  cout << "RoiManagementActionView::update" << endl ;
#endif

  int id = _private->myRoiManagementAction->selectedHierarchyId() ;

  if (!(myUpdatingFlag || mySelectingGraph || mySelectingImage || mySelectingRegion )  ){
    _private->myNeuroAction->setChecked(
      id == RoiManagementActionView_Private::NEURO ? true : false );
    _private->myLateralNeuroAction->setChecked(
      id == RoiManagementActionView_Private::LATERALNEURO ? true : false );
    _private->mySulciAction->setChecked(
      id == RoiManagementActionView_Private::SULCI ? true : false );
    _private->myRatWbAction->setChecked(
      id == RoiManagementActionView_Private::RAT_WB ? true : false );
//     _private->myUserDefinedFrameWorkMenu->setChecked(
//       id == RoiManagementActionView_Private::USERDEFINED ? true : false ) ;
    _private->myUserDefinedFrameWorkMenu->setTitle(
      ( id == RoiManagementActionView_Private::USERDEFINED ?
          QString::fromUtf8( "\xe2\x9c\x93  " ) : QString() )
      + tr( "User defined" ) );
    _private->myFreeFrameworkAction->setChecked(
      id == RoiManagementActionView_Private::FREE ? true : false );
  }
  if( _private->myRoiManagementAction->savableGraph() )
    _private->mySaveGraphAction->setEnabled( true );
  else
    _private->mySaveGraphAction->setEnabled( false );

  if( !mySelectingGraph && !myGettingGraphNames )
  {
    QStringList graphNames = getGraphNames() ;
    if ( graphNames != myGraphNames ){
      myGraphNames = graphNames ;
      _private->mySelectGraph->clear() ;
      _private->mySelectGraph->addItems( myGraphNames ) ;
      _private->mySelectGraph
	->setCurrentRow( _private->myRoiManagementAction->currentGraphId() );
    } else if( _private->mySelectGraph->currentRow() !=
	       _private->myRoiManagementAction->currentGraphId() )
      _private->mySelectGraph
	->setCurrentRow( _private->myRoiManagementAction->currentGraphId() );
  }

  if( !mySelectingImage && !myGettingImageNames ){
    QStringList imageNames = getImageNames() ;
    if ( imageNames != myImageNames ){
      myImageNames = imageNames ;
      _private->mySelectImage->clear() ;
      _private->mySelectImage->addItems( myImageNames ) ;
      _private->mySelectImage
	->setCurrentRow( _private->myRoiManagementAction->currentImageId() ) ;
    } else if ( _private->mySelectImage->currentRow() !=
		_private->myRoiManagementAction->currentImageId() )
      _private->mySelectImage
	->setCurrentRow( _private->myRoiManagementAction->currentImageId() ) ;
  }

  if( !mySelectingRegion && !myGettingRegionNames ){
    QStringList regions = getCurrentGraphRegions() ;
    if ( regions != myRegions ){
      myRegions = regions ;
      _private->mySelectRegion->clear();
      _private->mySelectRegion->addItems( myRegions );
      _private->mySelectRegion->setCurrentRow(
        _private->myRoiManagementAction->currentRegionId() );
    }
    else if( _private->mySelectRegion->currentRow() !=
             _private->myRoiManagementAction->currentRegionId() )
      _private->mySelectRegion
	->setCurrentRow(_private->myRoiManagementAction->currentRegionId() );
  }
  myUpdatingFlag = false ;
  //cout << "RoiManagementActionView::update done\n";

  if( !myChangingRegionTransparency ){
    int transparency =  int(SelectFactory::selectColor().a * 100) ;
    _private->myRegionTransparency->setValue( transparency );
    _private->myRegionTransparencyLabel->setText( QString::number( transparency ) ) ;
  }
  if(!myChangingGraphTransparency ) {
    int transparency = int(_private->myRoiManagementAction->graphTransparency() * 100) ;
    _private->myGraphTransparency->setValue( transparency );
    _private->myGraphTransparencyLabel->setText( QString::number( transparency ) ) ;
  }
}

void
RoiManagementActionView::objectLoaded( anatomist::Observable* o )
{
  AObject	*ao = dynamic_cast<AObject *>( o );
  if( ao && ( ao->type() == AObject::VOLUME || ao->type() == AObject::GRAPH ) )
    RoiManagementActionSharedData::instance()->refresh();
}

void
RoiManagementActionView::regionTransparencyChange( int alpha )
{
  #ifdef ANA_DEBUG
  cout << "RoiManagementActionView::regionTransparencyChange" << endl ;
#endif
  if( myUpdatingFlag)
    return ;

  myChangingRegionTransparency = true ;
  _private->myRoiManagementAction->changeRegionTransparency( float(alpha) / 100. ) ;
  myChangingRegionTransparency = false ;

  _private->myRegionTransparencyLabel->setText( QString::number( alpha ) ) ;
}

void
RoiManagementActionView::graphTransparencyChange( int alpha )
{
  #ifdef ANA_DEBUG
  cout << "RoiManagementActionView::graphTransparencyChange" << endl ;
#endif
  if( myUpdatingFlag)
    return ;

  myChangingGraphTransparency = true ;
  _private->myRoiManagementAction->changeGraphTransparency( float(alpha) / 100. ) ;
  myChangingGraphTransparency = false ;
  _private->myGraphTransparencyLabel->setText( QString::number( alpha ) ) ;
}


void RoiManagementActionView::contextMenu( const QPoint & pos )
{
  QMenu pop;
  pop.addMenu( _private->mySessionMenu );
  pop.addMenu( _private->myRegionMenu );
  pop.addMenu( _private->myFrameWorkMenu );
  // pop.addMenu( _private->myUserDefinedFrameWorkMenu );
  pop.addMenu( _private->myWindowMenu );
  pop.exec( pos );
}


void RoiManagementActionView::selectGraphContextMenu( const QPoint & pos )
{
  contextMenu( _private->mySelectGraph->mapToGlobal( pos ) );
}


void RoiManagementActionView::selectRegionContextMenu( const QPoint & pos )
{
  contextMenu( _private->mySelectRegion->mapToGlobal( pos ) );
}


void RoiManagementActionView::selectImageContextMenu( const QPoint & pos )
{
  contextMenu( _private->mySelectImage->mapToGlobal( pos ) );
}


// ---

RoiManagementActionSharedData* RoiManagementActionSharedData::_instance = 0 ;

RoiManagementActionSharedData* RoiManagementActionSharedData::instance(){
  if( _instance == 0 )
    _instance = new RoiManagementActionSharedData ;
  return _instance ;
}

RoiManagementActionSharedData::RoiManagementActionSharedData()
  : anatomist::Observable(), Observer(),
    myHierarchyNamesChanged(true), myGraphNamesChanged(true),
    myImageNamesChanged(true), myCurrentGraphRegionsChanged(true),
    myCurrentHierarchyRoiNamesChanged(true), mySelectedHierarchy(""),
    mySelectedHierarchyId( RoiManagementActionView_Private::NEURO ),
    myGraphName(""), myCurrentGraph(""),
    myCurrentGraphId(-1), myCurrentImage(""), myCurrentImageId(-1),
    myRegionName(""), myPartialRegionName(""), myCurrentRegionId(-1)
{
}

RoiManagementActionSharedData::~RoiManagementActionSharedData()
{}


void RoiManagementActionSharedData::refresh()
{
  #ifdef ANA_DEBUG
  cout << "RoiManagementActionSharedData::refresh()\n";
  #endif
  myImageNamesChanged = true ;
  myGraphNamesChanged = true ;
  myHierarchyNamesChanged = true ;
  myCurrentGraphRegionsChanged = true ;

  setChanged() ;
  notifyObservers() ;
}


void
RoiManagementActionSharedData::update( const anatomist::Observable* /*obs*/,
                                       void* )
{
  #ifdef ANA_DEBUG
  cout << "RoiManagementActionSharedData::update" << endl ;
#endif

  setChanged() ;
  notifyObservers(  ) ;

  //PaintActionSharedData::instance()->update( obs, 0 ) ;
}



void
RoiManagementActionSharedData::unregisterObservable( anatomist::Observable* o )
{
  Observer::unregisterObservable( o );
  AObject	*ao = dynamic_cast<AObject *>( o );
  if( ao )
    {
      if( ao->type() == AObject::VOLUME )
        myImageNamesChanged = true;
      else if( ao->type() == AObject::GRAPH )
        myGraphNamesChanged = true;
      else if( ao->type() == Hierarchy::classType() )
        myHierarchyNamesChanged = true;
      setChanged();
      notifyObservers();
    }
}

// ----

RoiManagementAction::RoiManagementAction() : Action()
{
  _sharedData = RoiManagementActionSharedData::instance() ;

}

RoiManagementAction::~RoiManagementAction()
{

}


string RoiManagementAction::name() const
{
  return QT_TRANSLATE_NOOP( "ControlSwitch", "RoiManagementAction" );
}

Action*
RoiManagementAction::creator()
{
  return new RoiManagementAction( ) ;
}

set<string>
RoiManagementAction::getHierarchyNames()
{
  #ifdef ANA_DEBUG
  cout << "RoiManagementAction::getHierarchyNames()" << endl ;
#endif

  if ( !_sharedData->myHierarchyNamesChanged )
    return _sharedData->myHierarchyNames ;

  _sharedData->myHierarchyNames.clear() ;

  set<AObject*> objs = theAnatomist->getObjects() ;

  set<AObject*>::iterator iter( objs.begin() ), last( objs.end() ) ;
  string name ;

  while (iter != last)
    {
      if( (*iter)->type( ) == Hierarchy::classType( ) )
        {
          _sharedData->myHierarchyNames.insert( (*iter)->name() ) ;
          (*iter)->addObserver( _sharedData ) ;
        }
	
      ++iter ;
    }

  if( !_sharedData->mySelectedHierarchy.empty() )
    {
      set<string>::iterator	is, es = _sharedData->myHierarchyNames.end();
      for( is=_sharedData->myHierarchyNames.begin();
           is!=es && *is!=_sharedData->mySelectedHierarchy; ++is ) {}
      if( is == es )
        {
          _sharedData->mySelectedHierarchy = "";
          _sharedData->myCurrentHierarchyRoiNamesChanged = true ;
          setChanged() ;
        }
    }

  if( _sharedData->mySelectedHierarchy.empty()
      && _sharedData->myHierarchyNames.size() != 0 )
    {
      _sharedData->mySelectedHierarchy
        = *( _sharedData->myHierarchyNames.begin() ) ;
      _sharedData->myCurrentHierarchyRoiNamesChanged = true ;
      setChanged() ;
    }
  _sharedData->myHierarchyNamesChanged = false ;

  return _sharedData->myHierarchyNames ;
}

set<string>
RoiManagementAction::getGraphNames()
{
  #ifdef ANA_DEBUG
  cout << "RoiManagementAction::getGraphNames()" << endl ;
#endif

  if ( !_sharedData->myGraphNamesChanged )
    return _sharedData->myGraphNames ;

  _sharedData->myGraphNames.clear() ;

  set<AObject*> objs = theAnatomist->getObjects() ;

  set<AObject*>::iterator iter( objs.begin() ), last( objs.end() ) ;
  string name ;

  while ( iter != last )
    {
      if( (*iter)->type() == AObject::GRAPH )
        {
          _sharedData->myGraphNames.insert( (*iter)->name() ) ;
          (*iter)->addObserver( _sharedData ) ;
        }

      ++iter ;
    }

  if( !_sharedData->myCurrentGraph.empty() )
    {
      set<string>::iterator	is, es = _sharedData->myGraphNames.end();
      int			n = 0;
      for( is=_sharedData->myGraphNames.begin();
           is!=es && *is!=_sharedData->myCurrentGraph; ++is, ++n ) {}
      if( is == es )
        {
          _sharedData->myCurrentGraph = "";
          _sharedData->myCurrentGraphId = -1;
          _sharedData->myCurrentGraphRegionsChanged = true ;
          setChanged();
        }
      else if( _sharedData->myCurrentGraphId != n )
        {
          _sharedData->myCurrentGraphId = n;
          _sharedData->myCurrentGraphRegionsChanged = true ;
          setChanged();
        }
    }

  if( _sharedData->myCurrentGraphId == -1
      && _sharedData->myGraphNames.size() != 0 )
    {
      _sharedData->myCurrentGraph = *( _sharedData->myGraphNames.begin() ) ;
      _sharedData->myCurrentGraphId = 0 ;
      _sharedData->myCurrentGraphRegionsChanged = true ;
      setChanged() ;
    }

  _sharedData->myGraphNamesChanged = false ;
  return _sharedData->myGraphNames ;
}

set<string>
RoiManagementAction::getImageNames()
{
  #ifdef ANA_DEBUG
  cout << "RoiManagementAction::getImageNames() : " ;
  #endif
  if ( !_sharedData->myImageNamesChanged )
    return _sharedData->myImageNames ;

  _sharedData->myImageNames.clear() ;

  set<AObject*> objs = theAnatomist->getObjects() ;

  set<AObject*>::iterator iter( objs.begin() ), last( objs.end() ) ;
  string name ;

  while (iter != last)
    {
      if( (*iter)->Is2DObject() && ( (*iter)->type() == AObject::VOLUME || (*iter)->type() == AObject::FUSION2D ) )
      {
        const AObject::ParentList & parents = (*iter)->parents();
        AObject::ParentList::const_iterator ip, ep = parents.end();
        bool hidden = false;
        for( ip=parents.begin(); ip!=ep; ++ip )
          if( (*ip)->type() == AObject::GRAPH
            || (*ip)->type() == AObject::GRAPHOBJECT )
          {
            hidden = true;
            break;
          }
          if( !hidden )
          {
            _sharedData->myImageNames.insert( (*iter)->name() ) ;
            (*iter)->addObserver( _sharedData );
          }
      }

      ++iter ;
    }

  if( !_sharedData->myCurrentImage.empty() )
    {
      set<string>::iterator	is, es = _sharedData->myImageNames.end();
      int			n = 0;
      for( is=_sharedData->myImageNames.begin();
           is!=es && *is != _sharedData->myCurrentImage; ++is, ++n ) {}
      if( is == es )
        {
          _sharedData->myCurrentImage = "";
          _sharedData->myCurrentImageId = -1;
          setChanged();
        }
      else if( _sharedData->myCurrentImageId != n )
        {
          _sharedData->myCurrentImageId = n;
          setChanged();
        }
    }

  if( _sharedData->myCurrentImageId == -1
      && _sharedData->myImageNames.size() != 0 )
    {
      bool foundImage = false ;
      std::set< AObject * > objs = view()->aWindow()->Objects() ;
      std::set< AObject * >::iterator it = objs.begin() ;
      while( it != objs.end() && !foundImage ){
	if( (*it)->type() == AObject::VOLUME ){
	  foundImage = true ;
	  _sharedData->myCurrentImage = (*it)->name() ;
	}
	++it ;
      }

      if( foundImage ){
	int n = 0 ;
	set<string>::iterator itS = _sharedData->myImageNames.begin() ;
	while( itS != _sharedData->myImageNames.end() ){
	  if( _sharedData->myCurrentImage == *itS ){
	    _sharedData->myCurrentImageId = n;
	    setChanged();
	    break ;
	  }
	  ++itS ; ++n ;
	}
      }else{
	_sharedData->myCurrentImage = *( _sharedData->myImageNames.begin() ) ;
	_sharedData->myCurrentImageId = 0 ;
	setChanged() ;
      }
    }

  _sharedData->myImageNamesChanged = false ;
  //cout << _sharedData->myImageNames.size() << endl;

  return _sharedData->myImageNames ;
}

void
RoiManagementAction::regionStats( )
{
  // UNUSED: AGraph * graph = RoiChangeProcessor::instance()->getGraph(0) ;
  Bucket * currentRegion = RoiChangeProcessor::instance()->getCurrentRegion(0) ;
  if( currentRegion ){
     Point3df voxSize = currentRegion->VoxelSize() ;
     cout << "Graph : " << _sharedData->myCurrentGraph << endl << "Region : "
	  << _sharedData->myRegionName << "\tVolume = "
	  << currentRegion->bucket()[0].size()*voxSize[0]*voxSize[1]*voxSize[2] << endl ;
  }

}

set<string>
RoiManagementAction::getCurrentGraphRegions()
{
  #ifdef ANA_DEBUG
  cout << "RoiManagementAction::getCurrentGraphRegions()" << endl ;
#endif

  if( !_sharedData->myCurrentGraphRegionsChanged )
    return _sharedData->myCurrentGraphRegions ;

  AObject * obj = _sharedData->getObjectByName( AObject::GRAPH, _sharedData->myCurrentGraph ) ;
  AGraph * graph = dynamic_cast<AGraph*>( obj ) ;

  _sharedData->myCurrentGraphRegions.clear() ;
  if( !graph )
    return _sharedData->myCurrentGraphRegions ;

  string name ;
  AGraph::iterator iter( graph->begin() ), last( graph->end() ) ;
  while( iter != last )
    {
      AGraphObject * go = dynamic_cast<AGraphObject*>( *iter ) ;
      if( go ){
	go->attributed()->getProperty("name", name ) ;
	_sharedData->myCurrentGraphRegions.insert(name) ;
      }
      ++iter ;
    }

  if( _sharedData->myCurrentGraphId == -1 && _sharedData->myGraphNames.size() != 0 ){
    _sharedData->myCurrentGraph = *( _sharedData->myGraphNames.begin() ) ;
    _sharedData->myCurrentGraphId = 0 ;
    setChanged() ;
  }

  _sharedData->myCurrentGraphRegionsChanged = false ;
  return _sharedData->myCurrentGraphRegions ;
}

set<string>
RoiManagementAction::getCurrentHierarchyRoiNames()
{
  #ifdef ANA_DEBUG
  cout << "RoiManagementAction::getCurrentHierarchyRoiNames()" << endl ;
#endif
  //cout << "BEFORE" <<endl ;
  //printState() ;

  if ( !_sharedData->myCurrentHierarchyRoiNamesChanged )
    return _sharedData->myCurrentHierarchyRoiNames ;

  // map<string, anatomist::Hierarchy*>::iterator found( _sharedData->myHierarchies.find( _sharedData->mySelectedHierarchy ) ) ;

  AObject * obj = _sharedData->getObjectByName( Hierarchy::classType(),
						_sharedData->mySelectedHierarchy ) ;
  Hierarchy * hierarchy = dynamic_cast<Hierarchy*>( obj ) ;

  // Get current hierarchy
  if( !hierarchy )
    return set<string>() ;

  _sharedData->myCurrentHierarchyRoiNames.clear() ;

  recursiveHierarchyNameExtraction( hierarchy->tree().get(),
				    _sharedData->myCurrentHierarchyRoiNames ) ;

  _sharedData->myCurrentHierarchyRoiNamesChanged = false ;

  //cout << "AFTER" <<endl ;
  //printState() ;

  return _sharedData->myCurrentHierarchyRoiNames ;
}

void
RoiManagementAction::recursiveHierarchyNameExtraction( Tree * subtree, set<string>& names )
{
  #ifdef ANA_DEBUG
  cout << "RoiManagementAction::recursiveHierarchyNameExtraction()" << endl ;
#endif

  string name ;
  Tree * t ;
  subtree->getProperty( "name", name ) ;
  if ( name != "" )
    names.insert( name ) ;

  if ( ! subtree->isLeaf( ) )
    {
      for( std::list<BaseTree *>::const_iterator iter = subtree->begin() ; iter != subtree->end() ; ++iter){
	t = dynamic_cast<Tree*>( *iter ) ;
	if( t )
	  recursiveHierarchyNameExtraction( t, names ) ;
      }
    }
}

bool
RoiManagementAction::getSelectedGraphName( string & name )
{
  #ifdef ANA_DEBUG
  cout << "RoiManagementAction::getSelectedGraphName( " << name << " )" << endl ;
#endif

  AGraph * graph = dynamic_cast<AGraph*>( _sharedData->getSelectedObject( AObject::GRAPH ) ) ;
  if( !graph )
    return false ;

  graph->attributed()->getProperty("name", name) ;
  _sharedData->myCurrentGraph = name ;
  return true ;
}

bool
RoiManagementAction::savableGraph()
{
  AObject * obj = _sharedData->getObjectByName( AObject::GRAPH,
						_sharedData->myCurrentGraph ) ;
  if( ! obj )
    return false ;
  AGraph * graph = dynamic_cast<AGraph *>( obj ) ;
  if( ! graph )
    return false ;
  // return ( graph->fileName() != "" ) ;
  /* now an graph with empty filename is saveable but save will
     call saveAs() instead */
  return true;
}

void
RoiManagementAction::selectHierarchy( const string& hieName, int hieId )
{
  #ifdef ANA_DEBUG
  cout << "RoiManagementAction::selectHierarchy( " << hieName << " )" << endl ;
#endif

  AObject * hier = _sharedData->getObjectByName( Hierarchy::classType(),
						 _sharedData->mySelectedHierarchy ) ;

  if( hier ){
#ifdef ANA_DEBUG
    cout << "hiename " << hier->name() << endl ;
    cout << "_sharedData->mySelectedHierarchy = "<< _sharedData->mySelectedHierarchy << endl
	 << "_sharedData->myUserDefinedHierarchy = " << _sharedData->myUserDefinedHierarchy << endl ;
#endif
    if( _sharedData->mySelectedHierarchy == _sharedData->myUserDefinedHierarchy &&
	_sharedData->mySelectedHierarchy != "Free" ){
      int res = QMessageBox::warning( 0,
				RoiManagementActionView::tr("Change Framework ?"),
				RoiManagementActionView::tr("If you change of framework, your \n"
				    "user defined one will be deleted.\n"
							    "Do you want to save it ?"),
				RoiManagementActionView::tr("&Yes"),
				RoiManagementActionView::tr("&No"),
				      QString::null, 0, 1 ) ;
      //cout << "Mess box res = " << res << endl ;
      if( res == 0 )
	saveUDHierarchy() ;
    }
    vector<AObject*>		objv ;
    objv.push_back( hier ) ;
    Command	*cmd = new DeleteObjectCommand( objv );
    theProcessor->execute( cmd );
  }
  _sharedData->mySelectedHierarchy = hieName ;
  _sharedData->mySelectedHierarchyId = hieId ;

  _sharedData->myCurrentHierarchyRoiNamesChanged = true ;

  if( hieName != "Free" ){
    AObject * obj = _sharedData->getObjectByName( Hierarchy::classType(), hieName ) ;
    if ( ! obj ){
      Command	*cmd = new LoadObjectCommand( Path().hierarchy() + "/"
					      + hieName ) ;
      theProcessor->execute( cmd ) ;
      _sharedData->myHierarchyNamesChanged = true ;
    }
  }
  setChanged() ;
  notifyObservers() ;
}

void
RoiManagementAction::loadHierarchy( )
{
  #ifdef ANA_DEBUG
  cout << "RoiManagementAction::loadHierarchy()" << endl ;
#endif

  string filter = ControlWindow::tr( "Hierarchies" ).toStdString()
    + " (*.hie)";
  string caption = RoiManagementActionView::tr(
    "Load Hierarchy" ).toStdString();

  ControlWindow::theControlWindow()->loadObject( filter, caption ) ;

  _sharedData->myHierarchyNamesChanged = true ;

  _sharedData->refreshGraphs() ;

  setChanged() ;
  notifyObservers() ;
}

string
RoiManagementAction::newUDHierarchy( const string& name )
{
  #ifdef ANA_DEBUG
  cout << "RoiManagementAction::newUDHierarchy" << endl ;
#endif

  TreeFactory tf ;
  Hierarchy * hie = new Hierarchy( tf.makeTree("hierarchy") ) ;
  string newUserDefHie = theAnatomist->makeObjectName( name ) ; ;
  hie->setName(newUserDefHie) ;
  hie->setFileName(name) ;
  hie->tree()->setProperty( "graph_syntax", string("RoiArg" ) ) ;
  theAnatomist->registerObject(hie) ;
  //theAnatomist->registerObjectName(name, hie) ;
  #ifdef ANA_DEBUG
  cout << "hiename : "<<  newUserDefHie << endl ;
#endif

  AObject * hier = _sharedData->getObjectByName( Hierarchy::classType(),
						 _sharedData->mySelectedHierarchy ) ;

  if( hier ){
    #ifdef ANA_DEBUG
  cout << "hiename " << hier->name() << endl ;
    cout << "_sharedData->mySelectedHierarchy = "<< _sharedData->mySelectedHierarchy << endl
	 << "_sharedData->myUserDefinedHierarchy = " << _sharedData->myUserDefinedHierarchy << endl ;
#endif
    if( _sharedData->mySelectedHierarchy == _sharedData->myUserDefinedHierarchy &&
	_sharedData->mySelectedHierarchy != "Free" ){
      int res = QMessageBox::warning( 0,
				RoiManagementActionView::tr("Change Framework ?"),
				RoiManagementActionView::tr("If you change of framework, your \n"
				    "user defined one will be deleted.\n"
							    "Do you want to save it ?"),
				RoiManagementActionView::tr("&Yes"),
				RoiManagementActionView::tr("&No"),
				      QString::null, 0, 1 ) ;
      //cout << "Mess box res = " << res << endl ;
      if( res == 0 )
	saveUDHierarchy() ;
    }
    vector<AObject*>		objv ;
    objv.push_back( hier ) ;
    Command	*cmd = new DeleteObjectCommand( objv );
    theProcessor->execute( cmd );
  }

  _sharedData->myUserDefinedHierarchy = newUserDefHie ;
  _sharedData->mySelectedHierarchy = newUserDefHie ;
  _sharedData->mySelectedHierarchyId
    = RoiManagementActionView_Private::USERDEFINED;

  _sharedData->myCurrentHierarchyRoiNamesChanged = true ;

  setChanged() ;
  notifyObservers() ;

  return hie->name() ;
}

string
RoiManagementAction::loadUDHierarchy( const string& hierarchyName )
{
#ifdef ANA_DEBUG
  cout << Settings::localPath() + FileUtil::separator()
    + "frameWorks " + FileUtil::separator() + hierarchyName << endl ;
#endif

  LoadObjectCommand	*cmd4 =
    new LoadObjectCommand( hierarchyName ) ;

  theProcessor->execute( cmd4 ) ;

  AObject * loadedObj = cmd4->loadedObject() ;
  loadedObj->setFileName(
    QFileInfo( hierarchyName.c_str() ).fileName().toStdString() );

  string newUserDefHie =
    theAnatomist->makeObjectName(
      QFileInfo( hierarchyName.c_str() ).fileName().toStdString() );
  loadedObj->setName(newUserDefHie) ;
  theAnatomist->registerObject(loadedObj) ;
  //theAnatomist->registerObjectName(name, hie) ;
#ifdef ANA_DEBUG
  cout << "hiename : "<<  newUserDefHie << endl ;
#endif

  AObject * hier = _sharedData->getObjectByName( Hierarchy::classType(),
						 _sharedData->mySelectedHierarchy ) ;

  if( hier ){
#ifdef ANA_DEBUG
    cout << "hiename " << hier->name() << endl ;
    cout << "_sharedData->mySelectedHierarchy = "<< _sharedData->mySelectedHierarchy << endl
	 << "_sharedData->myUserDefinedHierarchy = " << _sharedData->myUserDefinedHierarchy << endl ;
#endif
    if( _sharedData->mySelectedHierarchy == _sharedData->myUserDefinedHierarchy &&
	_sharedData->mySelectedHierarchy != "Free" ){
      int res = QMessageBox::warning( 0,
				RoiManagementActionView::tr("Change Framework ?"),
				RoiManagementActionView::tr("If you change of framework, your \n"
				    "user defined one will be deleted.\n"
							    "Do you want to save it ?"),
				RoiManagementActionView::tr("&Yes"),
				RoiManagementActionView::tr("&No"),
				      QString::null, 0, 1 ) ;
      //cout << "Mess box res = " << res << endl ;
      if( res == 0 )
	saveUDHierarchy() ;
    }
    vector<AObject*>		objv ;
    objv.push_back( hier ) ;
    Command	*cmd = new DeleteObjectCommand( objv );
    theProcessor->execute( cmd );
  }
  _sharedData->myUserDefinedHierarchy = newUserDefHie ;
  _sharedData->mySelectedHierarchy = _sharedData->myUserDefinedHierarchy ;
  _sharedData->mySelectedHierarchyId
    = RoiManagementActionView_Private::USERDEFINED;

  _sharedData->myCurrentHierarchyRoiNamesChanged = true ;

  setChanged() ;
  notifyObservers() ;

  return _sharedData->myUserDefinedHierarchy ;
}

bool
RoiManagementAction::saveUDHierarchy( )
{
  AObject * obj = _sharedData->getObjectByName( Hierarchy::classType(),
						_sharedData->myUserDefinedHierarchy ) ;
  // std::cerr << "1" << std::endl ;
  Hierarchy * hie = dynamic_cast<Hierarchy*>( obj ) ;
  if( !hie )
    {
      AWarning( "This hierarchy has been deleted !" ) ;
      return false ;
    }
  // std::cerr << "2" << std::endl ;

//   if( hieName->fileName() == "" )
//     saveUDHierarchyAs() ;

  QDir dir( ( Settings::localPath() + FileUtil::separator()
              + "frameworks" ).c_str() );
  // std::cerr << "3" << std::endl ;
  if( !dir.exists() )
    if( ! dir.mkdir(
        string( Settings::localPath() + FileUtil::separator()
        + "frameworks" ).c_str() ) )
      return false ;

  // std::cerr << "4" << std::endl ;
  string hieName = Settings::localPath() + FileUtil::separator()
    + "frameworks" + FileUtil::separator() + hie->fileName() ;

  //std::cerr << "5" << std::endl ;
  hie->save(hieName) ;
//   SaveObjectCommand	*c
//     = new SaveObjectCommand( hie, hieName ) ;
//   theProcessor->execute( c ) ;

  // std::cerr << "6" << std::endl ;
  return true ;
}

void
RoiManagementAction::defineNewFWRegionName(const std::string & name, int red, int green, int blue )
{
  AObject * obj = _sharedData->getObjectByName( Hierarchy::classType(),
						_sharedData->myUserDefinedHierarchy ) ;
  Hierarchy * hie = dynamic_cast<Hierarchy*>( obj ) ;
  if( !hie )
    {
      AWarning( "This hierarchy has been deleted !" ) ;
      return ;
    }

  TreeFactory trFact ;
#ifdef ANA_DEBUG
  cout << hie->tree()->getSyntax() << endl ;
#endif
  Tree * tree = trFact.makeTree("fold_name", false) ;

  tree->setProperty("name", name) ;
  vector<int> color(3) ;
  color[0] = red ;
  color[1] = green ;
  color[2] = blue ;

  tree->setProperty("color", color) ;
  hie->tree()->insert(tree) ;

  _sharedData->myCurrentHierarchyRoiNamesChanged = true ;

  setChanged() ;
  notifyObservers() ;
}

void
RoiManagementAction::deleteFWRegionName(const std::string & name )
{
  AObject * obj = _sharedData->getObjectByName( Hierarchy::classType(),
						_sharedData->myUserDefinedHierarchy ) ;
  Hierarchy * hie = dynamic_cast<Hierarchy*>( obj ) ;
  if( !hie )
    {
      AWarning( "This hierarchy has been deleted !" ) ;
      return ;
    }

  TreeFactory trFact ;
#ifdef ANA_DEBUG
  cout << hie->tree()->getSyntax() << endl ;
#endif
  set<Tree*> toDelete = hie->tree()->getElementsWith("name", name) ;

  for( set<Tree*>::iterator it = toDelete.begin() ; it != toDelete.end() ; ++it )
    hie->tree()->remove( *it ) ;

  _sharedData->myCurrentHierarchyRoiNamesChanged = true ;

  setChanged() ;
  notifyObservers() ;
}

void
RoiManagementAction::modifyUDFWRegionName(const std::string & oldName, const std::string & newName )
{
  if( _sharedData->mySelectedHierarchy != _sharedData->myUserDefinedHierarchy )
    return ;

  AObject * obj = _sharedData->getObjectByName( Hierarchy::classType(),
						_sharedData->myUserDefinedHierarchy ) ;
  Hierarchy * hie = dynamic_cast<Hierarchy*>( obj ) ;
  if( !hie )
    {
      AWarning( "This hierarchy has been deleted !" ) ;
      return ;
    }

  set<Tree*> st = hie->tree()->getElementsWith("name", oldName ) ;
  for(set<Tree*>::iterator it = st.begin() ; it != st.end(); ++it )
    (*it)->setProperty("name", newName) ;
}

void
RoiManagementAction::modifyUDFWRegionColor( const std::string & name,
					    int red, int green, int blue )
{
  if( _sharedData->mySelectedHierarchy != _sharedData->myUserDefinedHierarchy )
    return ;

  AObject * obj = _sharedData->getObjectByName( Hierarchy::classType(),
						_sharedData->myUserDefinedHierarchy ) ;
  Hierarchy * hie = dynamic_cast<Hierarchy*>( obj ) ;
  if( !hie )
    {
      AWarning( "This hierarchy has been deleted !" ) ;
      return ;
    }

  vector<int> col(3) ;
  col[0] = red ; col[1] = green ; col[2] = blue ;
  set<Tree*> st = hie->tree()->getElementsWith("name", name ) ;
  for(set<Tree*>::iterator it = st.begin() ; it != st.end(); ++it )
    (*it)->setProperty("color", col) ;
}

// bool
// RoiManagementAction::saveUDHierarchyAs()
// {
//   AObject * obj = _sharedData->getObjectByName( AObject::HIERARCHY,
// 						_sharedData->myUserDefinedHierarchy ) ;
//   Hierarchy * hie = dynamic_cast<Hierarchy*>( obj ) ;
//   if( !hie )
//     {
//       AWarning( "This hierarchy has been deleted !" ) ;
//       return ;
//     }

//   set<AObject*> toSave ;
//   toSave.insert( hie ) ;
//   string fileName =
//     ObjectActions::specificSaveStatic( toSave,
// 				       string( ( const char * )
//                                                (ControlWindow::tr( "Hierarchies" )
//                                                 + " (*.hie)" ) ),
// 				       string( "Save User Defined Framework" ) ) ;
// }

void
RoiManagementAction::selectGraph( const string & graphName, int graphId  )
{
  #ifdef ANA_DEBUG
  cout << "RoiManagementAction::selectGraph( " << graphName << " )" << endl ;
#endif

  if ( _sharedData->myCurrentGraph == graphName && _sharedData->myCurrentGraphId == graphId )
    return ;

  _sharedData->myCurrentGraph = graphName ;
  _sharedData->myCurrentGraphId = graphId ;

  _sharedData->myCurrentGraphRegionsChanged = true ;
  getCurrentGraphRegions() ;

  if( _sharedData->myCurrentGraphRegions.size() != 0 )
    selectRegion( *( _sharedData->myCurrentGraphRegions.begin() ), 0 ) ;
  else
    selectRegion( "", -1 ) ;

  RoiChangeProcessor::instance()->setUndoable( RoiChangeProcessor::instance()->undoable() ) ;
  setChanged() ;
  notifyObservers() ;
}

// void
// RoiManagementAction::setGraphName( const string& name )
// {
//   _sharedData->myGraphName = name ;
// }

void
RoiManagementAction::newGraph( const string& /* name */ )
{
  //cout << "newGraph( " << name << " )" << endl ;

  AObject * obj = _sharedData->getObjectByName( -1,
                                                _sharedData->myCurrentImage );

  if(! obj )
    {
      AWarning("An Image must be selected, or selected image no more existing") ;
      return ;
    }
  string graphName
    = theAnatomist->makeObjectName( FileUtil::removeExtension(obj->name())
                                    + "_ROI.arg" );

  Command	*cmd = new CreateGraphCommand( obj, graphName, "RoiArg" ) ;
  theProcessor->execute( cmd ) ;

  obj = _sharedData->getObjectByName( AObject::GRAPH, graphName ) ;
  AGraph * graph = dynamic_cast<AGraph*>( obj ) ;

  if ( !graph) {
    AWarning("Major bug : graph has not been created !") ;
    return ;
  }

  //graph->setFileName( graphName + ".arg" ) ;
/*  graph->setFileName( FileUtil::removeExtension(obj->name())
		      + ".arg" );*/
  graph->setName( graphName ) ;
  theAnatomist->registerObjectName( graph->name(), graph ) ;
  graph->setLabelsVolumeDimension( static_cast<int>( graph->MaxX2D()
						     - graph->MinX2D() ) + 1,
				   static_cast<int>( graph->MaxY2D()
						     - graph->MinY2D() ) + 1,
				   static_cast<int>( graph->MaxZ2D()
						     - graph->MinZ2D() ) + 1 ) ;
  graph->volumeOfLabels(0) ;

  theAnatomist->NotifyObjectChange( graph ) ;

  _sharedData->myGraphNamesChanged = true ;
  getGraphNames() ;

  set<string>::const_iterator iterName( _sharedData->myGraphNames.begin() ),
    lastName( _sharedData->myGraphNames.end() ) ;
  int id = 0 ;
  while ( iterName != lastName )
    {
      if ( *iterName == graphName )
	break ;
      ++id ;
      ++iterName ;
    }

  if ( iterName != lastName ){
    _sharedData->myGraphName = graphName ;
    selectGraph( _sharedData->myGraphName, id ) ;
    //cout << "graph selected " << _sharedData->myGraphName << endl ;
  }
  // If no hierarchy is loaded, load neuronames.hie
  set<AObject *> objs = theAnatomist->getObjects() ;
  set<AObject *>::iterator iter( objs.begin() ), last( objs.end() ), found ;

  int objCount = 0 ;
  while ( iter != last )
    {
      if( (*iter)->type() == Hierarchy::classType() ){
	++objCount ;
	found = iter ;
	break ;
      }
      ++iter ;
    }

  if( objCount == 0 ){
    Command	*cmd4 = new LoadObjectCommand( Path().hierarchy()
					       + "/neuronames.hie" ) ;
    theProcessor->execute( cmd4 ) ;
    _sharedData->myHierarchyNamesChanged = true ;
    _sharedData->refreshGraphs() ;
  }

  setChanged() ;
  notifyObservers() ;

  objs = view()->aWindow()->Objects() ;
  iter = objs.begin(), last = objs.end() ;
  set<AObject*>::iterator foundGraph = last, foundVolume = last, obj2d = last;

  while( iter != last ) {
    if( (*iter)->type() == AObject::GRAPH ){
      foundGraph = iter ;
    }
    if( (*iter)->type() == AObject::VOLUME ){
      foundVolume = iter ;
    }
    else if( (*iter)->Is2DObject() )
      obj2d = iter;
    ++iter ;
  }

  if( foundVolume == last && obj2d != last )
    foundVolume = obj2d;

  if ( foundGraph == last && foundVolume != last){
    if( fabs( (*foundVolume)->VoxelSize()[0] - graph->VoxelSize()[0] )
        > .000001 ||
        fabs( (*foundVolume)->VoxelSize()[1] - graph->VoxelSize()[1] )
        > .000001 ||
        fabs( (*foundVolume)->VoxelSize()[1] - graph->VoxelSize()[1] )
        > .000001 )
    {
      AWarning("Incompatible voxel size !") ;
      cout << "Image Voxel Size - Graph Voxel Size = "
           << (*foundVolume)->VoxelSize() - graph->VoxelSize() << endl ;
      return ;
    }

    if( (*foundVolume)->MinX2D() != graph->MinX2D() ||
        (*foundVolume)->MinY2D() != graph->MinY2D() ||
        (*foundVolume)->MinZ2D() != graph->MinZ2D() ||
        (*foundVolume)->MaxX2D() != graph->MaxX2D() ||
        (*foundVolume)->MaxY2D() != graph->MaxY2D() ||
        (*foundVolume)->MaxZ2D() != graph->MaxZ2D() )
    {
      AWarning("Incompatible bounding box !") ;
      return ;
    }

    // There's no roi graph in this RoiManagementAction associated window. Include one.
    objs.clear() ;
    objs.insert( graph ) ;

    set<AWindow*> wins ;
    wins.insert( view()->aWindow() ) ;

    Command	*cmd2 = new AddObjectCommand( objs, wins ) ;
    theProcessor->execute( cmd2 ) ;

    Command	*cmd3 = new SetControlCommand( wins, "PaintControl" ) ;
    theProcessor->execute( cmd3 ) ;
  }
}

void
RoiManagementAction::selectImage( const string& imageName, int id )
{
  #ifdef ANA_DEBUG
  cout << "RoiManagementAction::selectImage( " << imageName << " )" << endl ;
#endif

  _sharedData->myCurrentImage = imageName ;
  _sharedData->myCurrentImageId = id ;

  setChanged() ;
  notifyObservers() ;
}

void
RoiManagementAction::refresh()
{
  _sharedData->refresh();
}

void
RoiManagementAction::renameGraph( const string& name, int id )
{
  //cout << "renameGraph( " << name << " )" << endl ;
  if( name == "" )
    return ;

  AObject * obj = _sharedData->getObjectByName(AObject::GRAPH, _sharedData->myCurrentGraph ) ;
  AGraph * graph = dynamic_cast<AGraph*>( obj ) ;

  _sharedData->myCurrentGraphId = id ;

  if ( !graph )
    {
      AWarning("Select a graph to rename, or graph selected no more existing") ;
      return ;
    }

  PythonHeader dummy ;
  theAnatomist->unregisterObjectName( graph->name() ) ;
  //graph->setFileName( dummy.removeExtension(name) + ".arg" ) ;
  graph->setFileName( "" );
  graph->setName( theAnatomist->makeObjectName( name ) ) ;
  theAnatomist->registerObjectName( graph->name(), graph ) ;

  _sharedData->myCurrentGraph = graph->name() ;

  theAnatomist->NotifyObjectChange( graph ) ;
  _sharedData->myGraphNamesChanged = true ;

  setChanged() ;
  notifyObservers() ;
}

void
RoiManagementAction::deleteGraph( )
{
  //cout << "deleteGraph( )" << endl ;


  AObject * obj = _sharedData->getObjectByName(AObject::GRAPH, _sharedData->myCurrentGraph ) ;
  AGraph * graph = dynamic_cast<AGraph*>( obj ) ;

  if ( !graph )
    {
      AWarning("No graph selected") ;
      return ;
    }

  vector<AObject*>		objv ;
  objv.push_back( graph ) ;
  Command	*cmd = new DeleteObjectCommand( objv );
  theProcessor->execute( cmd );

  _sharedData->myGraphNamesChanged = true ;
  getGraphNames() ;

  if ( _sharedData->myGraphNames.size() != 0 )
    selectGraph( *( _sharedData->myGraphNames.begin() ), 0 ) ;
  else{
    _sharedData->myCurrentGraph = "" ;
    _sharedData->myCurrentGraphId = -1 ;
  }

  _sharedData->myCurrentGraphRegionsChanged = true ;

  setChanged() ;
  notifyObservers() ;
}

void
RoiManagementAction::loadGraph( const QStringList& filenames )
{
  //cout << "loadGraph()" << endl ;

  LoadObjectCommand *command = 0 ;
  AObject	* loadedObj = 0 ;

  Object	options = Object::value( Dictionary() );
  Dictionary	rot;
  ObjectVector	dt;
  dt.push_back( Object::value( string( "all" ) ) );
  rot[ "Graph" ] = Object::value( dt );
  rot[ "__syntax__" ] = Object::value( string( "dictionary" ) ); // temp (bug)
  options->setProperty( "restrict_object_types", rot );
  options->setProperty( "__syntax__", string( "dictionary" ) ); // temp (bug)

  for ( QStringList::ConstIterator it = filenames.begin();
        it != filenames.end(); ++it ) {
    command = new LoadObjectCommand( (*it).toStdString(), -1, "", false, options );
    theProcessor->execute( command );
    loadedObj = command->loadedObject() ;
    if( loadedObj )
      loadedObj->setFileName( (*it).toStdString() ) ;
  }
  if( !command )
    return ;

  if(! loadedObj)
    return ;

  theAnatomist->getControlWindow()->UnselectAllObjects();

  _sharedData->myGraphNamesChanged = true ;
  _sharedData->myCurrentGraphRegionsChanged = true ;

  _sharedData->refreshGraphs() ;

  getGraphNames() ;

  int newId = 0 ;
  std::set<string>::const_iterator iterName( _sharedData->myGraphNames.begin() ),
    lastName( _sharedData->myGraphNames.end() ) ;
  while( iterName != lastName ){
    if( *iterName == loadedObj->name() )
      break ;

    ++iterName ; ++newId ;
  }

  _sharedData->myCurrentGraph = "" ;
  _sharedData->myCurrentGraphId = 0 ;
  selectGraph( loadedObj->name(), newId ) ;


  // If no hierarchy is loaded, load neuronames.hie
  set<AObject *> objs = theAnatomist->getObjects() ;
  set<AObject *>::iterator iter( objs.begin() ), last( objs.end() ), found ;

  int objCount = 0 ;
  while ( iter != last )
    {
      if( (*iter)->type() == Hierarchy::classType() )
      {
	++objCount ;
	found = iter ;
	break ;
      }
      ++iter ;
    }

  if( objCount == 0 ){
    Command	*cmd4 = new LoadObjectCommand( Path().hierarchy()
					       + "/neuronames.hie" ) ;
    theProcessor->execute( cmd4 ) ;
    _sharedData->myHierarchyNamesChanged = true ;
    _sharedData->refreshGraphs() ;
  }

  AObject * obj = _sharedData->getObjectByName(AObject::GRAPH, _sharedData->myCurrentGraph ) ;
  AGraph * gr = dynamic_cast<AGraph*>( obj ) ;
#ifdef ANA_DEBUG
  cout << "gr = " << gr << endl ;
#endif
  if( gr){
#ifdef ANA_DEBUG
  cout << "Graph exists" << endl ;
#endif
    AGraph::iterator iterG( gr->begin() ), lastG( gr->end() ) ;
    string regionName ;
    int roiLabel ;
    while( iterG != lastG )
      {
        AGraphObject * go = dynamic_cast<AGraphObject*>( *iterG ) ;
        if( go ){
          if( go->attributed()->getProperty("name", regionName ) ){
#ifdef ANA_DEBUG
            cout << "go " << regionName << " exists !" << endl ;
#endif
            if( regionName == "unknown" || regionName == "Unknown" )
            {
              if( go->attributed()->getProperty("roi_label", roiLabel) ){
                go->attributed()->setProperty( "name", toString(roiLabel) ) ;
#ifdef ANA_DEBUG
  cout << "roi_label = " << roiLabel << endl ;
#endif
              }
              else
                cout << "No attribute roi label" << endl ;
            }
          }
        }
        ++iterG ;
      }
  }

  refresh( ) ;
  setChanged() ;
  notifyObservers() ;

  objs = view()->aWindow()->Objects() ;
  iter = objs.begin(), last = objs.end() ;
  set<AObject*>::iterator foundGraph = last, foundVolume = last, obj2d = last ;

  while( iter != last ) {
    if( (*iter)->type() == AObject::GRAPH ){
      foundGraph = iter ;
    }
    if( (*iter)->type() == AObject::VOLUME ){
      foundVolume = iter ;
    }
    else if( (*iter)->Is2DObject() )
      obj2d = iter;
    ++iter ;
  }
  if( foundVolume == last && obj2d != last )
    foundVolume = obj2d;

  if ( foundGraph == last && foundVolume != last){
    if( (*foundVolume)->VoxelSize()[0] - loadedObj->VoxelSize()[0] > .000001 ||
	(*foundVolume)->VoxelSize()[1] - loadedObj->VoxelSize()[1] > .000001 ||
	(*foundVolume)->VoxelSize()[1] - loadedObj->VoxelSize()[1] > .000001 ){
      AWarning("Incompatible voxel size !") ;
      cout << "Image Voxel Size - Graph Voxel Size = "
	   << (*foundVolume)->VoxelSize() - loadedObj->VoxelSize() << endl ;
      return ;
    }

    if( (*foundVolume)->MinX2D() != loadedObj->MinX2D() ||
	(*foundVolume)->MinY2D() != loadedObj->MinY2D() ||
	(*foundVolume)->MinZ2D() != loadedObj->MinZ2D() ||
	(*foundVolume)->MaxX2D() != loadedObj->MaxX2D() ||
	(*foundVolume)->MaxY2D() != loadedObj->MaxY2D() ||
	(*foundVolume)->MaxZ2D() != loadedObj->MaxZ2D() ){
      AWarning("Incompatible bounding box !") ;
      cout << "Volume ( " << (*foundVolume)->MinX2D() << " , "
	   << (*foundVolume)->MinY2D() << " , "
	   << (*foundVolume)->MinZ2D() << " ) , ( "
	   << (*foundVolume)->MaxX2D() << " , "
	   << (*foundVolume)->MaxY2D() << " , "
	   << (*foundVolume)->MaxZ2D() << " ) " << endl ;
      cout << "Graph ( " << loadedObj->MinX2D() << " , "
	   << loadedObj->MinY2D() << " , "
	   << loadedObj->MinZ2D() << " ) , ( "
	   << loadedObj->MaxX2D() << " , "
	   << loadedObj->MaxY2D() << " , "
	   << loadedObj->MaxZ2D() << " ) " << endl ;
      return ;
    }
  }

  // Include roi graph in this RoiManagementAction associated window
  objs.clear() ;
  objs.insert( loadedObj ) ;

  set<AWindow*> wins ;
  wins.insert( view()->aWindow() ) ;

  Command	*cmd2 = new AddObjectCommand( objs, wins, false, true );
  theProcessor->execute( cmd2 ) ;

  Command	*cmd3 = new SetControlCommand( wins, "PaintControl" );
  theProcessor->execute( cmd3 ) ;
}

void
RoiManagementAction::saveGraphAs( )
{
  #ifdef ANA_DEBUG
  cout << "saveGraphAs()" << endl ;
#endif

  AObject * obj = _sharedData->getObjectByName(AObject::GRAPH, _sharedData->myCurrentGraph ) ;
  AGraph * graph = dynamic_cast<AGraph*>( obj ) ;

  if( !graph )
    {
      AWarning( "This ROI graph doesn't exist any more !" ) ;
      return ;
    }
  set<AObject*> toSave ;
  toSave.insert( graph ) ;
  string fileName =
    ObjectActions::specificSaveStatic(
      toSave, ControlWindow::tr( "Graphs" ).toStdString() + " (*.arg)",
      ControlWindow::tr( "Save ROI Graph" ).toStdString() );
}

void
RoiManagementAction::reloadGraph( )
{
  //To do
}

void
RoiManagementAction::saveGraph( )
{
  AObject * obj = _sharedData->getObjectByName(AObject::GRAPH,
                                               _sharedData->myCurrentGraph ) ;
  AGraph * graph = dynamic_cast<AGraph*>( obj ) ;
  if( !graph )
    {
      AWarning( "This ROI graph doesn't exist any more !" ) ;
      return ;
    }

  if(  graph->fileName() == "" )
  {
    saveGraphAs();
    return;
  }

  cout << "save graph filename: " << graph->fileName() << endl;
  SaveObjectCommand	*c
    = new SaveObjectCommand( graph, graph->fileName() );
  theProcessor->execute( c );
}

void
RoiManagementAction::selectRegion( const string& regionName, int id )
{
//   cout << "selectRegion( " << regionName << " )" << endl ;
  set<AObject*> objSet ;

  if( id != -1 )
  {
    AGraphObject * graphObject
      = _sharedData->getGraphObjectByName( _sharedData->myCurrentGraph,
                                           regionName ) ;
    if( ! graphObject )
    {
      AWarning( "No such region exists" ) ;
      return ;
    }
    objSet.insert( graphObject ) ;
  }

  Command	*cmd = new SelectCommand( objSet );
  theProcessor->execute( cmd );

  _sharedData->myCurrentRegionId = id ;

  setChanged() ;
  notifyObservers() ;
}

void
RoiManagementAction::selectRegionName( const string& regionName )
{
  //cout << "selectRegionName( " << regionName << " )" << endl ;

  _sharedData->myRegionName = regionName ;
}

void
RoiManagementAction::smartSelectRegionName( const string & partialRegionName )
{
  //cout << "partialRegionName( " << partialRegionName << " )" << endl ;

  _sharedData->myPartialRegionName = partialRegionName ;
  //...
}

void
RoiManagementAction::newRegion( const string& name )
{
  AObject * obj = _sharedData->getObjectByName(AObject::GRAPH, _sharedData->myCurrentGraph ) ;
  AGraph * graph = dynamic_cast<AGraph*>( obj ) ;

  if( !graph )
    {
      AWarning("Current graph no more existing !") ;
      return ;
    }

  if( name == "" || name == "Unknown" )
    {
      AWarning("Please specify a name !") ;
      return ;
    }

  if( _sharedData->getGraphObjectByName( _sharedData->myCurrentGraph, name ) ){
    AWarning("Such a region name already exists, please select another name") ;
    return ;
  }

  _sharedData->myRegionName = name ;
  Command	*cmd = new AddNodeCommand( graph, _sharedData->myRegionName );
  theProcessor->execute( cmd );

  _sharedData->myCurrentGraphRegionsChanged = true ;
  getCurrentGraphRegions() ;

  set<string>::const_iterator iter( _sharedData->myCurrentGraphRegions.begin() ),
    last( _sharedData->myCurrentGraphRegions.end() ) ;
  int id = 0 ;
  while ( iter != last )
    {
      if ( *iter == _sharedData->myRegionName )
	break ;
      ++id ;
      ++iter ;
    }

  if ( iter != last )
    _sharedData->myCurrentRegionId = id ;

  selectRegion( _sharedData->myRegionName, _sharedData->myCurrentRegionId ) ;

  setChanged() ;
  notifyObservers() ;
}

void
RoiManagementAction::renameRegion( const string & name, int id )
{
  //cout << "renameRegion( " << name << " )" << endl ;

  if ( name == "" ) return ;
  if( _sharedData->getGraphObjectByName( _sharedData->myCurrentGraph, name ) ){
    AWarning("Such a region name already exists, please select another name") ;
    return ;
  }

  AObject * obj = _sharedData->getSelectedObject( AObject::GRAPHOBJECT ) ;
  AGraphObject * graphObject = dynamic_cast<AGraphObject*>( obj ) ;

  obj = _sharedData->getObjectByName(AObject::GRAPH, _sharedData->myCurrentGraph ) ;
  AGraph * graph = dynamic_cast<AGraph*>( obj ) ;

  _sharedData->myCurrentRegionId = id ;

  if( graphObject ){
    theAnatomist->unregisterObjectName( graphObject->name() ) ;
    graphObject->setName( theAnatomist->makeObjectName( name ) ) ;
    theAnatomist->registerObjectName( graphObject->name(), graphObject ) ;
    graphObject->attributed()->setProperty("name", name ) ;
    _sharedData->myRegionName = name ;

    selectRegion( _sharedData->myRegionName, _sharedData->myCurrentRegionId ) ;
    theAnatomist->NotifyObjectChange( graph ) ;
    theAnatomist->NotifyObjectChange( graphObject ) ;
  }

  _sharedData->myCurrentGraphRegionsChanged = true ;

  getCurrentGraphRegions() ;

  set<string>::const_iterator iter( _sharedData->myCurrentGraphRegions.begin() ),
    last( _sharedData->myCurrentGraphRegions.end() ) ;

  int newId = 0 ;
  while ( iter != last )
    {
      if ( *iter == _sharedData->myRegionName )
	break ;
      ++newId ;
      ++iter ;
    }

  if ( iter != last )
    _sharedData->myCurrentRegionId = newId ;

  selectRegion( _sharedData->myRegionName, _sharedData->myCurrentRegionId ) ;

  setChanged() ;
  notifyObservers() ;
}

void
RoiManagementAction::deleteRegion( )
{
  //cout << "deleteRegion()" << endl ;

  AObject * obj = _sharedData->getSelectedObject( AObject::GRAPHOBJECT ) ;
  AGraphObject * graphObject = dynamic_cast<AGraphObject*>( obj ) ;
  AGraph * gr = RoiChangeProcessor::instance()->getGraph(0) ;

  if( !graphObject ){
    AWarning("Select the region you want to delete !") ;
    return ;
  }

  // First, erase region in volume of labels
  if(!gr)
    return ;

  Bucket * currentRegion = RoiChangeProcessor::instance()->getCurrentRegion(0) ;
  if(currentRegion){
    list< pair< Point3d, ChangesItem> >* changes = new list< pair< Point3d, ChangesItem> >;
    BucketMap<Void>::Bucket::iterator
      iter( currentRegion->bucket()[0].begin() ),
      last( currentRegion->bucket()[0].end() ) ;

    AimsData<AObject*>& labels( gr->volumeOfLabels() ) ;

    if( labels.dimX() != ( gr->MaxX2D() - gr->MinX2D() + 1 ) ||
	labels.dimY() != ( gr->MaxY2D() - gr->MinY2D() + 1 ) ||
	labels.dimZ() != ( gr->MaxZ2D() - gr->MinZ2D() + 1 ) ){
      gr->clearLabelsVolume() ;
      gr->setLabelsVolumeDimension( static_cast<int>( gr->MaxX2D() - gr->MinX2D() ) + 1,
				    static_cast<int>( gr->MaxY2D() - gr->MinY2D() ) + 1,
				    static_cast<int>( gr->MaxZ2D() - gr->MinZ2D() ) + 1 ) ;
    }

    AimsData<AObject*>& volOfLabels( gr->volumeOfLabels() ) ;

    while ( iter != last){
      ChangesItem item ;
      item.after = 0 ;
      item.before = graphObject ;
      changes->push_back(pair<Point3d, ChangesItem>( iter->first,  item ) )  ;

      volOfLabels( iter->first ) = 0  ;
      ++iter ;
    }

    if ( ! (*changes).empty() )
      RoiChangeProcessor::instance()->applyChange( changes ) ;
  }
  // Now, destroy region
  GenericObject	*ao = graphObject->attributed();
  vector<AObject*>		objv ;
  objv.push_back( graphObject ) ;

  Command	*cmd = new DeleteObjectCommand( objv ) ;
  theProcessor->execute( cmd ) ;

  obj = _sharedData->getObjectByName(AObject::GRAPH,
				     _sharedData->myCurrentGraph ) ;
  AGraph * graph = dynamic_cast<AGraph*>( obj ) ;

  // delete Graph vertex
  Vertex	*v = dynamic_cast<Vertex *>( ao );
  if( v && graph )
    ((Graph *) graph->attributed())->removeVertex( v );

  theAnatomist->NotifyObjectChange( graph ) ;

  _sharedData->myCurrentGraphRegionsChanged = true ;
  _sharedData->myCurrentRegionId = -1 ;

  getCurrentGraphRegions() ;

  if( _sharedData->myCurrentGraphRegions.size() != 0 )
    selectRegion( *( _sharedData->myCurrentGraphRegions.begin() ), 0 ) ;

  setChanged() ;
  notifyObservers() ;
}

void
RoiManagementAction::exportAsMask( )
{
  //cout << "exportAsMask()" << endl ;

  AObject * obj = _sharedData->getSelectedObject( AObject::GRAPHOBJECT ) ;
  AGraphObject * graphObject = dynamic_cast<AGraphObject*>( obj ) ;

  if( !graphObject ){
    AWarning("Select the region you want to export !") ;
    return ;
  }

  exportRegion( graphObject ) ;
}

void
RoiManagementAction::exportRegion( AGraphObject * o)
{
  //cout << "exportRegion()" << endl ;

  if( !o )
    {
      cerr << "exportRegion : object is not a graph object\n";
      return;
    }

  AObject::ParentList	pl = o->Parents();
  AObject::ParentList::iterator	ip, ep = pl.end();
  AGraph * gra = 0;

  for( ip=pl.begin(); ip!=ep && !gra; ++ip )
    gra = dynamic_cast<AGraph *>( *ip );

  vector<int>	bbmax;
  if( gra )
    gra->attributed()->getProperty( "boundingbox_max", bbmax );

  #ifdef ANA_DEBUG
  cout << "Bounding box : " << bbmax[0] << " , " << bbmax[1]<< " , " << bbmax[2] << endl ;
#endif

  while( bbmax.size() < 3 )
    bbmax.push_back( 0 );

  MObject::iterator	io, eo = o->end();
  Bucket		*bck;
  AimsData<int16_t>	*vol = 0;

  for( io=o->begin(); io!=eo; ++io )
    {
      bck = dynamic_cast<Bucket *>( *io );
      if( bck )
	{
	  Converter<BucketMap<Void>, AimsData<int16_t> > conv;
	  if( !vol )
	    {
	      vol = new AimsData<int16_t>( bbmax[0] + 1, bbmax[1] + 1,
					   bbmax[2] + 1,
					   bck->bucket().size() );
	      conv.convert( bck->bucket(), *vol );
	    }
	  else
	    conv.convert( bck->bucket(), *vol );
	}
    }

  if( vol )
    {
      AVolume<int16_t>	avol = *vol;
      vector<int> dim ;
      dim.push_back(avol.volume()->getSizeX()) ;
      dim.push_back(avol.volume()->getSizeY()) ;
      dim.push_back(avol.volume()->getSizeZ()) ;
      avol.attributed()->setProperty( "volume_dimension", dim ) ;
      // take care of SPM origin/orientation properties
      vector<float>	org;
      try
      {
        Object
        ob = gra->attributed()->getProperty( "transformations" );
        avol.attributed()->setProperty( "transformations", ob );
        ob = gra->attributed()->getProperty( "referentials" );
        avol.attributed()->setProperty( "referentials", ob );
      }
      catch( ... )
      {
      }
      // old-style Analyze
      if( gra->attributed()->getProperty( "origin", org ) )
        avol.attributed()->setProperty( "origin", org );
      try
        {
          Object
            ob = gra->attributed()->getProperty( "spm_radio_convention" );
          bool n = ob->getScalar();
          avol.attributed()->setProperty( "spm_radio_convention", n );
        }
      catch( ... )
        {
        }
      try
        {
          Object
            ob = gra->attributed()->getProperty( "spm_normalized" );
          bool n = ob->getScalar();
          avol.attributed()->setProperty( "spm_normalized", n );
        }
      catch( ... )
        {
        }
      try
        {
          Object
            ob = gra->attributed()->getProperty( "spm_spm2_normalization" );
          bool n = ob->getScalar();
          avol.attributed()->setProperty( "spm_spm2_normalization", n );
        }
      catch( ... )
        {
        }

      theAnatomist->registerObject( &avol, false );
      set<AObject *>	so;
      so.insert( &avol );
      ObjectActions::saveStatic( so );
    }
}

void
RoiManagementAction::regionsFusion( const set<string>& regions,
				    const string& newName )
{
  newRegion("tmp0000") ;
  AGraphObject * region = _sharedData->getGraphObjectByName( _sharedData->myCurrentGraph,
							     "tmp0000" ) ;
  if( !region )
    return ;

  Bucket		*fusionBk = 0;
  AGraphObject::iterator	ic, ec = region->end();

  for( ic=region->begin(); ic!=ec; ++ic )
    if( ( fusionBk = dynamic_cast<Bucket *>( *ic ) ) )
      break;

  if( !fusionBk)
    return ;
  AObject * obj = _sharedData->getObjectByName( AObject::GRAPH,
						_sharedData->myCurrentGraph ) ;
  AGraph * graph = dynamic_cast<AGraph*>( obj ) ;
  if (!graph)
    return ;

  set<string>::const_iterator iter( regions.begin() ), last( regions.end() ) ;
  AGraphObject * grao ;
  while( iter !=last )
    {
      if( (grao = _sharedData->getGraphObjectByName( _sharedData->myCurrentGraph, *iter )) ){
	Bucket		*bk = 0;
	AGraphObject::iterator	ic, ec = grao->end();
	
	for( ic=grao->begin(); ic!=ec; ++ic )
	  if( ( bk = dynamic_cast<Bucket *>( *ic ) ) )
	    break;
	
	if( bk )
	  fusionBk->insert( bk->bucket() ) ;

	GenericObject	*ao = grao->attributed();

	vector<AObject*>		objv ;
	objv.push_back( grao ) ;
	
	Command	*cmd = new DeleteObjectCommand( objv ) ;
	theProcessor->execute( cmd ) ;
	
	
	// delete Graph vertex
	Vertex	*v = dynamic_cast<Vertex *>( ao );
	if( v )
	  ((Graph *) graph->attributed())->removeVertex( v );
	
	theAnatomist->NotifyObjectChange( graph ) ;
	
      }
      ++iter ;
    }

  theAnatomist->unregisterObjectName( region->name() ) ;
  region->setName( theAnatomist->makeObjectName( newName ) ) ;
  theAnatomist->registerObjectName( region->name(), region ) ;
  region->attributed()->setProperty("name", newName ) ;
  theAnatomist->NotifyObjectChange( region ) ;

  theAnatomist->NotifyObjectChange( graph ) ;

  _sharedData->myCurrentGraphRegionsChanged = true ;
  _sharedData->myCurrentRegionId = -1 ;

  getCurrentGraphRegions() ;

  if( _sharedData->myCurrentGraphRegions.size() != 0 )
    selectRegion( *( _sharedData->myCurrentGraphRegions.begin() ), 0 ) ;

  PaintActionSharedData::instance()->noMoreUndoable() ;

  setChanged() ;
  notifyObservers() ;
}

void
RoiManagementAction::createWindow( const string& type )
{
  AObject * graph
    = _sharedData->getObjectByName( AObject::GRAPH,
                                    _sharedData->myCurrentGraph );

  AObject * volume
    = _sharedData->getObjectByName( AObject::VOLUME,
                                    _sharedData->myCurrentImage );

  AObject * fusion2D
    = _sharedData->getObjectByName( AObject::FUSION2D,
                                    _sharedData->myCurrentImage );


  CreateWindowCommand	*cmd = new CreateWindowCommand( type ) ;
  theProcessor->execute( cmd ) ;
  AWindow * win = cmd->createdWindow() ;

  set<AWindow*> wins ;
  wins.insert( win ) ;

  if( view()->aWindow()->Group() != 0 )
  {
    Command *cmd1 = new LinkWindowsCommand( wins, view()->aWindow()->Group() );
    theProcessor->execute( cmd1 );
  }

  set<AObject*> objs ;
  Command *cmd2;
  if( graph )
    objs.insert(graph) ;
  else {
    set<AObject*> winObj = view()->aWindow()->Objects() ;
    for( set<AObject*>::iterator it = winObj.begin() ; it != winObj.end() ; ++it )
      if( (*it)->type() == AObject::GRAPH )
	objs.insert( *it ) ;
  }
  if( volume || fusion2D ){
    if( volume )
      objs.insert(volume) ;
    if( fusion2D )
      objs.insert( fusion2D ) ;
  } else {
    set<AObject*> winObj = view()->aWindow()->Objects() ;
    for( set<AObject*>::iterator it = winObj.begin() ; it != winObj.end() ; ++it )
      if( (*it)->type() == AObject::VOLUME  || (*it)->type() == AObject::FUSION2D )
	objs.insert( *it ) ;
  }
  cmd2 = new AddObjectCommand( objs, wins, false, true ) ;
  theProcessor->execute( cmd2 ) ;

  Command	*cmd3 = new SetControlCommand( wins, "PaintControl" ) ;
  theProcessor->execute( cmd3 ) ;
}


AObject *
RoiManagementActionSharedData::getObjectByName( int objType, const string& name ) const
{
  set<AObject *> objs = theAnatomist->getObjects() ;
  set<AObject *>::iterator iter( objs.begin() ), last( objs.end() ), found ;

  int objCount = 0 ;
  while ( iter != last )
    {
      if( objType < 0 || (*iter)->type() == objType )
        {
          if ( name  != "" )
          {
            if( name == (*iter)->name() ){
              ++objCount ;
              found = iter ;
              break ;
            }
          }
          else{
              ++objCount ;
              found = iter ;
              break ;
          }
        }
      ++iter ;
    }
  if ( objCount != 1 )
    {
      //AWarning("Please choose only one object to select.") ;
      return 0 ;
    }

  return *found ;
}

AGraphObject *
RoiManagementActionSharedData::getGraphObjectByName( const string& graphName,
					   const string& roiName ) const
{
  set<AObject *> objs = theAnatomist->getObjects() ;
  set<AObject *>::iterator iter( objs.begin() ), last( objs.end() ) ;

  while ( iter != last ){
    if( (*iter)->type() == AObject::GRAPHOBJECT ){
      AGraphObject * grao = dynamic_cast<AGraphObject*>(*iter) ;
      if ( grao ){
	string name ;
	grao->attributed()->getProperty( "name", name ) ;
	//cout << "grao->name = " << name << "\tRoiName = " << roiName <<endl ;
	if ( name == roiName ){
	  AObject::ParentList parents = grao->Parents() ;
	  AObject::ParentList::iterator iter2( parents.begin() ), last2( parents.end() ) ;
	  while ( iter2 != last2 ){
	    if( (*iter2)->type() == AObject::GRAPH )
	      break ;
	    ++iter2 ;
	  }
	  if( iter2 != last2 ){
	    //cout << "graph->name = " << (*iter)->name() << endl ;
	    if ( (*iter2)->name() == graphName )
	      return grao ;
	  }
	}
      }
    }
    ++iter ;
  }

  //cout << "Returning no graph object. Was searching " << roiName << " in "<< graphName <<  endl ;
  //cout << "Current graph regions are " << endl ;

  return 0 ;
}

AObject *
RoiManagementActionSharedData::getSelectedObject( int objType ) const
{
  map< unsigned, set<AObject *> > selectedObjs = SelectFactory::factory()->selected() ;
  map< unsigned, set<AObject *> >::iterator iterMap( selectedObjs.begin() ), lastMap( selectedObjs.end() ) ;

  set<AObject *>::const_iterator found ;

  int objCount = 0 ;
  while( iterMap != lastMap ){
    set<AObject *>::const_iterator
      iter( iterMap->second.begin() ), last( iterMap->second.end() ) ;
    while ( iter != last )
      {
	if( objType != AObject::OTHER )
	  if( (*iter)->type() == objType ){
	    ++objCount ;
	    found = iter ;
	    break ;
	  }
	++iter ;
    }
    ++iterMap ;
  }
  if ( objCount != 1 )
    {
      //AWarning("Please select only one object.") ;
      return 0 ;
    }

  return *found ;
}



QWidget *
RoiManagementAction::actionView( QWidget * parent )
{
  RoiManagementActionView * obs = new RoiManagementActionView( this, parent ) ;
  //   _sharedData->myObserver = new PaintActionView( this, parent ) ;

  return obs ;
}

void
RoiManagementActionSharedData::printState()
{
  cout << "String sets sizes :" << endl ;
  cout << "\thierarchyNames " << myHierarchyNames.size() << endl ;
  cout << "\tgraphNames " << myGraphNames.size() << endl ;
  cout << "\timageNames " << myImageNames.size() << endl ;
  cout << "\tcurrentGraphRegions " <<myCurrentGraphRegions.size() << endl ;
  cout << "\tcurrentHierarchyRoiNames " << myCurrentHierarchyRoiNames.size() << endl ;

  cout << "\nParameters :" << endl ;
  cout << "\tselectedHierarchy : " << mySelectedHierarchy << " , " << mySelectedHierarchyId << endl ;
  cout << "\tcurrentGraph : " << myCurrentGraph << " , " << myCurrentGraphId << endl ;
  cout << "\tgraphName : " << myGraphName << endl ;
  cout << "\tcurrentImage : " << myCurrentImage << " , " << myCurrentImageId << endl ;
  cout << "\tcurrentRegion : " << myRegionName << " , " << myCurrentRegionId << endl ;

  set<string>::const_iterator iter( myCurrentGraphRegions.begin() ), last( myCurrentGraphRegions.end() ) ;

  int count = 0 ;
  while( iter != last ){
    cout << "\t" << *iter << " , " << count << endl ;
    ++count ;
    ++iter ;
  }
}

void RoiManagementActionSharedData::refreshGraphs() const
{
  //	refresh graphs

  set<AObject *>			obj = theAnatomist->getObjects();
  set<AObject *>::const_iterator	io, fo=obj.end();
  AGraph				*ag;

  for( io=obj.begin(); io!=fo; ++io )
    {
      ag = dynamic_cast<AGraph *>( *io ) ;
      if( ag )
	{
	  ag->SetMaterial( ag->material() ) ;
	  ag->notifyObservers( (void *) this ) ;
	  // completeSelection( ag ) ;
	}
    }
}

void
RoiManagementActionSharedData::completeSelection( AGraph * graph ) const
{
  string name ;
  AGraph::iterator iter( graph->begin() ), last( graph->end() ) ;
  set<AObject*> objs ;
  while( iter != last )
    {
      AGraphObject * go = dynamic_cast<AGraphObject*>( *iter ) ;
      if( go ){
	objs.insert( go ) ;
      }
      ++iter ;
    }
  Command 	*cmd = new SelectCommand( objs ) ;
  theProcessor->execute( cmd ) ;
}


void
RoiManagementAction::cleanSession( )
{
  AGraph * graph = RoiChangeProcessor::instance()->getGraph(0) ;
  if( !graph )
    return;

  AGraph::iterator iter( graph->begin() ), last( graph->end() ) ;
  while( iter != last )
  {
    AGraphObject * go = dynamic_cast<AGraphObject*>( *iter ) ;
    cleanRegion( go ) ;
    ++iter ;
  }
}

void
RoiManagementAction::cleanRegion( AGraphObject * grao )
{
  if (grao == 0)
    RoiChangeProcessor::instance()->getCurrentRegion(view()->aWindow() ) ;

  list< pair< Point3d, ChangesItem> >* changes = new list< pair< Point3d, ChangesItem> > ;
  ChangesItem change ;
  change.after = 0 ;
  change.before = grao ;

  Bucket		*bk = 0;
  if( grao != 0 ){
    AGraphObject::iterator	ic, ec = grao->end();

    for( ic=grao->begin(); ic!=ec; ++ic )
      if( ( bk = dynamic_cast<Bucket *>( *ic ) ) )
	break;
  } else {
    bk = RoiChangeProcessor::instance()->getCurrentRegion(view()->aWindow() ) ;
  }

  if( !bk )
    return ;
  BucketMap<Void>::Bucket::iterator
    bckIter( bk->bucket()[0].begin( ) ),
    bckLast( bk->bucket()[0].end( ) ) ;

  while( bckIter != bckLast ){
    bool lonely = true ;
    if(  bk->bucket()[0].find( bckIter->first + Point3d(-1, 0, 0) ) != bk->bucket()[0].end() )
      lonely = false ;
    if(  bk->bucket()[0].find( bckIter->first + Point3d(1, 0, 0) ) != bk->bucket()[0].end() )
      lonely = false ;
    if(  bk->bucket()[0].find( bckIter->first + Point3d(0, -1, 0) ) != bk->bucket()[0].end() )
      lonely = false ;
    if(  bk->bucket()[0].find( bckIter->first + Point3d(0, 1, 0) ) != bk->bucket()[0].end() )
      lonely = false ;
    if(  bk->bucket()[0].find( bckIter->first + Point3d(0, 0, -1) ) != bk->bucket()[0].end() )
      lonely = false ;
    if(  bk->bucket()[0].find( bckIter->first + Point3d(0, 0, 1) ) != bk->bucket()[0].end() )
      lonely = false ;

    if( lonely )
      changes->push_back(pair<Point3d, ChangesItem>( bckIter->first, change ) )  ;

    ++bckIter ;
  }

  if ( ! (*changes).empty() )
    RoiChangeProcessor::instance()->applyChange( changes ) ;
}


void
RoiManagementAction::changeRegionTransparency( float alpha )
{
  SelectFactory::HColor	hcol = SelectFactory::selectColor();
  hcol.a = alpha;
  hcol.na = false;
  SelectFactory::setSelectColor( hcol );

  setChanged() ;
  notifyObservers() ;
}

void
RoiManagementAction::changeGraphTransparency( float alpha )
{
  //cerr << "changeGraphTransparency" << endl ;
  AGraph * g = RoiChangeProcessor::instance()->getGraph(0) ;

  if( g == 0 )
    return ;
  const Material& m = g->material() ;
  if( m.Diffuse(3) != alpha ){
    //cerr << "Really changeGraphTransparency" << endl ;
    Material& mat = g->GetMaterial() ;
    mat.SetDiffuseA( alpha ) ;

    g->setChanged() ;
    g->SetMaterial( mat );
    g->notifyObservers( this ) ;
    g->clearHasChangedFlags() ;

    //   for( set<AWindow*>::iterator it = g->WinList().begin() ; it != g->WinList().end() ; ++it )
    //     (*it)->Refresh() ;

    setChanged() ;
    notifyObservers() ;
  }
}

float
RoiManagementAction::graphTransparency()
{
  AGraph * g = RoiChangeProcessor::instance()->getGraph(0) ;

  if( g == 0 )
    return 0. ;

  //cout << "get Graph Transparency : " << g->material().Diffuse(3) << endl ;
  return g->material().Diffuse(3) ;
}




