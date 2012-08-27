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


#include <anatomist/color/wRendering.h>

#include <qlayout.h>
#include <aims/qtcompat/qvgroupbox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qtabbar.h>
#include <qradiobutton.h>
#include <aims/qtcompat/qbutton.h>
#include <aims/qtcompat/qgrid.h>
#include <aims/qtcompat/qhbox.h>
#include <aims/qtcompat/qvbox.h>
#include <aims/qtcompat/qvbuttongroup.h>
#include <qpixmap.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/object/Object.h>
#include <anatomist/processor/Processor.h>
#include <anatomist/commands/cSetMaterial.h>
#include <anatomist/object/objectparamselect.h>
#include <anatomist/application/settings.h>
#include <anatomist/application/globalConfig.h>
#include <anatomist/surface/globject.h>
#include <cartobase/object/object.h>
#include <QDebug>

using namespace anatomist;
using namespace std;


struct RenderingWindow::Private
{
  Private( const set<AObject *> & o ) : operating( false ), initial( o ), modified( false )
  {}
  bool			operating;
  set<AObject *>	initial;
  ObjectParamSelect	*objsel;
  bool			modified;
};


//	Real RenderingWindow class


RenderingWindow::RenderingWindow( const set<AObject *> &objL, QWidget* parent, 
				const char *name, Qt::WindowFlags f ) 
  : QWidget( parent, f ), _parents( objL ), 
    _privdata( new Private( objL ) )
{



  setObjectName(name);
  setAttribute(Qt::WA_DeleteOnClose);
  setupUi(this);


  if( _parents.size() > 0 )
    _material = (*_parents.begin())->GetMaterial();

  for (set<AObject *>::iterator it=_parents.begin();
       it!=_parents.end();++it)
    (*it)->addObserver( (Observer*)this );

  setCaption( name );
  if( windowFlags() & Qt::Window )
    {
      QPixmap anaicon( Settings::findResourceFile( "icons/icon.xpm"
        ).c_str() );
      if( !anaicon.isNull() )
        setIcon( anaicon );
    }



  ObjectParamSelect *sel = new ObjectParamSelect( _parents, select_container_widget);
  _privdata->objsel = sel;
  QVBoxLayout	*vboxlayout = new QVBoxLayout(select_container_widget);
  vboxlayout->addWidget(sel);
  vboxlayout->setSpacing(0.);
  select_container_widget->layout()->setMargin(0.); 

  GlobalConfiguration   *cfg = theAnatomist->config();
  bool use_glshader = Shader::isActivated();
  if (!use_glshader) shader_tab->setEnabled(false);
  if (theAnatomist->userLevel() < 3) reload_pushButton->hide();

  // connections
  connect( sel, SIGNAL( selectionStarts() ), this, SLOT( chooseObject() ) );
  connect( sel, 
           SIGNAL( objectsSelected( const std::set<anatomist::AObject *> & ) ),
           this, 
           SLOT( objectsChosen( const std::set<anatomist::AObject *> & ) ) );

  connect( enable_shaders_checkBox, SIGNAL( stateChanged( int ) ), this, 
           SLOT( enableShadersClicked( int ) ) );
  connect( rendering_buttonGroup, SIGNAL( buttonClicked( int ) ), this, 
           SLOT( renderModeChanged( int ) ) );
  connect( rendering_buttonGroup, SIGNAL( buttonClicked( int ) ), this, 
           SLOT( renderModeChanged( int ) ) );
  connect( display_buttonGroup, SIGNAL( buttonClicked( int) ), this, 
           SLOT( renderPropertyChanged( int ) ) );
  connect( lighting_model_buttonGroup, SIGNAL( buttonClicked( int) ), this, 
           SLOT( lightingModelChanged( int ) ) );
  connect( interpolation_model_buttonGroup, SIGNAL( buttonClicked( int)), this, 
           SLOT( interpolationModelChanged( int ) ) );
  connect( coloring_model_buttonGroup, SIGNAL( buttonClicked( int) ), this, 
           SLOT( coloringModelChanged( int ) ) );
  connect( reload_pushButton, SIGNAL( clicked( ) ), this, 
           SLOT( reloadClicked( ) ) );

  bool state = (Shader::isUsedByDefault());
  lighting_model_groupBox->setEnabled(state);
  interpolation_model_groupBox->setEnabled(state);
  coloring_model_groupBox->setEnabled(state);
  Qt::CheckState  check_state = (state ? Qt::Checked : Qt::Unchecked);
  enable_shaders_checkBox->setCheckState(check_state);


  default_lighting_model_radioButton->hide();
  default_interpolation_model_radioButton->hide();
  default_coloring_model_radioButton->hide();
  directions_coloring_model_radioButton->hide();

  updateInterface();
}


RenderingWindow::~RenderingWindow()
{
  /* sending SetMaterial command only when window is closed, to avoid sending 
     thousands of commands while tuning the colors */
  runCommand();

  cleanupObserver();

  delete _privdata;
}


void RenderingWindow::update( const Observable*, void* arg )
{
  if( arg != this && !_privdata->operating )
    updateInterface();
}


void RenderingWindow::unregisterObservable( Observable* obs )
{
  runCommand();
  Observer::unregisterObservable( obs );
  AObject	*o = dynamic_cast<AObject *>( obs );
  if( !o )
    return;
  _parents.erase( o );
  _privdata->objsel->updateLabel( _parents );
  updateInterface();
}


void RenderingWindow::updateObjectsRendering()
{
  set<AObject *>::const_iterator io, fo=objects().end();

  for( io=objects().begin(); io!=fo; ++io )
    {
      (*io)->SetMaterial( getMaterial() );
      (*io)->notifyObservers( this );
      (*io)->clearHasChangedFlags();
    }
}


void RenderingWindow::updateObjectsShading()
{
  set<AObject *>::const_iterator io, fo=objects().end();
  GLComponent *glc = NULL;

  for( io=objects().begin(); io!=fo; ++io )
  {
    glc = (*io)->glAPI();
    glc->SetShader( getShader() );
    glc->glSetChanged(GLComponent::glBODY);
  }
  for( io=objects().begin(); io!=fo; ++io )
  {
    (*io)->notifyObservers( this );
    (*io)->clearHasChangedFlags();
  }
}


void RenderingWindow::removeObjectsShading()
{
  set<AObject *>::const_iterator io, fo=objects().end();
  GLComponent *glc = NULL;

  for( io=objects().begin(); io!=fo; ++io )
  {
    glc = (*io)->glAPI();
    glc->removeShader();
    glc->glSetChanged(GLComponent::glBODY);
  }
  for( io=objects().begin(); io!=fo; ++io )
  {
    (*io)->notifyObservers( this );
    (*io)->clearHasChangedFlags();
  }
}


namespace
{

  void toto()
  {
  }

  void setButtonState( QButton* b, int x )
  {
    QCheckBox	*cb = dynamic_cast<QCheckBox *>( b );
    if( !cb )
      return;
    switch( x )
      {
      case 0:
        cb->setCheckState( Qt::Unchecked );
        break;
      case 1:
        cb->setCheckState( Qt::Checked );
        break;
      default:
        cb->setCheckState( Qt::PartiallyChecked );
      }
  }

}

void RenderingWindow::updateInterface()
{
	if( _privdata->operating )
		return;

	_privdata->operating = true;

	blockSignals( true );

	unsigned	panel, i, n = tabWidget->count();

	if( _parents.empty() )
	{
		tabWidget->setEnabled( false );
	}
	else
	{
		tabWidget->setEnabled( true );

		//material
		_material = (*_parents.begin())->GetMaterial();
		rendering_buttonGroup->button(-_material.renderProperty(Material::RenderMode) - 3)->setChecked(true);
		setButtonState(lighting_checkBox, _material.renderProperty(Material::RenderLighting));
		setButtonState(smooth_shading_checkBox, _material.renderProperty(Material::RenderSmoothShading));
		setButtonState(smooth_polygons_checkBox, _material.renderProperty(Material::RenderFiltering));
		setButtonState(depth_buffer_checkBox, _material.renderProperty(Material::RenderZBuffer));
		setButtonState(cull_polygon_faces_checkBox, _material.renderProperty(Material::RenderFaceCulling));

		//shader
  		GLComponent *glc = (*_parents.begin())->glAPI();
		const Shader *shader = glc->getShader();
		if (shader) _shader = *shader;
		else _shader = Shader();
		lighting_model_buttonGroup->button(-_shader.getLightingModel() - 3)->setChecked(true);
		interpolation_model_buttonGroup->button(-_shader.getInterpolationModel() - 3)->setChecked(true);
		coloring_model_buttonGroup->button(-_shader.getColoringModel() - 3)->setChecked(true);
	}

	blockSignals( false );
	_privdata->operating = false;
}




void RenderingWindow::chooseObject()
{
  set<AObject *>::iterator	ir = _privdata->initial.begin(), 
    er = _privdata->initial.end(), ir2;
  while( ir!=er )
    if( theAnatomist->hasObject( *ir ) )
      ++ir;
    else
      {
        ir2 = ir;
        ++ir;
        _privdata->initial.erase( ir2 );
      }

  _privdata->objsel->selectObjects( _privdata->initial, _parents );
}


void RenderingWindow::objectsChosen( const set<AObject *> & o )
{
  _privdata->operating = true;
  runCommand();

  set<AObject *>::const_iterator	i, e = o.end();
  while( !_parents.empty() )
    (*_parents.begin())->deleteObserver( this );
  _parents = o;
  for( i=o.begin(); i!=e; ++i )
    (*i)->addObserver( this );

  _privdata->objsel->updateLabel( o );
  _privdata->operating = false;
  updateInterface();
}


void RenderingWindow::runCommand()
{
  if( _privdata->modified && !_parents.empty() )
    {
      string	rmode;

      switch( _material.renderProperty( Material::RenderMode ) )
        {
        case Material::Normal:
          rmode = "normal";
          break;
        case Material::Wireframe:
          rmode = "wireframe";
          break;
        case Material::Outlined:
          rmode = "outlined";
          break;
        case Material::HiddenWireframe:
          rmode = "hiddenface_wireframe";
          break;
        default:
          rmode = "default";
        }

      SetMaterialCommand	*com 
        = new SetMaterialCommand
        ( _parents, _material.Ambient(), _material.Diffuse(), 
          _material.Emission(), _material.Specular(), _material.Shininess(), 
          true, _material.renderProperty( Material::RenderLighting ), 
          _material.renderProperty( Material::RenderSmoothShading ), 
          _material.renderProperty( Material::RenderFiltering ), 
          _material.renderProperty( Material::RenderZBuffer ), 
          _material.renderProperty( Material::RenderFaceCulling ), 
          rmode );
      theProcessor->execute( com );
      _privdata->modified = false;
    }
}


void RenderingWindow::renderModeChanged( int x )
{
  if( _privdata->operating )
    return;
  _privdata->operating = true;

  _material.setRenderProperty( Material::RenderMode, -x - 3 );

  _privdata->modified = true;
  updateObjectsRendering();
  _privdata->operating = false;
}


void RenderingWindow::renderPropertyChanged( int x )
{
  if( _privdata->operating )
    return;
  _privdata->operating = true;

  int	y;
  switch(( (QCheckBox *) display_buttonGroup->button(x))->checkState() )
    {
    case Qt::Checked:
      y = 1;
      break;
    case Qt::Unchecked:
      y = 0;
      break;
    default:
      y = -1;
    }

  _material.setRenderProperty( (Material::RenderProperty) (-x - 2), y );

  _privdata->modified = true;
  updateObjectsRendering();
  _privdata->operating = false;
}

void RenderingWindow::enableShadersClicked( int x )
{
  bool state = (x == Qt::Checked);

  lighting_model_groupBox->setEnabled(state);
  interpolation_model_groupBox->setEnabled(state);
  coloring_model_groupBox->setEnabled(state);

  if (state)
  {
    _shader.load_if_needed();
    updateObjectsShading();
  }
  else
    removeObjectsShading();
}


void RenderingWindow::lightingModelChanged( int x )
{
  //XXX : skip default (window default)
  if (x == -2)
  {
    std::cout << "Default not implemented yet" << std::endl;
  } else {
    _shader.setLightingModel((Shader::LightingModel) (-x - 3));
    _shader.load_if_needed();
    updateObjectsShading();
  }
}

void RenderingWindow::interpolationModelChanged( int x )
{
  //XXX : skip default (window default)
  if (x == -2)
  {
    std::cout << "Default not implemented yet" << std::endl;
  } else {
    _shader.setInterpolationModel((Shader::InterpolationModel) (-x - 3));
    _shader.load_if_needed();
    updateObjectsShading();
  }
}

void RenderingWindow::coloringModelChanged( int x )
{
  //XXX : skip default (window default)
  if (x == -2)
  {
    std::cout << "Default not implemented yet" << std::endl;
  } else {
    _shader.setColoringModel((Shader::ColoringModel) (-x - 3));
    _shader.load_if_needed();
    updateObjectsShading();
  }
}


void RenderingWindow::reloadClicked(void)
{
  set<AObject *>::const_iterator io, fo=objects().end();
  GLComponent *glc = NULL;

  _shader.reload();

  for( io=objects().begin(); io!=fo; ++io )
    {
      glc = (*io)->glAPI();
      if (not glc->getShader()) glc->SetShader( getShader() );
      glc->glSetChanged(GLComponent::glBODY);
      (*io)->notifyObservers( this );
      (*io)->clearHasChangedFlags();
    }
}
