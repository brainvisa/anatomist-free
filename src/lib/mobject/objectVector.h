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


#ifndef ANA_MOBJECT_OBJECTVECTOR_H
#define ANA_MOBJECT_OBJECTVECTOR_H

#include <anatomist/mobject/MObject.h>

namespace anatomist
{

  class ObjectVector;
  class const_ObjectVectorIterator;
  class Referential;
  class Geometry;

  class ObjectVectorIterator : public BaseIterator
  {
  public:
    friend class ObjectVector;
    friend class GLObjectVector;
    typedef std::list<carto::shared_ptr<AObject> > datatype;
    ObjectVectorIterator( const datatype::iterator & );
    virtual ~ObjectVectorIterator();
    virtual BaseIterator* clone() const;
    virtual bool operator != ( const BaseIterator & ) const;
    virtual bool operator != ( const ObjectVectorIterator & ) const;
    virtual bool operator != ( const const_ObjectVectorIterator & ) const;
    virtual AObject* operator * () const;
    virtual carto::shared_ptr<AObject> smart() const;
    virtual ObjectVectorIterator & operator ++ ();
    virtual ObjectVectorIterator & operator -- ();

  protected:
    datatype::iterator	_dataIt;

  private:
  };


  class const_ObjectVectorIterator : public BaseIterator
  {
  public:
    friend class ObjectVector;
    friend class GLObjectVector;
    typedef std::list<carto::shared_ptr<AObject> > datatype;
    const_ObjectVectorIterator( const datatype::const_iterator & );
    virtual ~const_ObjectVectorIterator();
    virtual BaseIterator* clone() const;
    virtual bool operator != ( const BaseIterator & ) const;
    virtual bool operator != ( const ObjectVectorIterator & ) const;
    virtual bool operator != ( const const_ObjectVectorIterator & ) const;
    virtual AObject* operator * () const;
    virtual carto::shared_ptr<AObject> smart() const;
    virtual const_ObjectVectorIterator & operator ++ ();
    virtual const_ObjectVectorIterator & operator -- ();

  protected:
    datatype::const_iterator	_dataIt;

  private:
  };


  inline bool 
  ObjectVectorIterator::operator != ( const BaseIterator & x ) const
  {
    return( x.operator != ( *this ) );
  }


  inline bool 
  const_ObjectVectorIterator::operator != ( const BaseIterator & x ) const
  {
    return( x.operator != ( *this ) );
  }


  inline bool 
  ObjectVectorIterator::operator != ( const ObjectVectorIterator & x ) const
  {
    return( _dataIt != x._dataIt );
  }


  inline bool 
  const_ObjectVectorIterator::operator != ( const 
					    const_ObjectVectorIterator & x ) 
    const
  {
    return( _dataIt != x._dataIt );
  }


  inline bool 
  ObjectVectorIterator::operator != ( const const_ObjectVectorIterator & ) 
    const
  {
    return( true );
  }


  inline bool 
  const_ObjectVectorIterator::operator != ( const ObjectVectorIterator & ) 
    const
  {
    return( true );
  }


  inline AObject * ObjectVectorIterator::operator * () const
  {
    return( _dataIt->get() );
  }


  inline AObject * const_ObjectVectorIterator::operator * () const
  {
    return( _dataIt->get() );
  }


  inline carto::shared_ptr<AObject> ObjectVectorIterator::smart() const
  {
    return( *_dataIt );
  }


  inline carto::shared_ptr<AObject> const_ObjectVectorIterator::smart() const
  {
    return( *_dataIt );
  }


  inline ObjectVectorIterator & ObjectVectorIterator::operator ++ ()
  {
    ++_dataIt;
    return( *this );
  }


  inline const_ObjectVectorIterator & 
  const_ObjectVectorIterator::operator ++ ()
  {
    ++_dataIt;
    return( *this );
  }


  inline ObjectVectorIterator & ObjectVectorIterator::operator -- ()
  {
    --_dataIt;
    return( *this );
  }


  inline const_ObjectVectorIterator & 
  const_ObjectVectorIterator::operator -- ()
  {
    --_dataIt;
    return( *this );
  }


  inline BaseIterator *ObjectVectorIterator::clone() const
  {
    return( new ObjectVectorIterator( *this ) );
  }


  inline BaseIterator *const_ObjectVectorIterator::clone() const
  {
    return( new const_ObjectVectorIterator( *this ) );
  }



  /**	it's a list, in fact...
   */
  class ObjectVector : public MObject
  {
  public:
    ObjectVector();
    virtual ~ObjectVector();

    virtual int MType() const { return( AObject::VECTOR ); }

    /**@name	Container methods */
    //@{
    virtual size_t size() const;
    virtual iterator begin();
    virtual const_iterator begin() const;
    virtual iterator end();
    virtual const_iterator end() const;
    virtual void insert( AObject * );
    virtual void insert( AObject *, int pos );
    virtual void insert( const carto::shared_ptr<AObject> &, int pos = -1 );
    virtual const_iterator find( const AObject * ) const;
    virtual const_iterator find( const carto::shared_ptr<AObject> & ) const;
    virtual void erase( iterator & );
    //@}
    virtual bool CanRemove( AObject *obj );

    virtual Tree* optionTree() const;
    static Tree*	_optionTree;

  protected:
    typedef std::list<carto::shared_ptr<AObject> > datatype;
    datatype	_data;
  };


  inline size_t ObjectVector::size() const
  {
    return( _data.size() );
  }


  inline MObject::iterator ObjectVector::begin()
  {
    return( iterator( new ObjectVectorIterator( _data.begin() ) ) );
  }


  inline MObject::const_iterator ObjectVector::begin() const
  {
    return( const_iterator
	    ( new const_ObjectVectorIterator( _data.begin() ) ) );
  }


  inline MObject::iterator ObjectVector::end()
  {
    return( iterator( new ObjectVectorIterator( _data.end() ) ) );
  }


  inline MObject::const_iterator ObjectVector::end() const
  {
    return( const_iterator( new const_ObjectVectorIterator( _data.end() ) ) );
  }


  inline void ObjectVector::insert( AObject * x )
  {
    _data.push_back( carto::shared_ptr<AObject>(
                     carto::shared_ptr<AObject>::WeakShared, x ) );
    _insertObject( x );
  }

}


#endif
