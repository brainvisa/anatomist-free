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


#include <assert.h>
#include <aims/qtcompat/qgroupbox.h> // needs to be 1st for now
#include <anatomist/winperf/perfWin.h>
#include <anatomist/winperf/qfloatspinbox.h>
#include <anatomist/perfusion/perfProcQtDeco.h>
#include <aims/perfusion/perfProcCenter.h>
#include <aims/perfusion/perfMask.h>
#include <aims/perfusion/perfSkip.h>
#include <aims/perfusion/perfAifPoints.h>
#include <aims/perfusion/perfPreInj.h>
#include <aims/perfusion/perfQuantif.h>
#include <aims/perfusion/perfAif.h>
#include <aims/perfusion/perfFit.h>
#include <aims/perfusion/perfDeconv.h>
#include <aims/perfusion/perfMaps.h>
#include <aims/perfusion/perfSVDInv.h>
#include <anatomist/winprof/profWin.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/window/winFactory.h>
#include <anatomist/volume/Volume.h>
#include <anatomist/processor/Processor.h>
#include <anatomist/commands/cAddObject.h>
#include <anatomist/commands/cCloseWindow.h>
#include <anatomist/object/actions.h>

#include <graph/tree/tree.h>

#include <aims/data/pheader.h>

#include <qlineedit.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <aims/qtcompat/qvbox.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qcombobox.h>

#include <stdio.h>

using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;

bool QAPerfusionWindow::_typePW = registerClass();


QAPerfusionWindow::QAPerfusionWindow( QWidget *p, const char *name ) 
  : QHBox( p )
{
  assert( theAnatomist );
  setCaption( "Perfusion" );
  setObjectName(name);
  setAttribute(Qt::WA_DeleteOnClose);
  setSpacing( 5 );

  procC = new PerfusionProcessingCenter();
  ppc = new PerfusionProcessingQtDecorator( procC, this );

  Q3GroupBox *gbox = new Q3GroupBox( 2, Qt::Horizontal, "Processing", this );
  QWidget *lpan = new QWidget( gbox, "lpan" );

  // Processing graph
  QGridLayout *gridLay = new QGridLayout( lpan, 21, 2 );
  gridLay->setSpacing( 5 );
  QLabel *img = new QLabel( "Image", lpan, "image" );
  img->setFrameStyle( QFrame::Panel | QFrame::Raised );
  img->setFixedSize( img->sizeHint() );
  QPushButton *skipVal = new QPushButton( "Signal stabilization", lpan, 
  					  "skip" );
  QPushButton *brainMask = new QPushButton( "Brain mask", lpan, "brainMask" );
  QPushButton *listaif = new QPushButton( "Possible AIF(s)", lpan, "listaif" );
  QPushButton *preinj = new QPushButton( "Signal baseline", lpan, "preinj" );
  QPushButton *quant = new QPushButton( "Quantification", lpan, "quantif" );
  QPushButton *aifw = new QPushButton( "AIF", lpan, "aif" );
  QPushButton *fit = new QPushButton( "Fit", lpan, "fit" );
  QPushButton *deconv = new QPushButton( "SVD deconvolution", lpan, "deconv" );
  QPushButton *perfm = new QPushButton( "Perfusion maps", lpan, "perfm" );
  Q3ButtonGroup *pbg = new Q3ButtonGroup( lpan );
  pbg->hide();

  PerfusionSkip pskip;
  pbg->insert( skipVal, pskip.id() );

  PerfusionMask pmask;
  pbg->insert( brainMask, pmask.id() );

  PerfusionAifPoints paifp;
  pbg->insert( listaif, paifp.id() );
  
  PerfusionPreInjection pinj;
  pbg->insert( preinj, pinj.id() );

  PerfusionQuantification pquant;
  pbg->insert( quant, pquant.id() );

  PerfusionAif paif;  
  pbg->insert( aifw, paif.id() );

  PerfusionFit pfit;
  pbg->insert( fit, pfit.id() );
  
  PerfusionDeconvolution pdeconv;
  pbg->insert( deconv, pdeconv.id() );

  PerfusionMaps pmap;
  pbg->insert( perfm, pmap.id() );

  connect( pbg, SIGNAL( clicked( int ) ), ppc, SLOT( apply( int ) ) );

  QLabel *lien1 = new QLabel( "|", lpan, "lien1" );
  QLabel *lien2 = new QLabel( "|", lpan, "lien2" );
  QLabel *lien3 = new QLabel( "|", lpan, "lien3" );
  QLabel *lien4 = new QLabel( "|", lpan, "lien4" );
  QLabel *lien5 = new QLabel( "|", lpan, "lien5" );
  QLabel *lien6 = new QLabel( "|", lpan, "lien6" );
  QLabel *lien7 = new QLabel( "|", lpan, "lien7" );
  QLabel *lien8 = new QLabel( "|", lpan, "lien8" );
  QLabel *lien9 = new QLabel( "|", lpan, "lien9" );
  gridLay->addWidget( img, 0, 0, Qt::AlignHCenter );
  gridLay->addWidget( lien1, 1, 0, Qt::AlignHCenter );
  gridLay->addWidget( skipVal, 2, 0, Qt::AlignHCenter );
  gridLay->addWidget( lien2, 3, 0, Qt::AlignHCenter );
  gridLay->addWidget( brainMask, 4, 0, Qt::AlignHCenter );
  gridLay->addWidget( lien3, 5, 0, Qt::AlignHCenter );
  gridLay->addWidget( listaif, 6, 0, Qt::AlignHCenter );
  gridLay->addWidget( lien4, 7, 0, Qt::AlignHCenter );
  gridLay->addWidget( preinj, 8, 0, Qt::AlignHCenter );
  gridLay->addWidget( lien5, 9, 0, Qt::AlignHCenter );
  gridLay->addWidget( quant, 10, 0, Qt::AlignHCenter );
  gridLay->addWidget( lien6, 11, 0, Qt::AlignHCenter );
  gridLay->addWidget( aifw, 12, 0, Qt::AlignHCenter );
  gridLay->addWidget( lien7, 13, 0, Qt::AlignHCenter );
  gridLay->addWidget( fit, 14, 0, Qt::AlignHCenter );
  gridLay->addWidget( lien8, 15, 0, Qt::AlignHCenter );
  gridLay->addWidget( deconv, 16, 0, Qt::AlignHCenter );
  gridLay->addWidget( lien9, 17, 0, Qt::AlignHCenter );
  gridLay->addWidget( perfm, 18, 0, Qt::AlignHCenter );

  // Image parameters
  QHBox *ipar = new QHBox( lpan, "ipar" );
  ipar->setSpacing( 5 );
  new QLabel( "Tr (ms):", ipar );
  trle = new QLineEdit( "0", ipar, "trle" );
  trle->setMaxLength( 7 );
  trle->setFixedWidth( trle->width() * 2 / 3 );
  new QLabel( "Te (ms):", ipar );
  tele = new QLineEdit( "0", ipar, "tele" );
  tele->setMaxLength( 7 );
  tele->setFixedWidth( tele->width() * 2 / 3 );
  gridLay->addWidget( ipar, 0, 1 );

  connect( trle, SIGNAL( textChanged( const QString& ) ), 
	   ppc, SLOT( setTr( const QString& ) ) );
  connect( tele, SIGNAL( textChanged( const QString& ) ), 
	   ppc, SLOT( setTe( const QString& ) ) );

  // Signal stabilization parameters
  QHBox *vbss = new QHBox( lpan );
  vbss->setSpacing( 5 );
  new QLabel( "Threshold (%):", vbss );
  QFloatSpinBox *flsb = new QFloatSpinBox( 0, 1000, 1, 1, vbss );
  flsb->setFixedWidth( flsb->width() * 2 / 3 );
  flsb->setValue( (int)( procC->parameters().skipThres() * 1000.0f + 0.5f ) );
  new QLabel( "Skip:", vbss );
  QSpinBox *sssb = new QSpinBox( 1, 20, 1, vbss );
  sssb->setFixedWidth( sssb->width() / 2 );
  sssb->setValue( procC->parameters().skip() );
  gridLay->addWidget( vbss, 2, 1 );

  connect( flsb, SIGNAL( valueChanged( int ) ), flsb, SLOT( valChange(int) ) );
  connect( flsb, SIGNAL( valueChanged( float ) ), 
	   ppc, SLOT( setSkipThreshold( float ) ) );
  connect( sssb, SIGNAL( valueChanged( int ) ), ppc, SLOT( setSkip( int ) ) );
  connect( ppc, SIGNAL( skipChanged( int ) ), sssb, SLOT( setValue( int ) ) );

  // Brain mask parameters
  QVBox *bmpar = new QVBox( lpan, "bmpar" );
  QCheckBox *mcb = new QCheckBox( "Apply V-Filter", bmpar );
  mcb->setChecked( true );
  QHBox *bmlay = new QHBox( bmpar );
  bmlay->setSpacing( 5 );
  new QLabel( "Threshold (%):", bmlay );
  new QLabel( "Brain", bmlay );
  QSpinBox *bthsb = new QSpinBox( 1, 100, 1, bmlay );
  bthsb->setValue( (int)( procC->parameters().bThres() * 100.0f ) );
  new QLabel( "CSF", bmlay );
  QSpinBox *thsb = new QSpinBox( 1, 100, 1, bmlay );
  thsb->setValue( (int)( procC->parameters().lvThres() * 100.0f ) );
  gridLay->addWidget( bmpar, 4, 1 );

  connect( mcb, SIGNAL( toggled(bool) ), ppc, SLOT( setMaskFilter(bool) ) );
  connect( bthsb, SIGNAL( valueChanged( int ) ), 
           ppc, SLOT( setBThreshold( int ) ) );
  connect( thsb, SIGNAL( valueChanged( int ) ), 
           ppc, SLOT( setLVThreshold( int ) ) );

  // Possible AIF(s) parameters
  QHBox *pahb = new QHBox( lpan );
  pahb->setSpacing( 5 );
  new QLabel( "Threshold (%):", pahb );
  QSpinBox *patsb = new QSpinBox( 0, 100, 1, pahb );
  patsb->setValue( (int)( procC->parameters().aifThreshold() * 100.0f ) );
  new QLabel( "List size:", pahb );
  QSpinBox *pasb = new QSpinBox( 0, 500, 1, pahb );
  pasb->setValue( procC->parameters().nAif() );
  gridLay->addWidget( pahb, 6, 1 );

  connect( patsb, SIGNAL( valueChanged( int ) ), 
           ppc, SLOT( setAifThreshold( int ) ) );
  connect( pasb, SIGNAL( valueChanged( int ) ), ppc, SLOT( setNAif( int ) ) );

  // Signal baseline parameters
  QHBox *pipar = new QHBox( lpan );
  pipar->setSpacing( 5 );
  new QLabel( "Bolus arrival time:", pipar );
  QSpinBox *pisb = new QSpinBox( 2, 20, 1, pipar );
  pisb->setValue( procC->parameters().preInj() );
  gridLay->addWidget( pipar, 8, 1 );

  connect( pisb, SIGNAL( valueChanged(int) ), ppc, SLOT( setPreInj(int) ) );
  connect( ppc, SIGNAL( preInjChanged(int) ), pisb, SLOT( setValue(int) ) );

  // Fit parameters
  QComboBox *cbf = new QComboBox( lpan );
  cbf->insertItem( "Non-linear gamma variate", 
                   (int)PerfusionFit::GammaVariate );
  cbf->insertItem( "Linearized gamma variate (faster)",
                   (int)PerfusionFit::LinearGammaVariate );
  cbf->setCurrentItem( (int)PerfusionFit::GammaVariate );
  gridLay->addWidget( cbf, 14, 1 );

  connect( cbf, SIGNAL( activated( int ) ), ppc, SLOT( setFitType( int ) ) );

  // Deconvolution parameters
  QVBox *dvb = new QVBox( lpan );
  QCheckBox *dcb = new QCheckBox( "Linearity correction", dvb );
  dcb->setChecked( true );
  QHBox *dhb = new QHBox( dvb );
  dhb->setSpacing( 5 );
  QComboBox *cbd = new QComboBox( dhb );
  cbd->insertItem( "Truncated", (int)PerfusionSVDInversion::Truncated );
  cbd->insertItem( "Tikhonov I",(int)PerfusionSVDInversion::Tikhonov );
  cbd->insertItem( "Tikhonov G",(int)PerfusionSVDInversion::TikhonovGradient );
  cbd->setCurrentItem( (int)PerfusionSVDInversion::Truncated );
  new QLabel( "Threshold (%):", dhb );
  QFloatSpinBox *dsb = new QFloatSpinBox( 0, 1000, 1, 1, dhb );
  dsb->setValue( (int)( procC->parameters().svdThreshold() * 1000.0f + .5f ) );
  gridLay->addWidget( dvb, 16, 1 );

  connect( dcb, SIGNAL( toggled(bool) ), ppc, SLOT( setCorrection(bool) ) );
  connect( cbd, SIGNAL( activated( int ) ), ppc, SLOT( setSVDType( int ) ) );
  connect( dsb, SIGNAL( valueChanged( int ) ), dsb, SLOT( valChange( int ) ) );
  connect( dsb, SIGNAL( valueChanged( float ) ), 
           ppc, SLOT( setSVDThreshold( float ) ) );

  // Maps parameters
  QHBox *mphb = new QHBox( lpan );
  mphb->setSpacing( 5 );
  new QLabel( "Dose (?):", mphb );
  char tmpTxt[20];
  sprintf( tmpTxt, "%f", procC->parameters().dose() );
  QLineEdit *dle = new QLineEdit( tmpTxt, mphb, "dle" );
  dle->setMaxLength( 7 );
  dle->setFixedWidth( dle->width() * 2 / 3 );
  new QLabel( "PhiGd:", mphb );
  sprintf( tmpTxt, "%f", procC->parameters().phiGd() );
  QLineEdit *ple = new QLineEdit( tmpTxt, mphb, "ple" );
  ple->setMaxLength( 7 );
  ple->setFixedWidth( ple->width() * 2 / 3 );
  gridLay->addWidget( mphb, 18, 1 );

  connect( dle, SIGNAL( textChanged( const QString& ) ), 
           ppc, SLOT( setDose( const QString& ) ) );
  connect( ple, SIGNAL( textChanged( const QString& ) ), 
           ppc, SLOT( setPhiGd( const QString& ) ) );

  // List of possible AIF(s)
  QVBox *rpan = new QVBox( this );
  rpan->setSpacing( 5 );
  new QLabel( "Possible AIF(s):", rpan );
  lview = new Q3ListView( rpan, "lview" );
  lview->setAllColumnsShowFocus( true );
  lview->setSelectionMode( Q3ListView::Extended );
  lview->setSorting( -1, true );
  lview->addColumn( "Delta", 56 );
  lview->addColumn( "t", 32 );
  lview->addColumn( "Point", 96 );

#if QT_VERSION >= 0x040000
  connect( lview, SIGNAL( clicked( Q3ListViewItem * ) ), 
	   ppc, SLOT( linkedCursor( Q3ListViewItem * ) ) );
#else
  connect( lview, SIGNAL( clicked( QListViewItem * ) ), 
	   ppc, SLOT( linkedCursor( QListViewItem * ) ) );
#endif

  // The various maps for choice
  Q3GroupBox *gb = new Q3GroupBox( 4, Qt::Vertical, "Maps", rpan );
  QCheckBox *mcbv = new QCheckBox( "CBV", gb );
  QCheckBox *mcbf = new QCheckBox( "CBF", gb );
  QCheckBox *mmtt = new QCheckBox( "MTT", gb );
  QCheckBox *mttp = new QCheckBox( "TTP", gb );
  QCheckBox *mdelay = new QCheckBox( "Delay", gb );
  QCheckBox *mh = new QCheckBox( "h(t)", gb );
  QCheckBox *mbbb = new QCheckBox( "BBB", gb );
  qgbm = new Q3ButtonGroup( rpan );
  qgbm->hide();
  qgbm->insert( mcbv, (int)PerfusionMapBase::cbv );
  qgbm->insert( mcbf, (int)PerfusionMapBase::cbf );
  qgbm->insert( mmtt, (int)PerfusionMapBase::mtt );
  qgbm->insert( mttp, (int)PerfusionMapBase::ttp );
  qgbm->insert( mdelay, (int)PerfusionMapBase::delay );
  qgbm->insert( mh, (int)PerfusionMapBase::h );
  qgbm->insert( mbbb, (int)PerfusionMapBase::bbb );

  connect( qgbm, SIGNAL( clicked( int ) ), ppc, SLOT( mapClicked( int ) ) );

  // Maps saving button
  QPushButton *qsm = new QPushButton( "Save Maps", rpan );
  qsm->setFixedSize( qsm->sizeHint() );

  connect( qsm, SIGNAL( clicked() ), ppc, SLOT( saveMaps() ) );

  setFixedSize( sizeHint() );
}


QAPerfusionWindow::~QAPerfusionWindow()
{
  delete ppc;
  delete procC;
}


bool QAPerfusionWindow::registerClass()
{
  AVolume< short > *vol = new AVolume< short >;
  Tree *tr = (Tree *)vol->optionTree();
  Tree *t2, *t3;

  t2 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "Perfusion" ) );
  tr->insert( t2 );
  t3 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", 
					  "Perfusion Control Window" ) );
  t3->setProperty( "callback", &createPerfusion );
  t2->insert( t3 );

  return true;
}


void QAPerfusionWindow::createPerfusion( const set< AObject * >& obj )
{
  set< AObject * >::const_iterator it = obj.begin(), itf = obj.end();

  while( it != itf && (*it)->type() != AObject::VOLUME )  ++it;

  if ( it != itf ) 
    {
      AVolume< byte > *volb = dynamic_cast< AVolume< byte > * >( *it );
      if ( !volb )
	{
	  AVolume< short > *vols;
	  vols = dynamic_cast< AVolume< short > * >( *it );
	  if ( !vols )
	    {
	      cerr << "WARNING: cannot register object that is not byte ";
	      cerr << "or short." << endl;
	      return;
	    }
	}

      QAPerfusionWindow *qpw = new QAPerfusionWindow( theAnatomist->getQWidgetAncestor(), "Perfusion" );
      qpw->setWindowFlags(Qt::Window);

      set< AWindow * > wlist;
      set< AObject * > olist;
      AWindow *prof = AWindowFactory::createWindow( "Profile" );
      wlist.insert( prof );

      AWindow *axiW = AWindowFactory::createWindow( "Axial" );
      wlist.insert( axiW );

      qpw->registerObject( *it );
      olist.insert( *it );

      Command *cmd = new AddObjectCommand( olist, wlist );
      theProcessor->execute( cmd );

      qpw->show();

      if ( (*it)->MaxT() > 1 )  
	((QAProfileWindow *)prof)->setDirection( QAProfileWindow::alongT );
      else
	{
	  Point3df	bmin, bmax;
	  (*it)->boundingBox( bmin, bmax );
	  if ( bmax[2] > 1 )
	    ((QAProfileWindow *)prof)->setDirection( QAProfileWindow::alongZ );
	}
    }
}


void QAPerfusionWindow::registerObject( AObject *object )
{
  if ( _sobjects.empty() )
    {
      _sobjects.insert( object );

      QString str = "Perfusion: ";
      str += object->name().c_str();
      setCaption( str );

      int dT = 1;
      PropertySet *aheader = 0;
      AVolume< byte > *volb = dynamic_cast< AVolume< byte > * >( object );
      if ( volb )
        {
	  aheader = &volb->volume()->header();
	  dT = volb->volume()->getSizeT();
	}
      else
	{
	  AVolume< short > *vols;
	  vols = dynamic_cast< AVolume< short > * >( object );
	  if ( vols )
	    {
	      aheader = &vols->volume()->header();
	      dT = vols->volume()->getSizeT();
	    }
	}

      float tmp;
      char val[50];
      if ( aheader && aheader->getProperty( "tr", tmp ) )
	{
	  sprintf( val, "%f", tmp );
	  trle->setText( QString( val ) );
	}

      if ( aheader && aheader->getProperty( "te", tmp ) )
	{
	  sprintf( val, "%f", tmp );
	  tele->setText( QString( val ) );
	}
    }
  else
    {
      cerr << "An object is already present!" << endl;
    }
}


void QAPerfusionWindow::unregisterObject( AObject *object )
{
  set< AObject * >::iterator it = _sobjects.find( object );

  if ( it != _sobjects.end() )  _sobjects.erase( it );
}
