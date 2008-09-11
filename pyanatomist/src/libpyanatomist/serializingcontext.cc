/* Copyright (c) 2007 CEA
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

#include <pyanatomist/serializingcontext.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/reference/transfSet.h>

using namespace anatomist;
using namespace carto;
using namespace std;


SerializingContext::SerializingContext( CommandContext & context )
  : Serializer(), _context( context )
{
  // copy unserializer to serializer
  const map<int, void *>  & ids = context.unserial->ids();
  map<int, void *>::const_iterator  iu, eu = ids.end();

  for( iu=ids.begin(); iu!=eu; ++iu )
  {
    _ptr2id[ iu->second ] = iu->first;
    if( iu->first > _id )
      _id = iu->first;
  }
}


void SerializingContext::updateUnserializer()
{
  map<void *, int>::const_iterator  is, es = _ptr2id.end();
  rc_ptr<Unserializer>  us = _context.unserial;
  string  t;
  void  *ptr;
  set<Referential *>  refs = theAnatomist->getReferentials();
  set<Referential *>::iterator  er = refs.end();
  ATransformSet *ts = ATransformSet::instance();

  for( is=_ptr2id.begin(); is!=es; ++is )
    if( !us->pointer( is->second ) )
    {
      ptr = is->first;
      t.clear();
      if( theAnatomist->hasObject( (AObject *) ptr ) )
        t = "AObject";
      else if( theAnatomist->hasWindow( (AWindow *) ptr ) )
        t = "AWindow";
      else if( refs.find( (Referential *) ptr ) != er )
        t = "Referential";
      else if( ts->hasTransformation( (Transformation *) ptr ) )
        t = "Transformation";
      if( !t.empty() )
        us->registerPointer( ptr, is->second, t );
    }
}


