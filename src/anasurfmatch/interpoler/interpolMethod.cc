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


#include <anatomist/interpoler/interpolMethod.h>
#include <anatomist/interpoler/interpoler.h>
#include <anatomist/surface/triangulated.h>
#include <anatomist/surface/texsurface.h>
#include <qobject.h>

using namespace anatomist;
using namespace std;


AInterpolerMethod::~AInterpolerMethod()
{
}

string AInterpolerMethod::ID() const
{
  return( QT_TRANSLATE_NOOP( "FusionChooser", "Interpoler" ) );
}

int AInterpolerMethod::canFusion( const set<AObject *> & obj )
{
  if( obj.size() != 2 )
    return 0;

  set<AObject *>::const_iterator	io = obj.begin();
  AObject				*o1, *o2;
  ATriangulated				*go1;
  ATexSurface				*go2;

  o1 = *io;
  ++io;
  o2 = *io;
  go1 = dynamic_cast<ATriangulated *>( o1 );
  if( !go1 )
    {
      go1 = dynamic_cast<ATriangulated *>( o2 );
      AObject	*o = o1;
      o1 = o2;
      o2 = o;
    }
  if( !go1 )
    return 0;

  go2 = dynamic_cast<ATexSurface *>( o2 );

  if( go2 )
    return 18;
  else
    return 0;
}


AObject* AInterpolerMethod::fusion( const vector<AObject *> & obj )
{
  vector<AObject *>::const_iterator	io = obj.begin();
  AObject				*o = *io;

  ++io;
  return( new AInterpoler( o, *io ) );
}
