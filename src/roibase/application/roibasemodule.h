/* This software and supporting documentation are distributed by
 *     Institut Federatif de Recherche 49
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


#ifndef ROI_BASE_MODULE_H
#define ROI_BASE_MODULE_H

#include "anatomist/application/module.h"
#include <string>
#include <set>

namespace anatomist
{  
  class AObject ;
  class AGraph ;
  class AGraphObject ;

  class RoiBaseModule : public anatomist::Module
  {
  public:
    RoiBaseModule() ;
    virtual ~RoiBaseModule() ;
    
    virtual std::string name() const ;
    virtual std::string description() const ;

    static AGraph *newGraph( AObject* source, const std::string & name, 
			     const std::string & syntax );
    /**	\par nodup if true, doesn't duplicate a node with same name */
    static AGraphObject* newRegion( AGraph* o, const std::string & roiName, 
				    const std::string & syntax, bool withbck, 
				    AObject*& bck, bool nodup = false );
    static bool noop();

  protected:
    virtual void objectsDeclaration() ;
    virtual void objectPropertiesDeclaration() ;
    virtual void viewsDeclaration() ;
    virtual void actionsDeclaration() ;
    virtual void controlsDeclaration() ;
    virtual void controlGroupsDeclaration() ;   

  private:
    static void addVolumeRoiOptions( Tree* tr ) ;
    static void addGraphRoiOptions( Tree* tr ) ;  
    static void addGraphObjectRoiOptions( Tree* tr ) ;      
    static void createGraph( const std::set<AObject *> & obj ) ;
    static void setGraphName( const std::set<AObject *> & obj ) ;
    static void setGraphObjectName( const std::set<AObject *> & obj ) ;
    static void addRegion( const std::set<AObject *> & obj ) ;
    static void exportRegion( const std::set<AObject *> & obj ) ;
    
    static const char * askName( const std::string& type, 
				 const std::string& originalName = "" ) ;
  };  
}

#endif
