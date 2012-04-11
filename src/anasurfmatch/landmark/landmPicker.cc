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


#include <anatomist/landmark/landmPicker.h>
#include <anatomist/surface/triangulated.h>
#include <anatomist/surface/texsurface.h>
#include <anatomist/surface/texture.h>
#include <anatomist/mobject/Fusion3D.h>
#include <anatomist/volume/Volume.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/landmark/landmFactory.h>
#include <anatomist/window/Window.h>
#include <anatomist/window/viewstate.h>
#include <anatomist/object/actions.h>
#include <graph/tree/tree.h>
#include <qlayout.h>
#include <aims/qtcompat/qvbox.h>
#include <qcombobox.h>
#include <qpushbutton.h>


using namespace anatomist;
using namespace carto;
using namespace std;


// initialize and register into ATriangulated options
static void addLandmarkOptions( Tree* tr )
{
  Tree			*t2, *t3;

  t2 = new Tree( true, "Landmarks" );
  tr->insert( t2 );
  t3 = new Tree( true, "Pick landmarks" );
  t3->setProperty( "callback", &ALandmarkPicker::startInterface );
  t2->insert( t3 );
}

static bool registerLandmarkPicker()
{
  Tree			*tr;
  rc_ptr<ATriangulated>	tri;
  rc_ptr<ATexture>	tex;
  rc_ptr<ATexSurface>	ts;
  rc_ptr<Fusion3D>	fus;
  rc_ptr<AVolume<short> > vol;

  tri.reset( new ATriangulated( "dummy" ) );
  tr = tri->optionTree();
  addLandmarkOptions( tr );

  tex.reset( new ATexture );
  ts.reset( new ATexSurface( tri.get(), tex.get() ) );
  tr = ts->optionTree();
  addLandmarkOptions( tr );

  tri.reset( new ATriangulated( "dummy" ) );
  vol.reset( new AVolume<short>( "dummyvol" ) );
  vector<AObject *>	so(2);
  so[0] = tri.get();
  so[1] = vol.get();
  fus.reset( new Fusion3D( so ) );
  tr = fus->optionTree();
  addLandmarkOptions( tr );

  return( true );
}


static bool dummy = registerLandmarkPicker();


//	private struct

namespace anatomist
{

  struct ALandmarkPicker_privateData
  {
    ALandmarkPicker_privateData() : /*grabView( false ),*/ mode( Euclidian ) {}

    enum Mode
    {
      Euclidian
    };

    QComboBox	*modebox;
    //bool	grabView;
    Mode	mode;
  };

}


//	class

ALandmarkPicker::ALandmarkPicker( const set<AObject *> & obj ) 
  : QWidget( theAnatomist->getQWidgetAncestor(), Qt::Window ), _obj( obj ),
    _privdata( new ALandmarkPicker_privateData )
{
  QString				title = tr( "Landmark picker : " );
  set<AObject *>::const_iterator	io, fo = obj.end();
   setObjectName("LandmarkPicker");
   setAttribute(Qt::WA_DeleteOnClose);
  for( io=obj.begin(); io!=fo; ++io )
    {
      (*io)->addObserver( this );
      title += (*io)->name().c_str();
      title + " ";
    }
  
  setCaption( title );

  QHBoxLayout	*mainlay = new QHBoxLayout( this );
  mainlay->setSpacing( 10 );
  mainlay->setMargin( 5 );

  QVBox		*leftpan = new QVBox( this );
  QVBox		*rightpan = new QVBox( this );
  mainlay->addWidget( leftpan );
  mainlay->addWidget( rightpan );

  _privdata->modebox = new QComboBox( leftpan );
  _privdata->modebox->insertItem( tr( "Nearest" ) );

  QPushButton	*gobtn = new QPushButton( tr( "Pick landmark" ), rightpan, 
					  "pick" );
  connect( gobtn, SIGNAL( clicked() ), this, SLOT( pickPoint() ) );
}



ALandmarkPicker::~ALandmarkPicker()
{
  /*if( _privdata->grabView )
    {
      theAnatomist->setViewControl( 0 );
      cout << "Controler removed\n";
      }*/

  delete _privdata;
}


void ALandmarkPicker::startInterface( const set<AObject *> & obj )
{
  ALandmarkPicker	*pk = new ALandmarkPicker( obj );
  pk->show();
}


void ALandmarkPicker::pickPoint()
{
  //	temp...
  set<AWindow *>	w = theAnatomist->getWindowsInGroup( 0 );
  if( w.empty() )
    {
      cout << "No window in base group - can't pick current cursor position\n";
      return;
    }

  Point3df	pt = (*w.begin())->GetPosition();
  cout << "Point : " << pt << endl;

  createLandmark( pt );

  /*  if( theAnatomist->viewControl() )
    {
      cerr << "View controler already active: finish started action first.\n";
      return;
    }
  theAnatomist->setViewControl( new APointCollectorTrigger( 1, 
							    clickPointCbk, 
							    this ) );
  _privdata->grabView = true;
  cout << tr( "OK, click point in any 2D/3D window" ) << endl;*/
}


/*void ALandmarkPicker::clickPointCbk( APointCollector* caller, 
				     void *clientdata )
{
  ALandmarkPicker		*pk = (ALandmarkPicker *) clientdata;
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
  pk->_privdata->grabView = false;
  }*/


void ALandmarkPicker::createLandmark( const Point3df & pt )
{

  // find nearest vertex
  unsigned	index = 0;
  float		dist = 0;
  Point3df	vpos;

  if( nearestVertex( pt, index, dist, vpos ) )
    {
      string		name = "Landmark";
      ATriangulated	*lmk 
	= ALandmarkFactory::createCube( vpos, name, 3, Point3df( 1, 0, 0 ) );
      theAnatomist->registerObject( lmk );

      // display landmark in same windows as objects
      set<AWindow *>			wl;
      set<AWindow *>::const_iterator	iw, fw=wl.end();
      set<AObject *>::const_iterator	io, fo = _obj.end();

      for( io=_obj.begin(); io!=fo; ++io )
	{
	  const set<AWindow *>	& owin = (*io)->WinList();
	  wl.insert( owin.begin(), owin.end() );
	}

      for( iw=wl.begin(); iw!=fw; ++iw )
	{
	  (*iw)->registerObject( lmk );
	  (*iw)->Refresh();
	}
    }
}


AObject* ALandmarkPicker::nearestVertex( const Point3df & pt, 
					 unsigned & index, float & dist, 
					 Point3df & minvpos )
{
  set<AObject *>::const_iterator	io, fo=_obj.end();
  AObject				*obj, *nobj = 0;
  GLComponent				*gobj;
  float					d = 0, dmin = 1e38;
  unsigned				iobj = 0;
  Point3df				vpos;

  index = 0;
  for( io=_obj.begin(); io!=fo; ++io )
    // scan surfacic objects only (should always be OK in principle)
    if( ( gobj = (*io)->glAPI() ) )
      {
	obj = *io;
	switch( _privdata->mode )
	  {
	  case ALandmarkPicker_privateData::Euclidian:
	    d = nearestEuclidian( gobj, pt, iobj, vpos );
	    break;
	  default:
	    cout << "Unknown Distance mode --- BUG\n";
	    d = 1e38;
	  }
	if( d < dmin )
	  {
	    index = iobj;
	    nobj = obj;
	    dmin = d;
	    minvpos = vpos;
	  }
      }

  if( !nobj )
    {
      cerr << "nearest object not found --- BUG...\n";
      return( 0 );
    }
  dist = sqrt( dmin );

  cout << "nearest object : " << nobj->name() << endl;
  cout << "nearest vertex index : " << index << endl;
  cout << "nearest vertex position : " << vpos << endl;
  cout << "distance to surface : " << dist << endl;
  
  return( nobj );
}


float ALandmarkPicker::nearestEuclidian( GLComponent* obj, 
                                         const Point3df & pt, 
                                         unsigned & index, Point3df & vpos )
{
  unsigned	i, nv;
  float		x, y, z, d, dmin = 1e38;
  const float	*v;

  ViewState	state( 0 );	// TODO : take time into account
  nv = obj->glNumVertex( state );
  v = obj->glVertexArray( state );

  //	MISSING: transform clicked point to object referential

  // find nearest vertex
  for( i=0; i<nv; ++i )
    {
      x = *v++;
      y = *v++;
      z = *v++;

      d = ( x - pt[0] ) * ( x - pt[0] ) 
	+ ( y - pt[1] ) * ( y - pt[1] ) 
	+ ( z - pt[2] ) * ( z - pt[2] );
      if( d < dmin )
	{
	  dmin = d;
	  index = i;
	  vpos = Point3df( x, y, z );
	}
    }

  return( dmin );
}


void ALandmarkPicker::update( const anatomist::Observable*, void* arg )
{
  if( arg == 0 )
    {
      cout << "called obsolete ALandmarkPicker::update( obs, NULL )\n";
      delete this;	// object destroyed: suicide
    }
}


void ALandmarkPicker::unregisterObservable( anatomist::Observable* o )
{
  Observer::unregisterObservable( o );
  delete this;
}
