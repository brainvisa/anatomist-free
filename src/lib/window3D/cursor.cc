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

#include <anatomist/window3D/cursor.h>
#include <anatomist/application/settings.h>
#include <anatomist/application/globalConfig.h>
#include <anatomist/object/Object.h>
#include <anatomist/application/Anatomist.h>
#include <cartobase/stream/fileutil.h>
#include <cartobase/stream/directory.h>

using namespace anatomist;
using namespace carto;
using namespace std;


namespace
{

  struct CursorPrivateStatic
  {
    CursorPrivateStatic();
    ~CursorPrivateStatic();
    void loadCursors( const string & path );
    void cleanup();

    map<string, rc_ptr<AObject> > cursors;
    string			current;
  };


  CursorPrivateStatic::CursorPrivateStatic()
  {
  }

  CursorPrivateStatic::~CursorPrivateStatic()
  {
    cleanup();
  }

  void CursorPrivateStatic::cleanup()
  {
    current.clear();
    cursors.clear();
  }

  void CursorPrivateStatic::loadCursors( const string & path )
  {
    Directory	d( path );
    set<string>	files = d.files(), done;
    set<string>::iterator	i, e = files.end(), notdone = done.end();
    string	name, noext;
    string::size_type	pos;

    for( i=files.begin(); i!=e; ++i )
      {
        pos = i->rfind( '.' );
        if( pos != string::npos )
        {
          if( i->substr( pos, i->length() - pos ) == ".minf" )
            continue;
          else if( done.find( i->substr( 0, pos ) ) != notdone )
            continue;
        }
        name = path + FileUtil::separator() + *i;
        AObject	*o = AObject::load( name.c_str() );
        if( o )
          {
            Cursor::addCursor( *i, o );
            done.insert( *i );
          }
      }
  }

  CursorPrivateStatic & cursorPrivateStatic()
  {
    static bool	done = false;
    static CursorPrivateStatic	x;
    if( !done )
      {
        done = true;
        Cursor::loadCursors();
        GlobalConfiguration	*gc = theAnatomist->config();
        string			name = "cross.mesh";
        gc->getProperty( "cursorShape", name );
        Cursor::setCurrentCursor( name );
      }
    return x;
  }

}


void Cursor::cleanStatic()
{
  cursorPrivateStatic().cleanup();
}


AObject* Cursor::currentCursor()
{
  CursorPrivateStatic & st = cursorPrivateStatic();
  map<string, rc_ptr<AObject> >::const_iterator i = st.cursors.find( st.current );
  if( i == st.cursors.end() )
    {
      if( st.cursors.empty() )
        return 0;
      i = st.cursors.begin();
      st.current = i->first;
      return i->second.get();
    }
  return i->second.get();
}


string Cursor::currentCursorName()
{
  CursorPrivateStatic & st = cursorPrivateStatic();
  return st.current;
}


void Cursor::setCurrentCursor( const std::string & name )
{
  CursorPrivateStatic & st = cursorPrivateStatic();
  map<string, rc_ptr<AObject> >::const_iterator i = st.cursors.find( name );
  if( i != st.cursors.end() )
    st.current = name;
}


std::set<std::string> Cursor::cursors()
{
  CursorPrivateStatic & st = cursorPrivateStatic();
  set<string>	c;
  map<string, rc_ptr<AObject> >::const_iterator	i, e = st.cursors.end();
  for( i=st.cursors.begin(); i!=e; ++i )
    c.insert( i->first );
  return c;
}


void Cursor::addCursor( const std::string & name, AObject* obj )
{
  CursorPrivateStatic & st = cursorPrivateStatic();
  st.cursors[ name ] = rc_ptr<AObject>( obj );
}


void Cursor::deleteCusror( const std::string & name )
{
  CursorPrivateStatic & st = cursorPrivateStatic();
  map<string, rc_ptr<AObject> >::iterator i = st.cursors.find( name );
  if( i != st.cursors.end() )
    st.cursors.erase( i );
}


void Cursor::loadCursors()
{
  CursorPrivateStatic & st = cursorPrivateStatic();
  string	path = Settings::globalPath();
  st.loadCursors( Settings::globalPath() + FileUtil::separator() + "cursors" );
  st.loadCursors( Settings::localPath() + FileUtil::separator() + "cursors" );
}


