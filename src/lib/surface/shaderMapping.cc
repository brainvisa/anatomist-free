#include "shaderMapping.h"
#include <anatomist/surface/shaderMapping.h>

#include <anatomist/surface/blinnPhongIlluminationModel.h>
#include <anatomist/surface/depthPeelingEffect.h>

namespace anatomist
{
    // Initialize the shader module registry
  std::unordered_map<std::string, std::vector<std::shared_ptr<IShaderModule>>> shaderMapping::moduleRegistry;

  void shaderMapping::registerModule(const std::string& id, const std::vector<std::shared_ptr<IShaderModule>>& module)
  {
    moduleRegistry[id] = module;
  }

  std::vector<std::shared_ptr<IShaderModule>> shaderMapping::getModules(const std::string& id)
  {
    auto it = moduleRegistry.find(id);
    return (it != moduleRegistry.end()) ? it->second : std::vector<std::shared_ptr<IShaderModule>>{};
  }

  void shaderMapping::printModules()
  {
      for(auto it=moduleRegistry.begin(); it != moduleRegistry.end(); ++it)
      {
        std::cout << "Modules associés à l'ID " << it->first << " : ";
        for(const auto& module : it->second)
        {
          module->printModule();
        }
        std::cout << std::endl;
      }
  }



  void shaderMapping::initShaderMapping()
  {
    auto blinnPhong = std::make_shared<BlinnPhongIlluminationModel>();
    auto depthPeeling = std::make_shared<DepthPeelingEffect>();

    shaderMapping::registerModule(blinnPhong->getID(), {blinnPhong});
    shaderMapping::registerModule(depthPeeling->getID(), {depthPeeling});
    std::string blinnPhongAndDepthPeeling = blinnPhong->getID() + depthPeeling->getID();
    shaderMapping::registerModule(blinnPhongAndDepthPeeling, {blinnPhong, depthPeeling});
  }
}

