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


#include <anatomist/object/optionMatcher.h>
#include <anatomist/object/Object.h>
#include <anatomist/object/actions.h>
#include <anatomist/object/objectmenu.h>
#include <graph/tree/tree.h>


using namespace anatomist;
using namespace carto;
using namespace std;


void OptionMatcher::commonOptions( const set<AObject *> & obj, Tree & tr )
{
  if( obj.size() == 0 )
    return;
  const Tree				*ot = NULL;
  rc_ptr<ObjectMenu>                    om( 0 );
  set<AObject *>::const_iterator	io = obj.begin(), fo = obj.end();

  /* For a while optionTree will remain. These few lines switch between the
     old system and the new one. A default menu is available when any menu
     were found either in optionTree() nor optionMenu(). */
  ot = (*io)->optionTree();
  if (!ot)
  {
    om.reset( (*io)->optionMenu() );
    if (!om) om = AObject::getObjectMenu("__default__");
    if( om )
      ot = om->tree();
  }

  if( !ot )
  {
    return;	// no options
  }
  // copy first object tree
  copyTree( tr, *ot );

  for( ++io; io!=fo; ++io )
    {
      ot = (*io)->optionTree();
      intersect( tr, ot );
    }
}


void OptionMatcher::intersect( Tree & tr, const Tree *ot )
{
  if( !ot )
    {
      while( tr.size() > 0 )
	tr.remove( *tr.begin() );
      return;
    }

  Tree::const_iterator	it, ft = tr.end(), it2;
  Tree::const_iterator	io, fo=ot->end();
  Tree			*st;
  const Tree		*sot;
  bool			erase;
  OptionFunction	cb, cb2;
  rc_ptr<ObjectMenuCallback>    ocb1, ocb2;

  for( it=tr.begin(); it!=ft; ++it )
    {
      st = (Tree *) *it;
      erase = false;
      for( io=ot->begin(); io!=fo; ++io )
	{
	  sot = (Tree *) *io;
	  if( st->getSyntax() == sot->getSyntax() )
	    {
	      if( st->size() == 0 )
              {
                if( !st->getProperty( "objectmenucallback", ocb1 ) )
                {
                  if( st->getProperty( "callback", cb ) )
                    ocb1.reset( new ObjectMenuCallbackFunc( cb ) );
                  else
                    ocb1.reset( 0 );
                }
                if( !sot->getProperty( "objectmenucallback", ocb2 ) )
                {
                  if( sot->getProperty( "callback", cb2 ) )
                    ocb2.reset( new ObjectMenuCallbackFunc( cb2 ) );
                  else
                    ocb2.reset( 0 );
                }
                if( ocb1 && ocb2 && *ocb1 == *ocb2 )
                  break;
                erase = true;
                break;
              }
	      intersect( *st, sot );
	      if( st->size() == 0 )	// no matching sub-menus
		erase = true;
	      break;
	    }
	}
      if( erase || io == fo )	// item *it not found in ot
	{
	  it2 = it;
	  --it;
	  tr.remove( *it2 );
	}
    }
}


void OptionMatcher::copyTree( Tree & tout, const Tree & tin )
{
  tout.setSyntax( tin.getSyntax() );

  OptionFunction	      f;
  rc_ptr<ObjectMenuCallback>  cbk;

  if( tin.getProperty( "objectmenucallback", cbk ) )
    tout.setProperty( "objectmenucallback", cbk );
  else if( tin.getProperty( "callback", f ) )
    tout.setProperty( "callback", f );

  Tree::const_iterator	it, ft=tin.end();
  Tree			*ntr;

  for( it=tin.begin(); it!=ft; ++it )
    {
      ntr = new Tree;
      tout.insert( ntr );
      copyTree( *ntr, *(Tree *) *it );
    }
}

