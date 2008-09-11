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

#include <anatomist/reference/transformobserver.h>
#include <anatomist/reference/Transformation.h>
#include <map>

// uncomment this to enable debug output
// #define ANA_DEBUG

using namespace anatomist;
using namespace std;


struct TransformationObserver::Private
{
  Private( const set<const Referential *> & r );
  mutable set<const Referential *>	refs;
  map<Observer *, unsigned>		obscounts;
};


TransformationObserver::Private::Private( const set<const Referential *> & r )
  : refs( r )
{
}


TransformationObserver::TransformationObserver( const set<const Referential *> 
                                                & refs )
  : Observable(), d( new Private( refs ) )
{
  // cout << "TransformationObserver " << this << " created\n";
}


TransformationObserver::~TransformationObserver()
{
  // cout << "~TransformationObserver " << this << endl;
  delete d;
}


bool TransformationObserver::involves( const Referential* r ) const
{
  return d->refs.find( r ) != d->refs.end();
}


bool TransformationObserver::involves( const Transformation* t ) const
{
  return involves( t->source() ) && involves( t->destination() );
}


void TransformationObserver::addObserver( Observer *observer )
{
#ifdef ANA_DEBUG
  cout << "TransformationObserver::addObserver: this: " << this 
       << ", obs: " << observer << endl;
#endif
  unsigned	& n = d->obscounts[ observer ];
  ++n;
  if( n == 1 )
    Observable::addObserver( observer );
#ifdef ANA_DEBUG
  cout << "obs: " << n << endl;
#endif
}


void TransformationObserver::deleteObserver( Observer *observer )
{
#ifdef ANA_DEBUG
  cout << "TransformationObserver::deleteObserver " << observer << endl;
#endif
  map<Observer *, unsigned>::iterator	i = d->obscounts.find( observer );
  if( i == d->obscounts.end() )
    {
#ifdef ANA_DEBUG
      cerr << "TransformationObserver::deleteObserver: obs not found\n";
#endif
      return;
    }
  --i->second;
#ifdef ANA_DEBUG
  cout << "observer refcount: " << i->second << endl;
#endif
  if( i->second == 0 )
    {
      d->obscounts.erase( i );
      Observable::deleteObserver( observer );
    }
}


