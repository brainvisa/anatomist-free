#ifndef SHADERMAPPING_H
#define SHADERMAPPING_H

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>

#include <anatomist/surface/IShaderModule.h>


/**
 * @class shaderMapping
 * @brief Manages the registration and retrieval of shader modules.
 *
 * This class allows registering shader modules with a unique identifier
 * and retrieving them later. It uses a hash table to store lists of shader modules
 * as shared pointers.
 */

namespace anatomist
{
  class shaderMapping
  {
    public:
      /**
      * @brief Registry of shader modules.
      *
      * A hash table mapping unique string identifiers to lists of shared pointers
      * for shader modules.
      */
      static std::unordered_map<std::string, std::vector<carto::rc_ptr<IShaderModule>>> moduleRegistry;


      /**
      * @brief Registers a shader module.
      * 
      * Associates a unique identifier with a list of shader modules.
      * 
      * @param id Unique identifier of the shader module.
      * @param module List of shared pointers to the shader modules to be registered.
      */
      static void registerModule(const std::string& id, const std::vector<carto::rc_ptr<IShaderModule>>& module);


      /**
      * @brief Retrieves shader modules by their identifier.
      * 
      * Searches the registry for shader modules associated with the given identifier.
      * 
      * @param id Identifier of the shader modules to retrieve.
      * @return A vector of shared pointers to shader modules if found, otherwise an empty vector.
      */
      static std::vector<carto::rc_ptr<IShaderModule>> getModules(const std::string& id);

      /**
      * @brief Prints all registered shader modules.
      * 
      * Iterates through the registry and prints the shader modules associated 
      * with each identifier. If the registry is empty, it indicates that no modules
      * have been registered yet.
      */
      static void printModules();

      /**
      * @brief Initializes the shader module mapping.
      * 
      * This function is responsible for setting up predefined shader mappings.
      * It registers common shader module combinations so that they can be accessed
      * later using their corresponding identifiers.
      */
      static void initShaderMapping();
  };
}
#endif // SHADERMAPPING_H
