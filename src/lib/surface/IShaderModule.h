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


#ifndef ISHADER_MODULE_H
#define ISHADER_MODULE_H

#include <iostream>
#include <string>
#include <QOpenGLShaderProgram>
//#include <anatomist/window/glwidgetmanager.h>
#include <cartobase/smart/rcobject.h>



namespace anatomist
{
  class GLComponent;
  class GLWidgetManager;

  /**
   * @class IShaderModule
   * @brief Interface for modular shader components used in dynamic shader construction.
   *
   * The IShaderModule interface defines the contract that any shader module must implement
   * in order to be integrated into a dynamically built shader program.
   * 
   * Each module provides:
   * - GLSL uniform declarations
   * - Function implementations
   * - Function calls
   *
   * This modular design enables the combination of multiple shader features
   * (such as illumination models or post-processing effects) into a single
   * generated shader at runtime.
   *
   * Implementations of this interface can represent illumination models
   * (e.g., Blinn–Phong, Lambertian) or visual effects (e.g., depth peeling, transparency).
   */
  class IShaderModule : public carto::RCObject
  {

    public:
      virtual ~IShaderModule() = default;

      /// Returns the module unique string identifier.
      virtual std::string getID() const {return _id;}

      /// Returns true if this module defines an illumination model.
      virtual bool isIlluminationModel() const {return _isIlluminationModel;}

      /// return the human-readable name of the module
      virtual void printModule() const{std::cout << _name<< std::endl;};
      


      /**
      * Returns GLSL uniform declarations required by this module.
      *
      * The returned string is expected to be a *raw string literal* (R"(...)" syntax),
      * so it can contain multi-line GLSL code directly.
      * Example:
      * \code
      * return R"(
      * uniform vec4 u_materialDiffuse;
      * uniform vec4 u_materialSpecular;
      * uniform float u_materialShininess;
      * )";
      * \endcode
      *
      * See the implementation of \c BlinnPhongIlluminationModel for reference.
      */
      virtual std::string getUniformDeclarations() const = 0;

      /**
      * Returns the GLSL function implementation for this module.
      *
      * This function defines one or several GLSL functions that perform the
      * computation associated with the module. It should also be written as a raw
      * string literal.
      *
      * See \c blinnPhongIlluminationModel.cc for a concrete implementation.
      */
      virtual std::string getFunctionImplementation() const = 0;
      

      /**
      * Returns the GLSL code that applies this module effect to the current color.
      *
      * The returned snippet is appended inside the fragment shader \c main() 
      * function, and is expected to *combine its result with the variable* 
      * \c color, which carries the current fragment color. It should also be written as a raw
      * string literal.
      *
      * Example:
      * \code
      * return R"(
      * vec4 myModuleResult = myModuleFunction(...);
      * color = mix(color, myModuleResult, 0.5);
      * )";
      * \endcode
      */
      virtual std::string getFunctionCall() const = 0;
     

      /**
      * Sets per-object uniforms before drawing.
      * This method is typically used to set uniforms such as material properties.
      * Automatically called for each rendered object by \c GLObjectUniforms::callList().
      */
      virtual void setupObjectUniforms(QOpenGLShaderProgram& program, GLComponent& obj) const = 0;

      /**
      * Sets scene-level uniforms shared by all objects using the same program.
      *
      * This method is typically used to set lighting or camera-related parameters
      * (for example the viewer position or light direction).
      * Automatically called for each rendered object by \c GLSceneUniforms::callList().
      */
      virtual void setupSceneUniforms(QOpenGLShaderProgram& program, GLWidgetManager& scene) const  = 0;


    
    protected:
      /// unique string identifier of the module
      std::string _id;
      /// human-readable name of the module
      std::string _name;
      bool _isIlluminationModel;

  };
}



#endif // ISHADER_MODULE_H