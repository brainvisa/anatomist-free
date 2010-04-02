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


#include <anatomist/application/roibasemodule.h>
#include <anatomist/action/paintaction.h>
#include <anatomist/action/blobsegmentation.h>
#include <anatomist/control/paintcontrol.h>
#include <anatomist/action/roimanagementaction.h>
#include <anatomist/control/roimanagementcontrol.h>
#include <anatomist/action/levelsetaction.h>
#include <anatomist/control/levelsetcontrol.h>
#include <anatomist/action/labelnaming.h>
#include <anatomist/control/labelnamingcontrol.h>
#include <anatomist/object/Object.h>
#include <anatomist/graph/Graph.h>
#include <anatomist/graph/GraphObject.h>
#include <anatomist/volume/Volume.h>
#include <anatomist/selection/selectFactory.h>
#include <anatomist/graph/GraphObject.h>
#include <anatomist/controler/actiondictionary.h>
#include <anatomist/controler/controldictionary.h>
#include <anatomist/controler/controlmanager.h>
#include <anatomist/controler/icondictionary.h>
#include <anatomist/color/Material.h>
#include <anatomist/control/wControl.h>
#include <anatomist/bucket/Bucket.h>
#include <anatomist/window/Window.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/application/settings.h>
#include <anatomist/object/actions.h>
#include <anatomist/misc/error.h>
#include <anatomist/observer/observcreat.h>
#include <anatomist/object/objectConverter.h>
#include <aims/graph/graphmanip.h>
#include <aims/io/datatypecode.h>
#include <aims/utility/converter_bucket.h>
#include <aims/vector/vector.h>
#include <graph/tree/tree.h>
#include <graph/graph/graph.h>
#include <cartobase/stream/fileutil.h>
#include <qdialog.h>
#include <qpushbutton.h>
#include <qlineedit.h>

using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;

static bool initRoiBaseModule()
{
  RoiBaseModule	*a = new RoiBaseModule;
  a->init();
  return( true );
}

static bool garbage = initRoiBaseModule();

std::string 
RoiBaseModule::name() const
{ 
  return QT_TRANSLATE_NOOP( "ControlWindow", "Regions of Interest Base" );
}

std::string 
RoiBaseModule::description() const 
{ 
  return QT_TRANSLATE_NOOP( "ControlWindow", 
			    "Segmentation and manipulation of regions of "
			    "interest : Base module" );
} 

RoiBaseModule::RoiBaseModule() : Module() { }

RoiBaseModule::~RoiBaseModule() 
{ }

void RoiBaseModule::objectsDeclaration() { }

void RoiBaseModule::objectPropertiesDeclaration()
{
  AVolume<short>	vol;
  
  Tree			*tr = vol.optionTree();
  addVolumeRoiOptions( tr );

  Graph		*g = new Graph( "dummy" );
  AGraph	ag( g, "graph", false );

  tr = ag.optionTree();
  addGraphRoiOptions( tr );

  
  AGraphObject	ago(0) ;  
  tr = ago.optionTree();
  addGraphObjectRoiOptions(tr) ;
}

void RoiBaseModule::viewsDeclaration()
{
}

void RoiBaseModule::actionsDeclaration()
{
  ActionDictionary::instance()->addAction("PaintAction", PaintAction::creator ) ;
  ActionDictionary::instance()->addAction("RoiManagementAction", RoiManagementAction::creator ) ;
  ActionDictionary::instance()->addAction("ConnectivityThresholdAction", RoiLevelSetAction::creator ) ;
  ActionDictionary::instance()->addAction("LabelNamingAction", RoiLabelNamingAction::creator ) ;
  ActionDictionary::instance()->addAction("BlobSegmentationAction", RoiBlobSegmentationAction::creator ) ;
  ObservableCreatedNotifier::registerNotifier
    ( &RoiManagementActionView::objectLoaded );
}

void RoiBaseModule::controlsDeclaration()
{
  ControlDictionary::instance()->addControl("PaintControl",
					    PaintControl::creator, 102 ) ;
  ControlDictionary::instance()->addControl("ConnectivityThresholdControl",
					    RoiLevelSetControl::creator, 103 ) ;
  ControlDictionary::instance()->addControl("LabelNamingControl",
					    RoiLabelNamingControl::creator, 110 ) ;

  ControlManager::instance()->addControl( "QAGLWidget3D", AObject::objectTypeName( AObject::GRAPHOBJECT ),
					  "PaintControl" ) ;
  ControlManager::instance()->addControl( "QAGLWidget3D", AObject::objectTypeName( AObject::BUCKET ), 
					  "PaintControl" ) ;
  ControlManager::instance()->addControl( "QAGLWidget3D", AObject::objectTypeName( AObject::VOLUME ), 
					  "PaintControl" ) ;
  ControlManager::instance()->addControl( "QAGLWidget3D", AObject::objectTypeName( AObject::FUSION2D ), 
					  "PaintControl" ) ;
  
  ControlManager::instance()->addControl( "QAGLWidget3D", AObject::objectTypeName( AObject::BUCKET ), 
					  "ConnectivityThresholdControl" ) ;
  ControlManager::instance()->addControl( "QAGLWidget3D", AObject::objectTypeName( AObject::VOLUME ), 
					  "ConnectivityThresholdControl" ) ;
  ControlManager::instance()->addControl( "QAGLWidget3D", AObject::objectTypeName( AObject::GRAPHOBJECT ), 
					  "ConnectivityThresholdControl" ) ;

  ControlManager::instance()->addControl( "QAGLWidget3D", AObject::objectTypeName( AObject::BUCKET ), 
					  "LabelNamingControl" ) ;
  ControlManager::instance()->addControl( "QAGLWidget3D", AObject::objectTypeName( AObject::VOLUME ), 
					  "LabelNamingControl" ) ;
  ControlManager::instance()->addControl( "QAGLWidget3D", AObject::objectTypeName( AObject::GRAPHOBJECT ), 
					  "LabelNamingControl" ) ;

  {
    QPixmap	p;
    if( p.load( ( Settings::globalPath() + "/icons/draw.xpm" ).c_str() ) )
      IconDictionary::instance()->addIcon( "PaintControl", p );
  }
  {
    QPixmap	p;
    if( p.load( ( Settings::globalPath() + "/icons/level.xpm" ).c_str() ) )
      IconDictionary::instance()->addIcon( "ConnectivityThresholdControl", p );
  }
  {
    QPixmap	p;
    if( p.load( ( Settings::globalPath() + "/icons/name.xpm" ).c_str() ) )
      IconDictionary::instance()->addIcon( "LabelNamingControl", p );
  }
}

void RoiBaseModule::controlGroupsDeclaration()
{

}

void RoiBaseModule::addVolumeRoiOptions( Tree* tr )
{
  Tree			*t2, *t3;

  t2 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "Roi" ) );
  tr->insert( t2 );

  t3 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", 
					  "Create associated roi graph" ) );

  t3->setProperty( "callback", &RoiBaseModule::createGraph );

  t2->insert( t3 );
}


void RoiBaseModule::addGraphRoiOptions( Tree* tr )
{
  Tree			*t2, *t3, *t4 ;

  t2 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "Roi" ) );
  tr->insert( t2 );
  t3 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "Set name" ) );
  t3->setProperty( "callback", &RoiBaseModule::setGraphName );
  t2->insert( t3 );

  t4 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "Add region" ) );
  t4->setProperty( "callback", &RoiBaseModule::addRegion );
  t2->insert( t4 );
}

void RoiBaseModule::addGraphObjectRoiOptions( Tree* tr )
{
  Tree			*t2, *t3 ;

  t2 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "File" ) );
  tr->insert( t2 );
  t3 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", 
					  "Export mask as volume" ) );
  t3->setProperty( "callback", &RoiBaseModule::exportRegion );
  t2->insert( t3 );

  t2 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "Roi" ) );
  tr->insert( t2 );
  t3 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "Set name" ) );
  t3->setProperty( "callback", &RoiBaseModule::setGraphObjectName );
  t2->insert( t3 );
}

void RoiBaseModule::createGraph ( const set<AObject *> & obj )
{
  if( obj.empty() )
    return;
  
  set<AObject *>::const_iterator 
    iter( obj.begin() ), last( obj.end() ), found ;
  int volumeCount = 0 ;
  while ( iter != last )
    {
      if( (*iter)->Is2DObject() )
	{
	  ++ volumeCount ;
	  found = iter ;
	}
      ++iter ;
    }
  
  if ( volumeCount != 1 )
    {
      AWarning("Roi can be associated to one single volume only") ;
      return ;
    }

  AObject	*o = *found;

  newGraph( o, "RoiArg", "RoiArg" );
}

void
RoiBaseModule::setGraphName( const set<AObject *> & obj ) 
{
  if( obj.empty() )
    return;
  
  set<AObject *>::const_iterator iter( obj.begin() ), last( obj.end() ), 
    found ;
  int graphCount = 0 ;
  while ( iter != last )
    {
      if( (*iter)->type() == AObject::GRAPH )
	{
	  ++ graphCount ;
	  found = iter ;
	}
      ++iter ;
    }
  
  if ( graphCount != 1 ){
    AWarning("Please choose only one roi graph to rename.") ;
    return ;
  }
  
  const char * roiName ( RoiBaseModule::askName("graph", (*found)->name()  ) ) ;
  
  theAnatomist->unregisterObjectName( (*found)->name() ) ;
  // (*found)->setFileName( roiName ) ;
  (*found)->setFileName( "" );
  (*found)->setName( theAnatomist->makeObjectName( string(roiName) ) ) ;
  theAnatomist->registerObjectName( (*found)->name(), (*found) ) ;
  
  theAnatomist->NotifyObjectChange( *found ) ;
}


void
RoiBaseModule::setGraphObjectName( const set<AObject *> & obj ) 
{
  if( obj.empty() )
    return;
  
  set<AObject *>::const_iterator iter( obj.begin() ), last( obj.end() ), 
    found ;
  int graphCount = 0 ;
  while ( iter != last )
    {
      if( (*iter)->type() == AObject::GRAPHOBJECT )
	{
	  ++ graphCount ;
	  found = iter ;
	}
      ++iter ;
    }
  
  if ( graphCount != 1 ){
    AWarning("Please choose only one roi region to rename.") ;
    return ;
  }
  
  const char * roiName ( RoiBaseModule::askName("graph node", (*found)->name() ) );
  
  theAnatomist->unregisterObjectName( (*found)->name() ) ;
  (*found)->setFileName( roiName ) ;
  (*found)->setName( theAnatomist->makeObjectName( string(roiName) ) ) ;
  theAnatomist->registerObjectName( (*found)->name(), (*found) ) ;
  
  theAnatomist->NotifyObjectChange( *found ) ;
}


const char *
RoiBaseModule::askName( const string & type, const string& originalName )
{
  string message("Enter ") ;
  message += type ;
  message += string(" name") ;
  QDialog * nameSetter = new QDialog( 0, "", true ) ;
  nameSetter->setCaption( message.c_str() ) ;
  nameSetter->setMinimumSize( 250, 30 ) ;
  QLineEdit * lineEdition= new QLineEdit( QString( originalName.c_str() ), 
					  nameSetter/*, message.c_str()*/ ) ;
  lineEdition->setMinimumWidth(250) ;
  QObject::connect( lineEdition , SIGNAL(  returnPressed( ) ), nameSetter, 
		    SLOT( accept ( ) ) ) ;
  if( nameSetter->exec() )
    {
      (const char *) lineEdition->text()  ;
      if( string( "" ) == lineEdition->text().utf8().data() )
	return "Unknown" ;
      return lineEdition->text() ;
    }
  return "Unknown" ;
}

void
RoiBaseModule::addRegion( const set<AObject *> & obj ) 
{
  if( obj.empty() )
    return;
  
  set<AObject *>::const_iterator 
    iter( obj.begin() ), last( obj.end() ), found ;
  int graphCount = 0 ;
  while ( iter != last )
    {
      if( (*iter)->type() == AObject::GRAPH )
	{
	  ++ graphCount ;
	  found = iter ;
	  break ;
	}
      ++iter ;
    }  
  if ( graphCount != 1 )
    {
      AWarning("Please choose only one roi graph to rename.") ;
      return ;
    } 
  
  AGraph * gra = static_cast<AGraph*>( *iter ) ;

  string	regionName = "roi";
  AObject	*bck;
  AGraphObject	*ago = newRegion( gra, regionName, "roi", true, bck );

  set<AWindow *>	sw = gra->WinList();
  if( !sw.empty() )
    {
      set<AObject *> so;
      so.insert( ago );
      set<unsigned> groups;
      set<AWindow *>::const_iterator	iw, fw=sw.end();
      set<unsigned>::const_iterator	ig, fg=groups.end();
      SelectFactory			*sel = SelectFactory::factory();

      for( iw=sw.begin(); iw!=fw; ++iw )
	groups.insert( (*iw)->Group() );
      for( ig=groups.begin(); ig!=fg; ++ig )
	sel->select( *ig, so );
      sel->refresh();
    }
}


AGraphObject* RoiBaseModule::newRegion( AGraph* gra, const string & regionName, 
				    const string & syntax, bool withbck, 
				    AObject*& bck, bool nodup )
{
  Graph		* gr = gra->graph() ;

  if( nodup )
    {
      set<Vertex *>	vset = gr->getVerticesWith( "name", regionName );
      if( !vset.empty() )
	{
	  Vertex	*v = *vset.begin();
	  AObject	*ao = 0;
	  AGraphObject	*ago = 0;
	  if( v->getProperty( "ana_object", ao ) 
	      && (ago = dynamic_cast<AGraphObject *>( ao ) ) )
	    {
	      ago->GetMaterial().SetDiffuse ( 0., 0., 1., 0.5) ;
	      MObject::iterator	ic, ec = ago->end();
	      Bucket		*bk = 0;
	      for( ic=ago->begin(); ic!=ec; ++ic )
		if( ( bk = dynamic_cast<Bucket *>( *ic ) ) )
		  break;
	      if( bk ){
		bck = bk;
		bck->GetMaterial().SetDiffuse( 0., 0., 1., 0.5) ;
	      }
	      return( ago );
	    }
	}
    }

  Vertex	*v = gr->addVertex( syntax );  

  v->setProperty( "name", regionName );
  //v->setProperty( "roi_label", (int) gr->order() ) ;
  AGraphObject	*ago = new AGraphObject( v );
  Material m ;
  m.SetDiffuse( 0., 0., 1., 0.5) ;
  ago->SetMaterial( m ) ;

  gra->insert( ago, v );
  ago->setReferentialInheritance( gra );
  ago->setName( theAnatomist->makeObjectName( regionName ) );
  theAnatomist->registerObject( ago, false );
  theAnatomist->registerSubObject( gra, ago );
  theAnatomist->releaseObject( ago );

  if( withbck )
    {
      bck = new Bucket ;

      bck->setVoxelSize( gra->VoxelSize() ) ;
      bck->setName( theAnatomist->makeObjectName( regionName ).c_str() );
      bck->setVoxelSize( gra->VoxelSize() ) ;
      bck->setReferentialInheritance( ago );

      // Temporaire
      bck->setGeomExtrema() ;

      ago->insert( bck );
      theAnatomist->registerObject( bck, false );
      theAnatomist->registerSubObject( ago, bck );

      rc_ptr<BucketMap<Void> > b
          = ObjectConverter<BucketMap<Void> >::ana2aims( bck );
      GraphManip::storeAims( *gr, v, "aims_roi", b );

      //gra->setGeomExtrema();
      //gra->clearLabelsVolume();

      v->setProperty( "aims_roi_ana", (AObject *) bck );
    }

  ago->setChanged();
  gra->setChanged();
  ago->notifyObservers( gra );
  ago->internalUpdate();

  return( ago );
}


AGraph* RoiBaseModule::newGraph( AObject* o, const string & roiName, 
                                 const string & syntax )
{
  Point3df	vs = o->VoxelSize();
  
  Graph		*gr = new Graph( syntax );
  AGraph	*agr = new AGraph( gr, "", false ) ;
  // agr->setFileName( roiName ) ;
  vector<int>	bb ;
  bb.push_back(0) ;
  bb.push_back(0) ;
  bb.push_back(0) ;  
  
  agr->setName( theAnatomist->makeObjectName( roiName ) );
  theAnatomist->registerObject( agr );
  agr->setVoxelSize( o->VoxelSize() );
  agr->setReferential( o->getReferential() );

  gr->setProperty( syntax + "_VERSION", string( "1.0" ) );
  gr->setProperty( "boundingbox_min", bb );
  Point3df	bmin, bmax;
  o->boundingBox( bmin, bmax );

  bb[0] = static_cast<int>( rint( (bmax[0] - bmin[0]) / o->VoxelSize()[0] ) );
  bb[1] = static_cast<int>( rint( (bmax[1] - bmin[1]) / o->VoxelSize()[1] ) );
  bb[2] = static_cast<int>( rint( (bmax[2] - bmin[2]) / o->VoxelSize()[2] ) );

  agr->setLabelsVolumeDimension( bb[0], bb[1], bb[2] ) ;
  --bb[0];
  --bb[1];
  --bb[2];
  gr->setProperty( "boundingbox_max", bb );

  const PythonAObject	*pa = dynamic_cast<const PythonAObject *>( o );
  if( pa )
    {
      const GenericObject	*go = pa->attributed();
      if( go )
        {
          vector<float>		origin;
          if( go->getProperty( "origin", origin ) && origin.size() >= 3 )
            gr->setProperty( "origin", origin );
          try
            {
              Object	ob = go->getProperty( "spm_normalized" );
              bool	n = (bool) ob->getScalar();
              if( n )
                gr->setProperty( "spm_normalized", (int) n );
            }
          catch( ... )
            {
            }
          try
            {
              Object	ob = go->getProperty( "spm_spm2_normalization" );
              bool	n = (bool) ob->getScalar();
              gr->setProperty( "spm_spm2_normalization", (int) n );
            }
          catch( ... )
            {
            }
          try
            {
              Object	ob = go->getProperty( "spm_radio_convention" );
              bool	n = (bool) ob->getScalar();
              gr->setProperty( "spm_radio_convention", (int) n );
            }
          catch( ... )
            {
            }
          try
          {
            if( go->hasProperty( "referentials" ) 
                && go->hasProperty( "transformations" ) )
            {
              gr->setProperty( "referentials", 
                               go->getProperty( "referentials" ) );
              gr->setProperty( "transformations", 
                               go->getProperty( "transformations" ) );
            }
          }
          catch( ... )
          {
          }
        }
    }

  cout << "newGraph: roiName: " << roiName << endl;
  gr->setProperty( "filename_base", string( "*" ) );
  agr->setGeomExtrema();


  Material m = agr->GetMaterial() ;
  m.SetDiffuse(0., 0.5, 1., 0.5) ;

  agr->SetMaterial(m) ;

  return( agr );
}


void
RoiBaseModule::exportRegion( const set<AObject *> & obj ) 
{
  if( obj.size() != 1 )
    {
      cerr << "exportRegion : select one region at a time\n";
      return;
    }
  
  AGraphObject	*o = dynamic_cast<AGraphObject *>( *obj.begin() );

  RoiManagementAction::exportRegion( o ) ; 
}


bool RoiBaseModule::noop()
{
  return true;
}


