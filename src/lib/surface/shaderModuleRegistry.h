#ifndef ANATOMIST_SURFACE_SHADERMODULEREGISTRY_H
#define ANATOMIST_SURFACE_SHADERMODULEREGISTRY_H

#include <anatomist/surface/shaderTypes.h>
#include <anatomist/surface/IShaderModule.h>
#include <cartobase/smart/rcptr.h>
#include <map>
#include <set>
#include <vector>

namespace anatomist
{
  /**
  * \class ShaderModuleRegistry
  * \brief Application-wide registry of shader modules.
  *
  * \details
  * Holds one instance of each shader module (IShaderModule), created on
  * first use and shared by all shader programs and all windows.
  *
  * Module combinations are not registered: resolve() builds the list of
  * modules of a program on demand, from a set of ShaderModuleId.
  *
  * To add a module, see ShaderModuleId.
  */
  class ShaderModuleRegistry
  {
  public:
    /**
    * \brief Returns the module instances matching a set of identifiers.
    *
    * \details
    * Modules are returned in ShaderModuleId order. This order does not
    * matter for program generation: dynamicShaderBuilder separates the
    * illumination model from the effects itself.
    * Unknown identifiers are reported with a warning and skipped.
    *
    * \param ids Identifiers of the requested modules.
    * \return The matching module instances.
    */
    static std::vector<carto::rc_ptr<IShaderModule> >
    resolve( const std::set<ShaderModuleId> & ids );

    /**
    * \brief Prints all registered modules (debug).
    */
    static void printModules();

  private:
    /**
    * \brief Table of registered modules, built on first call.
    *
    * The table is a function-local static, so its initialization is
    * thread-safe and happens exactly once. New modules are registered
    * in its implementation.
    */
    static std::map<ShaderModuleId, carto::rc_ptr<IShaderModule> > & modules();
  };
}

#endif