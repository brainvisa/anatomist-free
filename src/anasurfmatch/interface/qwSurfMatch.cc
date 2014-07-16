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


#include <anatomist/interface/qwSurfMatch.h>
#include <anatomist/surfmatcher/surfMatcher.h>
#include <anatomist/window/Window.h>
#include <anatomist/surface/triangulated.h>
#include <anatomist/browser/stringEdit.h>
#include <anatomist/application/Anatomist.h>
#include <cartobase/thread/thread.h>
#include <cartobase/thread/semaphore.h>
#include <graph/tree/tree.h>
#include <aims/qtcompat/qvbox.h>
#include <aims/qtcompat/qhbox.h>
#include <qlabel.h>
#include <aims/qtcompat/qhgroupbox.h>
#include <aims/qtcompat/qvgroupbox.h>
#include <aims/qtcompat/qgrid.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <aims/qtcompat/qlistbox.h>
#include <qcursor.h>
#include <qevent.h>
#include <qapplication.h>
#include <iostream>


using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;

class QSurfMatchWin::ASThread : public Thread
{
public:
  ASThread( QSurfMatchWin* sw ) : Thread(), _sw( sw ) {}
  virtual ~ASThread() {}
  virtual void doRun()
  { _sw->processThread(); }

private:
  QSurfMatchWin	*_sw;
};


struct QSurfMatchWin::Widgets
{
  Widgets( QSurfMatchWin* sw ) : processThread( sw ) {}

  QLabel		*orgLabel;
  QLabel		*dstLabel;
  QPushButton		*startBtn;
  QPushButton		*stopBtn;
  QPushButton		*resetBtn;
  bool			processRunning;
  bool			triggerProcessState;
  ASThread		processThread;
  Semaphore		interfaceSem;
  bool			beingDestroyed;
  bool			needsNotifyUpdate;
  QListBox		*orgPtsBox;
  QListBox		*dstPtsBox;
  vector<unsigned>	orgPtsVec;
  vector<Point3df>	dstPtsVec;
  bool			grabView;
};


QSurfMatchWin::QSurfMatchWin( QWidget* parent, ASurfMatcher* obj )
  : QWidget( parent, "QSurfMatchWin", Qt::WDestructiveClose ), _obj( obj ),
    _widgets( new QSurfMatchWin::Widgets( this ) )
{
  obj->addObserver( this );

  QString	str = tr( "Surface matching control: " );
  str += obj->name().c_str();
  setCaption( str );

  QHBoxLayout	*mainLay = new QHBoxLayout( this, 5 );
  QVBox		*leftPanel = new QVBox( this );
  mainLay->addWidget( leftPanel );
  QHGroupBox	*direction = new QHGroupBox( tr( "Direction :" ), leftPanel );
  new QLabel( tr( "From:" ), direction );
  _widgets->orgLabel = new QLabel( direction );
  new QLabel( tr( "to:" ), direction );
  _widgets->dstLabel = new QLabel( direction );
  _widgets->orgLabel->setFrameStyle( QFrame::Sunken | QFrame::Panel );
  _widgets->dstLabel->setFrameStyle( QFrame::Sunken | QFrame::Panel );
  _widgets->orgLabel->setBackgroundColor( QColor( 255, 255, 255 ) );
  _widgets->dstLabel->setBackgroundColor( QColor( 255, 255, 255 ) );
  fillDirectionLabel();
  QPushButton	*chgDirBtn = new QPushButton( tr( "Change" ), direction );
  chgDirBtn->setFixedSize( chgDirBtn->sizeHint() );

  QHGroupBox	*recBox = new QHGroupBox( tr( "Record over time:" ),
					  leftPanel );
  QCheckBox	*rec = new QCheckBox( tr( "On / Off" ), recBox );
  rec->setChecked( _obj->record() );

  QHGroupBox	*proc = new QHGroupBox( tr( "Processing:" ), leftPanel );
  _widgets->startBtn = new QPushButton( tr( "Start" ), proc );
  _widgets->startBtn->setFixedSize( _widgets->startBtn->sizeHint() );
  _widgets->stopBtn = new QPushButton( tr( "Stop" ), proc );
  _widgets->stopBtn->setFixedSize( _widgets->stopBtn->sizeHint() );
  _widgets->stopBtn->setEnabled( false );
  _widgets->resetBtn = new QPushButton( tr( "Reset" ), proc );
  _widgets->resetBtn->setFixedSize( _widgets->resetBtn->sizeHint() );

  mainLay->addWidget( paramWidget( this ) );
  mainLay->addWidget( ctrlPointsWidget( this ) );

  // thread part
  _widgets->processRunning = false;
  _widgets->triggerProcessState = false;
  _widgets->beingDestroyed = false;
  _widgets->needsNotifyUpdate = false;

  _widgets->grabView = false;

  resize( minimumSize() );

  connect( chgDirBtn, SIGNAL( clicked() ), this, SLOT( invertDirection() ) );
  connect( _widgets->startBtn, SIGNAL( clicked() ),
	   this, SLOT( startProcess() ) );
  connect( _widgets->stopBtn, SIGNAL( clicked() ),
	   this, SLOT( stopProcess() ) );
  connect( _widgets->resetBtn, SIGNAL( clicked() ),
	   this, SLOT( resetProcess() ) );
  connect( rec, SIGNAL( toggled( bool ) ), this, SLOT( record( bool ) ) );
}


QSurfMatchWin::~QSurfMatchWin()
{
  // cout << "destroy QSurfMatchWin\n";
  _widgets->beingDestroyed = true;

  /*if( _widgets->grabView )
    theAnatomist->setViewControl( 0 );*/

  if( _widgets->processRunning )
    {
      stopProcess();
      _widgets->interfaceSem.post();
      _widgets->processThread.join();
      if( _widgets->needsNotifyUpdate )
        notifyUpdate();
    }

  _obj->deleteObserver( this );
  delete _widgets;
}


void QSurfMatchWin::update( const anatomist::Observable*, void* arg )
{
  //cout << "update\n";
  if( arg == 0 )
    {
      cout << "called obsolete QSurfMatchWin::update( obs, NULL )\n";
      delete this;	// object destroyed: suicide
    }
}


void QSurfMatchWin::unregisterObservable( anatomist::Observable* o )
{
  Observer::unregisterObservable( o );
  if( !_widgets->beingDestroyed )
    delete this;
}


void QSurfMatchWin::fillDirectionLabel()
{
  AObject			*o1, *o2;
  ASurfMatcher::const_iterator	io = _obj->begin();

  if( _obj->ascending() )
    {
      o1 = *io;
      ++io;
      o2 = *io;
    }
  else
    {
      o2 = *io;
      ++io;
      o1 = *io;
    }

  _widgets->orgLabel->setText( o1->name().c_str() );
  _widgets->dstLabel->setText( o2->name().c_str() );
}


void QSurfMatchWin::invertDirection()
{
  _obj->setAscending( !_obj->ascending() );
  fillDirectionLabel();
  _widgets->orgPtsVec.clear();
  _widgets->dstPtsVec.clear();
  _widgets->orgPtsBox->clear();
  _widgets->dstPtsBox->clear();
  _obj->setOrgControlPoints( _widgets->orgPtsVec );
  _obj->setDestControlPoints( _widgets->dstPtsVec );
}


void QSurfMatchWin::startProcess()
{
  // cout << "C'est parti...\n";
  _widgets->startBtn->setEnabled( false );
  _widgets->stopBtn->setEnabled( true );
  _widgets->resetBtn->setEnabled( false );

  _widgets->triggerProcessState = true;
  _widgets->processRunning = true;

  _widgets->processThread.launch();
}


void QSurfMatchWin::stopProcess()
{
  _widgets->startBtn->setEnabled( true );
  _widgets->stopBtn->setEnabled( false );
  _widgets->resetBtn->setEnabled( true );

  if( !_widgets->processRunning )
    {
      cerr << "Already stopped.\n";
      return;
    }
  _widgets->triggerProcessState = false;
}


void QSurfMatchWin::resetProcess()
{
  _obj->resetProcess();
  notifyUpdate();
}


bool QSurfMatchWin::event( QEvent *e )
{
  if( e->type() == QEvent::User + 103 )
  {
    processStepFinished();
    e->accept();
    return true;
  }
  else
    return QWidget::event( e );
}


void QSurfMatchWin::processThread()
{
  // cout << "QSurfMatchWin::processThread\n";
  do
    {
      _widgets->needsNotifyUpdate = true;
      _obj->processStep();

      // cout << "process step finished\n";
      // envoie le signal au thread de l'interface pour faire un refresh
      QApplication::postEvent( this, new QCustomEvent( QEvent::User + 103 ) );
      // attendre que ce soit fait pour continuer
      if( !_widgets->beingDestroyed )
        _widgets->interfaceSem.wait();
    }
  while( _widgets->triggerProcessState && !_obj->processFinished() );

  // end of thread
  _widgets->triggerProcessState = false;
  _widgets->processRunning = false;
  /* déclencher un dernier signal pour arrêter tout du côté du thread
     interface */
  QApplication::postEvent( this, new QCustomEvent( QEvent::User + 103 ) );
  // cout << "thread stopped\n";
}


void QSurfMatchWin::processStepFinished()
{
  if( !_widgets->processRunning )
    return;

  notifyUpdate();

  // cout << "interface update finished. posting\n";
  // dire au process qu'on a fini
  _widgets->interfaceSem.post();
}


void QSurfMatchWin::notifyUpdate()
{
  ASurfMatcher::const_iterator	io = _obj->begin();

  ++io;	// reach 3rd object (modified one)
  ++io;

  if( io != _obj->end() )
    {
      AObject	*surf = *io;
      float	time = surf->MaxT();

      if( _obj->record() )	// records: show last time step
      {
        const set<AWindow *>			& wins = surf->WinList();
        set<AWindow *>::const_iterator	iw, fw=wins.end();

        for( iw=wins.begin(); iw!=fw; ++iw )
          (*iw)->setTime( time );
      }

      (*io)->notifyObservers( this );
    }

  _widgets->needsNotifyUpdate = false;
}


void QSurfMatchWin::record( bool r )
{
  _obj->setRecord( r );
}


QWidget* QSurfMatchWin::paramWidget( QWidget* parent )
{
  SyntaxSet	ss( _obj->paramSyntax() );
  Tree		par( _obj->parameters() );

  QWidget		*pw = new QVGroupBox( tr( "Matching parameters:" ),
					      parent );
  Object		is;
  QGrid			*gd = new QGrid( 2, pw );
  QLabel		*lab;
  QLineEdit		*ed;
  string		str;

  gd->setSpacing( 5 );

  for( is=par.objectIterator(); is->isValid(); is->next() )
    {
      str = is->key();
      lab = new QLabel( str.c_str(), gd );
      ed = new QSurfMatchWin_AttEdit( str, strAttribute( str, par, ss ), gd );
      lab->setMinimumSize( lab->sizeHint() );
      ed->setMinimumSize( ed->sizeHint() );
      connect( ed, SIGNAL( textChanged( const std::string &,
					const QString & ) ),
	       this,
	       SLOT( paramChanged( const std::string &, const QString & ) ) );
    }

  return( pw );
}


QString QSurfMatchWin::strAttribute(  const string & attr,
				      const AttributedObject & ao,
				      const SyntaxSet & synt )
{
  QString			attval;
  SyntaxSet::const_iterator	iss = synt.find( ao.getSyntax() );

  if( iss == synt.end() )
    return( attval );

  Syntax::const_iterator	is = (*iss).second.find( attr );
  if( is == (*iss).second.end() )
    return( attval );

  const Semantic		& sem = (*is).second;

  if( sem.type == "int" )
    {
      int	val;
      ao.getProperty( attr, val );
      attval = QString::number( val );
    }
  else if( sem.type == "float" )
    {
      float	val;
      ao.getProperty( attr, val );
      attval = QString::number( val );
    }
  else
    {
      attval = "unknown type";
    }
  return( attval );
}


void QSurfMatchWin::paramChanged( const string & attrib, const QString & val )
{
  SyntaxSet	synt( _obj->paramSyntax() );
  Tree		par( _obj->parameters() );
  SyntaxSet::const_iterator	iss = synt.find( par.getSyntax() );

  if( iss == synt.end() )
    return;

  Syntax::const_iterator	is = (*iss).second.find( attrib );
  if( is == (*iss).second.end() )
    return;

  const Semantic		& sem = (*is).second;

  //cout << attrib << " changed to " << val << endl;

  if( sem.type == "int" )
    par.setProperty( attrib, val.toInt() );
  else if( sem.type == "float" )
    par.setProperty( attrib, val.toFloat() );
  else
    return;

  _obj->setParameters( par );
}


QWidget* QSurfMatchWin::ctrlPointsWidget( QWidget* parent )
{
  QVGroupBox	*box = new QVGroupBox( tr( "Control points:" ), parent );
  QGrid		*listsBox = new QGrid( 2, box );
  listsBox->setSpacing( 10 );
  new QLabel( tr( "Origin surface:" ), listsBox );
  new QLabel( tr( "Destination surface:" ), listsBox );
  _widgets->orgPtsBox = new QListBox( listsBox );
  _widgets->dstPtsBox = new QListBox( listsBox );
  QPushButton	*addc1 = new QPushButton( tr( "Add point (click)" ),
					  listsBox );
  QPushButton	*addc2 = new QPushButton( tr( "Add point (click)" ),
					  listsBox );
  QPushButton	*addn1 = new QPushButton( tr( "Add point (number)" ),
					  listsBox );
  QPushButton	*addn2 = new QPushButton( tr( "Add point (number)" ),
					  listsBox );
  QPushButton	*del1 = new QPushButton( tr( "Delete point" ), listsBox );
  QPushButton	*del2 = new QPushButton( tr( "Delete point" ), listsBox );
  fillCtrlPoints();

  connect( addc1, SIGNAL( clicked() ), this, SLOT( addPointByClickOrg() ) );
  connect( addc2, SIGNAL( clicked() ), this, SLOT( addPointByClickDst() ) );
  connect( addn1, SIGNAL( clicked() ), this, SLOT( addPointByNumOrg() ) );
  connect( addn2, SIGNAL( clicked() ), this, SLOT( addPointByNumDst() ) );
  connect( del1, SIGNAL( clicked() ), this, SLOT( deletePointOrg() ) );
  connect( del2, SIGNAL( clicked() ), this, SLOT( deletePointDst() ) );

  return( box );
}


void QSurfMatchWin::fillCtrlPoints()
{
  _widgets->orgPtsVec = _obj->orgControlPoints();
  _widgets->dstPtsVec = _obj->destControlPoints();

  QListBox			*org = _widgets->orgPtsBox;
  QListBox			*dst = _widgets->dstPtsBox;
  const vector<unsigned>	& ov = _widgets->orgPtsVec;
  const vector<Point3df>	& dv = _widgets->dstPtsVec;
  unsigned			i, n;

  org->clear();
  dst->clear();

  for( i=0, n=ov.size(); i<n; ++i )
    org->insertItem( QString::number( ov[i] ) );
  for( i=0, n=dv.size(); i<n; ++i )
    dst->insertItem( QString( "( " ) + QString::number( dv[i][0] )
		     + ", " + QString::number( dv[i][1] )
		     + ", " + QString::number( dv[i][2] ) + " )" );
}


void QSurfMatchWin::addPointByClickOrg()
{
  //	temp...
  set<AWindow *>	w = theAnatomist->getWindowsInGroup( 0 );
  if( w.empty() )
    {
      cout << "No window in base group - can't pick current cursor position\n";
      return;
    }

  Point3df			pt = (*w.begin())->getPosition();
  float				d, dmin = 1e38;
  ATriangulated			*ms = _obj->movingSurface();
  rc_ptr<AimsSurfaceTriangle>	s = ms->surface();
  if( !s )
    {
      resetProcess();
      s = ms->surface();
    }
  const vector<Point3df>	& vert
    = _obj->movingSurface()->surface()->vertex();
  unsigned			index = 0, i, n = vert.size();

  // find nearest vertex
  for( i=0; i<n; ++i )
    {
      const Point3df	& v = vert[i];
      d = ( v[0] - pt[0] ) * ( v[0] - pt[0] )
	+ ( v[1] - pt[1] ) * ( v[1] - pt[1] )
	+ ( v[2] - pt[2] ) * ( v[2] - pt[2] );
      if( d < dmin )
	{
	  dmin = d;
	  index = i;
	}
    }

  _widgets->orgPtsVec.push_back( index );
  _widgets->orgPtsBox->insertItem( QString::number( index ) );
  _obj->setOrgControlPoints( _widgets->orgPtsVec );
}


void QSurfMatchWin::addPointByClickDst()
{
  //	temp...
  set<AWindow *>	w = theAnatomist->getWindowsInGroup( 0 );
  if( w.empty() )
    {
      cout << "No window in base group - can't pick current cursor position\n";
      return;
    }

  Point3df	pt = (*w.begin())->getPosition();
  _widgets->dstPtsVec.push_back( pt );
  _widgets->dstPtsBox->insertItem( QString( "( " )
				   + QString::number( pt[0] ) + ", "
				   + QString::number( pt[1] ) + ", "
				   + QString::number( pt[2] ) + " )" );
  _obj->setDestControlPoints( _widgets->dstPtsVec );

  /*if( theAnatomist->viewControl() )
    {
      cerr << "View controller already active: finish started action first.\n";
      return;
    }
  theAnatomist->setViewControl( new APointCollectorTrigger( 1,
							    clickDstPointCbk,
							    this ) );
  _widgets->grabView = true;
  cout << tr( "OK, click point in any 2D/3D window" ) << endl;*/
}


void QSurfMatchWin::addPointByNumOrg()
{
  QStringEdit	*sed = new QStringEdit( "", QCursor::pos().x(),
					QCursor::pos().y(), -1, -1 );

  if( sed->exec() )
    {
      bool	valid;
      unsigned	num = QString( sed->text().c_str() ).toUInt( &valid );
      if( valid )
	{
	  _widgets->orgPtsBox->insertItem( QString::number( num ) );
	  _widgets->orgPtsVec.push_back( num );
	  _obj->setOrgControlPoints( _widgets->orgPtsVec );
	}
    }

  delete sed;
}


void QSurfMatchWin::addPointByNumDst()
{
  QStringEdit	*sed = new QStringEdit( "", QCursor::pos().x(),
					QCursor::pos().y(), -1, -1 );

  if( sed->exec() )
    {
      bool	valid;
      unsigned	num = QString( sed->text().c_str() ).toUInt( &valid );
      if( valid )
	{
	  ATriangulated		*surf = _obj->destSurface();
	  if( surf->surface()->vertex().size() > num )
	    {
	      const Point3df	& pt = surf->surface()->vertex()[num];

	      _widgets->dstPtsBox->insertItem( QString( "( " )
					       + QString::number( pt[0] )
					       + ", "
					       + QString::number( pt[1] )
					       + ", "
					       + QString::number( pt[2] )
					       + " )" );
	      _widgets->dstPtsVec.push_back( pt );
	      _obj->setDestControlPoints( _widgets->dstPtsVec );
	    }
	}
    }

  delete sed;
}


void QSurfMatchWin::deletePointOrg()
{
  int	item = _widgets->orgPtsBox->currentItem();

  if( item >= 0 && _widgets->orgPtsBox->isSelected( item ) )
    {
      delete _widgets->orgPtsBox->item( item );
      _widgets->orgPtsVec.erase( _widgets->orgPtsVec.begin() + item );
      _obj->setOrgControlPoints( _widgets->orgPtsVec );
    }
}


void QSurfMatchWin::deletePointDst()
{
  int	item = _widgets->dstPtsBox->currentItem();

  if( item >= 0 && _widgets->dstPtsBox->isSelected( item ) )
    {
      delete _widgets->dstPtsBox->item( item );
      _widgets->dstPtsVec.erase( _widgets->dstPtsVec.begin() + item );
      _obj->setDestControlPoints( _widgets->dstPtsVec );
    }
}


/*void QSurfMatchWin::clickOrgPointCbk( APointCollector* caller,
				      void *clientdata )
{
  const vector<Point3df>	& pts = caller->points();
  QSurfMatchWin			*sw = (QSurfMatchWin *) clientdata;
  QSurfMatchWin_widgets		*widgets = sw->_widgets;

  if( pts.size() != 1 )
    cerr << "pb : " << pts.size() << " points (expecting 1)\n";
  if( !pts.empty() )
    {
      const Point3df		& pt = pts[0];
      float			d, dmin = 1e38;
      const vector<Point3df>	& vert
	= sw->_obj->movingSurface()->surface()->vertex();
      unsigned			index = 0, i, n = vert.size();

      // find nearest vertex
      for( i=0; i<n; ++i )
	{
	  const Point3df	& v = vert[i];
	  d = ( v[0] - pt[0] ) * ( v[0] - pt[0] )
	    + ( v[1] - pt[1] ) * ( v[1] - pt[1] )
	    + ( v[2] - pt[2] ) * ( v[2] - pt[2] );
	  if( d < dmin )
	    {
	      dmin = d;
	      index = i;
	    }
	}

      widgets->orgPtsVec.push_back( index );
      widgets->orgPtsBox->insertItem( QString::number( index ) );
      sw->_obj->setOrgControlPoints( widgets->orgPtsVec );
      //widgets->dstPtsBox->triggerUpdate( false );
    }

  // delete controller
  theAnatomist->setViewControl( 0 );
  widgets->grabView = false;
  }*/


/*void QSurfMatchWin::clickDstPointCbk( APointCollector* caller,
				      void *clientdata )
{
  const vector<Point3df>	& pts = caller->points();
  QSurfMatchWin			*sw = (QSurfMatchWin *) clientdata;
  QSurfMatchWin_widgets		*widgets = sw->_widgets;

  if( pts.size() != 1 )
    cerr << "pb : " << pts.size() << " points (expecting 1)\n";
  if( !pts.empty() )
    {
      const Point3df		& pt = pts[0];

      widgets->dstPtsVec.push_back( pt );
      widgets->dstPtsBox->insertItem( QString( "( " )
				      + QString::number( pt[0] ) + ", "
				      + QString::number( pt[1] ) + ", "
				      + QString::number( pt[2] ) + " )" );
      sw->_obj->setDestControlPoints( widgets->dstPtsVec );
      //widgets->dstPtsBox->triggerUpdate( false );
    }

  // delete controller
  theAnatomist->setViewControl( 0 );
  widgets->grabView = false;
  }*/


// ------------


QSurfMatchWin_AttEdit::QSurfMatchWin_AttEdit( const string & attr,
					      const QString & txt,
					      QWidget* parent )
  : QLineEdit( txt, parent ), _attrib( attr )
{
  connect( this, SIGNAL( returnPressed() ),
	   SLOT( textChangedSlot() ) );
}


QSurfMatchWin_AttEdit::~QSurfMatchWin_AttEdit()
{
}


void QSurfMatchWin_AttEdit::textChangedSlot()
{
  emit textChanged( _attrib, text() );
}


void QSurfMatchWin_AttEdit::leaveEvent( QEvent* ev )
{
  emit textChanged( _attrib, text() );
  QLineEdit::leaveEvent( ev );
}


void QSurfMatchWin_AttEdit::focusOutEvent( QFocusEvent* ev )
{
  emit textChanged( _attrib, text() );
  QLineEdit::focusOutEvent( ev );
}

