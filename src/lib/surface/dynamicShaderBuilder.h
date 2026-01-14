/* This software and supporting documentation are distributed by
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


#ifndef DYNAMIC_SHADER_BUILDER_H
#define DYNAMIC_SHADER_BUILDER_H

#include <string>
#include <vector>
#include <memory>
#include <QOpenGLShaderProgram>

#include "IShaderModule.h"

namespace anatomist
{
  /**
  * @class dynamicShaderBuilder
  * @brief Dynamically builds and compiles OpenGL shader programs.
  *
  * The dynamicShaderBuilder class allows the user to assemble shader programs
  * from modular components at runtime. It supports:
  * - Setting a shader version directive.
  * - Using a base template that defines the shader’s structure.
  * - Injecting an illumination model and multiple effects into the template.
  *
  * The builder replaces specific placeholder tags in the base shader template with
  * the corresponding uniform declarations, function implementations, and function calls
  * provided by the shader modules. Illumination models are inserted first, followed
  * by effects in the order they were added.
  *
  * Example of supported placeholders:
  * - `{Illumination Model Uniforms}`
  * - `{Illumination Model Functions}`
  * - `{Illumination Model Call}`
  * - `{Effect Uniforms}`
  * - `{Effect Functions}`
  * - `{Effect Call}`
  */
  class dynamicShaderBuilder
  {
    public:
      dynamicShaderBuilder();

      /**
      * @brief Sets the GLSL version to be used in the generated shader.
      * @param version GLSL version number (e.g., 150 for "#version 150 compatibility").
      */
      void setGLSLVersion(int version);

      /**
      * @brief Defines the base shader template as a string.
      * @param templateSource The shader template source code. 
      * This may contain placeholder tags that will be replaced when generating the final shader.
      */
      void setBaseTemplate(const std::string &templateSource);

      /**
      * @brief Sets the illumination model used by the shader.
      * @param model rc_ptr pointer to an illumination model implementing IShaderModule.
      */
      void setIlluminationModel(carto::rc_ptr<IShaderModule> model);

      /**
      * @brief Adds an effect module to the shader.
      * @param effect rc_ptr pointer to an effect implementing IShaderModule.
      * 
      * Effects are applied sequentially, in the order they were added.
      */
      void addEffect(carto::rc_ptr<IShaderModule> effect);

      /**
      * @brief Reads a shader template file and adds the GLSL version header.
      * @param filePath Path to the shader template file.
      * @return The complete shader source code with version header prepended.
      *
      * The version directive is automatically inserted at the top of the file.
      */
      std::string readShaderFile(const std::string &filePath);

      /**
      * @brief Generates the final shader source code.
      * @return The complete GLSL shader code with placeholders replaced by actual module content.
      *
      * This method replaces placeholders in the base template with the corresponding
      * code provided by the illumination model and effects.
      */
      std::string generateShaderSource() const;

      /**
      * @brief Creates and compiles a shader program from a set of shader modules.
      * @param shaderIDs Identifiers corresponding to the shader modules to use.
      * @param vsTemplate Name of the vertex shader template file present in /shader/template/
      * @param fsTemplate Name of the fragment shader template file present in /shader/template/
      * @return A rc_ptr pointer to the compiled QOpenGLShaderProgram.
      *
      * The method automatically loads the templates, generates the shader source
      * using the defined modules, and compiles and links the final OpenGL program.
      */
      carto::rc_ptr<QOpenGLShaderProgram> initShader(const std::string shaderIDs, std::string vsTemplate = "main.vs.glsl", std::string fsTemplate = "main.fs.glsl");
      
      /**
      * @brief Initializes the shader program used for depth-peeling blending.
      * @return A rc_ptr pointer to the compiled QOpenGLShaderProgram.
      *
      * Loads the blending vertex and fragment shaders (`blend.vs.glsl` and `blend.fs.glsl`),
      * compiles them, and links the resulting program.
      */
      carto::rc_ptr<QOpenGLShaderProgram> initBlendingShader();
    
    private:
      std::string m_baseShaderTemplate;
      carto::rc_ptr<IShaderModule> m_illuminationModel;
      std::vector<carto::rc_ptr<IShaderModule>> m_effects;
      int m_GLSLversion;
      std::string m_anatomistVersion;
  };
}

#endif // DYNAMIC_SHADER_BUILDER_H

