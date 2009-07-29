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

#ifndef AUDIQ_CORE_DIFFCODE_H
#define AUDIQ_CORE_DIFFCODE_H

#include "wavheader.h"
#include <string>
#include <iostream>

namespace audiq
{
  struct WavHeader;

  class DiffCode
  {
  public:
    /// Compression information
    struct Compression
    {
      Compression( unsigned b = 0, size_t s = 0 );

      unsigned	bits;
      size_t	size;
    };


    /** Position in a compressed stream.
        This structure holds all needed information to allow decoding at the 
        selected position in the input stream. It includes data currently 
        in decoding phase for each audio track.
    */
    struct CompressedPos
    {
      CompressedPos( size_t p = 0, unsigned nchannels = 1 );
      CompressedPos( const CompressedPos & );
      ~CompressedPos();
      CompressedPos & operator = ( const CompressedPos & );

      size_t	pos;
      struct Internal;
      Internal	*d;
    };


    /// DiffCode compressed file information (including the .wav header)
    struct CompressInfo
    {
      CompressInfo( unsigned b = 0 );
      void read( std::istream & s, const std::string & filename = "" );

      unsigned	bits;
      WavHeader	hdr;
    };

    static Compression checkCompress( const std::string & filename, 
                                      bool verbose = false );
    static Compression checkCompress( const char* buf, const WavHeader & hdr, 
                                      bool verbose = false );
    static Compression checkCompress( std::istream & f, const WavHeader & hdr, 
                                      bool verbose = false );
    static Compression compress( const std::string & ifile, 
                                 const std::string & ofile, unsigned bits = 0, 
                                 bool verbose = false );
    static Compression compress( const char* buf, const WavHeader & hdr, 
                                 const std::string & ofile, unsigned bits = 0, 
                                 bool verbose = false );
    static Compression compress( std::istream & f, const WavHeader & hdr, 
                                 const std::string & ofile, unsigned bits = 0, 
                                 bool verbose = false );
    static WavHeader uncompress( const std::string & filename, char* buf, 
                                 size_t bufsize, size_t start = 0, 
                                 int len = -1 );
    static WavHeader uncompress( const std::string & filename, 
                                 const std::string & wavfile, 
                                 size_t start = 0, int len = -1 );
    static CompressedPos uncompress( const char* ibuf, 
                                     const CompressInfo & info, char* buf, 
                                     const CompressedPos & pos 
                                     = CompressedPos(), int len = -1 );
    static CompressedPos uncompress( std::istream & s, 
                                     const CompressInfo & info, 
                                     char* buf, const CompressedPos & pos = 
                                     CompressedPos(), int len = -1 );
  };

}

#endif

