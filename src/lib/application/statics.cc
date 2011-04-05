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

/*	Here I put a bunch of static variables, which may be initialized in 
	the correct order. I'm a bit fed up of bus errors due to wrong 
	ordering in initializations, depending on linkers and dynamic loaders
*/

//	AWindowFactory

#include <anatomist/window/winFactory.h>

using namespace anatomist;
using namespace std;

namespace anatomist
{
  class StaticInitializers
  {
    public:
      static bool init();
      static void cleanup();
  };
}

map<int, string>			AWindowFactory::TypeNames;
map<string, int>			AWindowFactory::TypeID;
map<int, carto::rc_ptr<AWindowCreator> >	AWindowFactory::Creators;

static bool	AWindowFactory_initialized = AWindowFactory::initTypes();

//	Commands Registry

#include <anatomist/processor/Registry.h>

Registry* Registry::_instance = 0;

//	AObject

#include <anatomist/object/Object.h>

map<string, int>	AObject::_objectTypes;
#ifndef _WIN32
/* Denis (2003/07)
   Well, on Win32-MinWG there is an assembler bug which causes
   a symbol (_D) to be defined twice if we declare both AObjects static 
   variables here. Strange, isnt't it ?
   Hopefully I figured out that declaring the other variable somewhere else in 
   the source solved the problem (!), so we'll do it a bit later...
*/
map<int,string> 	AObject::_objectTypeNames;
#endif


// second part of the MinGW assembler bug workaround, see earlier
#ifdef _WIN32
map<int,string> 	AObject::_objectTypeNames;
#endif

//	QObjectBrowser

#include <anatomist/browser/qwObjectBrowser.h>

//	Hierarchy

#include <anatomist/hierarchy/hierarchy.h>

int 		Hierarchy::_classType = Hierarchy::registerClass();


bool StaticInitializers::init()
{
  QObjectBrowser::registerClass();
  return true;
}


#include <anatomist/controler/controlmanager.h>
#include <anatomist/controler/controlgroupdictionary.h>
#include <anatomist/controler/controldictionary.h>
#include <anatomist/controler/actiondictionary.h>
#include <anatomist/controler/icondictionary.h>


void StaticInitializers::cleanup()
{
  AWindowFactory::TypeNames.clear();
  AWindowFactory::TypeID.clear();
  AWindowFactory::Creators.clear();
  
  AObject::_objectTypes.clear();
  AObject::_objectTypeNames.clear();
  AObject::_objectmenu_map.clear();
  
  delete ControlManager::instance();
  delete ControlGroupDictionary::instance();
  delete ControlDictionary::instance();
  delete ActionDictionary::instance();
  delete IconDictionary::instance();
  
  delete Registry::instance();
}

// run

namespace
{
  bool dummy = StaticInitializers::init();
}

