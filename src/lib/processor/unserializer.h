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


#ifndef ANA_PROCESSOR_UNSERIALIZER_H
#define ANA_PROCESSOR_UNSERIALIZER_H

#include <map>
#include <string>


namespace anatomist
{

  ///	Pointer decoder (or id to pointer map)
  class Unserializer
  {
  public:
    Unserializer();
    virtual ~Unserializer();

    void registerPointer( void* ptr, int id, const std::string & type = "" );
    void* pointer( int id ) const;
    void* pointer( int id, const std::string & typecheck ) const;
    std::string type( void *ptr ) const;
    const std::map<int, void *> & ids() const { return( _id2ptr ); }
    void unregister( int id );
    void garbageCollect();
    /** retreives the ID of an object. 
        If \c type is omitted, it will not be checked.
        \b Warning: this function is rather slow
    */
    int id( void* ptr, const std::string & type = "" ) const;
    /// ganaerates an ID for given pointer and registers it
    int makeID( void* ptr, const std::string & type = "" );
    /// provides a new free ID for future registration
    int freeID() const;

  protected:

  private:
    std::map<int, void*>		_id2ptr;
    std::map<void *, std::string>	_ptr2type;
  };

}


#endif
