#ifndef ANA_SURFACE_FUSIONTEXBUCKET_H
#define ANA_SURFACE_FUSIONTEXBUCKET_H

#include <anatomist/fusion/fusionFactory.h>


namespace anatomist
{

  class FusionTexBucketMethod : public FusionMethod
  {
  public:
    FusionTexBucketMethod() : FusionMethod() {}
    virtual ~FusionTexBucketMethod() {}

    virtual int canFusion( const std::set<AObject *> & obj );
    virtual AObject* fusion( const std::vector<AObject *> & obj );
    ///	identifier for the method
    virtual std::string ID() const;
    virtual std::string generatedObjectType() const;
  };

}


#endif
