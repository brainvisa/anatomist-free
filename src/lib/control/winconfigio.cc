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


#include <anatomist/control/winconfigio.h>
#include <anatomist/window/Window.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/window/winFactory.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/processor/Serializer.h>
#include <graph/tree/tree.h>
#include <graph/tree/twriter.h>
#include <vector>


using namespace anatomist;
using namespace std;


AWinConfigIO::AWinConfigIO()
{
}


AWinConfigIO::~AWinConfigIO()
{
}


void AWinConfigIO::saveConfig( const string & filename ) const
{
  ofstream				file( filename.c_str() );

  if( !file )
    {
      cerr << "cannot open " << filename << endl;
      return;
    }

  Tree					t( true, "EXECUTE" );
  set<AWindow *>			win = theAnatomist->getWindows();
  set<AWindow *>::const_iterator	iw, fw = win.end();
  Tree					*tr;
  AWindow				*w;
  vector<int>				dims;
  unsigned				wd, ht;
  TreeWriter				tw( Registry::instance()->syntax() );
  Serializer				ser;

  tw.attach( file );

  dims.push_back( 0 );
  dims.push_back( 0 );
  dims.push_back( 0 );
  dims.push_back( 0 );
  tr = new Tree( true, "CreateWindow" );
  t.insert( tr );

  for( iw=win.begin(); iw!=fw; ++iw )
    {
      w = *iw;
      tr->setProperty( "type", AWindowFactory::typeString( w->type(), 
							  w->subtype() ) );
      tr->setProperty( "res_pointer", ser.serialize( w ) );
      w->geometry( &dims[0], &dims[1], &wd, &ht );
      dims[2] = (int) wd;
      dims[3] = (int) ht;
      tr->setProperty( "geometry", dims );
      tw << t;
    }
}
