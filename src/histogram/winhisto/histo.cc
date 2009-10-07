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


#include <anatomist/winhisto/histo.h>
#include <anatomist/object/objectConverter.h>
#include <aims/histogram/simpleHisto.h>
#include <aims/utility/converter_volume.h>

using namespace anatomist;
using namespace carto;


QAHistogram::QAHistogram()
  : pmin( std::numeric_limits<float>::max() ),
    pmax( std::numeric_limits<float>::min() ),
    pdim( -1 )
{
}

QAHistogram::~QAHistogram()
{
}


bool QAHistogram::add( AObject* d )
{
  rc_ptr<AimsData< float > > adata
      = ObjectConverter< AimsData< float > >::ana2aims( d );

  if ( !adata )
    {
      rc_ptr<AimsData< int8_t > > a8
          = ObjectConverter< AimsData< int8_t > >::ana2aims( d );

      if ( a8 )
	{
	  adata.reset( new AimsData< float >( a8->dimX(), a8->dimY(),
                       a8->dimZ(), a8->dimT() ) );
	  carto::RawConverter< AimsData< int8_t >, AimsData< float > > conv;
	  conv.convert( *a8, *adata );
	}
      else
	{
	  rc_ptr<AimsData< uint8_t > > au8;
	  au8 = ObjectConverter< AimsData< uint8_t > >::ana2aims( d );

	  if ( au8 )
	    {
	      adata.reset( new AimsData< float >( au8->dimX(), au8->dimY(),
                           au8->dimZ(), au8->dimT() ) );
	      carto::RawConverter< AimsData< uint8_t >, AimsData< float > > conv;
	      conv.convert( *au8, *adata );
	    }
	  else
	    {
	      rc_ptr<AimsData< int16_t > > a16;
	      a16 = ObjectConverter< AimsData< int16_t > >::ana2aims( d );

	      if ( a16 )
		{
		  adata.reset( new AimsData< float >( a16->dimX(), a16->dimY(),
                               a16->dimZ(), a16->dimT() ) );
		  carto::RawConverter< AimsData< int16_t >, AimsData< float > > conv;
		  conv.convert( *a16, *adata );
		}
	      else
		{
		  rc_ptr<AimsData< uint16_t > > au16;
		  au16 = ObjectConverter< AimsData< uint16_t > >::ana2aims( d );

		  if ( au16 )
		    {
		      adata.reset( new AimsData< float >( au16->dimX(),
                                   au16->dimY(), au16->dimZ(),
                                              au16->dimT() ) );
		      carto::RawConverter< AimsData< uint16_t >, AimsData< float > > conv;
		      conv.convert( *au16, *adata );
		    }
		  else
		    {
		      rc_ptr<AimsData< int32_t > > a32;
		      a32 = 
			ObjectConverter< AimsData< int32_t > >::ana2aims( d );

		      if ( a32 )
			{
			  adata.reset( new AimsData< float >( a32->dimX(),
                                       a32->dimY(), a32->dimZ(),
                                           a32->dimT() ) );
			  carto::RawConverter< AimsData< int32_t >, AimsData< float > > conv;
			  conv.convert( *a32, *adata );
			}
		      else
			{
			  rc_ptr<AimsData< uint32_t > > au32;
			  au32 = 
			    ObjectConverter< AimsData< uint32_t > >::ana2aims( d );

			  if ( au32 )
			    {
			      adata.reset( new AimsData< float >(
                                           au32->dimX(), au32->dimY(),
                                  au32->dimZ(), au32->dimT() ) );
			      carto::RawConverter< AimsData< uint32_t >, AimsData< float > > conv;
			      conv.convert( *au32, *adata );
			    }
			}
		    }
		}
	    }
	}
    }

  if ( adata )
    {
      float bmin = adata->minimum();
      float bmax = adata->maximum();

      if ( bmin < pmin )
	pmin = bmin;

      if ( bmax > pmax )
	pmax = bmax;

      pdim = (int64_t)( pmax - pmin + 1.0f );
      crv.insert( std::make_pair( d, adata ) );

      return true;
    }

  return false;
}


void QAHistogram::remove( AObject* obj )
{
  crv.erase( obj );
}


double *QAHistogram::abscisse()
{
  double *x = new double[ pdim ];

  for ( int64_t t=0; t<pdim; t++ )
    x[ t ] = (double)t + pmin;

  return x;
}


double *QAHistogram::doit( AObject *d )
{
  SimpleHistogram< float > h;
  rc_ptr<AimsData< float > > adata = crv.find( d )->second;

  if ( adata )
    {

      h.doit( *adata );
      float bmin = (float)h.minValid();
      // float bmax = (float)h.maxValid();
      int64_t dX = pdim; // (int64_t)( bmax - bmin + 1.0f );
      if( dX > h.data().dimX() )
        dX = h.data().dimX(); // should rather rescale instead...
      // why this conversion/copy from SimpleHistogram to this ??
      double *y = new double[ pdim ];
      double *yptr = y + (int64_t)bmin - (int64_t)pmin;
      int64_t i;

      for( i = 0; i < dX; i++ )
        *yptr++ = h.data()( i );
      for( ; i < pdim; ++i )
        *yptr++ = 0;

      return y;
    }

  return NULL;
}
