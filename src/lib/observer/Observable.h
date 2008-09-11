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


#ifndef ANA_OBSERVER_OBSERVABLE_H
#define ANA_OBSERVER_OBSERVABLE_H


//--- header files ------------------------------------------------------------

#include <cartobase/smart/sharedptr.h>
#include <set>
#include <string>


namespace anatomist
{
  //--- forward declarations --------------------------------------------------

  class Observer;


  //--- class declarations ----------------------------------------------------

  /**
   *	This class can be subclassed to represent an object that the programmer
   *	wants to have observed. \\
   *	An observable object can have one or more observers. After an
   *	observable instance changes, an application calling the Observable's
   *	notifyObservers method causes all of its observers to be notified of
   *	the change by a call to their update method. \\
   *	This implementation of an observable is a mere copy of the Java
   *	\URL[Observable]{http://java.sun.com/products/jdk/1.2/docs/api/java.util.Observable.html}.
   */

  class Observable : public carto::SharedObject
  {
  public:
    /// Construct an Observable with zero observers
    Observable();

    /// does nothing
    virtual ~Observable();

    /**@name	Observer handling*/
    //@{

    /**
     *	Adds an observer to the set of observers for this object
     *	@param observer an observer to be added
     */
    void addObserver(Observer* observer);

    /**
     *	Deletes an observer from the set of observers of this object
     *	@param observer observer to be deleted
     */
    void deleteObserver(Observer* observer);

    /**
       Clears the observer list so that this object no longer has
       any observers (doesn't call any observer method - 
       see notifyUnregisterObservers() for this )
    */
    void deleteObservers();

    /**
     *	Returns the number of observers of this object.
     *	@return the number of observers of this object
     */
    int countObservers() const;

    /**
     *	If this object has changed, as indicated by the hasChanged
     *	method, then notify all of its observers. Then call the
     *	clearChanged method to indicate that this object has no
     *	longer changed. \\
     *	Each Observer.has its update method called. 
     *	@param arg anything you want to pass to the observers
     */
    virtual void notifyObservers(void* arg = 0);

    /// Notifies observable destruction to all observers and unregisters them
    virtual void notifyUnregisterObservers();

    //@}

    /**@name	Changed?*/
    //@{

    /**
     *	Tests if this object has changed. 
     *	@return true if the setChanged method has been called more
     *	recently than the clearChanged method on this object; false
     *	otherwise
     */
    bool hasChanged() const;
    /// only valid during an Observer::update()
    bool obsHasChanged( int ) const;
    /// only valid during an Observer::update()
    bool obsHasChanged( const std::string & ) const;

    /// Indicates that this object has changed
    void setChanged() const;

    //@}

  protected:
    /**
     *	Indicates that this object has no longer changed, or that it
     *	has already notified all of its observers of its most recent
     *	change. This method is called automatically by the
     *	notifyObservers methods. 
     *
     *  These changed-flags are temporary and are cleared after notifyObservers
     *  function is called: do not use them to keep an internal track of 
     *  things to update (up-to-date flags should be implemented internally 
     *  by objects)
     */
    void clearChanged() const;

    /// int-based change flags (use enums to address them)
    void obsSetChanged( int, bool = true ) const;
    /// string-based change flags
    void obsSetChanged( const std::string &, bool = true ) const;

    //@}

  private:
    struct Private;

    mutable bool _changed;
    mutable bool _updating;
    Private	*d;

    ///	the set of observers monitoring this observable
    std::set<Observer*> _observers;
  };


  //--- inline methods --------------------------------------------------------

  inline
  bool
  Observable::hasChanged() const
  {
    return _changed;
  }


  inline
  void
  Observable::setChanged() const
  {
    _changed = true;
  }

}


#endif
