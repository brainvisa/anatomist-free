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


#include <anatomist/window3D/wLightModel.h>
#include <qslider.h>
#include <anatomist/window3D/window3D.h>
#include <anatomist/dialogs/colorWidget.h>
#include <anatomist/color/Light.h>
#include <qlayout.h>
#include <qtabbar.h>
#include <aims/qtcompat/qvbox.h>
#include <aims/qtcompat/qvgroupbox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <vector>
#include <map>
#include <iostream>

using namespace anatomist;
using namespace std;


struct LightModelWindow_PrivateData
{
  LightModelWindow_PrivateData()
    : tab( 0 ), sourceAmbient( 0 ), sourceDiffuse( 0 ), sourceSpecular( 0 ), 
    modelAmbient( 0 ), modelBackground( 0 ) {}

  unsigned		tab;
  vector<QWidget *>	tabs;
  map<int, unsigned>	tabnum;

  QAColorWidget		*sourceAmbient;
  QAColorWidget		*sourceDiffuse;
  QAColorWidget		*sourceSpecular;
  QAColorWidget		*modelAmbient;
  QAColorWidget		*modelBackground;
  QSlider		*spotPos[3];
  QLabel		*spotPosLb[3];
  QSlider		*spotDir[3];
  QLabel		*spotDirLb[3];
  QSlider		*spotExpo;
  QLabel		*spotExpoLb;
  QSlider		*spotCutoff;
  QLabel		*spotCutoffLb;
  QSlider		*attConst;
  QLabel		*attConstLb;
  QSlider		*attLin;
  QLabel		*attLinLb;
  QSlider		*attQuad;
  QLabel		*attQuadLb;
};


LightModelWindow::LightModelWindow( AWindow3D *win )
  : QWidget( theAnatomist->getQWidgetAncestor(), Qt::Window ), Observer(),
    _window( win ), _pdat( new LightModelWindow_PrivateData )
{
  setObjectName("light model");
  setAttribute(Qt::WA_DeleteOnClose);
  setCaption( tr( "Light model : " ) + win->Title().c_str() );

  QVBoxLayout	*mainlay = new QVBoxLayout( this, 10, 10 );
  QTabBar	*tbar = new QTabBar( this );

#if QT_VERSION >= 0x040000
  _pdat->tabnum[ tbar->addTab( tr( "Source" ) ) ] = 0;
  _pdat->tabnum[ tbar->addTab( tr( "Model" ) ) ] = 1;
  _pdat->tabnum[ tbar->addTab( tr( "Spot" ) ) ] = 2;
  _pdat->tabnum[ tbar->addTab( tr( "Attenuation" ) ) ] = 3;
#else
  _pdat->tabnum[ tbar->addTab( new QTab( tr( "Source" ) ) ) ] = 0;
  _pdat->tabnum[ tbar->addTab( new QTab( tr( "Model" ) ) ) ] = 1;
  _pdat->tabnum[ tbar->addTab( new QTab( tr( "Spot" ) ) ) ] = 2;
  _pdat->tabnum[ tbar->addTab( new QTab( tr( "Attenuation" ) ) ) ] = 3;
#endif

  QColor	col;
  float		*c;
  unsigned	i;
  Light		*l = _window->light();
  float		f;

  //	Source tab
  QHBox	*source = new QHBox( this );
  _pdat->tabs.push_back( source );
  source->setSpacing( 5 );
  QVGroupBox	*vg = new QVGroupBox( tr( "Ambient :" ), source );
  c = l->Ambient();
  col = QColor( (int) ( c[0] * 255.9 ), (int) ( c[1] * 255.9 ), 
		(int) ( c[2] * 255.9 ) );
  _pdat->sourceAmbient = new QAColorWidget( col, vg, 
					    "lgtmodel_source_ambient" );
  vg = new QVGroupBox( tr( "Diffuse :" ), source );
  c = l->Diffuse();
  col = QColor( (int) ( c[0] * 255.9 ), (int) ( c[1] * 255.9 ), 
		(int) ( c[2] * 255.9 ) );
  _pdat->sourceDiffuse = new QAColorWidget( col, vg, 
					    "lgtmodel_source_diffuse" );
  vg = new QVGroupBox( tr( "Specular :" ), source );
  c = l->Specular();
  col = QColor( (int) ( c[0] * 255.9 ), (int) ( c[1] * 255.9 ), 
		(int) ( c[2] * 255.9 ) );
  _pdat->sourceSpecular = new QAColorWidget( col, vg, 
					     "lgtmodel_source_specular" );

  //	Model tab
  QHBox	*model = new QHBox( this );
  _pdat->tabs.push_back( model );
  model->setSpacing( 5 );
  model->hide();
  vg = new QVGroupBox( tr( "Ambient :" ), model );
  c = l->ModelAmbient();
  col = QColor( (int) ( c[0] * 255.9 ), (int) ( c[1] * 255.9 ), 
		(int) ( c[2] * 255.9 ) );
  _pdat->modelAmbient = new QAColorWidget( col, vg, 
					   "lgtmodel_model_ambient" );
  vg = new QVGroupBox( tr( "Background :" ), model );
  c = l->Background();
  col = QColor( (int) ( c[0] * 255.9 ), (int) ( c[1] * 255.9 ), 
		(int) ( c[2] * 255.9 ) );
  _pdat->modelBackground
    = new QAColorWidget( col, vg, "lgtmodel_model_backg", 0, true, false,
                         c[3] * 255.9 );
  vg = new QVGroupBox( tr( "Parameters :" ), model );
  QCheckBox	*lv = new QCheckBox( tr( "Local viewer" ), vg );
  lv->setChecked( l->ModelLocalViewer() );
  QCheckBox	*ts = new QCheckBox( tr( "Two side" ), vg );
  ts->setChecked( l->ModelTwoSide() );

  //	Spot tab
  QHBox	*spot = new QHBox( this );
  _pdat->tabs.push_back( spot );
  spot->setSpacing( 5 );
  spot->hide();

  vg = new QVGroupBox( tr( "Position :" ), spot );
  QHBox	*hb = new QHBox( vg );
  hb->setSpacing( 5 );
  new QLabel( tr( "X :" ), hb );
  f = l->Position( 0 );
  _pdat->spotPos[0] 
    = new QSlider( -1000, 1000, 1, (int) ( 100*f ), Qt::Horizontal, hb );
  _pdat->spotPosLb[0] = new QLabel( QString::number( f ), hb );
  hb = new QHBox( vg );
  hb->setSpacing( 5 );
  new QLabel( tr( "Y :" ), hb );
  f = l->Position( 1 );
  _pdat->spotPos[1] 
    = new QSlider( -1000, 1000, 1, (int) ( 100*f ), Qt::Horizontal, hb );
  _pdat->spotPosLb[1] = new QLabel( QString::number( f ), hb );
  hb = new QHBox( vg );
  hb->setSpacing( 5 );
  new QLabel( tr( "Z :" ), hb );
  f = l->Position( 2 );
  _pdat->spotPos[2] 
    = new QSlider( -1000, 1000, 1, (int) ( 100*f ), Qt::Horizontal, hb );
  _pdat->spotPosLb[2] = new QLabel( QString::number( f ), hb );

  vg = new QVGroupBox( tr( "Direction :" ), spot );
  hb = new QHBox( vg );
  hb->setSpacing( 5 );
  new QLabel( tr( "X :" ), hb );
  f = l->SpotDirection( 0 );
  _pdat->spotDir[0] 
    = new QSlider( -1000, 1000, 1, (int) ( 100*f ), Qt::Horizontal, hb );
  _pdat->spotDirLb[0] = new QLabel( QString::number( f ), hb );
  hb = new QHBox( vg );
  hb->setSpacing( 5 );
  new QLabel( tr( "Y :" ), hb );
  f = l->SpotDirection( 1 );
  _pdat->spotDir[1] 
    = new QSlider( -1000, 1000, 1, (int) ( 100*f ), Qt::Horizontal, hb );
  _pdat->spotDirLb[1] = new QLabel( QString::number( f ), hb );
  hb = new QHBox( vg );
  hb->setSpacing( 5 );
  new QLabel( tr( "Z :" ), hb );
  f = l->SpotDirection( 2 );
  _pdat->spotDir[2] 
    = new QSlider( -1000, 1000, 1, (int) ( f*10 ), Qt::Horizontal, hb );
  _pdat->spotDirLb[2] = new QLabel( QString::number( f ), hb );

  vg = new QVGroupBox( tr( "Parameters :" ), spot );
  new QLabel( tr( "Exponent :" ), vg );
  hb = new QHBox( vg );
  hb->setSpacing( 5 );
  f = l->SpotExponent();
  _pdat->spotExpo = new QSlider( 0, 2550, 1, (int) ( 10*f ), 
				 Qt::Horizontal, hb );
  _pdat->spotExpoLb = new QLabel( QString::number( f ), hb );
  new QLabel( tr( "Cutoff :" ), vg );
  hb = new QHBox( vg );
  hb->setSpacing( 5 );
  f = l->SpotCutoff();
  _pdat->spotCutoff = new QSlider( 0, 1800, 1, (int) ( 10*f ), 
				   Qt::Horizontal, hb );
  _pdat->spotCutoffLb = new QLabel( QString::number( f ), hb );

  for( i=0; i<3; ++i )
    {
      _pdat->spotPosLb[i]->setMinimumWidth( 30 );
      _pdat->spotDirLb[i]->setMinimumWidth( 30 );
    }
  _pdat->spotExpoLb->setMinimumWidth( 30 );
  _pdat->spotCutoffLb->setMinimumWidth( 30 );

  //	Attenuation tab
  QHBox	*attenuation = new QHBox( this );
  _pdat->tabs.push_back( attenuation );
  attenuation->setSpacing( 5 );
  attenuation->hide();
  vg = new QVGroupBox( tr( "Parameters :" ), attenuation );
  new QLabel( tr( "Constant :" ), vg );
  hb = new QHBox( vg );
  hb->setSpacing( 5 );
  f = l->ConstantAttenuation();
  _pdat->attConst = new QSlider( 0, 1280, 1, (int) ( 10*f ), 
				 Qt::Horizontal, hb );
  _pdat->attConstLb = new QLabel( QString::number( f ), hb );
  _pdat->attConstLb->setMinimumWidth( 30 );
  new QLabel( tr( "Linear :" ), vg );
  hb = new QHBox( vg );
  hb->setSpacing( 5 );
  f = l->LinearAttenuation();
  _pdat->attLin = new QSlider( 0, 1280, 1, (int) ( 10*f ), 
			       Qt::Horizontal, hb );
  _pdat->attLinLb = new QLabel( QString::number( f ), hb );
  _pdat->attLinLb->setMinimumWidth( 30 );
  new QLabel( tr( "Quadratic :" ), vg );
  hb = new QHBox( vg );
  hb->setSpacing( 5 );
  f = l->QuadraticAttenuation();
  _pdat->attQuad = new QSlider( 0, 1280, 1, (int) ( 10*f ), Qt::Horizontal, 
                                hb );
  _pdat->attQuadLb = new QLabel( QString::number( f ), hb );
  _pdat->attQuadLb->setMinimumWidth( 30 );

  // top-level widget

  mainlay->addWidget( tbar );
  mainlay->addWidget( source );
  mainlay->addWidget( model );
  mainlay->addWidget( spot );
  mainlay->addWidget( attenuation );

  connect( tbar, SIGNAL( selected( int ) ), this, SLOT( enableTab( int ) ) );
  connect( _pdat->sourceAmbient, SIGNAL( colorChanged() ), this, 
	   SLOT( sourceAmbientChanged() ) );
  connect( _pdat->sourceDiffuse, SIGNAL( colorChanged() ), this, 
	   SLOT( sourceDiffuseChanged() ) );
  connect( _pdat->sourceSpecular, SIGNAL( colorChanged() ), this, 
	   SLOT( sourceSpecularChanged() ) );
  connect( _pdat->modelAmbient, SIGNAL( colorChanged() ), this, 
	   SLOT( modelAmbientChanged() ) );
  connect( _pdat->modelBackground, SIGNAL( colorChanged() ), this, 
	   SLOT( modelBackgroundChanged() ) );
  connect( lv, SIGNAL( toggled( bool ) ), this, 
	   SLOT( setLocalViewer( bool ) ) );
  connect( ts, SIGNAL( toggled( bool ) ), this, 
	   SLOT( setTwoSide( bool ) ) );

  for( i = 0; i<3; ++i )
    {
      connect( _pdat->spotPos[i], SIGNAL( valueChanged( int ) ), this, 
	       SLOT( spotPositionChanged( int ) ) );
      connect( _pdat->spotDir[i], SIGNAL( valueChanged( int ) ), this, 
	       SLOT( spotDirectionChanged( int ) ) );
    }
  connect( _pdat->spotExpo, SIGNAL( valueChanged( int ) ), this, 
	   SLOT( spotExponentChanged( int ) ) );
  connect( _pdat->spotCutoff, SIGNAL( valueChanged( int ) ), this, 
	   SLOT( spotCutoffChanged( int ) ) );

  connect( _pdat->attConst, SIGNAL( valueChanged( int ) ), this, 
	   SLOT( attenuationChanged( int ) ) );
  connect( _pdat->attLin, SIGNAL( valueChanged( int ) ), this, 
	   SLOT( attenuationChanged( int ) ) );
  connect( _pdat->attQuad, SIGNAL( valueChanged( int ) ), this, 
	   SLOT( attenuationChanged( int ) ) );
}


LightModelWindow::~LightModelWindow()
{
  _window->lightWinDestroyed();
}


void LightModelWindow::update( const Observable*, void* arg )
{
  if( arg == 0 )
    {
      cout << "called obsolete LightModelWindow::update( obs, NULL )\n";
      delete this;
    }
}


void LightModelWindow::unregisterObservable( Observable* o )
{
  Observer::unregisterObservable( o );
  delete this;
}


void LightModelWindow::enableTab( int tabid )
{
  unsigned tab = _pdat->tabnum[ tabid ];
  if( tab != _pdat->tab )
    {
      _pdat->tabs[ _pdat->tab ]->hide();
      _pdat->tab = tab;
      _pdat->tabs[ tab ]->show();
    }
}


void LightModelWindow::sourceAmbientChanged()
{
  QColor	col = _pdat->sourceAmbient->color();
  _window->light()->SetAmbient( ( (float) col.red() ) / 255, 
				   ( (float) col.green() ) / 255, 
				   ( (float) col.blue() ) / 255, 1 );
  updateLights();
}


void LightModelWindow::sourceDiffuseChanged()
{
  QColor	col = _pdat->sourceDiffuse->color();
  _window->light()->SetDiffuse( ( (float) col.red() ) / 255, 
				   ( (float) col.green() ) / 255, 
				   ( (float) col.blue() ) / 255, 1 );
  updateLights();
}


void LightModelWindow::sourceSpecularChanged()
{
  QColor	col = _pdat->sourceSpecular->color();
  _window->light()->SetSpecular( ( (float) col.red() ) / 255, 
				    ( (float) col.green() ) / 255, 
				    ( (float) col.blue() ) / 255, 1 );
  updateLights();
}


void LightModelWindow::modelAmbientChanged()
{
  QColor	col = _pdat->modelAmbient->color();
  _window->light()->SetModelAmbient( ( (float) col.red() ) / 255, 
					( (float) col.green() ) / 255, 
					( (float) col.blue() ) / 255, 1 );
  updateLights();
}


void LightModelWindow::modelBackgroundChanged()
{
  QColor	col = _pdat->modelBackground->color();
  _window->light()->SetBackground( ( (float) col.red() ) / 255,
                                   ( (float) col.green() ) / 255,
                                   ( (float) col.blue() ) / 255,
                                   ( (float) _pdat->modelBackground->alpha() )
                                   / 255 );
  updateLights();
}


void LightModelWindow::updateLights()
{
  _window->setLight( *_window->light() );
  _window->setChanged();
  _window->notifyObservers( _window );
  _window->Refresh();

  /* FIXME
  std::set<AObject*> objects = _window->Objects();
  GLComponent *glc = NULL;

  for (it = objects.begin(), et = objects.end(); it != et; ++it)
  {
    (*it)->setLight(*_window->light());
    glc = (*it)->glAPI();
    glc->glSetChanged(GLComponent::glBODY);
    (*it)->notifyObservers( this );
    (*it)->clearHasChangedFlags();
  }
  */
}


void LightModelWindow::setLocalViewer( bool state )
{
  _window->light()->SetModelLocalViewer( state );
  updateLights();
}


void LightModelWindow::setTwoSide( bool state )
{
  _window->light()->SetModelTwoSide( state );
  updateLights();
}


void LightModelWindow::spotPositionChanged( int )
{
  unsigned	i;
  float		p[3];

  for( i=0; i<3; ++i )
    {
      p[i] = 0.01 * _pdat->spotPos[i]->value();
      _pdat->spotPosLb[i]->setText( QString::number( p[i] ) );
    }
  float	w = _window->light()->Position( 3 );
  _window->light()->SetPosition( p[0], p[1], p[2], w );
  updateLights();
}


void LightModelWindow::spotDirectionChanged( int )
{
  unsigned	i;
  float		p[3];

  for( i=0; i<3; ++i )
    {
      p[i] = 0.01 * _pdat->spotDir[i]->value();
      _pdat->spotDirLb[i]->setText( QString::number( p[i] ) );
    }
  _window->light()->SetSpotDirection( p[0], p[1], p[2] );
  updateLights();
}


void LightModelWindow::spotExponentChanged( int value )
{
  float	f = 0.1 * value;
  _pdat->spotExpoLb->setText( QString::number( f ) );
  _window->light()->SetSpotExponent( f );
  updateLights();
}


void LightModelWindow::spotCutoffChanged( int value )
{
  float	f = 0.1 * value;
  _pdat->spotCutoffLb->setText( QString::number( f ) );
  _window->light()->SetSpotCutoff( f );
  updateLights();
}


void LightModelWindow::attenuationChanged( int )
{
  Light	*l = _window->light();
  float	f = 0.1 * _pdat->attConst->value();
  _pdat->attConstLb->setText( QString::number( f ) );
  l->SetConstantAttenuation( f );
  f = 0.1 * _pdat->attLin->value();
  _pdat->attLinLb->setText( QString::number( f ) );
  l->SetLinearAttenuation( f );
  f = 0.1 * _pdat->attQuad->value();
  _pdat->attQuadLb->setText( QString::number( f ) );
  l->SetQuadraticAttenuation( f );
  updateLights();
}
