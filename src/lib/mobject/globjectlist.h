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

#ifndef ANA_MOBJECT_GLOBJECTLIST_H
#define ANA_MOBJECT_GLOBJECTLIST_H

#include <anatomist/mobject/glmobject.h>
#include <anatomist/mobject/ObjectList.h>

namespace anatomist
{

  class GLObjectList : public GLMObject
  {
  public:
    GLObjectList();
    virtual ~GLObjectList();

    virtual int MType() const { return( AObject::LIST ); }

    /**@name	Container methods */
    //@{
    virtual size_t size() const;
    virtual iterator begin();
    virtual const_iterator begin() const;
    virtual iterator end();
    virtual const_iterator end() const;
    virtual void insert( AObject * );
    virtual void insert( const carto::shared_ptr<AObject> &,
                         unsigned pos = 0 );
    virtual void insert( AObject *, unsigned pos );
    virtual const_iterator find( const AObject * ) const;
    virtual void erase( iterator & );
    //@}
    virtual bool CanRemove( AObject *obj );

    virtual Tree* optionTree() const;
    static Tree*	_optionTree;

  protected:
    ///		Data storage type, to be redefined by children classes
    typedef std::set<carto::shared_ptr<AObject> > datatype;
    datatype	_data;
  };


  inline size_t GLObjectList::size() const
  {
    return( _data.size() );
  }


  inline MObject::iterator GLObjectList::begin()
  {
    return( iterator( new ObjectListIterator( _data.begin() ) ) );
  }


  inline MObject::const_iterator GLObjectList::begin() const
  {
    return( const_iterator
	    ( new const_ObjectListIterator( _data.begin() ) ) );
  }


  inline MObject::iterator GLObjectList::end()
  {
    return( iterator( new ObjectListIterator( _data.end() ) ) );
  }


  inline MObject::const_iterator GLObjectList::end() const
  {
    return( const_iterator( new const_ObjectListIterator( _data.end() ) ) );
  }


  inline void GLObjectList::insert( AObject * x )
  {
    _data.insert( carto::shared_ptr<AObject>(
                  carto::shared_ptr<AObject>::WeakShared, x ) );
    _insertObject( x );
  }

}


#endif

