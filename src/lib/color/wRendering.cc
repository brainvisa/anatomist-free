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
#include <qcheckbox.h>
#include <qlabel.h>
#include <qtabbar.h>
#include <qradiobutton.h>
#include <qpixmap.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/object/Object.h>
#include <anatomist/processor/Processor.h>
#include <anatomist/commands/cSetMaterial.h>
#include <anatomist/object/objectparamselect.h>
#include <anatomist/application/settings.h>
#include <anatomist/application/globalConfig.h>
#include <anatomist/surface/glcomponent.h>
#include <anatomist/dialogs/colorDialog.h>
#include <cartobase/object/object.h>

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

  setWindowTitle( name );
  if( windowFlags() & Qt::Window )
  {
    QPixmap anaicon( Settings::findResourceFile( "icons/icon.xpm"
      ).c_str() );
    if( !anaicon.isNull() )
      setWindowIcon( anaicon );
  }


  ObjectParamSelect *sel
    = new ObjectParamSelect( _parents, select_container_widget);
  _privdata->objsel = sel;
  QVBoxLayout	*vboxlayout = new QVBoxLayout(select_container_widget);
  vboxlayout->addWidget(sel);
  vboxlayout->setSpacing(0.);
  select_container_widget->layout()->setMargin(0.); 

  GlobalConfiguration   *cfg = theAnatomist->config();
  bool use_glshader = Shader::isActivated();
  if (!use_glshader) shader_tab->setEnabled(false);
  if (theAnatomist->userLevel() < 3) reload_pushButton->hide();

  lineWidth_lineEdit->setValidator( new QDoubleValidator( 0., 100., 2, 0 ) );

  int mstate = _material.renderProperty( Material::UseShader );
  bool state = ( ( Shader::isUsedByDefault() && mstate != 0 ) || mstate > 0 );
  lighting_model_groupBox->setEnabled(state);
  interpolation_model_groupBox->setEnabled(state);
  coloring_model_groupBox->setEnabled(state);
  Qt::CheckState  check_state = (state ? Qt::Checked : Qt::Unchecked);
  enable_shaders_checkBox->setCheckState(check_state);

  default_lighting_model_radioButton->hide();
  default_interpolation_model_radioButton->hide();
  default_coloring_model_radioButton->hide();
  directions_coloring_model_radioButton->hide();

  // connections
  connect( sel, SIGNAL( selectionStarts() ), this, SLOT( chooseObject() ) );
  connect( sel, 
           SIGNAL( objectsSelected( const std::set<anatomist::AObject *> & ) ),
           this, 
           SLOT( objectsChosen( const std::set<anatomist::AObject *> & ) ) );

  connect( rendering_buttonGroup, SIGNAL( buttonClicked( int ) ), this, 
           SLOT( renderModeChanged( int ) ) );
  connect( selection_buttonGroup, SIGNAL( buttonClicked( int ) ), this,
           SLOT( selectionModeChanged( int ) ) );
  connect( lineWidth_lineEdit, SIGNAL( editingFinished() ), this,
           SLOT( lineWidthChanged() ) );
  connect( display_buttonGroup, SIGNAL( buttonClicked( int) ), this,
           SLOT( renderPropertyChanged( int ) ) );
  connect( unlitColor_pushButton, SIGNAL( clicked() ), this,
           SLOT( unlitColorClicked() ) );
  connect( enable_shaders_checkBox, SIGNAL( stateChanged( int ) ), this,
           SLOT( enableShadersClicked( int ) ) );
  connect( lighting_model_buttonGroup, SIGNAL( buttonClicked( int) ), this,
           SLOT( lightingModelChanged( int ) ) );
  connect( interpolation_model_buttonGroup, SIGNAL( buttonClicked( int)), this, 
           SLOT( interpolationModelChanged( int ) ) );
  connect( coloring_model_buttonGroup, SIGNAL( buttonClicked( int) ), this, 
           SLOT( coloringModelChanged( int ) ) );
  connect( reload_pushButton, SIGNAL( clicked( ) ), this, 
           SLOT( reloadClicked( ) ) );

  updateInterface();
  static_cast<QBoxLayout *>( layout() )->addStretch( 1 );

}


RenderingWindow::~RenderingWindow()
{
  _privdata->operating = true;

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
  int mstate = getMaterial().renderProperty( Material::UseShader );
  bool state = mstate > 0 || ( mstate < 0 && Shader::isUsedByDefault() );
  if( state )
    _shader.load_if_needed();
  else
    removeObjectsShading();

  for( io=objects().begin(); io!=fo; ++io )
  {
    GLComponent *glc = (*io)->glAPI();
    if( state )
      glc->setShader( getShader() );
    (*io)->SetMaterial( getMaterial() );
  }
  for( io=objects().begin(); io!=fo; ++io )
  {
    (*io)->notifyObservers( this );
    (*io)->clearHasChangedFlags();
  }
}


void RenderingWindow::updateObjectsShading()
{
  updateObjectsRendering();
}


void RenderingWindow::removeObjectsShading()
{
  set<AObject *>::const_iterator io, fo=objects().end();
  GLComponent *glc = NULL;

  for( io=objects().begin(); io!=fo; ++io )
  {
    glc = (*io)->glAPI();
    (*io)->GetMaterial().setRenderProperty( Material::UseShader, 0 );
    glc->removeShader();
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

  void setButtonState( QAbstractButton* b, int x )
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
    rendering_buttonGroup->button(
      -_material.renderProperty(Material::RenderMode) - 3)->setChecked(true);
    int selmode = _material.renderProperty( Material::SelectableMode );
    if( selmode < 0 )
      selmode = Material::SelectableWhenOpaque;
    selection_buttonGroup->button( -selmode - 2 )->setChecked(true);
    setButtonState(lighting_checkBox,
                   _material.renderProperty(Material::RenderLighting));
    setButtonState(smooth_shading_checkBox,
                   _material.renderProperty(Material::RenderSmoothShading));
    setButtonState(smooth_polygons_checkBox,
                   _material.renderProperty(Material::RenderFiltering));
    setButtonState(depth_buffer_checkBox,
                   _material.renderProperty(Material::RenderZBuffer));
    setButtonState(cull_polygon_faces_checkBox,
                   _material.renderProperty(Material::RenderFaceCulling));
    lineWidth_lineEdit->setText( QString::number( _material.lineWidth() ) );
    QPixmap pix( 32, 16 );
    pix.fill( QColor( (int) ( _material.unlitColor(0) * 255.9 ),
                      (int) ( _material.unlitColor(1) * 255.9 ),
                      (int) ( _material.unlitColor(2) * 255.9 ) ) );
    unlitColor_pushButton->setIcon( pix );

    //shader
    GLComponent *glc = (*_parents.begin())->glAPI();
    const Shader *shader = glc->getShader();
    int mstate = _material.renderProperty( Material::UseShader );
    bool state = ( ( Shader::isUsedByDefault() && mstate != 0 ) || mstate > 0 );
    enable_shaders_checkBox->setChecked( state );
    int lighting_model = _material.renderProperty( Material::RenderLighting );
    if( lighting_model < 0 )
      lighting_model = Material::BlinnPhongLighting;
    lighting_model_buttonGroup->button(-lighting_model - 3)->setChecked(true);
    int interpolation = _material.renderProperty(
      Material::RenderSmoothShading );
    if( interpolation < 0 )
      interpolation = Material::PhongShading;
    interpolation_model_buttonGroup->button(
      -interpolation - 3)->setChecked(true);
    int coloring_model = _material.renderProperty(
      Material::ShaderColorNormals );
    if( coloring_model < 0 )
      coloring_model = 0;
    coloring_model_buttonGroup->button(-coloring_model - 3)->setChecked(true);
    if (shader)
      _shader = *shader;
    else
    {
      _shader = Shader();
      _shader.setModels( (Shader::LightingModel) lighting_model,
                         (Shader::InterpolationModel) interpolation,
                         (Shader::ColoringModel) coloring_model,
                         Shader::DefaultMaterialModel );
    }
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
      string	rmode, smode;

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
      vector<float> unlit(4);
      unlit[0] = _material.unlitColor(0);
      unlit[1] = _material.unlitColor(1);
      unlit[2] = _material.unlitColor(2);
      unlit[3] = _material.unlitColor(3);
      switch( _material.renderProperty( Material::SelectableMode ) )
      {
      case Material::AlwaysSelectable:
        smode = "always_selectable";
        break;
      case Material::GhostSelection:
        smode = "ghost";
        break;
      case Material::SelectableWhenOpaque:
        smode = "selectable_when_opaque";
        break;
      case Material::SelectableWhenNotTotallyTransparent:
        smode = "selectable_when_not_totally_transparent";
        break;
      default:
        smode = "default";
        break;
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
          rmode,
          _material.renderProperty( Material::FrontFace ),
          _material.lineWidth(),
          unlit,
          smode,
          _material.renderProperty( Material::UseShader ),
          _material.renderProperty( Material::ShaderColorNormals ) //,
          // _material.renderProperty( Material::NormalIsDirection )
        );
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

  Material::RenderProperty prop = (Material::RenderProperty) (-x - 2);
  _material.setRenderProperty( prop, y );

  if( prop == Material::RenderLighting && y >= 0 )
  {
    if( y > 0 )
      y = 2; // if on, use blinn-phong
    if( y >= 0 )
    {
      _shader.setLightingModel( (Shader::LightingModel)( y ) );
      lighting_model_buttonGroup->button( -y - 3 )->setChecked(true);
    }
  }
  else if( prop == Material::RenderSmoothShading && y >= 0 )
  {
    _shader.setInterpolationModel( (Shader::InterpolationModel)( y ) );
    if( y >= 0 )
    interpolation_model_buttonGroup->button( -y - 3 )->setChecked(true);
  }

  _privdata->modified = true;
  updateObjectsRendering();
  _privdata->operating = false;
}

void RenderingWindow::enableShadersClicked( int x )
{
  bool state = (x == Qt::Checked);

  _material.setRenderProperty( Material::UseShader, int( state ) );

  lighting_model_groupBox->setEnabled(state);
  interpolation_model_groupBox->setEnabled(state);
  coloring_model_groupBox->setEnabled(state);

  updateObjectsRendering();
  _privdata->modified = true;
}


void RenderingWindow::lightingModelChanged( int x )
{
  _material.setRenderProperty( Material::RenderLighting, -x - 3 );
  setButtonState(lighting_checkBox,
                  _material.renderProperty(Material::RenderLighting));
  //XXX : skip default (window default)
  if (x == -2)
  {
    std::cout << "Default not implemented yet" << std::endl;
  }
  else
  {
    _shader.setLightingModel((Shader::LightingModel) (-x - 3));
    updateObjectsShading();
  }
  _privdata->modified = true;
}

void RenderingWindow::interpolationModelChanged( int x )
{
  _material.setRenderProperty( Material::RenderSmoothShading, -x - 3 );
  //XXX : skip default (window default)
  setButtonState(smooth_shading_checkBox,
                  _material.renderProperty(Material::RenderSmoothShading));
  if (x == -2)
  {
    std::cout << "Default not implemented yet" << std::endl;
  } else {
    _shader.setInterpolationModel((Shader::InterpolationModel) (-x - 3));
    _shader.load_if_needed();
    updateObjectsShading();
  }
  _privdata->modified = true;
}

void RenderingWindow::coloringModelChanged( int x )
{
  _material.setRenderProperty( Material::ShaderColorNormals, -x - 3 );
  //XXX : skip default (window default)
  if (x == -2)
  {
    std::cout << "Default not implemented yet" << std::endl;
  }
  else
  {
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
      if (not glc->getShader()) glc->setShader( getShader() );
      glc->glSetChanged(GLComponent::glBODY);
      (*io)->notifyObservers( this );
      (*io)->clearHasChangedFlags();
    }
}


void RenderingWindow::selectionModeChanged( int x )
{
  if( _privdata->operating )
    return;
  _privdata->operating = true;

  _material.setRenderProperty( Material::SelectableMode, -x - 2 );

  _privdata->modified = true;
  updateObjectsRendering();
  _privdata->operating = false;
}


void RenderingWindow::lineWidthChanged()
{
  if( _privdata->operating )
    return;
  _privdata->operating = true;

  bool ok = false;
  float value = lineWidth_lineEdit->text().toFloat( &ok );
  if( ok )
    _material.setLineWidth( value );
  else
    lineWidth_lineEdit->setText( QString::number( _material.lineWidth() ) );

  _privdata->modified = true;
  updateObjectsRendering();
  _privdata->operating = false;
}


void RenderingWindow::unlitColorClicked()
{
  if( _privdata->operating )
    return;
  _privdata->operating = true;

  const GLfloat* mcol = _material.unlitColor();
  int   alpha = (int) ( mcol[3] * 255.9 );

  QColor col
    = QAColorDialog::getColor(
      QColor( (int) ( mcol[0] * 255.9 ),
              (int) ( mcol[1] * 255.9 ),
              (int) ( mcol[2] * 255.9 ) ),
      this,
      tr( "Unlit color" ).toStdString().c_str(), &alpha, 0 );
  if( col.isValid() )
  {
    _material.setUnlitColor( ((float) col.red()) / 255,
                             ((float) col.green()) / 255,
                             ((float) col.blue()) / 255,
                             ((float) alpha) / 255 );

    QPixmap pix( 32, 16 );
    pix.fill( col );
    unlitColor_pushButton->setIcon( pix );
  }

  _privdata->modified = true;
  updateObjectsRendering();
  _privdata->operating = false;
}

