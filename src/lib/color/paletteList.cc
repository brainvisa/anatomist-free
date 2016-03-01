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


#include <anatomist/color/paletteList.h>
#include <anatomist/color/palette.h>
#include <aims/io/reader.h>
#include <aims/io/writer.h>
#include <aims/io/finder.h>
#include <cartobase/stream/directory.h>
#include <cartobase/stream/fileutil.h>
#include <cartobase/type/string_conversion.h>
#include <set>
#include <algorithm>
#include <iostream>
#include <errno.h>


using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;


PaletteList::PaletteList( const string & dirname )
{
  load( dirname );
}


PaletteList::~PaletteList()
{
}


void PaletteList::clear()
{
  _pal.clear();
}


void PaletteList::erase( rc_ptr<APalette> pal )
{
  list<rc_ptr<APalette> >::iterator ip = ::find( _pal.begin(), _pal.end(),
                                                 pal );
  if( ip != _pal.end() )
    _pal.erase( ip );
}


rc_ptr<APalette> PaletteList::loadPalette( const string & filename,
                                           const string & palettename )
{
  rc_ptr<APalette> pal;
  Finder  f;
  if( f.check( filename ) && f.objectType() == "Volume" )
    {
      string      dt;
      if( f.dataType() == "RGBA" )
        dt = f.dataType();
      else
      {
        vector<string>  pdt = f.possibleDataTypes();
        unsigned                ipt, ept = pdt.size();
        for( ipt=0; ipt!=ept; ++ipt )
          if( pdt[ipt] == "RGBA" )
          {
            dt = "RGBA";
            break;
          }
          else if( pdt[ipt] == "RGB" )
            dt = "RGB";
        if( dt.empty() && f.dataType() == "RGB" )
          dt = f.dataType();
      }
      if( dt.empty() )
        return pal;

      pal.reset( new APalette( palettename ) );

      string      format = f.format();

      if( dt == "RGBA" )
        try
        {
          //cout << "trying to read " << filename << " as RGBA...\n";
          Reader<AimsData<AimsRGBA> > dr( filename );
          dr.read( *pal, 0, &format );
          pal->update();
        }
        catch( exception & )
        {
        }
      else
        // try to load as RGB
        try
        {
          //cout << "failed. Trying as RGB...\n";
          Reader<AimsData<AimsRGB> >    dr2( filename );
          AimsData<AimsRGB>                     tpal;
          dr2.read( tpal, 0, &format );
          // copy to RGBA array
          AimsData<AimsRGBA>    tpal2( tpal.dimX(), tpal.dimY(),
                                        tpal.dimZ(), tpal.dimT() );
          long  x, y, z, t;
          ForEach4d( tpal, x, y, z, t )
            tpal2( x, y, z, t ) = tpal( x, y, z, t );
          (AimsData<AimsRGBA> &) *pal = tpal2;
          pal->setHeader( tpal.header()->cloneHeader() );
          pal->update();
        }
        catch( exception & )
        {
          //cout << "Failed\n";
        }
    }

  return pal;
}


void PaletteList::load( const string & dirname, bool clr )
{
  if( !dirname.empty() )
  {
    Directory		dir( dirname );
    set<string>		names = dir.files(), done;
    string::size_type	pos;
    set<string>::const_iterator	in, en = names.end();
    if ( clr )
      clear();
    bool newpal;

    for( in = names.begin(); in != en; ++in )
    {
      string palettename = *in;
      string filename = dirname + FileUtil::separator() + palettename;
      pos = palettename.rfind( '.' );
      if ( pos != string::npos )
        // remove extension
        palettename.erase( pos, palettename.length() - pos );
      set<string>::iterator  ff = done.find( palettename );
      if( ff != done.end() )
        continue; // already read
      done.insert( palettename );

      rc_ptr<APalette> pal = loadPalette( filename, palettename );
      if( pal )
      {
        rc_ptr<APalette> oldpal = find( palettename );
        if( !oldpal )
          _pal.push_back( pal );
        else
          oldpal = pal;
      }
    }
  }

  if( palettes().empty() )
  {
    // create a default greyscale palette
    rc_ptr<APalette> pal( new APalette( "B-W LINEAR", 256 ) );
    for( int i=0; i<256; ++i )
      (*pal)[i] = AimsRGBA( i, i, i, 255 );
    _pal.push_back( pal );
  }
}


void PaletteList::save( const string & dirname, bool bin ) const
{
  list<rc_ptr<APalette> >::const_iterator	ip, fp=_pal.end();
  string 				name;
  string::size_type			pos;

  for( ip=_pal.begin(); ip!=fp; ++ip )
    {
      name = (*ip)->name();
#ifndef _WIN32
      while( ( pos = name.find( '/' ) ) != string::npos )
	name.replace( pos, 1, "\\" );
#endif
      name = dirname + '/' + name;
      cout << "writing " << name << endl;
      Writer<AimsData<AimsRGBA> >	dw( name );
      dw.write( **ip, !bin );
    }
}


const rc_ptr<APalette> PaletteList::find( const string & name ) const
{
  list<rc_ptr<APalette> >::const_iterator	ip, fp=_pal.end();
  string				lname = stringLower( name );

  for( ip=_pal.begin(); ip!=fp; ++ip )
    if( stringLower( (*ip)->name() ) == lname )
      return( *ip );

  return rc_ptr<APalette>();
}


rc_ptr<APalette> PaletteList::find( const string & name )
{
  list<rc_ptr<APalette> >::iterator	ip, fp=_pal.end();
  string			lname = stringLower( name );

  for( ip=_pal.begin(); ip!=fp; ++ip )
    if( stringLower( (*ip)->name() ) == lname )
      return( *ip );

  return rc_ptr<APalette>();
}


void PaletteList::push_back( rc_ptr<APalette> pal )
{
  _pal.push_back( pal );
}
