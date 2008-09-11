
#ifndef ANAFOLD_MODULE_ANAFOLD_H
#define ANAFOLD_MODULE_ANAFOLD_H


#include <anatomist/application/module.h>


namespace anatomist
{

  class AnaFold : public Module
  {
  public:
    AnaFold();
    virtual ~AnaFold();

    virtual std::string name() const;
    virtual std::string description() const;

  protected:
    virtual void objectsDeclaration();
  };

}


#endif

