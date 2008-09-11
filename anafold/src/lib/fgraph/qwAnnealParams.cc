/*
 *  Copyright (C) 1998-2005 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 *
 */

#include <anatomist/application/fileDialog.h>
#include <anafold/fgraph/qwAnnealParams.h>
#include <si/graph/annealConfigurator.h>
#include <si/fold/frgraph.h>
#include <anafold/fgraph/afgraph.h>
#include <anatomist/threads/asyncHandler.h>
#include <cartobase/thread/thread.h>
#include <cartobase/thread/semaphore.h>
#include <qlayout.h>
#include <aims/qtcompat/qhbox.h>
#include <aims/qtcompat/qvbox.h>
#include <aims/qtcompat/qgrid.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qtooltip.h>
#include <aims/qtcompat/qfiledialog.h>
#include <qvalidator.h>
#include <iostream>

using namespace anatomist;
using namespace sigraph;
using namespace aims;
using namespace carto;
using namespace std;

class QAnnealParams::AnnealPThread : public Thread
{
public:
  AnnealPThread( QAnnealParams* ap ) : Thread(), _ap( ap ) {}
  virtual ~AnnealPThread() {}
  virtual void doRun()
  { _ap->annealThread(); }

private:
  QAnnealParams	*_ap;
};


struct QAnnealParams::PrivateData
{
  PrivateData( QAnnealParams* ap ) 
    : init( 0 ), mode( 0 ), iter( 0 ), 
      transl( 0 ), temp( 0 ), rate( 0 ), tempicm( 0 ), stoprate( 0 ), 
      verbose( 0 ), gibbschange( 0 ), removebrain( 0 ), setbrain( 0 ), 
      setweight( 0 ), outplot( 0 ), initlabels( 0 ), voidlabel( 0 ), 
      voidmode( 0 ), voidoccur( 0 ), extmode( 0 ), extoccur( 0 ), 
      ddrawlots( 0 ), stoppass( 0 ), annealThread( ap ), 
      annealThRunning( QAnnealParams::STOPPED ), 
      triggerAnnealState( QAnnealParams::STOPPED ), assync( 0 ), 
      threaded( true )
  {}
  ~PrivateData()
  {
    delete assync;
    assync = 0;
  }

  QComboBox			*init;
  QComboBox			*mode;
  QComboBox			*iter;
  QComboBox			*transl;
  QLineEdit			*temp;
  QLineEdit			*rate;
  QLineEdit			*tempicm;
  QLineEdit			*stoprate;
  QComboBox			*verbose;
  QLineEdit			*gibbschange;
  QComboBox			*removebrain;
  QComboBox			*setbrain;
  QLineEdit			*setweight;
  QComboBox			*outplot;
  QComboBox			*initlabels;
  QLineEdit			*voidlabel;
  QComboBox			*voidmode;
  QLineEdit			*voidoccur;
  QComboBox			*extmode;
  QLineEdit			*extoccur;
  QComboBox			*ddrawlots;
  QLineEdit			*stoppass;
  QAnnealParams::AnnealPThread	annealThread;
  Semaphore			interfaceSem;
  QAnnealParams::State		annealThRunning;
  QAnnealParams::State		triggerAnnealState;
  AsyncSync			*assync;
  bool				threaded;
};


QAnnealParams::QAnnealParams( QWidget* parent, const char*, 
			      AFGraph* fusion )
  : QWidget( parent, "QAnnealParams", Qt::WDestructiveClose ), 
  _fusion( fusion ), pdat( new QAnnealParams::PrivateData( this ) )
{
  _conf = new AnnealConfigurator;
  fusion->addObserver( this );

  setCaption( tr( "Annealing" ) + " : " + fusion->name().c_str() );

  QVBoxLayout	*lay1 = new QVBoxLayout( this, 10, -1, "QAnnealParamsLay1" );
  QHBox		*iobox = new QHBox( this );
  iobox->setSpacing( 10 );
  QPushButton	*loadBtn = new QPushButton( tr( "Load config" ), iobox );
  QPushButton	*saveBtn = new QPushButton( tr( "Save config" ), iobox );

  QCheckBox	*thrBox = new QCheckBox( tr( "Threaded interface" ), this );
  QToolTip::add( thrBox, tr( "Using threads allows to keep Anatomist user\n" 
			     "interface active during the several-hours\n" 
			     "long annealing process, and allows to stop it\n" 
			     "but it's more subject to bugs, so don't play\n" 
			     "too much with Anatomist during the process,\n" 
			     "it can easily crash.\n" 
			     "If not threaded, you won't be able to use\n" 
			     "this Anatomist until the relaxation is \n" 
			     "finished (hope 2-4 hours...)\n" 
			     "(expect ~400 passes for a complete "
			     "recognition)" ) );
  thrBox->setChecked( pdat->threaded );
  connect( thrBox, SIGNAL( toggled( bool ) ), this, 
	   SLOT( setThreaded( bool ) ) );

  //	params
  QGrid		*parbox = new QGrid( 2, this );
  parbox->setSpacing( 5 );
  QLabel	*l = new QLabel( tr( "Initialize annealing :" ), parbox );
  QToolTip::add( l, tr( "If not set, annealing will not be initialized\n"
			"(labels will be left unchanged, to continue "
			"an interrupted relaxation for instance)" ) );
  pdat->init = new QComboBox( parbox, "init" );
  pdat->init->insertItem( tr( "yes" ) );
  pdat->init->insertItem( tr( "no" ) );
  l = new QLabel( tr( "Mode :" ), parbox );
  QToolTip::add( l, tr( "Annealing transition accept/reject mode :\n" 
			"Gibbs sampler, Metropolis or ICM (deterministic)" ) );
  pdat->mode = new QComboBox( parbox, "mode" );
  pdat->mode->insertItem( "gibbs" );
  pdat->mode->insertItem( "metro" );
  pdat->mode->insertItem( "icm" );
  l = new QLabel( tr( "Iteration mode :" ), parbox );
  QToolTip::add( l, tr( "Transitions iterate mode\n" 
			"don't select CLIQUE - it's useless !" ) );
  pdat->iter = new QComboBox( parbox, "iter" );
  pdat->iter->insertItem( "VERTEX" );
  pdat->iter->insertItem( "CLIQUE" );
  l = new QLabel( tr( "Translation file :" ), parbox );
  QToolTip::add( l, tr( "Labels translation file\n" 
			 "Tells how to translate elements names\n" 
			 "from one nomenclature to the model one.\n" 
			 "If none, the default one is used\n" 
			 "(see SiGraph library for details)" ) );
  QHBox	*tb = new QHBox( parbox );
  tb->setSpacing( 5 );
  pdat->transl = new QComboBox( tb, "transl" );
  pdat->transl->insertItem( tr( "<default>" ) );
  QPushButton	*trbtn = new QPushButton( "...", tb );
  trbtn->setFixedHeight( pdat->transl->sizeHint().height() );
  trbtn->setFixedWidth( trbtn->sizeHint().width() );
  l = new QLabel( tr( "Temperature :" ), parbox );
  QToolTip::add( l, tr( "Starting annealing temperature, will decrease " 
			 "during the process" ) );
  pdat->temp = new QLineEdit( "50", parbox );
  l = new QLabel( tr( "Rate :" ), parbox );
  QToolTip::add( l, tr( "Temperature decreasing rate - at each pass\n"
			 "the temperature will be multiplied by "
			 "this factor" ) );
  pdat->rate = new QLineEdit( "0.98", parbox );
  l = new QLabel( tr( "ICM switching temp :" ), parbox );
  QToolTip::add( l, tr( "When the temperature gets lower than this\n" 
			 "threshold, annealing automatically switches\n" 
			 "to ICM deterministic mode.\n" 
			 "If left to 0, ICM is used only after a whole pass\n" 
			 "with no changes" ) );
  pdat->tempicm = new QLineEdit( "0", parbox );
  l = new QLabel( tr( "Stop rate :" ), parbox );
  QToolTip::add( l, tr( "% of accepted transitions below which the " 
			"annealing stops" ) );
  pdat->stoprate = new QLineEdit( "0.01", parbox );
  l = new QLabel( tr( "Verbose :" ), parbox );
  QToolTip::add( l, tr( "If set, verbose mode prints lots of counters\n" 
			"to keep you awaken during relaxation" ) );
  pdat->verbose = new QComboBox( parbox, "verbose" );
  pdat->verbose->insertItem( tr( "yes" ) );
  pdat->verbose->insertItem( tr( "no" ) );
  l = new QLabel( tr( "Gibbs nodes changes :" ), parbox );
  QToolTip::add( l, tr( "Number of nodes allowed to change simultaneously " 
			"to form a Gibbs transition\n" 
			"LEAVE IT TO 1 FOR FOLDS RECOGNITION, or annealing "
			"will last for months..." ) );
  pdat->gibbschange = new QLineEdit( "1", parbox );
  l = new QLabel( tr( "Remove brain :" ), parbox );
  QToolTip::add( l, tr( "Removes possible 'brain' nodes in graph -\n" 
			"this shouldn't happen in newer graphs, but it's " 
			"recommended to leave 'yes'" ) );
  pdat->removebrain = new QComboBox( parbox, "removebrain" );
  pdat->removebrain->insertItem( tr( "yes" ) );
  pdat->removebrain->insertItem( tr( "no" ) );
  l = new QLabel( tr( "Set weights :" ), parbox );
  QToolTip::add( l, tr( "Allows to set weights on each model element :\n" 
			"  0 : don't set anything (leaves it as in the "
			"model file)\n" 
			" -1 : explicitly unsets the weights (RECOMMENDED)\n" 
			"t>0 : sets nodes weights to t x num of relations" ) );
  pdat->setweight = new QLineEdit( "-1", parbox, "setweight" );
  l = new QLabel( tr( "Output plot file :" ), parbox );
  QToolTip::add( l, tr( "If a file is specified here, a 'plot file' will\n" 
			"be written during relaxation, with a line for \n" 
			"each pass, containing temperatures, numbers of\n" 
			"accepted transitions, energies, etc.\n" 
			"- Can be viewed with gnuplot for instance" ) );
  QHBox	*opb = new QHBox( parbox );
  opb->setSpacing( 5 );
  pdat->outplot = new QComboBox( opb, "outplot" );
  pdat->outplot->insertItem( tr( "<none>" ) );
  QPushButton	*outbtn = new QPushButton( "...", opb );
  outbtn->setFixedHeight( pdat->outplot->sizeHint().height() );
  outbtn->setFixedWidth( outbtn->sizeHint().width() );
  l= new QLabel( tr( "Initial labels :" ), parbox );
  QToolTip::add( l, tr( "if VOID, all labels are initialized to the void " 
			"(unrecognized) value (see below)\n" 
			"If NONE, labels are not initialized "
			"(left unchanged)\n" 
			"if RANDOM, labels are randomized: each node gets one "
			"of its possible labels") );
  pdat->initlabels = new QComboBox( parbox, "initlabels" );
  pdat->initlabels->insertItem( "VOID" );
  pdat->initlabels->insertItem( "NONE" );
  pdat->initlabels->insertItem( "RANDOM" );
  l = new QLabel( tr( "Void label :" ), parbox );
  QToolTip::add( l, tr( "Label used for unrecognized nodes" ) );
  pdat->voidlabel = new QLineEdit( "unknown", parbox );
  l = new QLabel( tr( "Void pass mode :" ), parbox );
  QToolTip::add( l, tr( "Void pass is a special relaxation pass wich can\n" 
			"occur from time to time to increase annealing " 
			"performance :\n" 
			"it consists in trying to 'remove' a whole fold\n" 
			"in a single transition to avoid aberrant labels " 
			"distributions to persist\n\n" 
			"NONE : don't perform such special passes\n" 
			"REGULAR : perform them regularly, with occurency " 
			"given below\n" 
			"STOCHASTIC : perform them irregularly on a " 
			"mean probability\n based on the occurency below" ) );
  pdat->voidmode = new QComboBox( parbox, "voidmode" );
  pdat->voidmode->insertItem( "NONE" );
  pdat->voidmode->insertItem( "REGULAR" );
  pdat->voidmode->insertItem( "STOCHASTIC" );
  //pdat->voidmode->setCurrentItem( 1 );
  l = new QLabel( tr( "Void pass occurency :" ), parbox );
  QToolTip::add( l, tr( "Occurency (1 out of n) or mean inverse probability\n" 
			"of Void passes (if used)" ) );
  pdat->voidoccur = new QLineEdit( "20", parbox );
  l = new QLabel( tr( "Extension mode :" ), parbox );
  QToolTip::add( l, tr( "List of extended passes used during relaxation\n" 
			"Extended passes are plug-ins that can be inserted\n" 
			"in the annealing process.\n" 
			"Up to now 2 extensions exist :\n\n" 
			"CONNECT_VOID is similar to Void passes, but only\n" 
			" tries to remove one connected component of\n" 
			" a fold at e time\n" 
			"CONNECT inversely tries to mute a connected \n" 
			" component of void label nodes to the same fold " 
			"label\n" 
			" - useful after VOID and/or CONNECT_VOID  passes\n\n" 
			"Both CONNECT_VOID and CONNECT passes seem to\n" 
			"significantly improve the recognition, so you\n" 
			"should certainly use them" ) );
  pdat->extmode = new QComboBox( parbox, "extmode" );
  pdat->extmode->insertItem( "" );
  pdat->extmode->insertItem( "CONNECT" );
  pdat->extmode->insertItem( "CONNECT_VOID" );
  pdat->extmode->insertItem( "CONNECT_VOID CONNECT" );
  pdat->extmode->setEditable( true );
  //pdat->extmode->setCurrentItem( 3 );
  l = new QLabel( tr( "Extension pass occurency :" ), parbox );
  QToolTip::add( l, tr( "Occurency (1 out of n) of extended passes\n" 
			"Up to now they happen regularly in their given\n" 
			"order; if occurency is the same as Void passes,\n" 
			"Void pass always happens first, then immediately\n" 
			"followed by extended passes" ) );
  pdat->extoccur = new QLineEdit( "10", parbox );
  l = new QLabel( tr( "Double drawing lots :" ), parbox );
  QToolTip::add( l, tr( "Performs 2 drawing lots before accepting a " 
			"transition\n(if my memory is good enough...)\n" 
			"This leads to accept only transitions with a high\n" 
			"probability, or no change at all.\n" 
			"NOT RECOMMENDED - it seems to give bad results\n" 
			"and it's theoretically an heresy..." ) );
  pdat->ddrawlots = new QComboBox( parbox, "ddrawlots" );
  pdat->ddrawlots->insertItem( tr( "yes" ) );
  pdat->ddrawlots->insertItem( tr( "no" ) );
  l = new QLabel( tr( "Number of stable passes before stopping :" ), parbox );
  QToolTip::add( l, tr( "Stopping when the number of changes just drops " 
                        "can sometimes be a bit too unstable,\n" 
                        "so specifying a number of consecutive passes " 
                        "below this level before stopping can avoid\n" 
                        "too abrupt stops." ) );
  pdat->stoppass = new QLineEdit( "1", parbox );
  pdat->stoppass->setValidator( new QIntValidator( 0, 10000, parbox ) );
  //pdat->ddrawlots->setCurrentItem( 1 );

  QHBox		*btnbox = new QHBox( this );
  btnbox->setSpacing( 10 );
  QPushButton	*startBtn = new QPushButton( tr( "Start relaxation" ), 
					     btnbox );
  QPushButton	*stopBtn = new QPushButton( tr( "Stop" ), btnbox );

  lay1->addWidget( iobox );
  lay1->addWidget( thrBox );
  lay1->addWidget( parbox );
  lay1->addWidget( btnbox );

  _conf->init();
  updateBoxes();

  connect( loadBtn, SIGNAL( clicked() ), this, SLOT( loadConfig() ) );
  connect( saveBtn, SIGNAL( clicked() ), this, SLOT( saveConfig() ) );
  connect( startBtn, SIGNAL( clicked() ), this, SLOT( start() ) );
  connect( stopBtn, SIGNAL( clicked() ), this, SLOT( stop() ) );

  connect( trbtn, SIGNAL( clicked() ), this, SLOT( selectTranslationFile() ) );
  connect( outbtn, SIGNAL( clicked() ), this, SLOT( selectPlotFile() ) );
}


QAnnealParams::~QAnnealParams()
{
  if( pdat->annealThRunning != STOPPED )
    {
      stop();
      pdat->annealThread.join();
    }
  _fusion->deleteObserver( this );
  delete _conf;
  delete pdat;
}


void QAnnealParams::update( const anatomist::Observable*, void* arg )
{
  if( arg == 0 )
    delete this;

  cout << "QAnnealParams::update\n";
}


void QAnnealParams::loadConfig()
{
  QString filter = tr( "Annealing configuration" );
  filter += " (*.cfg)";
  QFileDialog	& fd = fileDialog();
  fd.setFilters( filter );
  fd.setCaption( tr( "Open annealing configuration" ) );
  fd.setMode( QFileDialog::ExistingFile );
  if( !fd.exec() )
    return;
  
  QString	fname = fd.selectedFile();
  if( !fname.isEmpty() )
    {
      cout << "load config" << fname.utf8().data() << "\n";
      _conf->loadConfig( string( fname.utf8().data() ) );
      updateBoxes();
    }
}


void QAnnealParams::start()
{
  cout << "QAnnealParams::start\n";

  if( pdat->annealThRunning != STOPPED )
    {
      cerr << "Ça tourne déjà !\n";
      return;
    }

  pdat->triggerAnnealState = RUNNING;
  pdat->annealThRunning = RUNNING;

  updateConfig();

  //	version multi-threadée
  if( pdat->threaded )
    {
      if( !pdat->assync )
	{
	  pdat->assync = new AsyncSync;
	  connect( pdat->assync, SIGNAL( activated() ), this, 
		   SLOT( annealStepFinished() ) );
	}

      pdat->annealThread.launch();
    }
  else
    //	versionn mono-thread
    {
      // dans ce cas on appelle la fonction à la main
      annealThread();
    }
}


void QAnnealParams::annealThread()
{
  MGraph	*model = (MGraph *) _fusion->model()->graph();
  FRGraph	*fmodel = dynamic_cast<FRGraph *>( model );
  Anneal	an( *(CGraph *) _fusion->folds()->graph(), *model );
  ofstream	*plotf = 0;

  if( !_conf->plotFile.empty() )
    plotf = new ofstream( _conf->plotFile.c_str() );
  if( plotf && !*plotf )
    {
      cerr << "Cannot open plot file " << _conf->plotFile << endl;
      delete plotf;
      plotf = 0;
    }
  _conf->initAnneal( an, plotf );

  if( fmodel )
  {
    if( _conf->setWeights > 0 )
      fmodel->setWeights( _conf->setWeights );
    else if( _conf->setWeights < 0 )
      {
        cout << "remove weights\n";
        fmodel->removeWeights();
      }
  }

  an.reset();

  while( pdat->triggerAnnealState != STOPPED && !an.isFinished() )
    {
      an.fitStep();

      //	version multi-thread
      if( pdat->threaded )
	{
	  // envoie le signal au thread de l'interface pour faire un refresh
	  pdat->assync->asyncHandler();
	  // attendre que ce soit fait pour continuer
	  pdat->interfaceSem.wait();
	}
      else	// mono-thread
	{
	  updateInterface();
	}
    }

  if( plotf )
    plotf->close();
  delete plotf;

  pdat->triggerAnnealState = STOPPED;
  pdat->annealThRunning = STOPPED;
  if( pdat->threaded )
    {
      /* déclencher un dernier signal pour arrêter tout du côté du thread 
	 interface */
      pdat->assync->asyncHandler();
    }
}


void QAnnealParams::annealStepFinished()
{
  if( pdat->annealThRunning == STOPPED )
    {
      // stoppé ou fini: plus de thread
      cout << "Anneal Thread fini\n";
      //pthread_mutex_unlock( &_interfaceLock );
      return;
    }

  cout << "Anneal refresh\n";
  updateInterface();

  // dire au recuit qu'on s'en occupe
  pdat->interfaceSem.post();
}


void QAnnealParams::updateInterface()
{
  _fusion->folds()->setChanged();
  /*	si l'interface est multi-threadée, il faut faire un update plus 
	poussé pcq on peut aller regarder l'intérieur des graphes pendant que 
	ça tourne */
  if( pdat->threaded )
    _fusion->internalUpdate();
  else	// si c'est mono-thread, on s'emmerde pas
    _fusion->setColors();
  _fusion->notifyObservers( this );
}


void QAnnealParams::stop()
{
  cout << "QAnnealParams::stop\n";

  if( pdat->threaded )
    {
      if( pdat->annealThRunning == STOPPED )
	{
	  cerr << "Déjà arrêté.\n";
	  return;
	}
      pdat->triggerAnnealState = STOPPED;
    }
  else
    cout << "Ce bouton ne sert qu'en mode multi-thread, puisque en mono-" 
	 << "thread, on ne peut pas cliquer dessus pendant le recuit...\n";
}


void QAnnealParams::setThreaded( bool t )
{
  if( pdat->annealThRunning != STOPPED )
    {
      cout << "Ce mode ne peut pas être changé en cours de recuit\n";
      if( !pdat->threaded )
	{
	  cerr << "d'ailleurs on ne devrait aps pouvoir cliquer en ce "
	       << "moment... (BUG)\n";
	}
      return;
    }
  pdat->threaded = t;
  cout << "Interface ";
  if( !pdat->threaded )
    cout << "not ";
  cout << "threaded.\n";
}


void QAnnealParams::selectTranslationFile()
{
  QString	init;
  if( pdat->transl->currentItem() != 0 )
    init = pdat->transl->currentText();

  QString filter = tr( "Translation file" );
  filter += " (*.def)";
  QFileDialog	& fd = fileDialog();
  fd.setFilters( filter );
  fd.setCaption( tr( "Open translation file" ) );
  fd.setMode( QFileDialog::ExistingFile );
  if( !fd.exec() )
    return;

  QString	fname = fd.selectedFile();
  if( !fname.isNull() && !fname.isEmpty() )
    {
      unsigned	i, n = pdat->transl->count();
      for( i=1; i<n && pdat->transl->text( i )!=fname; ++i ) {}
      if( i == n )
        pdat->transl->insertItem( fname );
      pdat->transl->setCurrentItem( i );
    }
}


void QAnnealParams::selectPlotFile()
{
  QString	init;
  if( pdat->outplot->currentItem() != 0 )
    init = pdat->outplot->currentText();

  QString filter = tr( "Output plot file" );
  filter += " (*.dat)";
  QFileDialog	& fd = fileDialog();
  fd.setFilters( filter );
  fd.setCaption( tr( "Select output plot file" ) );
  fd.setMode( QFileDialog::AnyFile );
  if( !fd.exec() )
    return;

  QString	fname = fd.selectedFile();
  if( !fname.isNull() && !fname.isEmpty() )
    {
      unsigned	i, n = pdat->outplot->count();
      for( i=1; i<n && pdat->outplot->text( i )!=fname; ++i ) {}
      if( i == n )
        pdat->outplot->insertItem( fname );
      pdat->outplot->setCurrentItem( i );
    }
}


void QAnnealParams::updateConfig()
{
  //	init
  _conf->initMode = 1 - pdat->init->currentItem();

  //	mode
  _conf->mode = pdat->mode->currentText().utf8().data();

  //	iteration mode
  _conf->iterType = pdat->iter->currentText().utf8().data();

  //	translation
  if( pdat->transl->currentItem() == 0 )
    _conf->labelsMapFile = "";
  else
    _conf->labelsMapFile = pdat->transl->currentText().utf8().data();

  //	temp
  _conf->temp = pdat->temp->text().toFloat();

  //	rate
  _conf->rate = pdat->rate->text().toFloat();

  //	ICM temp
  _conf->tempICM = pdat->tempicm->text().toFloat();

  //	stop rate
  _conf->stopRate = pdat->stoprate->text().toFloat();

  //	verbose
  _conf->verbose = 1 - pdat->verbose->currentItem();

  //	gibbs changes
  _conf->gibbsChange = pdat->gibbschange->text().toInt();

  //	remove brain
  _conf->removeVoid = 1 - pdat->removebrain->currentItem();

  //	set weights
  _conf->setWeights = pdat->setweight->text().toFloat();

  //	ouput plot file
  if( pdat->outplot->currentItem() == 0 )
    _conf->plotFile = "";
  else
    _conf->plotFile = pdat->outplot->currentText().utf8().data();

  //	init labels
  _conf->initLabelTypeString = pdat->initlabels->currentText().utf8().data();

  //	void label
  _conf->voidLabel = pdat->voidlabel->text().utf8().data();

  //	void pass mode
  _conf->voidMode = pdat->voidmode->currentText().utf8().data();

  //	void pass occurency
  _conf->voidOccurency = pdat->voidoccur->text().toInt();

  //	extension mode
  _conf->extensionMode = pdat->extmode->currentText().utf8().data();

  //	extension pass occurency
  _conf->extPassOccurency = pdat->extoccur->text().toInt();

  //	double drawing lots
  _conf->doubleDrawingLots = 1 - pdat->ddrawlots->currentItem();

  //	stop steps
  _conf->niterBelowStopProp = pdat->stoppass->text().toInt();

  _conf->processParams();
}


void QAnnealParams::updateBoxes()
{
  unsigned	i, n;

  //	init
  pdat->init->setCurrentItem( 1 - _conf->initMode );

  //	mode
  n = pdat->mode->count();
  for( i=0; i<n && pdat->mode->text( i )!=_conf->mode.c_str(); ++i ) {}
  if( i == n )
    pdat->mode->insertItem( _conf->mode.c_str() );
  pdat->mode->setCurrentItem( i );

  //	iteration mode
  n = pdat->iter->count();
  for( i=0; i<n && pdat->iter->text( i )!=_conf->iterType.c_str(); ++i ) {}
  if( i == n )
    pdat->iter->insertItem( _conf->iterType.c_str() );
  pdat->iter->setCurrentItem( i );

  //	translation
  if( !_conf->labelsMapFile.empty() )
    {
      n = pdat->transl->count();
      for( i=1; i<n && pdat->transl->text( i )!=_conf->labelsMapFile.c_str();
           ++i ) {}
      if( i == n )
        pdat->transl->insertItem( _conf->labelsMapFile.c_str() );
      pdat->transl->setCurrentItem( i );
    }
  else
    pdat->transl->setCurrentItem( 0 );

  //	temp
  pdat->temp->setText( QString::number( _conf->temp ) );

  //	rate
  pdat->rate->setText( QString::number( _conf->rate ) );

  //	ICM temp
  pdat->tempicm->setText( QString::number( _conf->tempICM ) );

  //	stop rate
  pdat->stoprate->setText( QString::number( _conf->stopRate ) );

  //	verbose
  pdat->verbose->setCurrentItem( 1 - _conf->verbose );

  //	gibbs changes
  pdat->gibbschange->setText( QString::number( _conf->gibbsChange ) );

  //	remove brain
  pdat->removebrain->setCurrentItem( 1 - _conf->removeVoid );

  //	set weights
  pdat->setweight->setText( QString::number( _conf->setWeights ) );

  //	output plot file
  if( !_conf->plotFile.empty() )
    {
      n = pdat->outplot->count();
      for( i=1; i<n && pdat->outplot->text( i )!=_conf->plotFile.c_str();
           ++i ) {}
      if( i == n )
        pdat->outplot->insertItem( _conf->plotFile.c_str() );
      pdat->outplot->setCurrentItem( i );
    }
  else
    pdat->outplot->setCurrentItem( 0 );

  //	init labels
  n = pdat->initlabels->count();
  for( i=0; i<n 
       && pdat->initlabels->text( i )!=_conf->initLabelTypeString.c_str();
       ++i ) {}
  if( i == n )
    pdat->initlabels->insertItem( _conf->initLabelTypeString.c_str() );
  pdat->initlabels->setCurrentItem( i );

  //	void label
  pdat->voidlabel->setText( _conf->voidLabel.c_str() );

  //	void pass mode
  n = pdat->voidmode->count();
  for( i=0; i<n && pdat->voidmode->text( i )!=_conf->voidMode.c_str(); ++i ) {}
  if( i == n )
    pdat->voidmode->insertItem( _conf->voidMode.c_str() );
  pdat->voidmode->setCurrentItem( i );

  //	void pass occurency
  pdat->voidoccur->setText( QString::number( _conf->voidOccurency ) );

  //	extension mode
  n = pdat->extmode->count();
  for( i=0; i<n && pdat->extmode->text( i )!=_conf->extensionMode.c_str(); 
       ++i ) {}
  if( i == n )
    pdat->extmode->insertItem( _conf->extensionMode.c_str() );
  pdat->extmode->setCurrentItem( i );

  //	extension pass occurency
  pdat->extoccur->setText( QString::number( _conf->extPassOccurency ) );

  //	double drawing lots
  pdat->ddrawlots->setCurrentItem( 1 - _conf->doubleDrawingLots );

  //	stop steps
  pdat->stoppass->setText( QString::number( _conf->niterBelowStopProp ) );
}


void QAnnealParams::saveConfig()
{
  QString filter = tr( "Annealing configuration" );
  filter += " (*.cfg)";
  QFileDialog	& fd = fileDialog();
  fd.setFilters( filter );
  fd.setCaption( tr( "Save annealing config file" ) );
  fd.setMode( QFileDialog::AnyFile );
  if( !fd.exec() )
    return;

  QString	fname = fd.selectedFile();
  if( !fname.isNull() && !fname.isEmpty() )
    {
      updateConfig();
      _conf->modelFile = _fusion->model()->fileName();
      _conf->graphFile = _fusion->folds()->fileName();
      _conf->save = 1;
      _conf->saveConfig( fname.utf8().data() );
    }
}


