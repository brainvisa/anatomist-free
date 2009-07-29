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

#include <anatomist/commands/cChangePalette.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/color/palette.h>
#include <anatomist/color/paletteList.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/processor/unserializer.h>
#include <cartobase/object/syntax.h>
#include <graph/tree/tree.h>

using namespace anatomist;
using namespace carto;
using namespace std;


ChangePaletteCommand::ChangePaletteCommand( const string & name, 
					    const vector<AimsRGBA> & cols )
  : RegularCommand(), _palname( name ), _colors( cols )
{
}


ChangePaletteCommand::~ChangePaletteCommand()
{
}


bool ChangePaletteCommand::initSyntax()
{
  SyntaxSet	ss;
  Syntax	& s = ss[ "ChangePalette" ];

  s[ "name"       ].type = "string";
  s[ "name"       ].needed = true;
  s[ "colors"     ].type = "int_vector";
  s[ "colors"     ].needed = true;
  s[ "color_mode" ].type = "string";
  s[ "color_mode" ].needed = true;

  Registry::instance()->add( "ChangePalette", &read, ss );
  return( true );
}


void ChangePaletteCommand::doit()
{
  PaletteList	& pall = theAnatomist->palettes();
  rc_ptr<APalette> pal = pall.find( _palname );
  unsigned	i, n;

  if( !pal )
    {
      cerr << "ChangePaletteCommand : palette not found\n";
      return;
    }

  AimsData<AimsRGBA>	dat( _colors.size() );

  for( i=0, n=_colors.size(); i<n; ++i )
    dat[i] = _colors[i];
  pal->AimsData<AimsRGBA>::operator = ( dat );
}


Command* ChangePaletteCommand::read( const Tree & com, CommandContext* )
{
  string		name, cmode;
  vector<int>		cols;
  vector<AimsRGBA>	colors;
  int			i, j, n;

  com.getProperty( "name", name );
  com.getProperty( "colors", cols );
  com.getProperty( "color_mode", cmode );

  if( cmode == "RGBA" )
    for( i=0, n=cols.size()/4, j=0; i<n; ++i, j+=4 )
      colors.push_back( AimsRGBA( cols[j], cols[j+1], cols[j+2], cols[j+3] ) );
  else
    for( i=0, n=cols.size()/3, j=0; i<n; ++i, j+=3 )
      colors.push_back( AimsRGBA( cols[j], cols[j+1], cols[j+2], 255 ) );

  return( new ChangePaletteCommand( name, colors ) );
}


void ChangePaletteCommand::write( Tree & com, Serializer* ) const
{
  Tree		*t = new Tree( true, name() );
  vector<int>	cols;
  int		i, n;

  t->setProperty( "name", _palname );
  t->setProperty( "color_mode", string( "RGBA" ) );
  for( i=0, n=_colors.size(); i<n; ++i )
    {
      cols.push_back( _colors[i].red() );
      cols.push_back( _colors[i].green() );
      cols.push_back( _colors[i].blue() );
      cols.push_back( _colors[i].alpha() );
    }
  t->setProperty( "colors", cols );

  com.insert( t );
}
