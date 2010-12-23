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

#include <anatomist/application/syntax.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/application/settings.h>
#include <aims/def/path.h>
#include <cartobase/object/sreader.h>
#include <cartobase/stream/directory.h>
#include <cartobase/stream/fileutil.h>
#include <cartobase/config/verbose.h>
#include <cartobase/config/paths.h>
#include <sys/types.h>
#include <iostream>


using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;


SyntaxRepository	*SyntaxRepository::_syntaxRep = 0;


SyntaxRepository::SyntaxRepository()
{
  if( _path.empty() )
    {
      list<string> p = Paths::findResourceFiles( "syntax", "anatomist",
        theAnatomist->libraryVersionString() );
      _path.insert( _path.end(), p.rbegin(), p.rend() );
      p = Paths::findResourceFiles( "nomenclature/syntax" );
      _path.insert( _path.end(), p.rbegin(), p.rend() );
    }
}


SyntaxRepository::~SyntaxRepository()
{
}


SyntaxRepository & SyntaxRepository::instance()
{
  if( !_syntaxRep )
    _syntaxRep = new SyntaxRepository;
  return( *_syntaxRep );
}


SyntaxSet & SyntaxRepository::exportedSyntax()
{
  return( instance()._exported );
}


SyntaxSet & SyntaxRepository::internalSyntax()
{
  return( instance()._internal );
}


void SyntaxRepository::scanInternalSyntaxes( const string & directory )
{
  scanSyntaxes( instance()._internal, directory );
}


void SyntaxRepository::scanExportedSyntaxes( const string & directory )
{
  SyntaxSet		& ss = instance()._exported;
  scanSyntaxes( ss, directory );
  SyntaxSet::iterator	is, es = ss.end();
  //SyntaxSet		& intern = instance()._internal;

  for( is=ss.begin(); is!=es; ++is )
    instance()._internal.insert( *is );
}


void SyntaxRepository::addPath( const string & directory )
{
  instance()._path.push_back( directory );
}


void SyntaxRepository::setPath( const string & directory )
{
  SyntaxRepository	& sr = instance();
  sr._path.clear();
  sr._path.push_back( directory );
}


void SyntaxRepository::addPath( const vector<string> & dirs )
{
  SyntaxRepository			& sr = instance();
  vector<string>::const_iterator	id, ed = dirs.end();

  for( id=dirs.begin(); id!=ed; ++id )
    sr._path.push_back( *id );
}


void SyntaxRepository::setPath( const vector<string> & dirs )
{
  SyntaxRepository			& sr = instance();
  vector<string>::const_iterator	id, ed = dirs.end();

  sr._path.clear();
  for( id=dirs.begin(); id!=ed; ++id )
    sr._path.push_back( *id );
}


void SyntaxRepository::scanSyntaxes( SyntaxSet & ss, const string & directory )
{
  SyntaxRepository & sr = instance();
  vector<string>::const_iterator	ip, ep=sr._path.end();

  for( ip=sr._path.begin(); ip!=ep; ++ip )
    scanSyntaxesInDir( ss, *ip );

  if( !directory.empty() )
    scanSyntaxesInDir( ss, directory );
}


bool SyntaxRepository::readSyntax( SyntaxSet & ss, const string & filename, 
				   const string & directory )
{
  SyntaxRepository			& sr = instance();
  vector<string>::reverse_iterator	ip, ep=sr._path.rend();

  if( !directory.empty() )
    if( tryReadSyntax( ss, directory + '/' + filename ) )
      return( true );

  //	try from the end of path list (highest priority)
  for( ip=sr._path.rbegin(); ip!=ep; ++ip )
    if( tryReadSyntax( ss, *ip + '/' + filename ) )
      return( true );

  return( false );	// not found at all
}


bool SyntaxRepository::tryReadSyntax( SyntaxSet & ss, const string & file )
{
  try
    {
      SyntaxReader	sr( file );
      sr >> ss;
    }
  catch( exception & e )
    {
      return( false );
    }
  return( true );
}


void SyntaxRepository::scanSyntaxesInDir( SyntaxSet & ss, 
					  const string & directory )
{
  Directory	dir( directory );
  set<string>	files = dir.files();
  set<string>::iterator	ifi, efi = files.end();
  string	fname;
  unsigned	pos;

  for( ifi=files.begin(); ifi!=efi; ++ifi )
  {
    fname = directory + FileUtil::separator() + *ifi;
    pos = fname.rfind( ".stx" );
    if( pos == fname.size() - 4 )
    {
      SyntaxSet	ts;
      if( tryReadSyntax( ts, fname ) )
      {
        if( carto::verbose )
          cout << "Syntax " << fname << " read\n";
        mergeSyntaxes( ss, ts );
      }
      else
        cout << "couldn't read " << fname << endl;
    }
  }
}


void SyntaxRepository::mergeSyntaxes( SyntaxSet & dest, const SyntaxSet & src, 
				      int warn_level )
{
  //cout << "mergeSyntaxes << " << src.size() << endl;

  SyntaxSet::const_iterator	iss, ess = src.end();
  SyntaxSet::iterator		ids, eds = dest.end();
  Syntax::const_iterator	is, es;
  Syntax::iterator		is2, es2;

  for( iss=src.begin(); iss!=ess; ++iss )
    {
      ids = dest.find( (*iss).first );
      if( ids == eds )
	dest[ (*iss).first ] = (*iss).second;
      else	// merge attributes
	{
	  Syntax	& s = (*ids).second;
	  es2 = s.end();
	  for( is=(*iss).second.begin(), es=(*iss).second.end(); is!=es; ++is )
	    {
	      if( warn_level >= 1 && ( is2 = s.find( (*is).first ) ) != es2 
		  && ( (*is2).second.type != (*is).second.type 
		       || ( warn_level >= 2 
			    && (*is2).second.needed 
			    != (*is).second.needed ) ) )
		cout << "warning : syntax mismatch in merging " 
		     << (*iss).first 
		     << " : attribute " << (*is).first << " :\n"
		     << "was { " << (*is2).second.type << ", " 
		     << ( (*is2).second.needed ? "mandatory" : "optional" )
		     << " }; is now { " << (*is).second.type << ", " 
		     << ( (*is).second.needed ? "mandatory" : "optional" ) 
		     <<  " }\n";
	      s.insert( *is );
	    }
	}
    }
}
