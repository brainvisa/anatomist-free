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


#include <anatomist/mobject/wFusion3D.h>

#include <qlayout.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qgroupbox.h>
#include <qradiobutton.h>
#include <qlabel.h>
#include <qslider.h>
#include <qpixmap.h>
#include <qlineedit.h>
#include <qvalidator.h>

#include <anatomist/mobject/Fusion3D.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/processor/Processor.h>
#include <anatomist/object/objectparamselect.h>
#include <anatomist/application/settings.h>


using namespace anatomist;
using namespace std;


struct Fusion3DWindow::Private
{
  Private() {}
  QButtonGroup			*modegrp;
  QButtonGroup			*methgrp;
  QButtonGroup			*submethgrp;
  QSlider			*linratesl;
  QLabel			*linratelab;
  QSlider			*depthsl;
  QLineEdit			*depthlab;
  QSlider			*stepsl;
  QLineEdit			*steplab;
  set<Fusion3D *>		parents;
  set<AObject *>		initial;
  Fusion3D::glTextureMode	mode;
  bool				modeHasChanged;
  int				rate;
  bool				rateHasChanged;
  Fusion3D::Method		method;
  bool				methodHasChanged;
  Fusion3D::SubMethod		submethod;
  bool				submethodHasChanged;
  float				depth;
  bool				depthHasChanged;
  float				step;
  bool				stepHasChanged;
  ObjectParamSelect		*objsel;
  QWidget			*main;
};


Fusion3DWindow::Fusion3DWindow( const set<AObject *> &objL, QWidget* parent, 
				const char *name ) 
  : QWidget( parent ), Observer(),
    d( new Private )
{
    setObjectName(name);
    setAttribute(Qt::WA_DeleteOnClose);
  set<AObject*>::const_iterator	io, fo = objL.end();

  for( io=objL.begin(); io!=fo; ++io )
    if ( (*io)->type() == AObject::FUSION3D )
      {
	d->parents.insert( (Fusion3D *) *io );
        d->initial.insert( *io );
	(*io)->addObserver( (Observer*) this );
      }

  if( !d->parents.empty() )
    {
      Fusion3D *fobj = (Fusion3D *)(*d->parents.begin());
      d->rate = (int) ( fobj->glTexRate() * 100 );
      d->mode = fobj->glTexMode();
      d->method = fobj->method();
      d->submethod = fobj->subMethod();
      d->depth = fobj->depth();
      d->step = fobj->step();
    }
  else
    cerr << "Fusion3DWindow::Fusion3DWindow : no 3D fusion parent\n";

  d->modeHasChanged = false;
  d->rateHasChanged = false;
  d->methodHasChanged = false;
  d->submethodHasChanged = false;
  d->depthHasChanged = false;
  d->stepHasChanged = false;

  setWindowTitle( name );
  if( parent == 0 )
  {
    QPixmap	anaicon( Settings::findResourceFile(
      "icons/icon.xpm" ).c_str() );
    if( !anaicon.isNull() )
      setWindowIcon( anaicon );
  }

  drawContents();
}


Fusion3DWindow::~Fusion3DWindow()
{
  cleanupObserver();
  delete d;
}


namespace
{

  bool filterFusion3D( const AObject* o )
  {
    return o->type() == AObject::FUSION3D;
  }

}


void Fusion3DWindow::drawContents()
{
  QVBoxLayout	*mainlay = new QVBoxLayout( this );
  mainlay->setMargin( 10 );
  mainlay->setSpacing( 10 );

  set<AObject *>	obj;
  obj.insert( d->parents.begin(), d->parents.end() );
  d->objsel = new ObjectParamSelect( obj, this );
  d->objsel->addFilter( filterFusion3D );
  mainlay->addWidget( d->objsel );

  d->main = new QWidget( this );
  QVBoxLayout *vlay = new QVBoxLayout( d->main );
  vlay->setSpacing( 10 );
  vlay->setMargin( 0 );
  mainlay->addWidget( d->main );

  QWidget *hbox = new QWidget( d->main );
  vlay->addWidget( hbox );
  QHBoxLayout *hlay = new QHBoxLayout( hbox );
  hlay->setMargin( 0 );
  QPushButton	*applybtn = new QPushButton( tr( "Apply" ), d->main );
  vlay->addWidget( applybtn );

  applybtn->setFixedSize( applybtn->sizeHint() );
  hlay->setSpacing( 10 );

  QGroupBox	*modebox = new QGroupBox( tr( "Fusion mode :" ), hbox );
  hlay->addWidget( modebox );
  vlay = new QVBoxLayout( modebox );
  QButtonGroup *modeg = new QButtonGroup( modebox );
  d->modegrp = modeg;
  modeg->setExclusive( true );
  QRadioButton	*geombtn = new QRadioButton( tr( "Geometrical" ), modebox );
  vlay->addWidget( geombtn );
  modeg->addButton( geombtn, 0 );
  QRadioButton	*linbtn = new QRadioButton( tr( "Linear" ), modebox );
  vlay->addWidget( linbtn );
  modeg->addButton( linbtn, 0 );
  d->linratelab = new QLabel( "50", modebox );
  vlay->addWidget( d->linratelab );
  d->linratesl = new QSlider( Qt::Horizontal, modebox );
  d->linratesl->setMinimum( 0 );
  d->linratesl->setMaximum( 100 );
  d->linratesl->setPageStep( 1 );
  d->linratesl->setValue( 50 );
  vlay->addWidget( d->linratesl );
  vlay->addWidget( new QLabel( tr( "Rate" ), modebox ) );
  vlay->addStretch( 1 );

  QGroupBox	*methbox = new QGroupBox( tr( "Methods :" ), hbox );
  hlay->addWidget( methbox );
  vlay = new QVBoxLayout( methbox );
  QButtonGroup *methg = new QButtonGroup( methbox );
  d->methgrp = methg;
  methg->setExclusive( true );
  QRadioButton *rb = new QRadioButton( tr( "Point to point" ), methbox );
  vlay->addWidget( rb );
  methg->addButton( rb, 0 );
  rb = new QRadioButton( tr( "Point to point with depth offset (inside)" ), 
                    methbox );
  vlay->addWidget( rb );
  methg->addButton( rb, 1 );
  rb = new QRadioButton( tr( "Point to point with depth offset (outside)" ), 
                    methbox );
  vlay->addWidget( rb );
  methg->addButton( rb, 2 );
  rb = new QRadioButton( tr( "Line to point" ), methbox );
  vlay->addWidget( rb );
  methg->addButton( rb, 3 );
  rb = new QRadioButton( tr( "Inside line to point" ), methbox );
  vlay->addWidget( rb );
  methg->addButton( rb, 4 );
  rb = new QRadioButton( tr( "Outside line to point" ), methbox );
  vlay->addWidget( rb );
  methg->addButton( rb, 5 );
  rb = new QRadioButton( tr( "Sphere to point" ), methbox );
  vlay->addWidget( rb );
  methg->addButton( rb, 6 );
  vlay->addStretch( 1 );

  QGroupBox	*submbox = new QGroupBox( tr( "Submethods :" ), hbox );
  hlay->addWidget( submbox );
  vlay = new QVBoxLayout( submbox );
  QButtonGroup *subg = new QButtonGroup( submbox );
  d->submethgrp = subg;
  subg->setExclusive( true );
  QRadioButton	*maxbtn = new QRadioButton( tr( "Max" ), submbox );
  vlay->addWidget( maxbtn );
  subg->addButton( maxbtn, 0 );
  QRadioButton	*minbtn = new QRadioButton( tr( "Min" ), submbox );
  vlay->addWidget( minbtn );
  subg->addButton( minbtn, 1 );
  QRadioButton	*meanbtn = new QRadioButton( tr( "Mean" ), submbox );
  vlay->addWidget( meanbtn );
  subg->addButton( meanbtn, 2 );
  QRadioButton	*cmeanbtn = new QRadioButton( tr( "Corrected mean" ), 
                                              submbox );
  vlay->addWidget( cmeanbtn );
  subg->addButton( cmeanbtn, 3 );
  QRadioButton	*emeanbtn = new QRadioButton( tr( "Enhanced mean" ), submbox );
  vlay->addWidget( emeanbtn );
  subg->addButton( emeanbtn, 4 );
  QRadioButton  *absmaxbtn = new QRadioButton( tr( "Abs. Max" ), submbox );
  vlay->addWidget( absmaxbtn );
  subg->addButton( absmaxbtn, 5 );
  QRadioButton  *medianbtn = new QRadioButton( tr( "Median" ), submbox );
  vlay->addWidget( medianbtn );
  subg->addButton( medianbtn, 6 );
  vlay->addStretch( 1 );

  QGroupBox	*parambox = new QGroupBox( tr( "Parameters :" ), hbox );
  hlay->addWidget( parambox );
  vlay = new QVBoxLayout( parambox );
  d->depthlab = new QLineEdit( "5", parambox );
  vlay->addWidget( d->depthlab );
  QDoubleValidator *dv = new QDoubleValidator( d->depthlab );
  dv->setBottom( 0 );
  d->depthlab->setValidator( dv );
  d->depthsl = new QSlider( Qt::Horizontal, parambox );
  d->depthsl->setMinimum( 0 );
  d->depthsl->setMaximum( 600 );
  d->depthsl->setPageStep( 1 );
  d->depthsl->setValue( 50 );
  vlay->addWidget( d->depthsl );
  vlay->addWidget( new QLabel( tr( "Depth (mm)" ), parambox ) );
  d->steplab = new QLineEdit( "2.5", parambox );
  vlay->addWidget( d->steplab );
  dv = new QDoubleValidator( d->steplab );
  dv->setBottom( 0 );
  d->steplab->setValidator( dv );
  d->stepsl = new QSlider( Qt::Horizontal, parambox );
  d->stepsl->setMinimum( 0 );
  d->stepsl->setMaximum( 200 );
  d->stepsl->setPageStep( 1 );
  d->stepsl->setValue( 25 );
  vlay->addWidget( d->stepsl );
  vlay->addWidget( new QLabel( tr( "Step (mm)" ), parambox ) );
  vlay->addStretch( 1 );

  updateWindow();

  connect( geombtn, SIGNAL( clicked() ), 
           this, SLOT( geometricalMode() ) );
  connect( linbtn, SIGNAL( clicked() ), this, SLOT( linearMode() ) );
  connect( d->linratesl, SIGNAL( valueChanged( int ) ), 
           this, SLOT( linRateChanged( int ) ) );
  connect( methg, SIGNAL( buttonClicked( int ) ), 
           this, SLOT( changeMethod( int ) ) );
  connect( maxbtn, SIGNAL( clicked() ), this, SLOT( maxSubMethod() ) );
  connect( minbtn, SIGNAL( clicked() ), this, SLOT( minSubMethod() ) );
  connect( meanbtn, SIGNAL( clicked() ), this, SLOT( meanSubMethod() ) );
  connect( cmeanbtn, SIGNAL( clicked() ), 
           this, SLOT( correctedMeanSubMethod() ) );
  connect( emeanbtn, SIGNAL( clicked() ), 
           this, SLOT( enhancedMeanSubMethod() ) );
  connect( absmaxbtn, SIGNAL( clicked() ), this, SLOT( absmaxSubMethod() ) );
  connect( medianbtn, SIGNAL( clicked() ), this, SLOT( medianSubMethod() ) );
  connect( d->depthsl, SIGNAL( valueChanged( int ) ), 
           this, SLOT( depthChanged( int ) ) );
  connect( d->stepsl, SIGNAL( valueChanged( int ) ), 
           this, SLOT( stepChanged( int ) ) );
  connect( applybtn, SIGNAL( clicked() ), this, SLOT( updateObjects() ) );
  connect( d->objsel, SIGNAL( selectionStarts() ), 
           this, SLOT( chooseObject() ) );
  connect( d->objsel, 
           SIGNAL( objectsSelected( const std::set<anatomist::AObject *> & ) ),
           this, 
           SLOT( objectsChosen( const std::set<anatomist::AObject *> & ) ) );
  connect( d->depthlab, SIGNAL( returnPressed() ), this,
           SLOT( depthLineChanged() ) );
  connect( d->steplab, SIGNAL( returnPressed() ), this,
           SLOT( stepLineChanged() ) );
}


void Fusion3DWindow::update(const Observable* observable, void* arg)
{
  if( arg == this )
    return;

  set<Fusion3D *>::iterator io, fo = d->parents.end();
  for( io=d->parents.begin(); io!=fo; ++io )
    if( (*io) == observable )
      break;

  if (io == fo )
    {
      cerr << "Fusion3DWindow::update : unknown observable\n";
      return;
    }

  if( arg == 0 )
    {
      cout << "called obsolete Fusion3DWindow::update( obs, NULL )\n";
      unregisterObservable( (Observable *) observable );
      delete this;
      return;
    }

  Fusion3D *fobj = *io;

  d->rate = (int) ( 100 * fobj->glTexRate() );
  d->mode = fobj->glTexMode();
  d->method = fobj->method();
  d->submethod = fobj->subMethod();
  d->depth = fobj->depth();
  d->step = fobj->step();
  d->modeHasChanged = false;
  d->rateHasChanged = false;
  d->methodHasChanged = false;
  d->submethodHasChanged = false;
  d->depthHasChanged = false;
  d->stepHasChanged = false;
  updateWindow();
}


void Fusion3DWindow::unregisterObservable( Observable* obs )
{
  // cout << "Fusion3DWindow::unregisterObservable\n";
  Observer::unregisterObservable( obs );
  AObject	*x = dynamic_cast<AObject *>( obs );
  if( !x || x->type() != AObject::FUSION3D )
    return;
  Fusion3D	*o = (Fusion3D *) x;
  set<Fusion3D *>::iterator	i = d->parents.find( o );
  if( i != d->parents.end() )
    {
      d->parents.erase( o );
      set<AObject *>	obj;
      obj.insert( d->parents.begin(), d->parents.end() );
      d->objsel->updateLabel( obj );
      updateWindow();
    }
}


void Fusion3DWindow::updateWindow()
{
  if( d->parents.empty() )
    {
      d->main->setEnabled( false );
      return;
    }

  d->main->setEnabled( true );
  Fusion3D	*fus = *d->parents.begin();
  int		mode = 0, submeth = 0;

  switch( fus->glTexMode() )
    {
    case Fusion3D::glLINEAR:
      mode = 1;
      break;
    default:
      mode = 0;
      break;
    }
  d->modegrp->button( mode )->setChecked( true );

  d->methgrp->button( (int) fus->method() )->setChecked( true );

  switch( fus->subMethod() )
    {
    case Fusion3D::MIN:
      submeth = 1;
      break;
    case Fusion3D::MEAN:
      submeth = 2;
      break;
    case Fusion3D::CORRECTED_MEAN:
      submeth = 3;
      break;
    case Fusion3D::ENHANCED_MEAN:
      submeth = 4;
      break;
    case Fusion3D::ABSMAX:
      submeth = 5;
      break;
    case Fusion3D::MEDIAN:
      submeth = 6;
      break;
    default:
      submeth = 0;
      break;
    }
  d->submethgrp->button( submeth )->setChecked( true );

  d->linratelab->setText( QString::number( 100 * fus->glTexRate() ) );
  d->linratesl->setValue( (int) ( 100 * fus->glTexRate() ) );
  d->depthlab->setText( QString::number( fus->depth() ) );
  d->depthsl->setValue( (int) ( 10 * fus->depth() ) );
  d->steplab->setText( QString::number( fus->step() ) );
  d->stepsl->setValue( (int) ( 10 * fus->step() ) );
}


void Fusion3DWindow::updateObjects()
{
  set<Fusion3D *>::const_iterator io, fo = d->parents.end();
  Fusion3D * objf;

  for( io=d->parents.begin(); io!=fo; ++io )
    {
      objf = *io;
      if( d->rateHasChanged ) objf->glSetTexRate( 0.01 * d->rate );
      if( d->modeHasChanged ) objf->glSetTexMode( d->mode );
      if( d->methodHasChanged ) objf->setMethod( d->method );
      if( d->submethodHasChanged ) objf->setSubMethod( d->submethod );
      if( d->depthHasChanged )  objf->setDepth( d->depth );
      if( d->stepHasChanged ) objf->setStep( d->step );
      if( d->rateHasChanged || d->modeHasChanged || d->methodHasChanged 
	  || d->submethodHasChanged || d->depthHasChanged 
          || d->stepHasChanged )
	objf->setChanged();
      objf->notifyObservers((void*)this);
      objf->clearHasChangedFlags();
    }
  d->modeHasChanged = false;
  d->rateHasChanged = false;
  d->methodHasChanged = false;
  d->submethodHasChanged = false;
  d->depthHasChanged = false;
  d->stepHasChanged = false;
}


void Fusion3DWindow::linRateChanged( int value )
{
  if( d->rate != value )
    {
      d->rate = value;  
      d->rateHasChanged = true;
      d->linratelab->setText( QString::number( d->rate ) );
      updateObjects();
    }
}

void Fusion3DWindow::linearMode()
{
  d->mode = Fusion3D::glLINEAR;
  d->modeHasChanged = true;
  updateObjects();
}

void Fusion3DWindow::geometricalMode()
{
  d->mode = Fusion3D::glGEOMETRIC;
  d->modeHasChanged = true;
  updateObjects();
}


void Fusion3DWindow::changeMethod( int x )
{
  d->method = (Fusion3D::Method) x;
  d->methodHasChanged = true;
}


void Fusion3DWindow::maxSubMethod()
{
  d->submethod = Fusion3D::MAX;
  d->submethodHasChanged = true;
}

void Fusion3DWindow::minSubMethod()
{
  d->submethod = Fusion3D::MIN;
  d->submethodHasChanged = true;
}

void Fusion3DWindow::meanSubMethod()
{
  d->submethod = Fusion3D::MEAN;
  d->submethodHasChanged = true;
}

void Fusion3DWindow::correctedMeanSubMethod()
{
  d->submethod = Fusion3D::CORRECTED_MEAN;
  d->submethodHasChanged = true;
}

void Fusion3DWindow::enhancedMeanSubMethod()
{
  d->submethod = Fusion3D::ENHANCED_MEAN;
  d->submethodHasChanged = true;
}

void Fusion3DWindow::absmaxSubMethod()
{
  d->submethod = Fusion3D::ABSMAX;
  d->submethodHasChanged = true;
}

void Fusion3DWindow::medianSubMethod()
{
  d->submethod = Fusion3D::MEDIAN;
  d->submethodHasChanged = true;
}


void Fusion3DWindow::depthChanged( int value )
{
  if( d->depth != ((float) value) * 0.1 )
    {
      d->depth = ((float) value) * 0.1;
      d->depthHasChanged = true;
      d->depthlab->setText( QString::number( d->depth ) );
    }
}


void Fusion3DWindow::stepChanged( int value )
{
  if( d->step != ((float) value) * 0.1 )
    {
      d->step = ((float) value) * 0.1;
      d->stepHasChanged = true;
      d->steplab->setText( QString::number( d->step ) );
    }
}


void Fusion3DWindow::depthLineChanged()
{
  bool ok = true;
  float value = d->depthlab->text().toFloat( &ok );
  if( ok && value > 0 && d->depth != value )
  {
    d->depth = value;
    d->depthHasChanged = true;
    d->depthsl->blockSignals( true );
    d->depthsl->setValue( int( d->depth * 10 ) );
    d->depthsl->blockSignals( false );
  }
  else
    d->depthlab->setText( QString::number( d->depth ) );
}


void Fusion3DWindow::stepLineChanged()
{
  bool ok = true;
  float value = d->steplab->text().toFloat( &ok );
  if( ok && value > 0 && d->step != value )
  {
    d->step = value;
    d->stepHasChanged = true;
    d->stepsl->blockSignals( true );
    d->stepsl->setValue( int( d->step * 10 ) );
    d->stepsl->blockSignals( false );
  }
  else
    d->steplab->setText( QString::number( d->step ) );
}


const set<Fusion3D *> & Fusion3DWindow::objects() const
{
  return d->parents;
}


void Fusion3DWindow::chooseObject()
{
  // cout << "chooseObject\n";
  // filter out objects that don't exist anymore
  set<AObject *>::iterator	ir = d->initial.begin(), 
    er = d->initial.end(), ir2;
  while( ir!=er )
    if( theAnatomist->hasObject( *ir ) )
      ++ir;
    else
      {
        ir2 = ir;
        ++ir;
        d->initial.erase( ir2 );
      }

  set<AObject *>	obj;
  obj.insert( d->parents.begin(), d->parents.end() );
  d->objsel->selectObjects( d->initial, obj );
}


void Fusion3DWindow::objectsChosen( const set<AObject *> & o )
{
  // cout << "objects chosen: " << o.size() << endl;

  while( !d->parents.empty() )
    (*d->parents.begin())->deleteObserver( this );

  set<AObject *>::const_iterator	io, eo = o.end();
  for( io=o.begin(); io!=eo; ++io )
    {
      d->parents.insert( (Fusion3D *) *io );
      (*io)->addObserver( this );
    }

  d->objsel->updateLabel( o );
  updateWindow();
}


