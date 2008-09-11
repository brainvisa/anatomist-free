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

#ifndef ANA_MOBJECT_GLOBJECTVECTOR_H
#define ANA_MOBJECT_GLOBJECTVECTOR_H

#include <anatomist/mobject/glmobject.h>
#include <anatomist/mobject/objectVector.h>

namespace anatomist
{

  class GLObjectVector : public GLMObject
  {
  public:
    GLObjectVector();
    virtual ~GLObjectVector();

    virtual int MType() const { return( AObject::VECTOR ); }

    /**@name	Container methods */
    //@{
    virtual size_t size() const;
    virtual iterator begin();
    virtual const_iterator begin() const;
    virtual iterator end();
    virtual const_iterator end() const;
    virtual void insert( AObject * );
    virtual void insert( const carto::shared_ptr<AObject> &,
                         int pos = -1 );
    virtual void insert( AObject *, int pos );
    virtual const_iterator find( const AObject * ) const;
    virtual const_iterator find( const carto::shared_ptr<AObject> & ) const;
    virtual void erase( iterator & );
    //@}
    virtual bool CanRemove( AObject *obj );

    virtual Tree* optionTree() const;
    static Tree*	_optionTree;

  protected:
    ///		Data storage type, to be redefined by children classes
    typedef std::list<carto::shared_ptr<AObject> > datatype;
    datatype	_data;
  };


  inline size_t GLObjectVector::size() const
  {
    return( _data.size() );
  }


  inline MObject::iterator GLObjectVector::begin()
  {
    return( iterator( new ObjectVectorIterator( _data.begin() ) ) );
  }


  inline MObject::const_iterator GLObjectVector::begin() const
  {
    return( const_iterator
	    ( new const_ObjectVectorIterator( _data.begin() ) ) );
  }


  inline MObject::iterator GLObjectVector::end()
  {
    return( iterator( new ObjectVectorIterator( _data.end() ) ) );
  }


  inline MObject::const_iterator GLObjectVector::end() const
  {
    return( const_iterator( new const_ObjectVectorIterator( _data.end() ) ) );
  }


  inline void GLObjectVector::insert( AObject * x )
  {
    _data.push_back( carto::shared_ptr<AObject>(
                     carto::shared_ptr<AObject>::WeakShared, x ) );
    _insertObject( x );
  }

}


#endif


