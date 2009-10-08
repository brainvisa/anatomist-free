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

#include <cstdlib>
#include "diffcode.h"
#include <fstream>
#include <vector>
#include <iomanip>
#include <stdexcept>

using namespace audiq;
using namespace std;

DiffCode::Compression::Compression( unsigned b, size_t s )
: bits( b ), size( s )
{
}


DiffCode::Compression DiffCode::checkCompress( const std::string & filename, 
                                               bool verbose )
{
  WavHeader	hdr;
  hdr.read( filename );
  ifstream	f( filename.c_str(), ios::in | ios::binary );
  if( !f )
    return Compression();
  f.unsetf( ios::skipws );
  f.seekg( hdr.headerOffset );
  return checkCompress( f, hdr, verbose );
}


namespace
{

  template<typename T> int _readOne( T & stream, unsigned nbytes, 
                                     bool sign, unsigned bo );

  template<typename T> inline unsigned char _read8( T & stream );


  template<> int _readOne( istream & s, unsigned nbytes, bool sign, unsigned )
  {
    int			x;

    if( sign )
      switch( nbytes )
        {
        case 2:
          {
            short	y;
            s.read( (char *) &y, 2 );
            x = y;
          }
          break;
        case 4:
          s.read( (char *) &x, 4 );
          break;
        default:
          {
            signed char	y;
            s.read( (char *) &y, 1 );
            x = y;
          }
        }
    else
      switch( nbytes )
        {
        case 2:
          {
            unsigned short	y;
            s.read( (char *) &y, 2 );
            x = y;
          }
          break;
        case 4:
          s.read( (char *) &x, 4 );
          break;
        default:
          {
            unsigned char	y;
            s.read( (char *) &y, 1 );
            x = y;
          }
        }
    return x;
  }


  template<> int _readOne( const char*& buf, unsigned nbytes, bool sign, 
                           unsigned )
  {
    int	x;
    if( sign )
      switch( nbytes )
        {
        case 2:
          x = *(short *) buf;
          buf += 2;
          break;
        case 4:
          x = *(unsigned *) buf;
          buf += 4;
          break;
        default:
          x = *(signed char *) buf;
          ++buf;
        }
    else
      switch( nbytes )
        {
        case 16:
          x = *(unsigned short *) buf;
          buf += 2;
          break;
        case 32:
          x = *(unsigned *) buf;
          buf += 4;
          break;
        default:
          x = *(unsigned char *) buf;
          ++buf;
        }
    return x;
  }


  template<> inline unsigned char _read8( istream & s )
  {
    unsigned char	x = s.get();
    return x;
  }


  template<> inline unsigned char _read8( const char* & buf )
  {
    return *buf++;
  }


  template<typename T> inline unsigned _read32( T & s )
  {
    return ( _read8( s ) << 24 ) | ( _read8( s ) << 16 ) 
      | ( _read8( s ) << 8 ) | _read8( s );
  }


  template<typename T>
  DiffCode::Compression _checkCompress( T & stream, const WavHeader & hdr, 
                                        bool verbose )
  {
    size_t		i;
    unsigned		x, j;
    unsigned		n = ( hdr.size - 1 ) * hdr.channels;
    unsigned		m = (n+1) * hdr.sampleSize;
    vector<unsigned>	last( hdr.channels );
    size_t		stats[ 32 ];
    const unsigned	nb = 32;
    int			c, y;
    unsigned		mask = ( 1 << (hdr.sampleSize*8) ) - 1, 
      test = 1 << (hdr.sampleSize*8-1), mask2 = ~mask;

    for( j=0; j<nb; ++j )
      stats[j] = 0;

    for( i=0; i<hdr.size; ++i )
      for( c=0; c<hdr.channels; ++c )
        {
          x = _readOne( stream, hdr.sampleSize, hdr.sign, 0 );
          if( i == 0 )
            last[ c ] = x;
          else
            {
              y = ( x - last[c] ) & mask;
              if( y & test )
                y |= mask2;
              y = abs( y );
              last[c] = x;
              for( j=0; j<nb; ++j )
                if( y < ( 1<<j ) )
                  ++stats[j];
            }
        }
    unsigned	w = (unsigned) ::ceil( log10( (float) n - 1 ) );
    unsigned	sz, imin = 0, msz = 0;

    for( j=0; j<nb; ++j )
      {
        sz = (unsigned) ::ceil( n * float(j+1)/8 )
          + ( n - stats[j] ) * hdr.sampleSize 
          + hdr.sampleSize * hdr.channels;
        if( sz < msz || imin == 0 )
          {
            imin = j+1;
            msz = sz;
          }

        if( verbose )
          cout << setw( 2 ) << j+1 << "  : " << setw( w ) << stats[j] << " - " 
               << setw( 5 ) << setprecision( 4 ) << 100. * stats[j] / n 
               << " % - final size : " << setw( w ) << sz << " - rate: " 
               << float(sz) / m << endl;
        if( stats[j] == n )
          break;
      }

    if( msz >= m )
      {
        imin = 0;
        msz = n + 1;
      }

    if( verbose )
    {
      if( imin == 0 )
        cout << "Can't compress efficiently this file\n";
      else
        cout << "best: " << imin << " bits coding : " << msz << " bytes\n";
    }
    DiffCode::Compression	dc( imin, msz );
    return dc;
  }


  unsigned _writeraw( unsigned x, ostream & f, unsigned ssz )
  {
    unsigned char	cb = x & 0xff;
    f.write( (char *) &cb, 1 );
    if( ssz >= 2 )
      {
        cb = (x>>8) & 0xff;
        f.write( (char *) &cb, 1 );
        if( ssz >= 3 )
          {
            cb = (x>>16) & 0xff;
            f.write( (char *) &cb, 1 );
            if( ssz >= 4 )
              {
                cb = (x>>24) & 0xff;
                f.write( (char *) &cb, 1 );
              }
          }
      }
    return ssz;
  }


  unsigned _flush( unsigned & code, unsigned & bit, ostream & f )
  {
    unsigned char	c;
    unsigned		l = 0;

    while( bit >= 8 )
      {
        c = code >> 24;
        f.write( (char *) &c, 1 );
        code = code << 8;
        bit -= 8;
        ++l;
      }
    if( bit != 0 )
      {
        c = code >> 24;
        f.write( (char *) &c, 1 );
        code = code << 8;
        bit -= 8;
        ++l;
        bit = 0;
      }
    return l;
  }

  unsigned _code( int y, unsigned bits, unsigned & code, unsigned & bit, 
                  ostream & f )
  {
    unsigned char	c;
    unsigned		l = 0;
    int			bleft = 32 - bit;

    if( bleft < (int) bits )
      {
        bits -= bleft;
        _code( y >> bits, bleft, code, bit, f );
      }
    code |= ( y & ( ( 1 << bits ) - 1 ) ) << ( bleft - bits );
    bit += bits;

    while( bit >= 8 )
      {
        c = code >> 24;
        f.write( (char *) &c, 1 );
        code = code << 8;
        bit -= 8;
        ++l;
      }
    return l;
  }


  template<typename T> T* _memostream( T & stream );

  template<> istream* _memostream( istream & )
  {
    return 0;
  }

  template<> const char** _memostream( const char * & stream )
  {
    const char	** x = new const char*;
    *x = stream;
    return x;
  }

  template<typename T> void _rewind( T & stream, T* copy );

  template<> void _rewind( const char* & stream, const char** copy )
  {
    stream = *copy;
    delete copy;
  }


  template<> void _rewind( istream & stream, istream* )
  {
    stream.seekg( 44, ios::beg );
  }


  template<typename T>
  DiffCode::Compression _compress( T & stream, const WavHeader & hdr, 
                                   const string & ofile, unsigned bits, 
                                   bool verbose )
  {
    DiffCode::Compression	comp( bits, 0 );
    if( bits == 0 )
      {
        T	*copy = _memostream( stream );
        comp = _checkCompress( stream, hdr, verbose );
        if( comp.bits == 0 )
          return comp;
        _rewind( stream, copy );
      }

    ofstream	f( ofile.c_str(), ios::out | ios::binary );
    if( !f )
      return DiffCode::Compression();

    // write header

    f << "DiffCode  1.0\n";
    unsigned char	cb = comp.bits & 0xff;
    f.write( (char *) &cb, 1 );
    cb = comp.bits >> 8;
    f.write( (char *) &cb, 1 );
    WavHeader	h2( hdr );
    h2.write( f );

    // write data

    size_t		i, l = hdr.headerOffset + 16;
    unsigned		x, code = 0, bit = 0;
    vector<unsigned>	last( hdr.channels );
    int			c, y;
    unsigned		lim = 1 << ( comp.bits - 1 ), 
      mask = ( 1 << (hdr.sampleSize*8) ) - 1, 
      test = 1 << (hdr.sampleSize*8-1), mask2 = ~mask;

    for( i=0; i<hdr.size; ++i )
      for( c=0; c<hdr.channels; ++c )
        {
          x = _readOne( stream, hdr.sampleSize, hdr.sign, 0 );
          if( i == 0 )
            l += _writeraw( x, f, hdr.sampleSize );
          else
            {
              y = ( x - last[c] ) & mask;
              if( y & test )
                y |= mask2;
              if( ((unsigned) abs( y )) < lim )
                l += _code( y, comp.bits, code, bit, f );
              else
                {
                  //cout << "overflow: " << (int) x << "/" << y << endl;
                  l += _code( lim, comp.bits, code, bit, f );
                  // write complete sample
                  l += _code( x, hdr.sampleSize*8, code, bit, f );
                  /*
                  l += _flush( code, bit, f );
                  l += _writeraw( x, f, hdr.sampleSize );
                  */
                }
            }
          last[c] = x;
        }
    l += _flush( code, bit, f );

    if( verbose )
      {
        cout << "estimated size: " << comp.size << "; exact size: " << l 
             << "; ratio: " 
             << 100. * l / ( hdr.size * hdr.sampleSize * hdr.channels ) 
             << " % of original" << endl;
      }
    comp.size = l;
    return comp;
  }

}


DiffCode::Compression DiffCode::checkCompress( const char* buf, 
                                               const WavHeader & hdr, 
                                               bool verbose )
{
  return _checkCompress( buf, hdr, verbose );
}


DiffCode::Compression DiffCode::checkCompress( istream & f, 
                                               const WavHeader & hdr, 
                                               bool verbose )
{
  return _checkCompress( f, hdr, verbose );
}


DiffCode::Compression DiffCode::compress( const string & ifile, 
                                          const string & ofile, unsigned bits, 
                                          bool verbose )
{
  WavHeader	hdr;
  hdr.read( ifile );
  ifstream	f( ifile.c_str(), ios::in | ios::binary );
  if( !f )
    return Compression();
  f.unsetf( ios::skipws );
  f.seekg( hdr.headerOffset );
  return compress( f, hdr, ofile, bits, verbose );
}


DiffCode::Compression DiffCode::compress( const char* buf, 
                                          const WavHeader & hdr, 
                                          const string & ofile, unsigned bits, 
                                          bool verbose )
{
  return _compress( buf, hdr, ofile, bits, verbose );
}


DiffCode::Compression DiffCode::compress( istream & f, const WavHeader & hdr, 
                                          const string & ofile, unsigned bits, 
                                          bool verbose )
{
  return _compress( f, hdr, ofile, bits, verbose );
}


// -- uncompress

struct DiffCode::CompressedPos::Internal
{
  Internal( unsigned nchan );
  Internal( const Internal & );
  void setChannels( unsigned nchan );

  Internal & operator = ( const Internal & );

  unsigned		left;
  vector<unsigned>	lastvalue;
  unsigned		code;
};


DiffCode::CompressedPos::Internal::Internal( unsigned nchan )
  : left( 0 ), lastvalue( nchan ), code( 0 )
{
}


DiffCode::CompressedPos::Internal::Internal( const Internal & x )
  : left( x.left ), lastvalue( x.lastvalue ), code( x.code )
{
}


DiffCode::CompressedPos::Internal & 
DiffCode::CompressedPos::Internal::operator = ( const Internal & x )
{
  if( &x != this )
    {
      left = x.left;
      lastvalue = x.lastvalue;
      code = x.code;
    }
  return *this;
}


void DiffCode::CompressedPos::Internal::setChannels( unsigned nchan )
{
  left = 0;
  lastvalue = vector<unsigned>( nchan );
  code = 0;
}


DiffCode::CompressedPos::CompressedPos( size_t p, unsigned nchan )
  : pos( p ), d( new Internal( nchan ) )
{
}


DiffCode::CompressedPos::CompressedPos( const CompressedPos & x )
  : pos( x.pos ), d( new Internal( *x.d ) )
{
}


DiffCode::CompressedPos::~CompressedPos()
{
  delete d;
}


DiffCode::CompressedPos & 
DiffCode::CompressedPos::operator = ( const CompressedPos & x )
{
  if( &x != this )
    {
      pos = x.pos;
      *d = *x.d;
    }

  return *this;
}


DiffCode::CompressInfo::CompressInfo( unsigned b )
  : bits( b )
{
}


namespace
{

  template<typename T>
  unsigned _decode( T & s, unsigned bits, DiffCode::CompressedPos & pos, 
                    unsigned c, unsigned ssize )
  {
    unsigned	code = pos.d->code, left = pos.d->left, value = 0;

    while( left < bits )
      {
        code = ( code << 8 ) | _read8( s );
        ++pos.pos;
        left += 8;
      }

    value = ( code >> ( left - bits ) ) & ( ( 1 << bits ) - 1 );
    left -= bits;
    pos.d->code = code;
    pos.d->left = left;

    unsigned	zero = unsigned(1 << (bits-1) ), bsz = ssize * 8;
    if( value == zero )
      {
        while( left < bsz )
          {
            code = ( code << 8 ) | _read8( s );
            ++pos.pos;
            left += 8;
          }
        pos.d->lastvalue[c] 
          = ( code >> ( left - bsz ) ) & ( ( 1 << bsz ) - 1 );
        //cout << "long value: " << pos.d->lastvalue[c] << " " << left << endl;
        left -= bsz;
        pos.d->code = code;
        pos.d->left = left;
      }
    else
      {
        if( value < zero )
          pos.d->lastvalue[c] += value;
        else
          pos.d->lastvalue[c] 
            += int( value | ( -1 << bits ) );
      }
    return pos.d->lastvalue[c];
  }


  template<typename T>
  DiffCode::CompressedPos 
  _uncompress( T & s, const DiffCode::CompressInfo & info, char* buf, 
               const DiffCode::CompressedPos & p, int len )
  {
    int				i, c;
    unsigned			code;
    DiffCode::CompressedPos	pos( p );

    /* cout << "DiffCode::uncompress, pos: " << pos.pos << ", channels: " 
         << pos.d->code.size() << ", samplesize: " << info.hdr.sampleSize 
         << endl; */

    if( pos.pos == 0 )
      {
        //cout << "uncompress: pos 0\n";
        int		c;
        for( c=0; c<info.hdr.channels; ++c )
          {
            pos.d->lastvalue[c] 
              = _readOne( s, info.hdr.sampleSize, info.hdr.sign, 0 );
            pos.pos += info.hdr.sampleSize;

          if( buf )
            {
              code = pos.d->lastvalue[c];
              *buf++ = code & 0xff;
              if( info.hdr.sampleSize > 1 )
                {
                  *buf++ = (code >> 8) & 0xff;
                  if( info.hdr.sampleSize > 2 )
                    {
                      *buf++ = (code >> 16) & 0xff;
                      if( info.hdr.sampleSize > 3 )
                        *buf++ = (code >> 24) & 0xff;
                    }
                }
            }
          }
        --len;
    }

    for( i=0; i<len; ++i )
      for( c=0; c<info.hdr.channels; ++c )
        {
          code = _decode( s, info.bits, pos, c, info.hdr.sampleSize );

          //cout << i << ": code: " << (int) code << endl;
          if( buf )
            {
              *buf++ = code & 0xff;
              if( info.hdr.sampleSize > 1 )
                {
                  *buf++ = (code >> 8) & 0xff;
                  if( info.hdr.sampleSize > 2 )
                    {
                      *buf++ = (code >> 16) & 0xff;
                      if( info.hdr.sampleSize > 3 )
                        *buf++ = (code >> 24) & 0xff;
                    }
                }
            }
        }

    return pos;
  }

}


WavHeader DiffCode::uncompress( const string & filename, char* buf, 
                                size_t bufsize, size_t start, int len )
{
  ifstream	ifile( filename.c_str(), ios::in | ios::binary );
  ifile.unsetf( ios::skipws );
  CompressInfo		info;
  info.read( ifile, filename );
  WavHeader		& hdr = info.hdr;

  if( len < 0 || len > int( hdr.size ) - int( start ) )
    len = hdr.size - start;
  if( bufsize < unsigned( len ) * hdr.sampleSize * hdr.channels )
    len = bufsize / ( hdr.sampleSize * hdr.channels );

  CompressedPos	pos;

  pos = _uncompress( (istream &) ifile, info, 0, pos, start );
  _uncompress( (istream &) ifile, info, buf, pos, len );
  return hdr;
}


WavHeader DiffCode::uncompress( const string & filename, 
                                const string & wavfile, size_t start, int len )
{
  ifstream		ifile( filename.c_str(), ios::in | ios::binary );
  ifile.unsetf( ios::skipws );
  CompressInfo		info;
  info.read( ifile, filename );
  WavHeader		& hdr = info.hdr;

  ofstream		ofile( wavfile.c_str(), ios::out | ios::binary );
  const unsigned	block = 1000;
  vector<char>		buf( block * hdr.sampleSize * hdr.channels );

  if( len < 0 || len > int( hdr.size ) - int( start ) )
    len = hdr.size - start;
  hdr.write( ofile );

  CompressedPos	pos( 0, hdr.channels );
  unsigned	l;

  if( start != 0 )
    pos = _uncompress( (istream &) ifile, info, 0, pos, start );
  while( start < (size_t) len )
    {
      l = len - start;
      if( l > block )
        l = block;
      // cout << "uncompress " << start << endl;
      pos = uncompress( ifile, info, &buf[0], pos, l );
      //cout << "pos: " << pos.pos << ", chan: " << pos.d->code.size() << endl;
      start += l;
      ofile.write( &buf[0], l * hdr.sampleSize * hdr.channels );
      ofile.flush();
      //cout << "read pos: " << ifile.tellg() << endl;
    }
  return hdr;
}


DiffCode::CompressedPos 
DiffCode::uncompress( const char* ibuf, const CompressInfo & info, char* buf, 
                      const CompressedPos & pos, int len )
{
  return _uncompress( ibuf, info, buf, pos, len );
}


DiffCode::CompressedPos 
DiffCode::uncompress( istream & s, const CompressInfo & info, char* buf, 
                      const CompressedPos & pos, int len )
{
  return _uncompress( s, info, buf, pos, len );
}


void DiffCode::CompressInfo::read( istream & s, const string & fname )
{
  char	buf[14];
  s.read( buf, 13 );
  buf[13] = '\0';
  if( !s || s.get() != '\n' || string( "DiffCode  1.0" ) != buf )
    throw runtime_error( fname + " : not a DiffCode file" );

  bits = s.get() + ( s.get() << 8 );
  if( bits > 32 )
    throw runtime_error( fname + " : corrupt DiffCode file (bad bits base)" );
  hdr.read( s, fname );
}
