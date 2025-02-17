#include <QFile>
#include <QTextStream>
#include <QString>
#include <string>
#include <iostream>
#include <memory>
#include <regex>

#include "dynamicShaderBuilder.h"

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


void dynamicShaderBuilder::setIlluminationModel(std::unique_ptr<IShaderModule>  model)
{
  m_illuminationModel = std::move(model);
}

void dynamicShaderBuilder::addEffect(std::unique_ptr<IShaderModule> effect)
{
  m_effects.push_back(std::move(effect));
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