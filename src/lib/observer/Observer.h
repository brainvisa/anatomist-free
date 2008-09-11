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


#ifndef ANA_OBSERVER_OBSERVER_H
#define ANA_OBSERVER_OBSERVER_H

#include <set>

namespace anatomist
{
  //--- forward declarations --------------------------------------------------

  class Observable;


  //--- class declarations ----------------------------------------------------

  /**
   *	A class can implement the Observer interface when it wants to be
   *	informed of changes in observable objects.
   */

  class Observer
  {
  public:
    virtual ~Observer();

    /**
     *	This method is called whenever the observed object is changed.
     *	The programmer calls an observable object's notifyObservers
     *	method to have all the object's observers notified of the
     *	change.
     *	@param observable the observable object
     */
    virtual void update( const Observable* observable, void* arg ) = 0;

  protected:
    /** call this function from inherited classes destructors. 
        Derived classes may have to manually call it, especially if they 
        have overloaded \c unregisterObservable() otherwise it may be 
        called by the destructor of a base class which will not be able to 
        access the correct overloaded functions anymore.

        It's not an error to call \c cleanupObserver() several times: 
        basically each destructor in the inheritance tree of a derived class 
        will call it, but only the first call will have any effect.
    */
    virtual void cleanupObserver();
    /// only called by Observable: don't use this function directly
    virtual void registerObservable( Observable* );
    /** Called when an observable is destroyed, only called by Observable: 
        don't use this function directly */
    virtual void unregisterObservable( Observable* );
    const std::set<Observable *> & observed() const { return _observed; }

  private:
    std::set<Observable *> _observed;

    friend class Observable;
  };

}


#endif
