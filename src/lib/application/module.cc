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


#include "module.h"
using anatomist::Module;
using anatomist::ModuleManager;
using anatomist::ModuleObserver;
using anatomist::ModuleInterface;

using namespace std;

ModuleManager* ModuleManager::my_manager = 0;


Module::Module()
{
  ModuleManager::instance()->insert( this );
}


Module::~Module()
{ 
  ModuleManager::instance()->remove( this );
}


void Module::init()
{
  objectsDeclaration() ;
  objectPropertiesDeclaration() ;
  viewsDeclaration() ;
  actionsDeclaration() ;
  controlsDeclaration() ;
  controlGroupsDeclaration() ;
}

void Module::objectsDeclaration() 
{}

void Module::objectPropertiesDeclaration() 
{}

void Module::viewsDeclaration()
{}

void Module::actionsDeclaration()
{}

void Module::controlsDeclaration()
{}

void Module::controlGroupsDeclaration()
{}


Tree* Module::controlWinOptions() const
{
  return 0;
}


ModuleManager::ModuleManager()
{
}


ModuleManager::~ModuleManager()
{
  iterator i, e = my_modules.end();
  while( !my_modules.empty() )
    delete my_modules.back();
  my_manager = 0;
}


ModuleManager* ModuleManager::instance()
{
  if( !my_manager )
    my_manager = new ModuleManager;
  return my_manager;
}


void ModuleManager::shutdown()
{
  delete my_manager;
}


