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


//--- header files ------------------------------------------------------------

#include <anatomist/observer/Observable.h>
#include <anatomist/observer/Observer.h>
// #define ANA_DEBUG
#ifdef ANA_DEBUG
#include <iostream>
#include <typeinfo>
#endif

using namespace anatomist;
using namespace std;

struct Observable::Private
{
  set<Observer *>	toinsert;
  set<Observer *>	todel;
  mutable set<int>	changedint;
  mutable set<string>	changedstr;
};

//--- methods -----------------------------------------------------------------

Observable::Observable() 
  : _changed(false), _updating( false ), d( new Private )
{
}


Observable::~Observable()
{
  delete d;
}


void
Observable::addObserver(Observer* observer)
{
#ifdef ANA_DEBUG
  cout << "Observable " << this << "::addObserver " 
       << " (" << typeid( *this ).name() << ") - " 
       << observer << " (" << typeid( *observer ).name() << ")" << endl;
#endif
  if( _updating )
    {
      if( !d )
        d = new Private;
      d->toinsert.insert( observer );
      d->todel.erase( observer );
    }
  else
    {
      _observers.insert(observer);
      observer->registerObservable( this );
    }
}


void
Observable::deleteObserver(Observer* observer)
{
#ifdef ANA_DEBUG
  cout << "Observable " << this << "::deleteObserver " 
       << " (" << typeid( *this ).name() << "), " 
       << observer << " (" << typeid( *observer ).name() << ")" 
       << ", updating: " << _updating << endl;
#endif
  if( _updating )
    {
      if( !d )
        d = new Private;
      d->todel.insert( observer );
      d->toinsert.erase( observer );
    }
  else
    {
      set<Observer *>::iterator	i = _observers.find( observer );
      if( i != _observers.end() )
        {
          _observers.erase(observer);
          // observer->unregisterObservable( this );
        }
#ifdef ANA_DEBUG
      else
        cout << "deleteObserver: observer not found\n";
#endif
      observer->unregisterObservable( this );
    }
}


void
Observable::deleteObservers()
{
  if( _updating )
    d->todel.insert( _observers.begin(), _observers.end() );
  else
    // we may have to test _updating for each observer ?
    while( !_observers.empty() )
      deleteObserver( *_observers.begin() );
}


int
Observable::countObservers() const
{
	return _observers.size();
}


void
Observable::notifyObservers(void* arg)
{
  if( _changed && !_updating )
    {
      _updating = true;	// lock
      set<Observer*>::iterator	o;
#ifdef ANA_DEBUG
      static int indent = 0;
      int	i, n = 0;
      for( i=0; i<indent; ++i ) cout << ' ';
      cout << "Observable::notifyObservers: observable: " << this 
           << " (" << typeid( *this ).name() << ")" 
           << ", arg: " << arg << " observers: " << _observers.size() << endl;
      for( i=0; i<indent; ++i ) cout << ' ';
      cout << "<";
      for( o=_observers.begin(); o != _observers.end(); ++o )
        cout << *o << ", ";
      cout << ">\n";
      indent += 2;
#endif
      for( o = _observers.begin(); o != _observers.end(); ++o )
        {
#ifdef ANA_DEBUG
          for( i=0; i<indent; ++i ) cout << ' ';
          cout << "Observer " << n << ": " << *o << " (" 
               << typeid( **o ).name() << ")";
#endif
          if ( !d || d->todel.find( *o ) == d->todel.end() )
            {
#ifdef ANA_DEBUG
              cout << " (updating)" << endl;
#endif
              (*o)->update(this, arg);
            }
#ifdef ANA_DEBUG
          else
            cout << " (not updating: in deletion)" << endl;
          ++n;
#endif
        }

      // handle delayed insertions/deletions
      if( d )
        {
          set<Observer *>::iterator	i, e = d->toinsert.end();
          for( i=d->toinsert.begin(); i!=e; ++i )
            _observers.insert( *i );
          d->toinsert.clear();
          for( i=d->todel.begin(), e=d->todel.end(); i!=e; ++i )
            _observers.erase( *i );
          d->todel.clear();
        }

      clearChanged();

#ifdef ANA_DEBUG
      indent -= 2;
      for( i=0; i<indent; ++i ) cout << ' ';
      cout << "Observable::notifyObservers end" << endl;
#endif
      _updating = false;
    }
}


void
Observable::clearChanged() const
{
  _changed = false;
  d->changedint.clear();
  d->changedstr.clear();
}


void
Observable::obsSetChanged( int f, bool x ) const
{
  if( x )
    {
      d->changedint.insert( f );
      setChanged();
    }
  else
    d->changedint.erase( f );
}


void
Observable::obsSetChanged( const string & f, bool x ) const
{
  if( x )
    {
      d->changedstr.insert( f );
      setChanged();
    }
  else
    d->changedstr.erase( f );
}


void
Observable::notifyUnregisterObservers()
{
  unsigned	s;
  set<Observer*>::iterator o = _observers.begin(), e = _observers.end();

  while( !_observers.empty() )
    {
      s = _observers.size();
      o = _observers.begin();
      (*o)->unregisterObservable( this );
      if( _observers.find( *o ) != e )
	_observers.erase( o );
    }
}


bool
Observable::obsHasChanged( int x ) const
{
  return d->changedint.find( x ) != d->changedint.end();
}


bool
Observable::obsHasChanged( const string & x ) const
{
  return d->changedstr.find( x ) != d->changedstr.end();
}


