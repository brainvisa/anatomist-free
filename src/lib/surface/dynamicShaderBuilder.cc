#include <QFile>
#include <QTextStream>
#include <QString>
#include <cstring>
#include <string>
#include <iostream>
#include <memory>
#include <regex>
#include <list>
#include <anatomist/config/version.h> 

#include <anatomist/surface/dynamicShaderBuilder.h>
#include <anatomist/surface/shaderMapping.h>
#include <cartobase/config/paths.h>
#include <anatomist/surface/blinnPhongIlluminationModel.h>
#include <anatomist/surface/depthPeelingEffect.h>

using namespace anatomist;

dynamicShaderBuilder::dynamicShaderBuilder(): m_anatomistVersion(
  std::to_string(ANATOMIST_VERSION_MAJOR) + "." +
  std::to_string(ANATOMIST_VERSION_MINOR)
)
{
}

void dynamicShaderBuilder::setGLSLVersion(int version) 
{
  m_GLSLversion = version;
}

void dynamicShaderBuilder::setBaseTemplate(const std::string &templateSource)
{
  m_baseShaderTemplate = templateSource;
}


void dynamicShaderBuilder::setIlluminationModel(carto::rc_ptr<IShaderModule>  model)
{
  m_illuminationModel = model;
}

void dynamicShaderBuilder::addEffect(carto::rc_ptr<IShaderModule> effect)
{
  m_effects.push_back(effect);
}

std::string dynamicShaderBuilder::readShaderFile(const std::string &filePath)
{
  QFile file(QString::fromStdString(filePath));
  if (!file.exists()) {
    std::cout << "Fichier introuvable:" << filePath<< std::endl;
  }

  if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    std::cerr << "Error: Could not open shader file : " << filePath << std::endl;
  }

  QTextStream in(&file);
  std::string shaderSource = "#version " + std::to_string(m_GLSLversion) + " compatibility" + '\n';
  shaderSource += in.readAll().toStdString();
  file.close();

  return shaderSource;
}

std::string dynamicShaderBuilder::generateShaderSource() const
{
  std::string shaderSource = m_baseShaderTemplate;
  
  // Replace the illumination model
  std::string illuminationModelUniforms;
  std::string illuminationModelFunctions;
  std::string illuminationModelCall;

  if(!m_illuminationModel)
  {
    illuminationModelUniforms = "\n";
    illuminationModelFunctions = "\n";
    illuminationModelCall = "\n";
  }
  else
  {
    // Uniforms
    illuminationModelUniforms = m_illuminationModel->getUniformDeclarations() + "\n";
    // Functions
    illuminationModelFunctions = m_illuminationModel->getFunctionImplementation() + "\n";
    // Calling function
    illuminationModelCall = m_illuminationModel->getFunctionCall() + "\n";
  }

  shaderSource = std::regex_replace(
    shaderSource,
    std::regex("\\{Illumination Model Uniforms\\}"),
    illuminationModelUniforms
  );

  shaderSource = std::regex_replace(
    shaderSource,
    std::regex("\\{Illumination Model Functions\\}"),
    illuminationModelFunctions
  );

  shaderSource = std::regex_replace(
    shaderSource,
    std::regex("\\{Illumination Model Call\\}"),
    illuminationModelCall
  );

  // Replace the effects
  std::string effectUniforms;
  std::string effectFunctions;
  std::string effectCalls;

  if(m_effects.empty())
  {
    effectUniforms = "\n";
    effectFunctions = "\n";
    effectCalls = "\n";
  }
  else
  {
    for(const auto &effect : m_effects)
    {
      // Uniforms
      effectUniforms += effect->getUniformDeclarations() + "\n";
      // Functions
      effectFunctions += effect->getFunctionImplementation() + "\n";
      // Calling function
      effectCalls += effect->getFunctionCall() + "\n";
    }
  }


  shaderSource = std::regex_replace(
    shaderSource,
    std::regex("\\{Effect Uniforms\\}"),
    effectUniforms
  );

  shaderSource = std::regex_replace(
    shaderSource,
    std::regex("\\{Effect Functions\\}"),
    effectFunctions
  );

  shaderSource = std::regex_replace(
    shaderSource,
    std::regex("\\{Effect Call\\}"),
    effectCalls
  );

  return shaderSource;
}

carto::rc_ptr<QOpenGLShaderProgram> dynamicShaderBuilder::initShader(const std::string shaderIDs, std::string vsTemplate, std::string fsTemplate)
{
  carto::rc_ptr<QOpenGLShaderProgram> program(new QOpenGLShaderProgram());
  std::string baseTemplate, vertexSource, fragmentSource;
  std::vector<carto::rc_ptr<IShaderModule>> shaderModules = shaderMapping::getModules(shaderIDs);
  std::list<std::string> path =  carto::Paths::findResourceFiles("shaders/templates", "anatomist", m_anatomistVersion);
  if (path.empty()) {
    std::cerr << "Error : No template shader found in shaders/templates." << std::endl;
    return carto::rc_ptr<QOpenGLShaderProgram>();
  }

  program->create();
  this->setGLSLVersion(330);

  // vertex shader
  baseTemplate =  readShaderFile(path.front()+"/"+vsTemplate);
  this->setBaseTemplate(baseTemplate);
  vertexSource = this->generateShaderSource();
  if(!program->addShaderFromSourceCode(QOpenGLShader::Vertex, QString::fromStdString(vertexSource)))
  {
    std::cout << "Vertex shader error : " << program->log().toStdString() << std::endl;
  }
  // fragment shader
  baseTemplate = readShaderFile(path.front()+"/"+fsTemplate);
  this->setBaseTemplate(baseTemplate);
  bool hasIlluminationModel = false;
  for(size_t i=0; i<shaderModules.size(); ++i)
  {
    if(shaderModules[i]->isIlluminationModel())
    {
      if(hasIlluminationModel)
      {
        std::cerr << "Error : Multiple illumination models found in shader modules." << std::endl;
        return carto::rc_ptr<QOpenGLShaderProgram>();
      }
      hasIlluminationModel = true;
      this->setIlluminationModel(shaderModules[i]);
    }
    else
    {
      this->addEffect(shaderModules[i]);
    }

  } 
  fragmentSource = this->generateShaderSource();
  if(!program->addShaderFromSourceCode(QOpenGLShader::Fragment, QString::fromStdString(fragmentSource)))
  {
    std::cerr << "Fragment shader error : " << program->log().toStdString() << std::endl;
  }

  if(!program->link())
  {
    std::cerr << "Shader linkage error : " << program->log().toStdString() << std::endl; 
  }
  return program;
}

carto::rc_ptr<QOpenGLShaderProgram> dynamicShaderBuilder::initBlendingShader()
{
  carto::rc_ptr<QOpenGLShaderProgram> program(new QOpenGLShaderProgram());
  std::string vertexSource, fragmentSource;
  
  std::list<std::string> path =  carto::Paths::findResourceFiles("shaders/templates", "anatomist", m_anatomistVersion);
  if (path.empty()) {
    std::cerr << "Error : No template shader found in shaders/templates." << std::endl;
    return carto::rc_ptr<QOpenGLShaderProgram>();
  }

  program->create();
  this->setGLSLVersion(330);
  vertexSource = readShaderFile(path.front()+"/blend.vs.glsl");

  if(!program->addShaderFromSourceCode(QOpenGLShader::Vertex, QString::fromStdString(vertexSource)))
  {
    std::cerr << "Vertex shader error : " << program->log().toStdString() << std::endl;
  }

  fragmentSource = readShaderFile(path.front()+"/blend.fs.glsl");
  if(!program->addShaderFromSourceCode(QOpenGLShader::Fragment, QString::fromStdString(fragmentSource)))
  {
    std::cout << "Fragment shader error : " << program->log().toStdString() << std::endl;
  }

  if(!program->link())
  {
    std::cerr << "Shader linkage error : " << program->log().toStdString() << std::endl; 
  }
  
  return program;
}
