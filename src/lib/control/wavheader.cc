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

#include "wavheader.h"
#include <fstream>
#include <stdexcept>

using namespace audiq;
using namespace std;


WavHeader::WavHeader() 
  : channels( 0 ), size( 0 ), sampleSize( 0 ), sign( true ), headerOffset( 44 )
{
}


WavHeader::~WavHeader()
{
}


void WavHeader::read( const std::string & fname )
{
  ifstream	file( fname.c_str(), ios::in | ios::binary );
  if( !file )
    throw runtime_error( string( "cannot open file " ) + fname );
  read( file, fname );
}


void WavHeader::read( istream & file, const string & fname )
{
  filename = fname;
  headerOffset = 44;

  char	buf[10];
  file.read( buf, 4 );
  buf[4] = '\0';
  if( string( "RIFF" ) != buf )
    throw runtime_error( fname + " : not a .WAV file " );
  unsigned char	c1, c2, c3, c4;
  file.read( (char *) &c1, 1 );
  file.read( (char *) &c2, 1 );
  file.read( (char *) &c3, 1 );
  file.read( (char *) &c4, 1 );
  size_t	len = (((size_t) c4) << 24 ) | (((size_t) c3) << 16 ) 
    | (((size_t) c2) << 8 ) | c1;
  cout << "len (bytes): " << len << endl;
  file.read( buf, 8 );
  buf[8] = '\0';
  if( string( "WAVEfmt " ) != buf )
    throw runtime_error( fname + " : not a .WAV file " );
  file.read( (char *) &c1, 1 );
  file.read( (char *) &c2, 1 );
  file.read( (char *) &c3, 1 );
  file.read( (char *) &c4, 1 );
  unsigned short	x = (((size_t) c4) << 24 ) | (((size_t) c3) << 16 ) 
    | (((size_t) c2) << 8 ) | c1;
  if( x != 0x10 )
    cerr << "warning: WAV length of FORMAT chunk is incorrect\n";
  file.read( (char *) &c1, 1 );
  file.read( (char *) &c2, 1 );
  x = (((unsigned short) c2) << 8 ) | c1;
  if( x != 1 )
    cerr << "warning: WAV format malformed, FORMAT chunk, offset 8-9 value: "
	 << x << ", should be 1\n";
  file.read( (char *) &c1, 1 );
  file.read( (char *) &c2, 1 );
  x = (((unsigned short) c2) << 8 ) | c1;
  channels = x;
  cout << "channels: " << channels << endl;
  file.read( (char *) &c1, 1 );
  file.read( (char *) &c2, 1 );
  file.read( (char *) &c3, 1 );
  file.read( (char *) &c4, 1 );
  len = (((size_t) c4) << 24 ) | (((size_t) c3) << 16 ) 
    | (((size_t) c2) << 8 ) | c1;
  rate = len;
  cout << "rate: " << rate << endl;
  file.read( (char *) &c1, 1 );
  file.read( (char *) &c2, 1 );
  file.read( (char *) &c3, 1 );
  file.read( (char *) &c4, 1 );
  len = (((size_t) c4) << 24 ) | (((size_t) c3) << 16 ) 
    | (((size_t) c2) << 8 ) | c1;
  cout << "bytes/sec: " << len << endl;
  file.read( (char *) &c1, 1 );
  file.read( (char *) &c2, 1 );
  x = (((unsigned short) c2) << 8 ) | c1;
  cout << "bytes/sample: " << x << endl;
  file.read( (char *) &c1, 1 );
  file.read( (char *) &c2, 1 );
  x = (((unsigned short) c2) << 8 ) | c1;
  cout << "bits/channel: " << x << endl;
  sampleSize = x/8;
  file.read( buf, 4 );
  buf[4] = '\0';
  if( string( "data" ) != buf )
    throw runtime_error( fname + " : not a .WAV file " );
  file.read( (char *) &c1, 1 );
  file.read( (char *) &c2, 1 );
  file.read( (char *) &c3, 1 );
  file.read( (char *) &c4, 1 );
  len = (((size_t) c4) << 24 ) | (((size_t) c3) << 16 ) 
    | (((size_t) c2) << 8 ) | c1;
  len /= sampleSize * channels;
  cout << "len in samples: " << len << endl;
  size = len;
}


void WavHeader::write( const std::string & fname )
{
  ofstream	file( fname.c_str(), ios::out | ios::binary );
  if( !file )
    throw runtime_error( string( "cannot write file " ) + fname );
  write( file );
}


void WavHeader::write( ostream & file )
{
  file << "RIFF";
  unsigned	len = size * sampleSize * channels + 36;
  file.put( len & 0xff );
  file.put( ( len >> 8 ) & 0xff );
  file.put( ( len >> 16 ) & 0xff );
  file.put( len >> 24 );
  file << "WAVEfmt ";
  file.put( 0x10 );
  file.put( 0 );
  file.put( 0 );
  file.put( 0 );
  file.put( 1 );
  file.put( 0 );
  file.put( channels & 0xff );
  file.put( ( channels >> 8 ) & 0xff );	// should be 0
  len = rate;
  file.put( len & 0xff );
  file.put( ( len >> 8 ) & 0xff );
  file.put( ( len >> 16 ) & 0xff );
  file.put( len >> 24 );
  len = rate * sampleSize * channels;	// bytes / sec
  file.put( len & 0xff );
  file.put( ( len >> 8 ) & 0xff );
  file.put( ( len >> 16 ) & 0xff );
  file.put( len >> 24 );
  len = sampleSize * channels;		// bytes / sample
  file.put( len & 0xff );
  file.put( ( len >> 8 ) & 0xff );
  len = sampleSize * 8;			// bits / sample
  file.put( len & 0xff );
  file.put( ( len >> 8 ) & 0xff );
  file << "data";
  len = size * sampleSize * channels;	// len of samples
  file.put( len & 0xff );
  file.put( ( len >> 8 ) & 0xff );
  file.put( ( len >> 16 ) & 0xff );
  file.put( len >> 24 );
}

