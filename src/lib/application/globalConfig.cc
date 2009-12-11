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

#include <anatomist/application/globalConfig.h>
#include <anatomist/application/localConfig.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/application/settings.h>
#include <anatomist/control/graphParams.h>
#include <aims/def/path.h>
#include <graph/tree/treader.h>
#include <graph/tree/twriter.h>
#define ANA_SAVE_SETTINGS_PYTHON
#ifdef ANA_SAVE_SETTINGS_PYTHON
#include <graph/graph/gwriter_python.h>
#endif
#include <cartobase/object/sreader.h>
#include <cartobase/stream/directory.h>
#include <cartobase/stream/fileutil.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <algorithm>
#include <iostream>
#include <qstring.h> // hm, Qt in non-graphical parts... but it's so convenient

using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;


GlobalConfiguration::GlobalConfiguration()
  : Tree( true, "anatomist_settings" )
{
  initSyntax();
}


GlobalConfiguration::~GlobalConfiguration()
{
  list<LocalConfiguration *>::iterator	ic, fc=_configs.end();
  for( ic=_configs.begin(); ic!=fc; ++ic )
    delete (*ic);
}


void GlobalConfiguration::initSyntax()
{
  string syntname = Settings::globalPath() + "/config/settings.stx";

  try
    {
      SyntaxReader	sr( syntname );
      sr >> _syntax;
    }
  catch( exception & e )
    {
      cerr << e.what() << endl;
    }
}


string GlobalConfiguration::configFilename( const string & initial )
{
  if( !initial.empty() )
    return( initial );

  char sep = FileUtil::separator();
  return Settings::localPath() + sep + "config" + sep + "settings.cfg";
}


void GlobalConfiguration::load( const string & filename )
{
  _filename = configFilename( filename );

  cout << "config file : " << _filename << endl;
  try
    {
      TreeReader	tr( _filename, _syntax );
      tr.read( *this );
    }
  catch( file_not_found_error & )
    {
      cerr << "(warning: no config file " << _filename << ": creating it)" 
           << endl;
      save();
    }
  catch( exception & e )
    {
      cerr << e.what() << endl;
    }


  /*string	ver;
  if( !getProperty( "anatomist_version", ver ) )
    cerr << "Configuration file has no version - anterior to 1.23\n";
  else if( ver != theAnatomist->versionString() )
  cerr << "Anatomist configuration file mismatch - please save config\n";*/

  apply();
  update();
}


void GlobalConfiguration::save( const string & filename )
{
  update();

  if( !filename.empty() || _filename.empty() )
    _filename = configFilename( filename );

  string	path = FileUtil::dirname( _filename );
  Directory	dir( path );
  dir.mkdir();

  try
    {
      // set syntax in case it is wrong
      setSyntax( "anatomist_settings" );
#ifdef ANA_SAVE_SETTINGS_PYTHON
      GraphWriter_Python	tw( _filename, _syntax );
      tw.write( *this );
#else
      TreeWriter	tw( _filename, _syntax );
      tw << *this;
#endif
    }
  catch( exception & e )
    {
      cerr << e.what() << endl;
      return;
    }
  cout << "saved global configuration file : " << _filename << endl;
}


void GlobalConfiguration::apply()
{
  list<LocalConfiguration *>::const_iterator	ic, fc=_configs.end();

  int ul = 0;
  string uls;
  if( getProperty( "userLevel", ul ) || getProperty( "userLevel", uls ) )
  {
    if( !uls.empty() )
    {
      QString qs = QString( uls.c_str() ).lower();
      bool ok = true;
      unsigned x = qs.toUInt( &ok );
      if( ok )
        ul = (int) x;
      else if( qs == "basic" )
        ul = 0;
      else if( qs == "advanced" )
        ul = 1;
      else if( qs == "expert" )
        ul = 2;
      else if( qs == "debugger" )
        ul = 3;
      else
        cerr << "Warning: userLevel value in config is not understood: "
            << uls << endl;
    }
    theAnatomist->setUserLevel( ul );
  }
  else
  {
    ul = 0;
    getProperty( "enableUnstable", ul );
    if( ul )
      theAnatomist->setUserLevel( 3 );
  }
  string rm;
  if( getProperty( "selectionRenderingMode", rm ) )
  {
    int rmi = GraphParams::graphParams()->selectRenderModeFromString( rm );
    if( rmi >= 0 )
      GraphParams::graphParams()->selectRenderMode = rmi;
  }

  for( ic=_configs.begin(); ic!=fc; ++ic )
    (*ic)->apply( this );
}


void GlobalConfiguration::update()
{
  list<LocalConfiguration *>::const_iterator	ic, fc=_configs.end();

  for( ic=_configs.begin(); ic!=fc; ++ic )
    (*ic)->update( this );

  setProperty( "anatomist_version", theAnatomist->versionString() );

  // convert old flags to new ones
  if( !hasProperty( "setAutomaticReferential" )
       && hasProperty( "useSpmOrigin" ) )
    try
    {
      Object autorefo = getProperty( "useSpmOrigin" );
      int autoref = 0;
      if( !autorefo.isNull() )
        autoref = (int) autorefo->getScalar();
      if( autoref )
        setProperty( "setAutomaticReferential", autoref );
    }
    catch( ... )
    {
    }

  if( theAnatomist->userLevel() == 0 )
  {
    if( hasProperty( "userLevel" ) )
      removeProperty( "userLevel" );
    if( hasProperty( "enableUnstable" ) )
      removeProperty( "enableUnstable" );
  }
  else
  {
    setProperty( "userLevel", theAnatomist->userLevel() );
    if( theAnatomist->userLevel() >= 3 )
      setProperty( "enableUnstable", (int) 1 );
    else if( hasProperty( "enableUnstable" ) )
      removeProperty( "enableUnstable" );
  }
  if( GraphParams::graphParams()->selectRenderMode > 0 )
    setProperty( "selectionRenderingMode",
                 GraphParams::graphParams()->selectRenderModes
                 [ GraphParams::graphParams()->selectRenderMode ] );
  else if( hasProperty( "selectionRenderingMode" ) )
    removeProperty( "selectionRenderingMode" );

  // remove obsolete flags
  /* let it live for one more version for compatibility
  if( hasProperty( "useSpmOrigin" ) )
    removeProperty( "enableUnstable" );
  */
}


void GlobalConfiguration::registerLocalConfiguration( LocalConfiguration* cfg )
{
  _configs.push_back( cfg );
}


void GlobalConfiguration::unregisterLocalConfiguration( LocalConfiguration* cfg )
{
  list<LocalConfiguration *>::iterator	ic 
    = find( _configs.begin(), _configs.end(), cfg );

  if( ic == _configs.end() )
    {
      cerr << "GlobalConfiguration::unregisterLocalConfiguration : config " << cfg 
	   << " not found\n";
      return;
    }
  _configs.erase( ic );
}
