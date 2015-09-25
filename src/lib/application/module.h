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


#ifndef ANATOMIST_APPLICATION_MODULE_H
#define ANATOMIST_APPLICATION_MODULE_H


#include <anatomist/config/anatomist_config.h>
#include <list>
#include <string>


class Tree;

namespace anatomist
{

  class ANATOMIST_API Module
  {
  public:
    Module();
    virtual ~Module() = 0;
    void init();

    virtual std::string name() const = 0;
    virtual std::string description() const = 0;

    virtual Tree* controlWinOptions() const;

  protected:
    virtual void objectsDeclaration();
    virtual void objectPropertiesDeclaration();
    virtual void viewsDeclaration();
    virtual void actionsDeclaration();
    virtual void controlsDeclaration();
    virtual void controlGroupsDeclaration();
  };

  class ANATOMIST_API ModuleManager
  {
    friend class Module;

  public:
    typedef std::list<Module*>::iterator iterator;
    typedef std::list<Module*>::const_iterator const_iterator;
    typedef std::list<Module*>::reverse_iterator reverse_iterator;
    typedef std::list<Module*>::const_reverse_iterator const_reverse_iterator;
    typedef std::list<Module*>::size_type size_type;
    ~ModuleManager();
    iterator begin() { return my_modules.begin(); }
    iterator end() { return my_modules.end(); }
    reverse_iterator rbegin() { return my_modules.rbegin(); } 
    reverse_iterator rend() { return my_modules.rend(); }
    bool empty() const { return my_modules.empty(); }
    size_type size() const { return my_modules.size(); }
    static ModuleManager* instance();
    /// delete the module manager
    static void shutdown();

  protected:
    void insert( Module* m ) { my_modules.push_back( m ); }
    void remove( Module* m ) { my_modules.remove( m ); }

  private:
    ModuleManager();

    std::list<Module*> my_modules;
    static ModuleManager* my_manager;
  };

  class ANATOMIST_API ModuleObserver
  {
  public:
  };

  class ANATOMIST_API ModuleInterface
  {
  public:
  };


}


/*!
  \class anatomist::Module module.h "anatomist/application/module.h"
  \brief Base class for dynamically loaded modules

  Module should be the standard way to implement modules for Anatomist.
  Just inherit Module and override standard methods.

  \sa anatomist::ModuleManager anatomist::ModuleInterface
*/


/*!
  \class anatomist::ModuleManager module.h "anatomist/application/module.h"
  \brief Manage Anatomist modules

  The ModuleManager maintains a list of the Module%s loaded by the
  application.

  \sa anatomist::Module anatomist::ModuleObserver
*/


/*!
  \class anatomist::ModuleObserver module.h "anatomist/application/module.h"
  \brief Base class for objects interested in module changes notification.

  Inherit ModuleObserver to stay informed of any change in modules.

  \sa anatomist::ModuleManager
*/


/*!
  \class anatomist::ModuleInterface module.h "anatomist/application/module.h"
  \brief Represent the Anatomist internals made available to Module%s

  \sa anatomist::Module
*/


#endif
