/*
 *  Copyright (C) 2000-2004 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 *
 */

#ifndef ANAFOLD_FGRAPH_FGMETHOD_H
#define ANAFOLD_FGRAPH_FGMETHOD_H


#include <anatomist/fusion/fusionFactory.h>


namespace anatomist
{

  ///	Method to fusion graphs
  class FGraphMethod : public FusionMethod
  {
  public:
    FGraphMethod() {}
    virtual ~FGraphMethod();

    virtual bool canFusion( const std::set<AObject *> & );
    virtual AObject* fusion( const std::vector<AObject *> & );
    virtual std::string ID() const;
  };

}

#endif


