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


#ifndef ANA_GRAPH_AGRAPHITERATOR_H
#define ANA_GRAPH_AGRAPHITERATOR_H


#include <anatomist/mobject/MObject.h>
#include <anatomist/graph/attribAObject.h>
#include <graph/graph/graph.h>


namespace anatomist
{

  class AGraph;
  class AGraphIterator;
  class const_AGraphIterator;

  //
  // class AGraphIterator
  //
  class AGraphIterator : public BaseIterator
  {
  public:
    friend class AGraph;
    friend class const_AGraphIterator;
    typedef std::set<carto::shared_ptr<AObject> > datatype;

    AGraphIterator( const datatype::iterator& thing );
    AGraphIterator( const AGraphIterator& other );
    virtual ~AGraphIterator();

    virtual BaseIterator* clone() const;
    virtual bool operator != ( const BaseIterator& other ) const;
    virtual bool operator != ( const AGraphIterator& other ) const;
    virtual bool operator != ( const const_AGraphIterator& other ) const;
    virtual AObject* operator * () const;
    virtual AGraphIterator& operator ++ ();
    virtual AGraphIterator& operator -- ();

  protected:
    datatype::iterator _dataIt;
  };


  //
  // class const_AGraphIterator
  //
  class const_AGraphIterator : public BaseIterator
  {
  public:
    friend class AGraph;
    friend class AGraphIterator;
    typedef std::set<carto::shared_ptr<AObject> > datatype;

    const_AGraphIterator( const datatype::const_iterator& thing );
    const_AGraphIterator( const const_AGraphIterator& other );
    virtual ~const_AGraphIterator();

    virtual BaseIterator* clone() const;
    virtual bool operator != ( const BaseIterator& other ) const;
    virtual bool operator != ( const const_AGraphIterator& other) const;
    virtual bool operator != ( const AGraphIterator& other) const;
    virtual AObject* operator * () const;
    virtual const_AGraphIterator& operator ++ ();
    virtual const_AGraphIterator& operator -- ();

  protected:
    datatype::const_iterator _dataIt;
  };


//
// AGraphIterator methods
//
inline
AGraphIterator::AGraphIterator
( const AGraphIterator::datatype::iterator& thing )
  : BaseIterator(), _dataIt( thing )
{
}


inline
AGraphIterator::AGraphIterator( const AGraphIterator& other )
  : BaseIterator(), _dataIt( other._dataIt )
{
}


inline
BaseIterator* AGraphIterator::clone() const
{
  return new AGraphIterator( *this );
}


inline
bool AGraphIterator::operator != 
( const BaseIterator& other ) const
{
  return other.operator != ( *this );
}


inline
bool AGraphIterator::operator != 
( const AGraphIterator& other ) const
{
  return _dataIt != other._dataIt;
}


inline
bool AGraphIterator::operator != 
( const const_AGraphIterator& other ) const
{
  return _dataIt != other._dataIt;
}


inline
AObject* AGraphIterator::operator * () const
{
  return _dataIt->get();
}


inline
AGraphIterator& AGraphIterator::operator ++ ()
{
  ++_dataIt;
  return *this;
}


inline
AGraphIterator& AGraphIterator::operator -- ()
{
  --_dataIt;
  return *this;
}


//
// const_AGraphIterator methods
//
inline
const_AGraphIterator::const_AGraphIterator
( const const_AGraphIterator::datatype::const_iterator& thing )
  : BaseIterator(), _dataIt( thing )
{
}


inline
const_AGraphIterator::const_AGraphIterator( const const_AGraphIterator& other )
  : BaseIterator(), _dataIt( other._dataIt )
{
}


inline
BaseIterator* const_AGraphIterator::clone() const
{
  return new const_AGraphIterator( *this );
}


inline
bool const_AGraphIterator::operator != 
( const BaseIterator& other ) const
{
  return other.operator != ( *this );
}


inline
bool const_AGraphIterator::operator != 
( const const_AGraphIterator& other ) const
{
  return _dataIt != other._dataIt;
}


inline
bool const_AGraphIterator::operator != 
( const AGraphIterator& other ) const
{
  return _dataIt != other._dataIt;
}


inline
AObject* const_AGraphIterator::operator * () const
{
  return _dataIt->get();
}


inline
const_AGraphIterator& const_AGraphIterator::operator ++ ()
{
  ++_dataIt;
  return *this;
}


inline
const_AGraphIterator& const_AGraphIterator::operator -- 
()
{
  --_dataIt;
  return *this;
}

}


#endif
