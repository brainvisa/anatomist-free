#include "shaderMapping.h"
#include <anatomist/surface/shaderMapping.h>

#include <anatomist/surface/blinnPhongIlluminationModel.h>
#include <anatomist/surface/depthPeelingEffect.h>

namespace anatomist
{
    // Initialize the shader module registry
  std::unordered_map<std::string, std::vector<carto::rc_ptr<IShaderModule>>> shaderMapping::moduleRegistry;

  void shaderMapping::registerModule(const std::string& id, const std::vector<carto::rc_ptr<IShaderModule>>& module)
  {
    moduleRegistry[id] = module;
  }

  std::vector<carto::rc_ptr<IShaderModule>> shaderMapping::getModules(const std::string& id)
  {
    auto it = moduleRegistry.find(id);
    return (it != moduleRegistry.end()) ? it->second : std::vector<carto::rc_ptr<IShaderModule>>{};
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
  auto blinnPhong   = carto::rc_ptr<anatomist::IShaderModule>(
  new BlinnPhongIlluminationModel );
auto depthPeeling = carto::rc_ptr<anatomist::IShaderModule>(
  new DepthPeelingEffect );

  shaderMapping::registerModule(blinnPhong->getID(),
    std::vector<carto::rc_ptr<anatomist::IShaderModule>>{ blinnPhong });

  shaderMapping::registerModule(depthPeeling->getID(),
    std::vector<carto::rc_ptr<anatomist::IShaderModule>>{ depthPeeling });

  std::string blinnPhongAndDepthPeeling = blinnPhong->getID() + depthPeeling->getID();

  shaderMapping::registerModule(blinnPhongAndDepthPeeling,
    std::vector<carto::rc_ptr<anatomist::IShaderModule>>{ blinnPhong, depthPeeling });
}
}

