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

#include <anatomist/observer/Observer.h>
#include <anatomist/observer/Observable.h>
#include <iostream>

using namespace anatomist;
using namespace std;

Observer::~Observer()
{
  cleanupObserver();
}


void Observer::cleanupObserver()
{
  unsigned    n;
  while( !_observed.empty() )
  {
    n = _observed.size();
    (*_observed.begin())->deleteObserver( this );
    if( _observed.size() == n )
    {
      cerr << "observable " << *_observed.begin() << "("
           << typeid(**_observed.begin()).name() << ") could not be removed "
           << "from observer " << this << " (" << typeid(*this).name() << ")"
           << endl;
      _observed.erase( _observed.begin() );
    }
  }
}


void Observer::registerObservable( Observable* o )
{
  _observed.insert( o );
}


void Observer::unregisterObservable( Observable* o )
{
  _observed.erase( o );
}


