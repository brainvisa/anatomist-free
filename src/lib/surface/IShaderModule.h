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

#include <string>
#include <QOpenGLShaderProgram>
//#include "glcomponent.h"
#include <anatomist/window3D/window3D.h>



namespace anatomist
{
  class GLComponent;
  //class AWindow3D;

  class IShaderModule
  {
    /** This class is the base interface for creating a new module for the shader
    * program. The module is responsible for creating the specific uniforms declaration,
    * the function implementation and the function call for the shader program.
    */


    public:
      /// Returns the uniform declarations for the shader module
      virtual std::string getUniformDeclarations() const = 0;
      /// Returns the function implementation for the shader module
      virtual std::string getFunctionImplementation() const = 0;
      /// Returns the function call module
      virtual std::string getFunctionCall() const = 0;
      virtual ~IShaderModule() = default;

      /// Sets up the uniforms for the shader module
      virtual void setupObjectUniforms(QOpenGLShaderProgram& program, GLComponent& obj) const = 0;
      virtual void setupSceneUniforms(QOpenGLShaderProgram& program, AWindow3D& scene) const  = 0;

      /// return the list of shader's id
      virtual std::string getID() const {return _id;}

      virtual void printModule() const{std::cout << _name<< std::endl;};

      virtual bool isIlluminationModel() const {return _isIlluminationModel;}
    
    protected:
      /// unique id associated to each shader
      std::string _id;
      bool _isIlluminationModel;
      std::string _name;
  };
}



#endif // ISHADER_MODULE_H