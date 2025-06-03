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
  if (!file.exists()) {
    std::cout << "Fichier introuvable:" << filePath<< std::endl;
  }

  if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    std::cerr << "Error: Could not open shader file : " << filePath << std::endl;
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

std::shared_ptr<QOpenGLShaderProgram> dynamicShaderBuilder::initShader(const std::string shaderIDs, std::string vsTemplate, std::string fsTemplate)
{
  auto program= std::make_shared<QOpenGLShaderProgram>();
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
  this->setVersion(150);

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
  this->setIlluminationModel(shaderModules[0]);
  bool hasIlluminationMoel = false;
  for(size_t i=0; i<shaderModules.size(); ++i)
  {
    if(shaderModules[i]->isIlluminationModel())
    {
      if(hasIlluminationMoel)
      {
        std::cerr << "Error : Multiple illumination models found in shader modules." << std::endl;
        return nullptr;
      }
      hasIlluminationMoel = true;
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

std::shared_ptr<QOpenGLShaderProgram> dynamicShaderBuilder::initBlendingShader()
{
  auto program = std::make_shared<QOpenGLShaderProgram>();
  std::string vertexSource, fragmentSource;
  
  std::list<std::string> path =  carto::Paths::findResourceFiles("shaders/templates", "anatomist");
  if (path.empty()) {
    std::cerr << "Error : No template shader found in shaders/templates." << std::endl;
    return nullptr;
  }

  program->create();
  this->setVersion(150);
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