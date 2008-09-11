/*
 *  Copyright (C) 1998-2005 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 */

#ifndef ANAFOLD_FGRAPH_AFGHELPERS_H
#define ANAFOLD_FGRAPH_AFGHELPERS_H


#include <cartobase/object/attributed.h>
#include <aims/qtcompat/qlistview.h>

class QObjectBrowserWidget;

namespace anatomist
{
  class AttDescr;
  class AObject;

  ///	Helpers for AttDescr, to describe model graphs
  class AFGHelpers
  {
  public:
    static void init();

    static void printCliqueSet( QObjectBrowserWidget*, 
				const carto::GenericObject &, 
				const std::string &, std::string & );
    static void printDomain( QObjectBrowserWidget*, 
			     const carto::GenericObject &, 
			     const std::string &, Q3ListViewItem *, 
			     const anatomist::AttDescr*, 
			     bool );
    static void printModel( QObjectBrowserWidget*, 
			    const carto::GenericObject &, 
			    const std::string &, Q3ListViewItem *, 
			    const anatomist::AttDescr*, 
			    bool);
    static void describeAFGraph( QObjectBrowserWidget*, 
				 AObject* obj, 
				 Q3ListViewItem * );
  };

}


#endif


