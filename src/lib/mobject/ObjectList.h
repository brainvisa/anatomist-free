/* Copyright (c) 1995-2005 CEA
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


#ifndef ANA_MOBJECT_OBJECTLIST_H
#define ANA_MOBJECT_OBJECTLIST_H

#include <anatomist/mobject/MObject.h>


namespace anatomist
{

  class ObjectList;
  class const_ObjectListIterator;
  class Referential;
  class Geometry;

  class ObjectListIterator : public BaseIterator
  {
  public:
    friend class ObjectList;
    friend class GLObjectList;
    typedef std::set<carto::shared_ptr<AObject> > datatype;
    ObjectListIterator( const datatype::iterator & );
    virtual ~ObjectListIterator();
    virtual BaseIterator* clone() const;
    virtual bool operator != ( const BaseIterator & ) const;
    virtual bool operator != ( const ObjectListIterator & ) const;
    virtual bool operator != ( const const_ObjectListIterator & ) const;
    virtual AObject* operator * () const;
    /// de-reference iterator to get the underlying smart pointer
    virtual carto::shared_ptr<AObject> smart() const;
    virtual ObjectListIterator & operator ++ ();
    virtual ObjectListIterator & operator -- ();

  protected:
    datatype::iterator	_dataIt;

  private:
  };


  class const_ObjectListIterator : public BaseIterator
  {
  public:
    friend class ObjectList;
    typedef std::set<carto::shared_ptr<AObject> > datatype;
    const_ObjectListIterator( const datatype::const_iterator & );
    virtual ~const_ObjectListIterator();
    virtual BaseIterator* clone() const;
    virtual bool operator != ( const BaseIterator & ) const;
    virtual bool operator != ( const ObjectListIterator & ) const;
    virtual bool operator != ( const const_ObjectListIterator & ) const;
    virtual AObject* operator * () const;
    /// de-reference iterator to get the underlying smart pointer
    virtual carto::shared_ptr<AObject> smart() const;
    virtual const_ObjectListIterator & operator ++ ();
    virtual const_ObjectListIterator & operator -- ();

  protected:
    datatype::const_iterator	_dataIt;

  private:
  };


  inline bool ObjectListIterator::operator != ( const BaseIterator & x ) const
  {
    return( x.operator != ( *this ) );
  }


  inline bool 
  const_ObjectListIterator::operator != ( const BaseIterator & x ) const
  {
    return( x.operator != ( *this ) );
  }


  inline bool 
  ObjectListIterator::operator != ( const ObjectListIterator & x ) const
  {
    return( _dataIt != x._dataIt );
  }


  inline bool 
  const_ObjectListIterator::operator != ( const const_ObjectListIterator & x ) 
    const
  {
    return( _dataIt != x._dataIt );
  }


  inline bool 
  ObjectListIterator::operator != ( const const_ObjectListIterator & ) const
  {
    return( true );
  }


  inline bool 
  const_ObjectListIterator::operator != ( const ObjectListIterator & ) const
  {
    return( true );
  }


  inline AObject * ObjectListIterator::operator * () const
  {
    return( _dataIt->get() );
  }


  inline AObject * const_ObjectListIterator::operator * () const
  {
    return( _dataIt->get() );
  }


  inline carto::shared_ptr<AObject> ObjectListIterator::smart() const
  {
    return( *_dataIt );
  }


  inline carto::shared_ptr<AObject> const_ObjectListIterator::smart() const
  {
    return( *_dataIt );
  }


  inline ObjectListIterator & ObjectListIterator::operator ++ ()
  {
    ++_dataIt;
    return( *this );
  }


  inline const_ObjectListIterator & const_ObjectListIterator::operator ++ ()
  {
    ++_dataIt;
    return( *this );
  }


  inline ObjectListIterator & ObjectListIterator::operator -- ()
  {
    --_dataIt;
    return( *this );
  }


  inline const_ObjectListIterator & const_ObjectListIterator::operator -- ()
  {
    --_dataIt;
    return( *this );
  }


  inline BaseIterator *ObjectListIterator::clone() const
  {
    return( new ObjectListIterator( *this ) );
  }


  inline BaseIterator *const_ObjectListIterator::clone() const
  {
    return( new const_ObjectListIterator( *this ) );
  }



  /**	Multi-object : AObject containing children objects
   */
  class ObjectList : public MObject
  {
  public:
    ObjectList();
    virtual ~ObjectList();

    /**@name	Identification handling functions */
    //@{
    virtual int MType() const { return( AObject::LIST ); }
    //@}

    /**@name	Container methods */
    //@{
    virtual size_t size() const;
    virtual iterator begin();
    virtual const_iterator begin() const;
    virtual iterator end();
    virtual const_iterator end() const;
    virtual void insert( AObject * );
    virtual void insert( const carto::shared_ptr<AObject> & );
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


  inline size_t ObjectList::size() const
  {
    return( _data.size() );
  }


  inline MObject::iterator ObjectList::begin()
  {
    return( iterator( new ObjectListIterator( _data.begin() ) ) );
  }


  inline MObject::const_iterator ObjectList::begin() const
  {
    return( const_iterator( new const_ObjectListIterator( _data.begin() ) ) );
  }


  inline MObject::iterator ObjectList::end()
  {
    return( iterator( new ObjectListIterator( _data.end() ) ) );
  }


  inline MObject::const_iterator ObjectList::end() const
  {
    return( const_iterator( new const_ObjectListIterator( _data.end() ) ) );
  }


  inline void ObjectList::insert( AObject * x )
  {
    _data.insert( carto::shared_ptr<AObject>(
                  carto::shared_ptr<AObject>::WeakShared, x ) );
    _insertObject( x );
  }


  inline void ObjectList::insert( const carto::shared_ptr<AObject> & x )
  {
    _data.insert( x );
    _insertObject( x.get() );
  }


  inline void ObjectList::erase( MObject::iterator & i )
  {
    //	ajouter un test du type d'iterateur avant le casting brutal
    datatype::iterator & it = ((ObjectListIterator&)i.subIterator())._dataIt;
    _eraseObject( it->get() );
    _data.erase( it );
  }

}


#endif
