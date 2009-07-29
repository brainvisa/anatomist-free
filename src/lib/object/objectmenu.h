/* Copyright (c) 2006 CEA
 *
 *  This software and supporting documentation were developed by
 *      CEA/DSV/SHFJ
 *      4 place du General Leclerc
 *      91401 Orsay cedex
 *      France
 *
 * This software is governed by the CeCILL license version 2 under
 * French law and abiding by the rules of distribution of free software.
 * You can  use, modify and/or redistribute the software under the
 * terms of the CeCILL license version 2 as circulated by CEA, CNRS
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
 * knowledge of the CeCILL license version 2 and that you accept its terms.
 */


#ifndef ANA_OBJECT_OBJECTMENU_H
#define ANA_OBJECT_OBJECTMENU_H

#include <cartobase/smart/rcptr.h>
#include <cartobase/object/object.h>
#include <string>
#include <vector>
#include <set>

class Tree;

namespace anatomist
{

  class AObject;

  class ObjectMenuCallback : public carto::RCObject
  {
  public:
    ObjectMenuCallback();
    virtual ~ObjectMenuCallback();

    /** executes the callback
        \param obj objects it was called on
    */
    virtual void doit( const std::set<AObject *> & obj ) = 0;
    virtual bool operator == ( const ObjectMenuCallback & ) const;
  };


  class ObjectMenuCallbackFunc : public ObjectMenuCallback
  {
  public:
    typedef void (*CallbackFunc)( const std::set<AObject *> & );
    ObjectMenuCallbackFunc( CallbackFunc cbk );
    virtual ~ObjectMenuCallbackFunc();

    virtual void doit( const std::set<AObject *> & obj );
    virtual bool operator == ( const ObjectMenuCallback & ) const;

  private:
    CallbackFunc  _func;
  };


  class ObjectMenu : public carto::RCObject
  {
  public:
    ObjectMenu();
    ObjectMenu( const ObjectMenu & );
    ObjectMenu( Tree & t);
    ~ObjectMenu();

    ObjectMenu & operator = ( const ObjectMenu & );

    /// inserts a sub-menu
    void insertItem( const std::vector<std::string> & inside,
                     const std::string & text );
    /** inserts a terminal menu (with callback).
        The ObjectMenu takes ownership of the callback, even if it fails to
        insert it in the menu tree.
    */
    void insertItem( const std::vector<std::string> & inside,
                     const std::string & text, ObjectMenuCallback* cbk );
    /// inserts a terminal menu (with callback as a function)
    void insertItem( const std::vector<std::string> & inside,
                     const std::string & text,
                     ObjectMenuCallbackFunc::CallbackFunc cbk );
    Tree* item( const std::vector<std::string> & pos,
                bool createSubtrees = true );
    Tree* tree();
    const Tree* tree() const;
    /// returns the internal tree and release ownership on it
    Tree* releaseTree();

  private:
    struct Private;
    Private *d;
  };


  class ObjectMenuRegistrerClass : public carto::RCObject
  {
  public:
    ObjectMenuRegistrerClass();
    virtual ~ObjectMenuRegistrerClass();

    virtual ObjectMenu* doit( const AObject*, ObjectMenu* ) = 0;
  };


  class ObjectMenuRegistrerFuncClass : public ObjectMenuRegistrerClass
  {
  public:
    typedef ObjectMenu* (*RegistrerFunc)( const AObject*, ObjectMenu* );
    ObjectMenuRegistrerFuncClass( RegistrerFunc );
    virtual ~ObjectMenuRegistrerFuncClass();

    virtual ObjectMenu* doit( const AObject*, ObjectMenu* );

  private:
    RegistrerFunc _f;
  };

}

namespace carto
{
DECLARE_GENERIC_OBJECT_TYPE( carto::rc_ptr<anatomist::ObjectMenuCallback> )
}
#endif

