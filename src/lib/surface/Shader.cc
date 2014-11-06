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

#include <anatomist/surface/Shader.h>
#include <anatomist/application/settings.h>
#include <anatomist/window/viewstate.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/application/globalConfig.h>
#include <anatomist/window/glwidgetmanager.h>
#include <anatomist/surface/glcomponent.h>
#include <anatomist/surface/texsurface.h>
#include <anatomist/volume/Volume.h>
#include <cartobase/stream/fileutil.h>
#include <cartobase/object/object.h>
#include <QDebug>
#include <QObject>
#include <QPointer>
#include <QGLShaderProgram>
#include <QGLShader>
#include <anatomist/window3D/glwidget3D.h> //FIXME to del ?
#include <anatomist/window3D/window3D.h> //FIXME to del ?

using namespace anatomist;
using namespace carto;
using namespace std;


Shader::LightingModel_::EnumType Shader::DefaultLightingModel =
				Shader::LightingModel_::BlinnPhong;
Shader::InterpolationModel_::EnumType Shader::DefaultInterpolationModel =
				Shader::InterpolationModel_::Gouraud;
Shader::ColoringModel_::EnumType Shader::DefaultColoringModel =
				Shader::ColoringModel_::Material;
Shader::MaterialModel_::EnumType Shader::DefaultMaterialModel =
				Shader::MaterialModel_::Classic;


struct Shader::Private
{
  Private();
  ~Private();
  Private( const Private & );
  Private & operator = ( const Private & );
  QSharedPointer<QGLShaderProgram *> _shader_program_p;
  bool enable;

  static QWeakPointer<QGLShaderProgram *> _shader_programs_p[3][3][3][2];
  static bool _dummy_init_shader_programs_p;
  static bool init_shader_programs_p(void);
  static QSharedPointer<QGLShaderProgram *> null_p;

  static QString _lightingDirs[3];
  static QString _interpolationBasenames[3];
  static QString _materialDirs[2];
  static QString _diffuseDirs[2];
};


QWeakPointer<QGLShaderProgram *> Shader::Private::_shader_programs_p[3][3][3][2];
bool Shader::Private::_dummy_init_shader_programs_p = Shader::Private::init_shader_programs_p();
QSharedPointer<QGLShaderProgram *> Shader::Private::null_p;

QString Shader::Private::_lightingDirs[3] = {
	"no_light_model", "phong_light_model", "blinn_phong_light_model" };
QString Shader::Private::_interpolationBasenames[3] = {
	"flat_shader", "gouraud_shader", "phong_shader" };
//QString Shader::Private::_materialDirs[2]; //TODO
QString Shader::Private::_diffuseDirs[2] = {
	"classic_diffuse_reflectance_model",
	"oren_nayar_diffuse_reflectance_model" };


bool Shader::Private::init_shader_programs_p(void)
{

  null_p = QSharedPointer<QGLShaderProgram *>((QGLShaderProgram **) 0);

  for (int i = 0; i < 3; ++i)
  for (int j = 0; j < 3; ++j)
  for (int k = 0; k < 3; ++k)
  for (int l = 0; l < 2; ++l)
    _shader_programs_p[i][j][k][l] = null_p;

  return true;
};


static void doDeleteInternal(QGLShaderProgram **obj)
{
  delete obj[0];
  delete[] obj;
}


Shader::Private::Private() : _shader_program_p((QGLShaderProgram **) 0, doDeleteInternal), enable(true)
{
}


Shader::Private::Private( const Private & p ) :
	_shader_program_p(p._shader_program_p), enable(p.enable)
{
}


Shader::Private::~Private()
{
}


Shader::Private & Shader::Private::operator = ( const Private & p )
{
  _shader_program_p = p._shader_program_p;
  return *this;
}


Shader::Shader()
  : d( new Private ),
    _lighting_model(DefaultLightingModel),
    _interpolation_model(DefaultInterpolationModel),
    _coloring_model(DefaultColoringModel),
    _material_model(DefaultMaterialModel)
{
}


Shader::Shader(const Shader &shader)
  : d( new Private( *shader.d ) ),
  _lighting_model(shader._lighting_model),
  _interpolation_model(shader._interpolation_model),
  _coloring_model(shader._coloring_model),
  _material_model(shader._material_model)
{
}


Shader::~Shader()
{
  delete d;
}


bool	Shader::isSupported(void) // cache version
{
  static bool	support_glshader = _isSupported();

  return support_glshader;
}


bool	Shader::_isSupported(void)
{
  QGLWidget     *shared_widget = GLWidgetManager::sharedWidget();
  shared_widget->makeCurrent();

  bool support_glshader = QGLShaderProgram::hasOpenGLShaderPrograms();
#ifdef GL_SHADING_LANGUAGE_VERSION
  const GLubyte *glshader_version = glGetString(GL_SHADING_LANGUAGE_VERSION);
  if(support_glshader and glshader_version)
  {
        try
        {
                QString v(QString((const char *) glshader_version).split(" ")[0]);
                support_glshader = (v.toFloat() >= 1.2);
        }
        catch(...)
        {
                support_glshader = false;
        }
  }
  else support_glshader = false;
#else
  /* GL_SHADING_LANGUAGE_VERSION is not defined: shaders are not
     available at compile time with the current OpenGL headers.
  */
  support_glshader = false;
#endif

  return support_glshader;
}


bool	Shader::getAnatomistDefaultBehaviour(void)
{
  return isSupported();
}


bool	Shader::isActivated(void)
{
  bool use_glshader = getAnatomistDefaultBehaviour();
  GlobalConfiguration   *cfg = theAnatomist->config();

  try
  {
    carto::Object  x = cfg->getProperty( "disableOpenGLShader" );
    if( !x.isNull() )
      use_glshader = (bool) !x->getScalar();
  }
  catch( ... )
  {
  }

  return use_glshader;
}


bool	Shader::isUsedByDefault(void)
{
  GlobalConfiguration   *cfg = theAnatomist->config();
  int useglshaderbydefault = 0;

  try
  {
    Object  x = cfg->getProperty( "shadersByDefault" );
    if( !x.isNull() )
      useglshaderbydefault = (int) x->getScalar();
  }
  catch( ... )
  {
  }
  return useglshaderbydefault;
}


void Shader::load_if_needed(void)
{  
  QWeakPointer<QGLShaderProgram *> &shader_program_w = d->_shader_programs_p[_lighting_model][_interpolation_model][_coloring_model][_material_model];


  if (not d->_shader_program_p.isNull())
    d->_shader_program_p.clear();

  if (not shader_program_w.data())
  {
    QGLWidget	*shared_widget = GLWidgetManager::sharedWidget();
    shared_widget->makeCurrent();
    QGLShaderProgram *pgm = new QGLShaderProgram(shared_widget->context());
    QGLShaderProgram **pgm_p = new QGLShaderProgram *[1];

    pgm_p[0] = pgm;
    d->_shader_program_p = QSharedPointer<QGLShaderProgram *>(pgm_p, doDeleteInternal);
    shader_program_w = d->_shader_program_p;
    load();
  }
  else d->_shader_program_p = shader_program_w;
}


void Shader::reload(void)
{

  if (d->_shader_program_p.isNull()) return;
  QWeakPointer<QGLShaderProgram *> &shader_program_w = d->_shader_programs_p[_lighting_model][_interpolation_model][_coloring_model][_material_model];

  QGLWidget	*shared_widget = GLWidgetManager::sharedWidget();
  shared_widget->makeCurrent();
  QGLShaderProgram *pgm = new QGLShaderProgram(shared_widget->context());
  (shader_program_w.data())[0] = pgm;
  load();
}


void Shader::load(void)
{
  QGLShader	vertex_shader(QGLShader::Vertex);
  QGLShader	fragment_shader(QGLShader::Fragment);
  QString	shader_basename;
  QString	fragment_shader_filename;
  QString	vertex_shader_filename;

  shader_basename = QString(Settings::globalPath().c_str()) + carto::FileUtil::separator() + "shaders" + carto::FileUtil::separator() + "meshes" + carto::FileUtil::separator() + d->_lightingDirs[_lighting_model] + carto::FileUtil::separator() + d->_interpolationBasenames[_interpolation_model];

  fragment_shader_filename = (shader_basename + ".frag");
  vertex_shader_filename = (shader_basename + ".vert");

  QGLShaderProgram *shader_program = (*(d->_shader_program_p));

  QGLWidget	*shared_widget = GLWidgetManager::sharedWidget();
  shared_widget->makeCurrent();

  if ((not fragment_shader.compileSourceFile(fragment_shader_filename)) ||
      (not shader_program->addShader(&fragment_shader)))
    qDebug() << "fragment shader error :" << fragment_shader_filename << "\n"
             << fragment_shader.log();

  if ((not vertex_shader.compileSourceFile(vertex_shader_filename)) ||
      (not shader_program->addShader(&vertex_shader)))
    qDebug() << "vertex shader error: " << vertex_shader_filename << "\n"
             << vertex_shader.log();
  
  if (not shader_program->link())
    qDebug() << "shader link error:\n" << shader_program->log();
}


void Shader::enable(void)
{
  d->enable = true;  
}


void Shader::disable(void)
{
  d->enable = false;
}

void Shader::setShaderParameters(const GLMObject &obj, const ViewState & state) const
{
  setShaderParameters( static_cast<const GLComponent &>( obj ), state );
}


void Shader::setShaderParameters( const GLComponent &obj,
                                  const ViewState & state ) const
{
  bool normalIsDirection = false;
  if( obj.glMaterial() )
    normalIsDirection = obj.glMaterial()->renderProperty(
      Material::NormalIsDirection ) > 0;
  QGLShaderProgram *shader_program = (*(d->_shader_program_p));
  int id = shader_program->uniformLocation( "normalIsDirection" );
  shader_program->setUniformValue( id, normalIsDirection );
}


void Shader::setShaderParameters(const ATexSurface &obj, const ViewState & state) const
{
  setShaderParameters( static_cast<const GLComponent &>( obj ), state );
  QGLShaderProgram *shader_program = (*(d->_shader_program_p));

  int id = shader_program->uniformLocation("is2dtexture");	
  unsigned int dim = obj.glDimTex(state);
  
  if (dim == 1)
    shader_program->setUniformValue(id, 0);
  else if (dim == 2)
    shader_program->setUniformValue(id, 1);

}


void Shader::setShaderParameters(const AVolumeBase &obj, const ViewState & state) const
{
  setShaderParameters( static_cast<const GLComponent &>( obj ), state );
  QGLShaderProgram *shader_program = (*(d->_shader_program_p));

  int id = shader_program->uniformLocation("is2dtexture");	
  
  shader_program->setUniformValue(id, 1);
}


void Shader::bind(const GLComponent &glc, const ViewState & state)
{
  if (not d->enable) return;
  if (d->_shader_program_p.isNull()) return;

  QGLShaderProgram *shader_program = (*(d->_shader_program_p));
  bool	bind;

  bind = shader_program->bind();

  // set coloring mode
  int coloring_model_id = shader_program->uniformLocation("coloringModel");	
  shader_program->setUniformValue(coloring_model_id, _coloring_model);

  glc.setShaderParameters(*this, state);

  if (not bind)
    qDebug() << "shader binding error: \n" << shader_program->log();
}

void Shader::release(void)
{
  if (not d->enable) return;
  if (d->_shader_program_p.isNull()) return;

  QGLShaderProgram *shader_program = (*(d->_shader_program_p));

  shader_program->release();
}


Shader &Shader::operator = (const Shader &theShader)
{
  if (this != &theShader)
    {
      _lighting_model = theShader._lighting_model;
      _interpolation_model = theShader._interpolation_model;
      _coloring_model = theShader._coloring_model;
      *d = *theShader.d;
    }

  return(*this);
}


std::ostream & anatomist::operator << (std::ostream &out, 
			   const anatomist::Shader &shader)
{
  out << shader._lighting_model << " "
      << shader._interpolation_model << " "
      << shader._coloring_model << endl;
  return(out);
}


bool Shader::operator != ( const Shader& shader ) const
{
  return(_lighting_model != shader._lighting_model ||
	_interpolation_model != shader._interpolation_model ||
	_coloring_model != shader._coloring_model ||
	d->_shader_program_p != shader.d->_shader_program_p);
}


void Shader::enable_all(void)
{
  const set<AObject *> objs = theAnatomist->getObjects();
  set<AObject *>::const_iterator it, et;
  GLComponent *glc = NULL;
  Shader *shader = NULL;

  for (it = objs.begin(), et = objs.end(); it != et; ++it)
  {
    glc = (*it)->glAPI();
    shader = glc->getShader();
    if (not shader) continue;
    shader->enable();
    glc->glSetChanged(GLComponent::glBODY);
  }
  for (it = objs.begin(), et = objs.end(); it != et; ++it)
  {
    glc = (*it)->glAPI();
    shader = glc->getShader();
    if (not shader) continue;
    (*it)->notifyObservers( NULL );
    (*it)->clearHasChangedFlags();
  } 
}


void Shader::disable_all(void)
{
  const set<AObject *> objs = theAnatomist->getObjects();
  set<AObject *>::const_iterator it, et;
  GLComponent *glc = NULL;
  Shader *shader = NULL;

  for (it = objs.begin(), et = objs.end(); it != et; ++it)
  {
    glc = (*it)->glAPI();
    shader = glc->getShader();
    if (not shader) continue;
    shader->disable();
    glc->glSetChanged(GLComponent::glBODY);
  }
  for (it = objs.begin(), et = objs.end(); it != et; ++it)
  {
    glc = (*it)->glAPI();
    shader = glc->getShader();
    if (not shader) continue;
    (*it)->notifyObservers( NULL );
    (*it)->clearHasChangedFlags();
  } 
}

