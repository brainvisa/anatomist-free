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

#ifndef ROI_MANAGEMENT_ACTION_H
#define ROI_MANAGEMENT_ACTION_H

#include <anatomist/controler/action.h>
#include <aims/qtcompat/qvbox.h>
#include <aims/qtcompat/qhbox.h>
#include <anatomist/observer/Observer.h>
#include <anatomist/observer/Observable.h>
#include <anatomist/object/Object.h>
#include <string>
#include <qstringlist.h>
#include <qlayout.h>
#include <aims/qtcompat/qlistbox.h>
#include <qdialog.h>
#include <qcombobox.h>
#include <map>

class AStringList ;
class AWindow3D ;

namespace anatomist
{
  class AObject ;
  class Transformation ;
  class AGraph ;
  class AGraphObject ;
  class Hierarchy ;

  class RoiManagementAction ;
  class RoiManagementActionSharedData ;
  struct RoiManagementActionView_Private ;
}

class RegionsFusionWindow : public QDialog 
{
  Q_OBJECT

public:
  RegionsFusionWindow( QWidget * parent, const QStringList& regions ) ;

  std::set<std::string> regionsToBeFusioned() ;
  std::string newRegionName() ;
  
public slots:
  void selectedRegionsChanged() ;

private:
  QStringList mySelectedRegions ;
  QString myNewRegionName ;
  QVBoxLayout * l ;
  QVBox * frame ;
  QListBox * selectRegions ;
  QComboBox * selectRegionName ;
  QHBox * buttons ;
  QPushButton * okButton ;
  QPushButton * cancelButton ;
} ;

class RoiManagementActionView : public QVBox, public anatomist::Observer
{
  
  Q_OBJECT
  
public:
  RoiManagementActionView( anatomist::RoiManagementAction *  myAction,
			   QWidget * parent ) ;
  ~RoiManagementActionView() ;

  virtual void update( const anatomist::Observable *, void * ) ;

  std::string askName( const std::string& type, 
		       const std::string& originalName, 
		       const std::string& message, bool noHierarchy = true ) ;
  static void objectLoaded( anatomist::Observable* );
  
private slots :
#if QT_VERSION >= 0x040000
  void selectGraph( Q3ListBoxItem * ) ;
  void renameGraph( Q3ListBoxItem * ) ;
  void selectRegion( Q3ListBoxItem * ) ;
  void renameRegion( Q3ListBoxItem * ) ;
  void selectImage( Q3ListBoxItem * ) ;
#else
  void selectGraph( QListBoxItem * ) ;
  void renameGraph( QListBoxItem * ) ;
  void selectRegion( QListBoxItem * ) ;
  void renameRegion( QListBoxItem * ) ;
  void selectImage( QListBoxItem * ) ;
#endif
  void newGraph( ) ;
  void deleteGraph( ) ;
  void loadGraph( ) ;
  void saveGraphAs( ) ;
  void reloadGraph( ) ;
  void saveGraph( ) ;
  void newRegion( ) ;
  void deleteRegion( ) ;
  void exportAsMask( ) ;
  void regionsFusion( ) ;
  // void refresh() ;
  void neuroFrameWork() ;
  void lateralNeuroFrameWork() ;
  void sulciFrameWork() ;
  void ratFrameWork() ;
  void freeFrameWork() ;
  void newUserDefinedFrameWork() ;
  void loadUserDefinedFrameWork() ;
  void saveUserDefinedFrameWork() ;
  void defineNewFWRegionName() ;
  void modifyFWRegionName() ;
  void modifyFWRegionColor() ;
  void deleteFWRegionName() ;
//   void saveUserDefinedFrameWorkAs() ;
  //void updateRecentFrameWorksMenu( QPopupMenu * pop ) ; 
  void createAxialWindow() ;
  void createCoronalWindow() ;
  void createSagittalWindow() ;
  //   void createObliqueWindow() ;
  void create3DWindow() ;
  void cleanSession() ;
  void cleanRegion() ;
  void regionTransparencyChange( int alpha ) ;
  void graphTransparencyChange( int alpha ) ;
  void regionStats() ;
  
private:
  anatomist::RoiManagementActionView_Private * _private ;
  
  bool myUpdatingFlag ;
  bool mySelectingImage ;
  bool mySelectingGraph ;
  bool mySelectingRegion ;
  bool myChangingGraphTransparency ;
  bool myChangingRegionTransparency ;
  bool myGettingImageNames ;
  bool myGettingGraphNames ;
  bool myGettingRegionNames ;
  bool myGettingHierarchyNames ;
  bool myGettingHierarchyRegionNames ;
  
  QString myHierarchyRoiName ;
  QStringList myHierarchyRoiNames ;

  anatomist::AObject * getObject( anatomist::AObject::ObjectType objType 
				  = anatomist::AObject::OTHER, 
				  const std::string& name = "" ) ;
  QStringList getImageNames() ;
  QStringList getHierarchyNames( ) ;
  QStringList getGraphNames() ;
  QStringList getCurrentGraphRegions() ;
  std::string getSelectedGraphName( ) ;
  QStringList getCurrentHierarchyRoiNames( ) ;

  QStringList myImageNames ;
  QStringList myGraphNames ;
  QStringList myRegions ;
  QStringList myRecentFrameWorks ;
} ;

namespace anatomist{

  class RoiManagementActionSharedData : public Observable, public Observer
  {
  public:
    virtual ~RoiManagementActionSharedData() ;
    
    AObject * getObjectByName( int objType, 
			       const std::string& name = "" ) const ;
    std::string currentImage() const { return myCurrentImage ; }
    std::string currentGraph() const { return myCurrentGraph ; }
    static RoiManagementActionSharedData* instance() ;

    AGraphObject * getGraphObjectByName( const std::string& graphName, 
					 const std::string& roiName ) const ;
    void refresh();
    virtual void update( const Observable*, void* );

  protected:
    virtual void unregisterObservable( Observable* );

  private:
    friend class RoiManagementAction ;
    
    RoiManagementActionSharedData() ;
    static RoiManagementActionSharedData * _instance ;

    // Attributes
    std::set<std::string> myHierarchyNames ;
    bool myHierarchyNamesChanged ;
    
    std::set<std::string> myGraphNames ;
    bool myGraphNamesChanged ;
    
    std::set<std::string> myImageNames ;
    bool myImageNamesChanged ;
    
    std::set<std::string> myCurrentGraphRegions ;
    bool myCurrentGraphRegionsChanged ;
    
    std::set<std::string> myCurrentHierarchyRoiNames ;
    bool myCurrentHierarchyRoiNamesChanged ;
    
    AObject * getSelectedObject( int objType ) const ;
    void completeSelection( AGraph * g ) const ;
    
    void printState() ;
    void refreshGraphs() const ;

    std::string mySelectedHierarchy ;
    int mySelectedHierarchyId ;
    std::string myUserDefinedHierarchy ;
    std::string myGraphName ;
    std::string myCurrentGraph ;
    int myCurrentGraphId ;
    std::string myCurrentImage ;
    int myCurrentImageId ;
    std::string myRegionName ;
    std::string myPartialRegionName ;
    int myCurrentRegionId ;
    
  };

  
  class RoiManagementAction : public Action
  {
  public:
    RoiManagementAction() ;
    virtual ~RoiManagementAction() ;
    
    virtual std::string name() const;
    
    static Action * creator() ;

    std::set<std::string> getHierarchyNames() ;
    std::set<std::string> getGraphNames() ;
    std::set<std::string> getImageNames() ;
    std::set<std::string> getCurrentGraphRegions() ;
    std::set<std::string> getCurrentHierarchyRoiNames( ) ;
    const std::string& getCurrentHierarchy() const { return _sharedData->mySelectedHierarchy ; }
    void recursiveHierarchyNameExtraction( Tree * subtree, 
					   std::set<std::string>& names ) ;
    bool getSelectedGraphName( std::string& ) ;
    void addRegion( const std::string& name ) ;
    void selectHierarchy( const std::string& hieName, int hieId ) ;
    void loadHierarchy( ) ;
    std::string newUDHierarchy( const std::string& name ) ;
    std::string loadUDHierarchy( const std::string& hierarchyName ) ;
    bool saveUDHierarchy( ) ;
    void defineNewFWRegionName(const std::string & name, int red, int green, int blue ) ;
    void deleteFWRegionName(const std::string & name) ;
    void modifyUDFWRegionName(const std::string & oldName, const std::string & newName ) ;
    void modifyUDFWRegionColor( const std::string & name, 
			        int red, int green, int blue ) ;
    void selectGraph( const std::string & graphName, int graphId ) ;
    void newGraph( const std::string& name ) ;
    void selectImage( const std::string & imageName, int imageId ) ;
    void refresh() ;
    void renameGraph( const std::string& name, int graphId ) ;
    void deleteGraph( ) ;
    void loadGraph( const QStringList& ) ;
    void saveGraphAs( ) ;
    void reloadGraph( ) ;
    void saveGraph( ) ;
    void selectRegion( const std::string& regionName, int regionId ) ;
    void selectRegionName( const std::string& regionName ) ;
    void smartSelectRegionName( const std::string & partialRegionName ) ;
    void newRegion( const std::string& name ) ;
    void renameRegion( const std::string & name, int regionId ) ;
    void deleteRegion( ) ;
    void exportAsMask( ) ;
    static void exportRegion( AGraphObject * o) ;
    void regionsFusion( const std::set<std::string>& regions,
			const std::string& newName) ;
    void createWindow( const std::string& type ) ;
    
    int selectedHierarchyId() { return _sharedData->mySelectedHierarchyId ; }
    int currentGraphId() 
      { return _sharedData->myCurrentGraphId ; }
    int currentImageId() { return _sharedData->myCurrentImageId ; }
    int currentRegionId() { return _sharedData->myCurrentRegionId ; }

    virtual QWidget * actionView( QWidget * ) ;
    virtual bool viewableAction( ) const { return true ; }
    
    // Transfer Observer property to shared data
    void changeRegionTransparency( float alpha ) ;
    void changeGraphTransparency( float alpha ) ;
    float graphTransparency( ) ;
    
    void addObserver (Observer *observer) { _sharedData->addObserver(observer) ; }
    void deleteObserver (Observer *observer) { _sharedData->deleteObserver(observer) ;}
    void notifyObservers (void *arg=0) { _sharedData->notifyObservers(arg) ; }
    void notifyUnregisterObservers ()  { _sharedData->notifyUnregisterObservers() ; }
    bool hasChanged () const { return _sharedData->hasChanged() ; }
    void setChanged () { _sharedData->setChanged() ; }
    void cleanSession() ;
    void cleanRegion( anatomist::AGraphObject * ) ;
    bool savableGraph() ;
    void regionStats() ;
    
    const std::string& selectedHierarchy() const { return _sharedData->mySelectedHierarchy ; }
    const std::string& userDefinedHierarchy() const { return _sharedData->myUserDefinedHierarchy ; }

  private:  
    //   void createGraph( AObject * volume ) ;
    //   void addRegion( AGraph * graph ) ;
    RoiManagementActionSharedData * _sharedData ;    

  } ;
}

#endif  

