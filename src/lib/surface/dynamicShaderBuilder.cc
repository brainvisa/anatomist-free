#include <QFile>
#include <QTextStream>
#include <QString>
#include <cstring>
#include <string>
#include <iostream>
#include <memory>
#include <regex>
#include <list>

#include <anatomist/surface/dynamicShaderBuilder.h>
#include <anatomist/surface/shaderMapping.h>
#include <cartobase/config/paths.h>
#include <anatomist/surface/blinnPhongIlluminationModel.h>
#include <anatomist/surface/depthPeelingEffect.h>

using namespace anatomist;

dynamicShaderBuilder::dynamicShaderBuilder()
{
}

void dynamicShaderBuilder::setVersion(int version)
{
  m_version = version;
}

void dynamicShaderBuilder::setBaseTemplate(const std::string &templateSource)
{
  m_baseShaderTemplate = templateSource;
}


void dynamicShaderBuilder::setIlluminationModel(std::shared_ptr<IShaderModule>  model)
{
  m_illuminationModel = model;
}

void dynamicShaderBuilder::addEffect(std::shared_ptr<IShaderModule> effect)
{
  m_effects.push_back(effect);
}

std::string dynamicShaderBuilder::readShaderFile(const std::string &filePath)
{
  QFile file(QString::fromStdString(filePath));
  if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    throw std::runtime_error("Error: Could not open shader file");
  }
  
  QTextStream in(&file);
  std::string shaderSource = "#version " + std::to_string(m_version) + " compatibility" + '\n';
  shaderSource += in.readAll().toStdString();
  file.close();

  return shaderSource;
}

std::string dynamicShaderBuilder::generateShaderSource() const
{
  std::string shaderSource = m_baseShaderTemplate;

  // Replace the illumination model
  if(m_illuminationModel)
  {
    // Uniforms
    shaderSource = std::regex_replace(
      shaderSource,
      std::regex("\\{Illumination Model Uniforms\\}"),
      m_illuminationModel->getUniformDeclarations()
    );

    // Functions
    shaderSource = std::regex_replace(
      shaderSource,
      std::regex("\\{Illumination Model Functions\\}"),
      m_illuminationModel->getFunctionImplementation()
    );

    // Calling function
    shaderSource = std::regex_replace(
      shaderSource,
      std::regex("\\{Illumination Model Call\\}"),
      m_illuminationModel->getFunctionCall()
    );
  }

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

std::unique_ptr<QOpenGLShaderProgram> dynamicShaderBuilder::initShader(const std::string shaderIDs)
{
  auto program= std::make_unique<QOpenGLShaderProgram>();
  std::string baseTemplate, vertexSource, fragmentSource;
  std::vector<std::shared_ptr<IShaderModule>> shaderModules = shaderMapping::getModules(shaderIDs);
  if (shaderModules.empty()) {
    std::cerr << "Error : no module found for ID " << shaderIDs << std::endl;
    return nullptr;
  
  }
  std::list<std::string> path =  carto::Paths::findResourceFiles("shaders/templates", "anatomist");
  if (path.empty()) {
    std::cerr << "Error : No template shader found in shaders/templates." << std::endl;
    return nullptr;
  }

  program->create();
  this->setVersion(330);

  // vertex shader
  baseTemplate =  readShaderFile(path.front()+"/main.vs.glsl" );
  this->setBaseTemplate(baseTemplate);
  vertexSource = this->generateShaderSource();
  if(!program->addShaderFromSourceCode(QOpenGLShader::Vertex, QString::fromStdString(vertexSource)))
  {
    std::cout << "Vertex shader error : " << program->log().toStdString() << std::endl;
  }


  // fragment shader
  baseTemplate = readShaderFile(path.front()+"/main.fs.glsl");
  this->setBaseTemplate(baseTemplate);
  this->setIlluminationModel(shaderModules[0]);
  for(size_t i=1; i<shaderModules.size(); ++i) //start at 1 because 0 is the illumination model
  {
    this->addEffect(shaderModules[i]); // might look for special effects that needs other shaders (depth peeling)
  } 
  fragmentSource = this->generateShaderSource();
  if(!program->addShaderFromSourceCode(QOpenGLShader::Fragment, QString::fromStdString(fragmentSource)))
  {
    std::cout << "Fragment shader error : " << program->log().toStdString() << std::endl;
  }

  if(!program->link())
  {
    std::cout << "Shader linkage error : " << program->log().toStdString() << std::endl; 
  }
  return program;
}