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

#include <anatomist/reference/refobserver.h>
#include <anatomist/reference/transfSet.h>
#include <anatomist/observer/Observable.h>
#include <map>

using namespace anatomist;
using namespace std;


Referentiable::Referentiable( Referential* r )
  : _ref( r )
{
}


Referentiable::~Referentiable()
{
}


// ------------

struct ReferentialObserver::Private
{
  Private( const Referential* r, Observer* obs );

  const Referential				*ref;
  Observer					*obs;
  map<const Referentiable*, const Referential*>	refs;
  map<const Referential*, unsigned>		counts;
};


ReferentialObserver::Private::Private( const Referential* r, Observer* obs )
  : ref( r ), obs( obs )
{
}


ReferentialObserver::ReferentialObserver( const Referential* ref, 
                                          Observer* obs ) 
  : Observer(), d( new Private( ref, obs ) )
{
}


ReferentialObserver::~ReferentialObserver()
{
  delete d;
}


void ReferentialObserver::listenReferentiable( const Referentiable* o )
{
  const Referential	*ref = o->referential();
  d->refs[ o ] = ref;
  if( ref )
    {
      unsigned	& c = d->counts[ ref ];
      if( c == 0 )
        ATransformSet::instance()->registerObserver( d->ref, ref, d->obs );
      ++c;
    }
}


void ReferentialObserver::unregisterReferentiable( const Referentiable* o )
{
  map<const Referentiable*, const Referential*>::iterator 
    i = d->refs.find( o );
  if( i != d->refs.end() )
    {
      map<const Referential*, unsigned>::iterator 
        j = d->counts.find( i->second );
      if( j != d->counts.end() )
        {
          --j->second;
          if( !j->second )
            {
              ATransformSet::instance()->unregisterObserver( d->ref, 
                                                             i->second, 
                                                             d->obs );
              d->counts.erase( j );
            }
        }
      d->refs.erase( i );
    }
}


void ReferentialObserver::update( const Observable* obs, void* )
{
  const Referentiable	*r = dynamic_cast<const Referentiable*>( obs );
  if( r )
    {
      /* if( obs == d->obs )
        {
          d->ref = r->referential();
        }
      else
      { */
      unregisterReferentiable( r );
      listenReferentiable( r );
      // }
    }
}


void ReferentialObserver::changeObserverReferential( const Referential* ref )
{
  // notify observers for old ref
  map<const Referential*, unsigned>::iterator j, ej = d->counts.end();

  for( j=d->counts.begin(); j!=ej; ++j )
    ATransformSet::instance()->unregisterObserver( d->ref, j->first, d->obs );

  d->ref = ref;

  // notify observers for new ref
  for( j=d->counts.begin(); j!=ej; ++j )
    ATransformSet::instance()->registerObserver( d->ref, j->first, d->obs );
}


