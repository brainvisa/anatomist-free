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


/*	Simple cmdline pour transformer un fichier .roi_names_template_machin 
	en hierarchie AnaQt.
	(cette cmdline n'utilise en fait QUE la librairie graph)
*/


#include <graph/tree/tree.h>
#include <graph/tree/twriter.h>
#include <iostream>

using namespace carto;
using namespace std;

void usage( char* name )
{
  cerr << "usage : " << name << " inputfile outputfile\n";
  cerr << "inputfile  : .roi_names_template ou un fichier de même format\n";
  cerr << "outputfile : fichier arbre de hierarchie AnaQt (.hie)\n";
  exit( 1 );
}


bool parse2( istream & inp, Tree* tl, Tree* tr, string & nxtline )
{
  string	buf;
  Tree		*tsl, *tsr;
  bool		allowrecurs = true;

  while( !nxtline.empty() || !inp.eof() )
    {
      while( buf.empty() && ( !nxtline.empty() || !inp.eof() ) )
	{
	  if( !nxtline.empty() )
	    {
	      buf = nxtline;
	      nxtline.erase( 0, nxtline.size() );
	    }
	  else
	    getline( inp, buf );

	  while( !buf.empty() && ( buf[0] == ' ' || buf[0] == '\t' ) )
	    buf.erase( 0, 1 );	// enlève les blancs
	  if( !buf.empty() && buf[0] == '#' )	// commentaire
	    buf.erase( 0, buf.size() );
	}

      if( !buf.empty() )
	{
	  // on a une vraie ligne maintenant
	  switch( buf[0] )	// code
	    {
	    case '%':	// feuille
	      buf.erase( 0, 1 );
	      tsl = new Tree( true, "fold_name" );
	      tsl->setProperty( "name", buf + "_left" );
	      tl->insert( tsl );
	      tsr = new Tree( true, "fold_name" );
	      tsr->setProperty( "name", buf + "_right" );
	      tr->insert( tsr );
	      allowrecurs = false;
	      break;
	    case '$':	// branche
	      if( allowrecurs )
		{
		  buf.erase( 0, 1 );
		  tsl = new Tree( true, "fold_name" );
		  tsl->setProperty( "name", buf + "_left" );
		  tl->insert( tsl );
		  tsr = new Tree( true, "fold_name" );
		  tsr->setProperty( "name", buf + "_right" );
		  tr->insert( tsr );
		  if( !parse2( inp, tsl, tsr, nxtline ) ) // niveau en dessous
		    return( false );
		}
	      else
		{
		  nxtline = buf;
		  return( true );
		}
	      break;
	    default:
	      cerr << "Ligne pas comprise :\n" << buf << "\nAbort\n";
	      return( false );
	    }
	  buf.erase( 0, buf.size() );
	}
    }

  return( true );
}


bool parse( istream & inp, Tree* tl, Tree* tr )
{
  string dummy;
  return( parse2( inp, tl, tr, dummy ) );
}


int main( int argc, char** argv )
{
  if( argc != 3 )
    usage( argv[0] );

  ifstream	inp( argv[1] );
  if( !inp )
    {
      cerr << argv[1] << " : file not found\n";
      exit( 1 );
    }

  Tree	hie( true, "hierarchy" );
  Tree	*t, *tl, *tr;

  hie.setProperty( "graph_syntax", (string) "graphe_sillon" );

  t = new Tree( true, "fold_name" );
  t->setProperty( "name", (string) "unknown unknown" );
  hie.insert( t );

  t = new Tree( true, "fold_name" );
  t->setProperty( "name", (string) "brain" );
  hie.insert( t );

  tl = new Tree( true, "fold_name" );
  tr = new Tree( true, "fold_name" );
  tl->setProperty( "name", (string) "hemisph_left" );
  tr->setProperty( "name", (string) "hemisph_right" );
  t->insert( tl );
  t->insert( tr );

  if( parse( inp, tl, tr ) )
    {
      SyntaxSet	ss;
      ss[ "hierarchy" ][ "graph_syntax" ].type = "string";
      ss[ "hierarchy" ][ "graph_syntax" ].needed = true;
      ss[ "fold_name" ][ "name" ].type = "string";
      ss[ "fold_name" ][ "name" ].needed = true;

      TreeWriter	tw( argv[2], ss );
      tw << hie;
    }
}
