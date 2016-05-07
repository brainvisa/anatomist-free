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


#include <anatomist/landmark/graphLandmark.h>
#include <anatomist/volume/Volume.h>
#include <anatomist/graph/Graph.h>
#include <graph/tree/tree.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qlabel.h>
//#include <anatomist/wincontrol/pointCollectorTrigger.h>
#include <anatomist/graph/GraphObject.h>
#include <anatomist/control/wControl.h>
#include <anatomist/landmark/landmFactory.h>
#include <anatomist/bucket/Bucket.h>
#include <anatomist/selection/selectFactory.h>
#include <anatomist/window/Window.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/object/actions.h>


using namespace anatomist;
using namespace std;


static bool initGraphLandmark()
{
  /*AVolume<short>	vol;
  Tree			*tr = vol.optionTree();
  GraphLandmarkPicker::addVolumeLandmarkOptions( tr );*/

  Graph		*g = new Graph( "dummy" );
  AGraph	ag( g, "graph", false );
  Tree		*tr = ag.optionTree();
  GraphLandmarkPicker::addGraphLandmarkOptions( tr );

  return( true );
}

static bool dummy = initGraphLandmark();


//	private struct

namespace anatomist
{

  struct GraphLandmarkPicker_privateData
  {
    GraphLandmarkPicker_privateData() : /*grabView( false ),*/landmType( 0 ) {}

    //bool		grabView;
    QComboBox	*landmType;
  };

}


//	class

/*void GraphLandmarkPicker::addVolumeLandmarkOptions( Tree* tr )
{
  Tree			*t2, *t3;

  t2 = new Tree( true, "Landmarks" );
  tr->insert( t2 );
  t3 = new Tree( true, "Create landmarks graph" );
  t3->setProperty( "callback", &GraphLandmarkPicker::createGraph );
  t2->insert( t3 );
  }*/


void GraphLandmarkPicker::addGraphLandmarkOptions( Tree* tr )
{
  Tree			*t2, *t3;

  t2 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "Landmarks" ) );
  tr->insert( t2 );
  t3 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "Pick landmarks" ) );
  t3->setProperty( "callback", &GraphLandmarkPicker::startInterface );
  t2->insert( t3 );
}


GraphLandmarkPicker::GraphLandmarkPicker( AGraph* ag, QWidget* parent, 
					  const char* name, Qt::WindowFlags f )
  : QWidget( parent, f ), _graph( ag ),
  pdat( new GraphLandmarkPicker_privateData )
{
  _graph->addObserver( this );
    setObjectName(name);
    setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle( tr( "Landmarks picker on " ) + _graph->name().c_str() );
  QVBoxLayout	*l = new QVBoxLayout( this );
  l->setMargin( 5 );
  l->setSpacing( 5 );
  QWidget	*ltbox = new QWidget( this );
  QHBoxLayout *ltboxlay = new QHBoxLayout;
  ltbox->setLayout( ltboxlay );
  ltboxlay->setSpacing( 15 );

  ltboxlay->addWidget( new QLabel( tr( "Landmark type :" ), ltbox ) );
  pdat->landmType = new QComboBox( ltbox );
  ltboxlay->addWidget( pdat->landmType );
  pdat->landmType->addItem( tr( "Point bucket" ) );

  QPushButton	*b = new QPushButton( tr( "Pick landmark" ), this );
  b->setFixedSize( b->sizeHint() );
  l->addWidget( ltbox );
  l->addWidget( b );
  connect( b, SIGNAL( clicked() ), this, SLOT( pickLandmark() ) );
}


GraphLandmarkPicker::~GraphLandmarkPicker()
{
  // release control
  /*if( pdat->grabView )
    theAnatomist->setViewControl( 0 );*/

  _graph->deleteObserver( this );
}


/*void GraphLandmarkPicker::createGraph( const set<AObject *> & obj )
{
  if( obj.empty() )
    return;
  AObject	*o = *obj.begin();

  Point3df	vs = o->VoxelSize();
  Graph		*gr = new Graph( "RoiArg" );
  AGraph	*agr = new AGraph( gr, "landmarks", false );
  vector<int>	bb;

  agr->setName( theAnatomist->makeObjectName( "LandmarkGraph" ) );
  theAnatomist->registerObject( agr );
  agr->setVoxelSize( o->VoxelSize() );
  bb.push_back( 0 );
  bb.push_back( 0 );
  bb.push_back( 0 );
  gr->setProperty( "RoiArg_VERSION", string( "1.0" ) );
  gr->setProperty( "boundingbox_min", bb );
  gr->setProperty( "boundingbox_max", bb );
  agr->setGeomExtrema();
  gr->setProperty( "filename_base", string( "landmarkGraph.data" ) );
  //gr->setProperty( "type.bck", string( "roi.bck" ) );
  //gr->setProperty( "type.tri", string( "roi.tri" ) );
  gr->setProperty( "type.global.bck", string( "roi.global.bck" ) );
  //gr->setProperty( "type.global.tri", string( "roi.global.tri" ) );
  //gr->setProperty( "roi.bck", string( "roi bucket_filename" ) );
  //gr->setProperty( "roi.tri", string( "roi Tmtktri_filename" ) );
  gr->setProperty( "roi.global.bck", 
		    string( "roi roi_global.bck roi_label" ) );
  //gr->setProperty( "roi.global.tri", 
  //	    string( "roi roi_global.mesh roi_label" ) );
  }*/


void GraphLandmarkPicker::startInterface( const set<AObject*> & obj )
{
  if( obj.empty() )
    {
      cerr << "GraphLandmarkPicker - No graph\n";
      return;
    }

  if( obj.size() != 1 )
    cerr << "GraphLandmarkPicker - warning : more than 1 object selected "
	 << "- taking first only\n";

  AGraph	*ag = dynamic_cast<AGraph *>( *obj.begin() );
  if( !ag )
    {
      cerr << "GraphLandmarkPicker - selected object is not a graph\n";
      return;
    }

  GraphLandmarkPicker	*lp 
    = new GraphLandmarkPicker( ag, theAnatomist->getQWidgetAncestor(), 0, Qt::Window );
  lp->show();
}


void GraphLandmarkPicker::update( const Observable*, void* arg )
{
  if( arg == 0 )
    {
      cout << "called obsolete GraphLandmarkPicker::update( obs, NULL )\n";
      delete this;
    }
}


void GraphLandmarkPicker::unregisterObservable( Observable* o )
{
  Observer::unregisterObservable( o );
  delete this;
}


void GraphLandmarkPicker::pickLandmark()
{
  //	temp...
  set<AWindow *>	w = theAnatomist->getWindowsInGroup( 0 );
  if( w.empty() )
    {
      cout << "No window in base group - can't pick current cursor position\n";
      return;
    }

  Point3df	pt = (*w.begin())->getPosition();
  cout << "Point : " << pt << endl;

  createLandmark( pt );

  /*if( theAnatomist->viewControl() )
    {
      cerr << "View controler already active: finish started action first.\n";
      return;
    }
  theAnatomist->setViewControl( new APointCollectorTrigger( 1, 
							    clickPointCbk, 
							    this ) );
  pdat->grabView = true;
  cout << tr( "OK, click point in any 2D/3D window" ) << endl;*/
}


/*void GraphLandmarkPicker::clickPointCbk( APointCollector* caller, 
					 void *clientdata )
{
  GraphLandmarkPicker		*pk = (GraphLandmarkPicker *) clientdata;
  const vector<Point3df>	& pts = caller->points();

  if( pts.size() != 1 )
    cerr << "warning : " << pts.size() << " points (expecting 1)\n";
  if( pts.empty() )
    return;

  const Point3df		& pt = pts[0];
  cout << "Point : " << pt << endl;

  pk->createLandmark( pt );

  // delete controller
  theAnatomist->setViewControl( 0 );
  pk->pdat->grabView = false;
  }*/


void GraphLandmarkPicker::createLandmark( const Point3df & pt )
{
  vector<int>	bbmi, bbma;
  Graph		*gr = _graph->graph();
  Point3df	vs = _graph->VoxelSize();
  Point3d	pi = Point3d( (short) ( pt[0] / vs[0] + 0.5 ), 
			      (short) ( pt[1] / vs[1] + 0.5 ), 
			      (short) ( pt[2] / vs[2] + 0.5 ) );

  cout << "createLandmark pos : " << pi << endl;

  if( _graph->size() == 0 )	// 1st node
    {
      bbmi.push_back( pi[0] );
      bbmi.push_back( pi[1] );
      bbmi.push_back( pi[2] );
      bbma.push_back( pi[0] );
      bbma.push_back( pi[1] );
      bbma.push_back( pi[2] );
      gr->setProperty( "boundingbox_min", bbmi );
      gr->setProperty( "boundingbox_max", bbma );
    }
  else
    {
      gr->getProperty( "boundingbox_min", bbmi );
      if( pi[0] < bbmi[0] )
	bbmi[0] = pi[0];
      if( pi[1] < bbmi[1] )
	bbmi[1] = pi[1];
      if( pi[2] < bbmi[2] )
	bbmi[2] = pi[2];
      gr->setProperty( "boundingbox_min", bbmi );
      gr->getProperty( "boundingbox_max", bbma );
      if( pi[0] > bbma[0] )
	bbma[0] = pi[0];
      if( pi[1] > bbma[1] )
	bbma[1] = pi[1];
      if( pi[2] > bbma[2] )
	bbma[2] = pi[2];
      gr->setProperty( "boundingbox_max", bbma );
    }

  Vertex	*v = gr->addVertex( "roi" );
  v->setProperty( "name", string( "unknown" ) );
  AGraphObject	*ago = new AGraphObject( v );
  v->setProperty( "ana_object", (AObject *) ago );
  //v->setProperty( "needs_saving", true );
  ago->setName( theAnatomist->makeObjectName( "LandmarkNode" ) );
  ago->RegisterParent( _graph );
  ago->addObserver( _graph );
  theAnatomist->registerObject( ago, false );
  theAnatomist->registerSubObject( _graph, ago );

  Bucket	*bck 
    = ALandmarkFactory::createPointBucket( pt, 
					   Point4df( vs[0], vs[1], vs[2], 1 ), 
					   "bck_landmark", 
					   Point3df( 1, 0, 0 ) );
  ago->insert( bck );
  theAnatomist->registerObject( bck, false );
  theAnatomist->registerSubObject( ago, bck );
  _graph->setGeomExtrema();
  _graph->clearLabelsVolume();

  map<AObject *, string>	objmap;
  int				index = 0, i = 0;
  set<int>			sind;
  set<int>::iterator		is, es;
  Graph::iterator		iv, ev=gr->end();

  v->getProperty( "objects_map", objmap );
  objmap[ bck ] = "roi_global.bck roi_label";
  v->setProperty( "objects_map", objmap );

  for( iv=gr->begin(); iv!=ev; ++iv )
    if( (*iv)->getProperty( "roi_label", index ) )
      sind.insert( index );
  for( is=sind.begin(), es=sind.end(); is!=es && *is==i; ++is, ++i ) {}
  v->setProperty( "roi_label", i );

  ago->setChanged();
  _graph->setChanged();
  ago->notifyObservers( this );
  ago->internalUpdate();

  set<AWindow *>	sw = _graph->WinList();
  if( !sw.empty() )
    {
      set<AObject *>	so;
      so.insert( ago );
      set<unsigned>	groups;
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
