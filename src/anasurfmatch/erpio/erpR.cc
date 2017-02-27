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


#include <anatomist/erpio/erpR.h>
#include <anatomist/object/oReader.h>
#include <anatomist/surface/texture.h>
#include <aims/mesh/texture.h>
#include <aims/io/byteswap.h>
#include <fstream>
#include <iomanip>
#include <stdio.h>


using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;


bool ErpReader::initialized = registerLoader();


bool ErpReader::registerLoader()
{
  ObjectReader::registerLoader( "erp", readErp );
  return( true );
}


ErpReader::ErpReader( const string & filename ) : _filename( filename )
{
}


ErpReader & ErpReader::operator >> ( ATexture & tex )
{
  string	filename = _filename;
  FILE	*f = fopen( filename.c_str(), "rb" );
  if( !f )
    {
      cerr << "Cannot open " << filename << endl;
      return( *this );
    }

  string	hdrname = filename;
  string::size_type	pos = hdrname.rfind( '/' );

  if( pos == string::npos )
    pos = 0;
  else
    ++pos;
  // keep only dirname
  hdrname.erase( pos, hdrname.length() - pos );
  hdrname += "erp.hdr";

  ifstream	hf( hdrname.c_str() );
  if( !hf )
    {
      cerr << "Cannot open header file " << hdrname << endl;
      fclose( f );
      return( *this );
    }

  string	format;
  bool		le, natle;

  float		dummy1 = 1.236e7, dummy2;
  unsigned char	*cd1 = reinterpret_cast<unsigned char *>( &dummy1 );
  unsigned	*cd2 = reinterpret_cast<unsigned *>( &dummy2 );
  unsigned	num;

  num = ((*cd1) << 24) + ((*cd1+1) << 16) + ((*cd1+2) << 8) + (*cd1+3);
  *cd2 = num;

  if( dummy1 == dummy2 )
    {
      natle = false;
      //cout << "native format : big endian\n";
    }
  else
    {
      //cout << "native format: little endian\n";
      natle = true;
    }

  hf >> format;
  if( format == "ieee-le" )
    {
      //cout << "data: little endian\n";
      le = true;
    }
  else if( format == "ieee-be" )
    {
      //cout << "data: big endian\n";
      le = false;
    }
  else
    {
      cerr << "warning : format not recognized, using native format\n";
      le = natle;
    }

  unsigned	dimx, dimt;

  hf >> dimx >> dimt;
  cout << "dimx : " << dimx << "\ndimt : " << dimt << endl;
  hf.close();

  rc_ptr<Texture1d>	tx( new Texture1d );
  unsigned		t, i;
  float			value;

  cd1 = (unsigned char *) &value;

  for( t=0; t<dimt; ++t )
    (*tx)[t].reserve( dimx );

  size_t nr;
  for( i=0; i<dimx; ++i )
    for( t=0; t<dimt; ++t )
      {
	nr = fread( &value, sizeof( float ), 1, f );
	if( le != natle )
	  value = byteswap( value );
	(*tx)[t].push_back( value );
      }

  fclose( f );

  tex.setTexture( tx );
  tex.normalize();
  tex.setFileName( filename.c_str() );
  tex.setChanged();

  return( *this );
}


list<AObject*> ErpReader::readErp( const string & filename,
                             ObjectReader::PostRegisterList &,
                             Object options )
{
  if( !options.isNull() )
    try
      {
        Object restricted = options->getProperty( "restrict_object_types" );
        if( !restricted->hasProperty( "Texture" ) )
          return list<AObject *>();
      }
    catch( ... )
      {
      }

  ATexture	*obj = new ATexture;
  ErpReader	er( filename );

  er >> *obj;

  if( obj->size() == 0 )
    {
      delete obj;
      return list<AObject *>();
    }
  list<AObject *> objs;
  objs.push_back( obj );
  return( objs );
}
