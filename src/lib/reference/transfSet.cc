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


#include <anatomist/reference/transfSet.h>
#include <anatomist/reference/Transformation.h>
#include <anatomist/reference/transformobserver.h>
#include <anatomist/reference/wReferential.h> // TODO: remove this bidouille
#include <anatomist/application/Anatomist.h>    // idem
#include <assert.h>
#include <iostream>

// uncomment this to enable debug output
// #define ANA_DEBUG


using namespace anatomist;
using namespace std;


ATransformSet* ATransformSet::instance()
{
  static ATransformSet	*_instance = 0;
  if( !_instance )
    _instance = new ATransformSet;
  return( _instance );
}


namespace
{

  struct TransfHolder
  {
    TransfHolder() : trans( 0 ), obs( 0 ) {}
    TransfHolder( const TransfHolder & );

    Transformation			*trans;
    mutable auto_ptr<TransformationObserver>	obs;
  };


  TransfHolder::TransfHolder( const TransfHolder & x )
    : trans( x.trans ), obs( x.obs )
  {
  }

}


struct ATransformSet::Private
{
  typedef map<Referential *,TransfHolder> Ts2;
  typedef map<Referential *, Ts2 > Ts;
  ~Private();

  TransfHolder *transformation( Referential* src, Referential* dst );
  const TransfHolder *transformation( Referential* src, 
                                      Referential* dst ) const;

  ///	Transformations listed by source referential
  Ts			trans;
  set<Transformation *>	tset;
};


ATransformSet::Private::~Private()
{
  // delete all transformations
  // should be OK since each transformation unregisters itself
  while( !trans.empty() )
    {
      TransfHolder & th = trans.begin()->second.begin()->second;
      th.obs.reset( 0 );
      if( th.trans )
        delete th.trans;
      else
        trans.begin()->second.erase( trans.begin()->second.begin() );
        
    }
}


TransfHolder *ATransformSet::Private::transformation( Referential* src, 
                                                      Referential* dst )
{
  Ts::iterator	is = trans.find( src );
  if( is == trans.end() )
    return 0;
  Ts2::iterator	id = is->second.find( dst );
  if( id == is->second.end() )
    return 0;
  return &id->second;
}


const TransfHolder *
ATransformSet::Private::transformation( Referential* src, 
                                        Referential* dst ) const
{
  Ts::const_iterator	is = trans.find( src );
  if( is == trans.end() )
    return 0;
  Ts2::const_iterator	id = is->second.find( dst );
  if( id == is->second.end() )
    return 0;
  return &id->second;
}


ATransformSet::ATransformSet() : d( new Private )
{
}


ATransformSet::~ATransformSet()
{
  delete d;
}


Transformation* ATransformSet::transformation( const Referential* src, 
					       const Referential* dst )
{
  TransfHolder	*th = d->transformation( const_cast<Referential *>( src ), 
                                         const_cast<Referential *>( dst ) );
  if( !th )
    return 0;
  return th->trans;
}


const Transformation* 
ATransformSet::transformation( const Referential* src, 
			       const Referential* dst ) const
{
  TransfHolder	*th = d->transformation( const_cast<Referential *>( src ), 
                                         const_cast<Referential *>( dst ) );
  if( !th )
    return 0;
  return th->trans;
}


void ATransformSet::registerTransformation( Transformation* t )
{
  // cout << "register Trans " << t << endl;
  if( t->source() == t->destination() )
    return;
  Private::Ts::iterator	is = d->trans.find( t->source() );
  if( is != d->trans.end() )
    {
      Private::Ts2::iterator 
        id = is->second.find( t->destination() );
      if( id != is->second.end() && id->second.trans )
	{
	  cerr << "registerTransformation : there was already a "
	       << "transformation between this source and destination" 
	       << "- deleting it ( " << id->second.trans << " )\n";
          delete id->second.trans;
	}
    }
  TransfHolder	& th = d->trans[ t->source() ][ t->destination() ];
  th.trans = t;
  d->tset.insert( t );
  if( !t->isGenerated() )
    completeTransformations( t );
  TransformationObserver	*to = th.obs.get();
  // cout << "register Trans " << t << endl;
  if( to )
    {
#ifdef ANA_DEBUG
      cout << "TransformObserver " << to << " activated\n";
#endif
      to->setChanged();
      to->notifyObservers( t );
    }
  ReferentialWindow *rw = theAnatomist->getReferentialWindow();
  if( rw )
    rw->refresh();
  // cout << "done: register Trans " << t << endl;
}


void ATransformSet::unregisterTransformation( Transformation* t )
{
#ifdef ANA_DEBUG
  cout << "unregister Trans " << t << endl;
#endif
  Referential		*r1 = t->source(), *r2 = t->destination();
  bool			notgen = !t->isGenerated();
  Private::Ts::iterator	is = d->trans.find( r1 ), es = d->trans.end();
  if( is != es )
    {
      Private::Ts2::iterator	id = is->second.find( r2 );
      if( id == is->second.end() )
	{
	  cerr << "unregisterTransformation : transformation not found";
	  return;
	}
      id->second.trans = 0;
      if( id->second.obs.get() == 0 )
        is->second.erase( id );
      else
        {
#ifdef ANA_DEBUG
          cout << "unregisterTrans: notify obs " << id->second.obs.get() 
               << endl;
#endif
          id->second.obs->setChanged();
          id->second.obs->notifyObservers( 0 );
        }

      if( is->second.empty() )
	d->trans.erase( is );
      d->tset.erase( t );

      if( notgen )
	deleteGeneratedConnections( r1, r2 );

      ReferentialWindow *rw = theAnatomist->getReferentialWindow();
      if( rw )
        rw->refresh();
    }
  else
    cerr << "unregisterTransformation : transformation not found";
#ifdef ANA_DEBUG
  cout << "done: unregister Trans " << t << endl;
#endif
}


set<Transformation *> ATransformSet::allTransformations() const
{
  return d->tset;
}


set<Transformation *> 
ATransformSet::transformationsWith( const Referential* ref ) const
{
#ifdef ANA_DEBUG
  cout << "transformationsWith " << ref << "..." << endl;
#endif
  set<Transformation *>			ts;
  Private::Ts::const_iterator		is, fs = d->trans.end();
  Private::Ts2::const_iterator		id, fd;

#ifdef ANA_DEBUG
  cout << "starts: " << d->trans.size() << endl;
#endif
  for( is=d->trans.begin(); is!=fs; ++is )
    {
      if( is->first == ref )
        {
          for( id=is->second.begin(), fd=is->second.end(); id!=fd; ++id )
            if( id->second.trans )
              ts.insert( id->second.trans );
        }
      else
	for( id=is->second.begin(), fd=is->second.end(); id!=fd; ++id )
	  if( id->first == ref && id->second.trans )
	    ts.insert( id->second.trans );
    }
#ifdef ANA_DEBUG
  cout << "transformationsWith " << ref << " END with n=" << ts.size() << "\n";
#endif
  return ts;
}


void ATransformSet::deleteTransformationsWith( const Referential* ref )
{
#ifdef ANA_DEBUG
  cout << "deleteTransformationsWith " << ref << "...\n";
#endif
  set<Transformation *>			ts = transformationsWith( ref );

  while( !ts.empty() )
    {
      delete *ts.begin();
      ts = transformationsWith( ref );
    }
#ifdef ANA_DEBUG
  cout << "deleteTransformationsWith " << ref << " END\n";
#endif
}


bool ATransformSet::hasTransformation( const Transformation* t ) const
{
  return( d->tset.find( const_cast<Transformation *>( t ) ) != d->tset.end() );
}


void ATransformSet::propagate( Referential* ref, Referential* r2, 
			       const set<Referential *> & others )
{
  //cout << "prop ref " << ref << " -> " << r2 << endl;
  set<Referential *>::const_iterator	ir, er = others.end();
  Transformation			*tr, *tr2, *rt;
  Referential				*s, *d;

  rt = transformation( ref, r2 );
  if( !rt )
    {
      tr = transformation( r2, ref );
      assert( tr );
      rt = new Transformation( ref, r2, true, true );
      *rt = *tr;
      rt->invert();
    }

  for( ir=others.begin(); ir!=er; ++ir )
    {
      s = ref;
      d = *ir;
      tr = transformation( s, d );
      if( !tr )
	{
	  //cout << "create transf...\n";
	  tr = new Transformation( s, d, true, true );
	}
      //cout << "update " << s << " -> " << d << endl;
      if( tr->isGenerated() )
	{
	  if( r2 == d )
	    {
	      *tr = *rt;
	    }
	  else
	    {
	      tr2 = transformation( r2, d );
	      assert( tr2 );

	      *tr = *tr2;
	      *tr *= *rt;

	      /*cout << "apres :\n";
	      cout << "T: " << tr->Translation(0) << ", " 
		   << tr->Translation(1) << ", " 
		   << tr->Translation(2) << endl;
	      cout << "R: " << rt->Rotation(0, 0) << ", " 
		   << tr->Rotation(0, 1) << ", " 
		   << tr->Rotation(0, 2) << endl;
	      cout << "   " << rt->Rotation(1, 0) << ", " 
		   << tr->Rotation(1, 1) << ", " 
		   << tr->Rotation(1, 2) << endl;
	      cout << "   " << rt->Rotation(2, 0) << ", " 
		   << tr->Rotation(2, 1) << ", " 
		   << tr->Rotation(2, 2) << endl;*/
	    }
	}
      tr2 = transformation( d, s );
      if( !tr2 )
	tr2 = new Transformation( d, s, true, true );
      //cout << "update " << d << " -> " << s << endl;
      if( tr2->isGenerated() )
	{
	  *tr2 = *tr;
	  tr2->invert();
	}
    }
  //cout << "prop done\n";
}


void ATransformSet::completeTransformations( Transformation* t )
{
  Referential	*r1 = t->source();
  Referential	*r2 = t->destination();
  Referential	*oref;
  //cout << "completeTransformations " << r1 << " -> " << r2 << endl;

  ///	fill inverse
  Transformation	*inv = transformation( r2, r1 );
  if( !inv )
    inv = new Transformation( r2, r1, true, true );
  *inv = *t;
  inv->invert();
  inv->setGenerated( true );

  Private::Ts::iterator		is, es = d->trans.end();
  Private::Ts2::iterator	id, ed, id2;

  //	front propagation

  set<Referential *>					done;
  map<Referential *, Transformation *>			front;
  set<Referential *>::iterator				dend = done.end();
  map<Referential *, Transformation *>::iterator	ir;
  Referential						*r, *ris;

  front[ r1 ] = t;

  while( !front.empty() )
    {
      ir = front.begin();
      r = ir->first;
      //cout << "front : " << r << endl;
      oref = ir->second->source();
      if( oref == r )
	oref = ir->second->destination();
      propagate( r, oref, done );
      done.insert( r );
      front.erase( ir );
      is=d->trans.begin();
      while( is != es )
	{
	  bool	incd = false;
	  //cout << "trans loop\n";
	  ed=is->second.end();
	  if( is->first == r )
	    {
	      //cout << "line\n";
	      id = is->second.begin();
	      ++is;
	      incd = true;
	      while( id != ed )
                if( id->second.trans )
                  if( id->second.trans->isGenerated() )
                    {
                      if( done.find( id->first ) == dend )
                        {
                          id2 = id;
                          ++id;
                          delete id2->second.trans;
                        }
                      else
                        ++id;
                    }
                  else
                    {
                      if( done.find( id->first ) == dend )
                        {
                          //cout << "insert " << id->first << endl;
                          front[ id->first ] = id->second.trans;
                        }
                      ++id;
                    }
                else
                  ++id;
            }
          else
            {
              //cout << "col\n";
              id = is->second.find( r );
              ris = is->first;
              ++is;
              incd = true;
              if( id != ed && id->second.trans )
              {
                if( id->second.trans->isGenerated() )
                  {
                    if( ris != r2 && done.find( ris ) == dend )
                      {
                        id2 = id;
                        ++id;
                        //cout << "delete " << ris << " -> " << r << endl;
                        delete id2->second.trans;
                        //cout << "(deleted)\n";
                      }
                  }
                else if( done.find( ris ) == dend )
                  {
                    //cout << "insert " << ris << endl;
                    front[ ris ] = id->second.trans;
                  }
              }
              if( !incd )
                ++is;
            }
        }
      //cout << "end loop is\n";
    }
}


set<Referential *> ATransformSet::connectedComponent( Referential* r0 ) const
{
  Private::Ts::const_iterator	is, es = d->trans.end();
  Private::Ts2::const_iterator	id, ed;

  //	front propagation

  set<Referential *>					done, front;
  set<Referential *>::iterator				ir, dend = done.end();
  Referential						*r, *ris;

  front.insert( r0 );

  while( !front.empty() )
    {
      ir = front.begin();
      r = *ir;
      //cout << "front : " << r << endl;
      done.insert( r );
      front.erase( ir );
      for( is=d->trans.begin(); is != es; ++is )
	{
	  //cout << "trans loop\n";
	  ed=is->second.end();
	  if( is->first == r )
	    {
	      //cout << "line\n";
	      for( id=is->second.begin(); id != ed; ++id )
		if( id->second.trans && !id->second.trans->isGenerated() 
		    && done.find( id->first ) == dend )
                  front.insert( id->first );
	    }
	  else
	    {
	      //cout << "col\n";
	      id = is->second.find( r );
	      ris = is->first;
	      if( id != ed && id->second.trans 
                  && !id->second.trans->isGenerated() 
		  && done.find( ris ) == dend )
		front.insert( ris );
	    }
	}
      //cout << "end loop is\n";
    }
  return( done );
}


void ATransformSet::deleteGeneratedConnections( Referential* r1, 
						Referential* r2 )
{
  set<Referential *>	s1 = connectedComponent( r1 );
  set<Referential *>	s2 = connectedComponent( r2 );
  set<Referential *>::iterator	ir1, er1 = s1.end(), ir2, er2 = s2.end();
  Private::Ts::iterator		is, es = d->trans.end();
  Private::Ts2::iterator	id, ed;
  Transformation		*t;
  Referential			*r;
  bool				stop;

  for( ir1=s1.begin(); ir1!=er1; ++ir1 )
    {
      is = d->trans.find( *ir1 );
      if( is != es )
	{
	  id = is->second.begin();
	  ed = is->second.end();
	  stop = false;
	  while( !stop && id != ed )
	    {
	      r = id->first;
	      t = id->second.trans;
	      ++id;
	      if( t && t->isGenerated() && s2.find( r ) != er2 )
		{
		  if( is->second.size() == 1 )
		    stop = true;
		  delete t;
		}
	    }
	}
    }

  for( ir2=s2.begin(); ir2!=er2; ++ir2 )
    {
      is = d->trans.find( *ir2 );
      if( is != es )
	{
	  id = is->second.begin();
	  ed = is->second.end();
	  stop = false;
	  while( !stop && id != ed )
	    {
	      r = id->first;
	      t = id->second.trans;
	      ++id;
	      if( t && t->isGenerated() && s1.find( r ) != er1 )
		{
		  if( is->second.size() == 1 )
		    stop = true;
		  delete t;
		}
	    }
	}
    }
}


unsigned ATransformSet::size() const
{
  return d->trans.size();
}


void ATransformSet::registerObserver( const Referential* src, 
                                      const Referential* dst, Observer* o )
{
  if( src == dst || !src || !dst )
    return;

  Referential	*sr = const_cast<Referential *>( src );
  Referential	*ds = const_cast<Referential *>( dst );
  if( sr > ds )
    {
      Referential	*tmp = ds;
      ds = sr;
      sr = tmp;
    }

  TransfHolder	& th = d->trans[sr][ds];
#ifdef ANA_DEBUG
  cout << "ATransformSet::registerObserver " << o << " between " 
       << src << ", " << dst << endl;
#endif
  if( !th.obs.get() )
    {
#ifdef ANA_DEBUG
      cout << "creating TransformationObserver\n";
#endif
      set<const Referential *>	s;
      s.insert( src );
      s.insert( dst );
      th.obs.reset( new TransformationObserver( s ) );
    }
  th.obs->addObserver( o );
}


void ATransformSet::unregisterObserver( const Referential* src, 
                                        const Referential* dst, Observer* o )
{
#ifdef ANA_DEBUG
  cout << "ATransformSet::unregisterObserver " << o << " between " 
       << src << ", " << dst << endl;
#endif
  if( src == dst || !src || !dst )
    return;

  Referential	*sr = const_cast<Referential *>( src );
  Referential	*ds = const_cast<Referential *>( dst );
  if( sr > ds )
    {
      Referential	*tmp = ds;
      ds = sr;
      sr = tmp;
    }
  Private::Ts::iterator	i = d->trans.find( sr );
  if( i != d->trans.end() )
    {
      Private::Ts2::iterator	j = i->second.find( ds );
      if( j != i->second.end() )
        {
          TransfHolder	& th = j->second;
          if( th.obs.get() )
            {
              th.obs->deleteObserver( o );
#ifdef ANA_DEBUG
              cout << "observers: " << th.obs->countObservers() << endl;
#endif
              if( th.obs->countObservers() == 0 )
                {
                  th.obs.reset( 0 );
                  if( !th.trans )
                    {
                      i->second.erase( j );
                      if( i->second.empty() )
                        d->trans.erase( i );
                    }
                }
            }
        }
      else
        cerr << "ATransformSet::unregisterObserver: ref " << ds 
             << " not found\n";
    }
  else
    cerr << "ATransformSet::unregisterObserver: ref " << sr << " not found\n";
}


void ATransformSet::setTransformationChanged( const Referential* src, 
                                              const Referential* dst )
{
  if( src == dst || !src || !dst )
    return;

  Referential	*sr = const_cast<Referential *>( src );
  Referential	*ds = const_cast<Referential *>( dst );
  if( sr > ds )
    {
      Referential	*tmp = ds;
      ds = sr;
      sr = tmp;
    }
  Private::Ts::iterator	i = d->trans.find( sr );
  if( i != d->trans.end() )
    {
      Private::Ts2::iterator	j = i->second.find( ds );
      if( j != i->second.end() )
        {
          TransfHolder	& th = j->second;
          if( th.obs.get() )
            th.obs->setChanged();
        }
    }
}


void ATransformSet::notifyTransformationChanged( const Referential* src, 
                                                 const Referential* dst, 
                                                 void* arg, bool setchanged )
{
  if( src == dst || !src || !dst )
    return;

  Referential	*sr = const_cast<Referential *>( src );
  Referential	*ds = const_cast<Referential *>( dst );
  if( sr > ds )
    {
      Referential	*tmp = ds;
      ds = sr;
      sr = tmp;
    }
  Private::Ts::iterator	i = d->trans.find( sr );
  if( i != d->trans.end() )
    {
      Private::Ts2::iterator	j = i->second.find( ds );
      if( j != i->second.end() )
        {
          TransfHolder	& th = j->second;
          if( th.obs.get() )
            {
              if( setchanged )
                th.obs->setChanged();
              th.obs->notifyObservers( arg );
            }
        }
    }
}


