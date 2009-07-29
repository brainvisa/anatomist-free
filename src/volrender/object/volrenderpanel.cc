/* Copyright (c) 2007 CEA
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

/****************************************************************************
** Form implementation generated from reading ui file 'volrender.ui'
**
** Created by: The User Interface Compiler ($Id: qt/main.cpp   3.3.8   edited Jan 11 14:47 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "volrenderpanel.h"
#include <anatomist/object/volrender.h>

#include <qvariant.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

using namespace anatomist;
using namespace std;

struct VolRenderPanel::Private
{
  Private();

  set<AObject *>  objects;
  bool modehaschanged;
  bool maxslicehaschanged;
  bool slabsizehaschanged;
};


VolRenderPanel::Private::Private()
  : modehaschanged( false ), maxslicehaschanged( false ),
    slabsizehaschanged( false )
{
}


/*
 *  Constructs a VolRenderPanel as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 */
VolRenderPanel::VolRenderPanel( const std::set<anatomist::AObject *> & obj,
                                QWidget* parent, const char* name,
                                Qt::WFlags fl )
  : QWidget( parent, name, fl ), Observer(), d( new Private )
{
  d->objects = obj;
  set<AObject *>::const_iterator  io, eo = obj.end();
  for( io=obj.begin(); io!=eo; ++io )
    (*io)->addObserver( this );

  if ( !name )
    setName( "VolRenderPanel" );
  VolRenderPanelLayout = new QVBoxLayout( this, 11, 6, "VolRenderPanelLayout");

  groupBox1 = new Q3GroupBox( this, "groupBox1" );
  groupBox1->setColumnLayout(0, Qt::Vertical );
  groupBox1->layout()->setSpacing( 6 );
  groupBox1->layout()->setMargin( 11 );
  groupBox1Layout = new QVBoxLayout( groupBox1->layout() );
  groupBox1Layout->setAlignment( Qt::AlignTop );

  renderingMode = new QComboBox( FALSE, groupBox1, "renderingMode" );
  groupBox1Layout->addWidget( renderingMode );

  layout2 = new QHBoxLayout( 0, 0, 6, "layout2");

  limitSlices = new QCheckBox( groupBox1, "limitSlices" );
  layout2->addWidget( limitSlices );

  maxSlices = new QSpinBox( groupBox1, "maxSlices" );
  maxSlices->setMaxValue( 10000 );
  maxSlices->setValue( 500 );
  layout2->addWidget( maxSlices );

  textLabel1 = new QLabel( groupBox1, "textLabel1" );
  layout2->addWidget( textLabel1 );
  groupBox1Layout->addLayout( layout2 );

  layout3 = new QHBoxLayout( 0, 0, 6, "layout3");

  slabSize = new QSpinBox( groupBox1, "slabSize" );
  slabSize->setMaxValue( 10000 );
  slabSize->setValue( 5 );
  layout3->addWidget( slabSize );

  textLabel2 = new QLabel( groupBox1, "textLabel2" );
  layout3->addWidget( textLabel2 );
  groupBox1Layout->addLayout( layout3 );
  VolRenderPanelLayout->addWidget( groupBox1 );
  languageChange();
  resize( QSize(306, 136).expandedTo(minimumSizeHint()) );
#if QT_VERSION < 0x040000
  clearWState( WState_Polished );
#endif

  updateWindow();

    // signals and slots connections
  connect( renderingMode, SIGNAL( activated( const QString & ) ),
           SLOT( renderModechanged( const QString & ) ) );
  connect( limitSlices, SIGNAL( toggled( bool ) ),
           SLOT( setSliceLimitEnabled( bool ) ) );
  connect( maxSlices, SIGNAL( valueChanged( int ) ),
           SLOT( maxSlicesChanged( int ) ) );
  connect( slabSize, SIGNAL( valueChanged( int ) ),
           SLOT( slabSizeChanged( int ) ) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
VolRenderPanel::~VolRenderPanel()
{
  delete d;
}


namespace
{
  string renderingModeNames[] =
  {
    "VRShader",
    "MIPShader",
    "SumShader",
    "MPVRShader"
  };
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void VolRenderPanel::languageChange()
{
  setCaption( tr( "Volume Rendering Properties" ) );
  groupBox1->setTitle( tr( "Volume rendering mode" ) );
  renderingMode->clear();
  renderingMode->insertItem( tr( renderingModeNames[0].c_str() ) );
  renderingMode->insertItem( tr( renderingModeNames[1].c_str() ) );
  renderingMode->insertItem( tr( renderingModeNames[2].c_str() ) );
  renderingMode->insertItem( tr( renderingModeNames[3].c_str() ) );
  limitSlices->setText( tr( "Limit number of slices :" ) );
  textLabel1->setText( tr( "Max." ) );
  textLabel2->setText( tr( "MPVR slab thickness" ) );
}

// --

void VolRenderPanel::renderModechanged( const QString & )
{
  // cout << "renderModechanged\n";
  d->modehaschanged = true;
  updateObjects();
}


void VolRenderPanel::setSliceLimitEnabled( bool )
{
  d->maxslicehaschanged = true;
  updateObjects();
}


void VolRenderPanel::maxSlicesChanged( int )
{
  d->maxslicehaschanged = true;
  updateObjects();
}


void VolRenderPanel::slabSizeChanged( int )
{
  d->slabsizehaschanged = true;
  updateObjects();
}


const set<AObject *> & VolRenderPanel::objects() const
{
  return d->objects;
}


void VolRenderPanel::update( const anatomist::Observable* observable,
                             void* arg )
{
  // cout << "VolRenderPanel::update\n";
  if( arg == this )
    return;
  const AObject	*o = dynamic_cast<const AObject *>( observable );
  if( !o )
    return;
  set<AObject *>::const_iterator io
      = d->objects.find( const_cast<AObject *>( o ) );
  if( io == d->objects.end() )
    return;
  updateWindow();
}


void VolRenderPanel::unregisterObservable( anatomist::Observable* obs )
{
  // cout << "VolRenderPanel::unregisterObservable\n";
  Observer::unregisterObservable( obs );
  AObject	*o = dynamic_cast<AObject *>( obs );
  if( !o )
    return;
  set<AObject *>::iterator	i = d->objects.find( o );
  if( i != d->objects.end() )
  {
    d->objects.erase( i );
    // d->objsel->updateLabel( d->parents );
    updateWindow();
  }
}


void VolRenderPanel::updateWindow()
{
  // cout << "VolRenderPanel::updateWindow\n";
  if( d->objects.empty() )
  {
    setEnabled( false );
    return;
  }
  setEnabled( true );
  blockSignals( true );
  VolRender *vr = static_cast<VolRender *>( *d->objects.begin() );
  renderingMode->setCurrentText( tr( vr->shaderType().c_str() ) );
  limitSlices->setChecked( vr->maxSlices() != 0 );
  maxSlices->setValue( vr->maxSlices() );
  slabSize->setValue( vr->slabSize() );
  blockSignals( false );
}


void VolRenderPanel::updateObjects()
{
  // cout << "VolRenderPanel::updateObjects\n";
  set<AObject *>::iterator  i, e = d->objects.end();
  VolRender *vr;

  for( i=d->objects.begin(); i!=e; ++i )
  {
    vr = static_cast<VolRender *>( *i );
    if( d->modehaschanged )
      vr->setShaderType( renderingModeNames[ renderingMode->currentItem() ] );
    if( d->maxslicehaschanged )
      vr->setMaxSlices( maxSlices->value() );
    if( d->slabsizehaschanged )
      vr->setSlabSize( slabSize->value() );
    vr->notifyObservers( this );
  }
  d->modehaschanged = false;
  d->maxslicehaschanged = false;
  d->slabsizehaschanged = false;
}

