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


#include <anatomist/selection/selector.h>
#include <anatomist/object/Object.h>
#include <anatomist/window/Window.h>
#include <anatomist/selection/selectFactory.h>
#include <anatomist/selection/wSelChooser.h>
#include <anatomist/mobject/MObject.h>


using namespace anatomist;
using namespace std;


Selector::Selector()
{
}


Selector::~Selector()
{
}


AObject* Selector::objectAt( AObject* o, const Point3df & pos, float t, 
			     float tolerence, const Referential* wref, 
			     const Point3df & wgeom ) const
{
  return( o->ObjectAt( pos[0], pos[1], pos[2], t, tolerence, wref, wgeom ) );
}


// --------------------


LowestLevelSelector::LowestLevelSelector()
{
}


LowestLevelSelector::~LowestLevelSelector()
{
}


AObject* LowestLevelSelector::objectAt( AObject* o, const Point3df & pos, 
					float t, float tolerence, 
					const Referential* wref, 
					const Point3df & wgeom ) const
{
  AObject	*so = o->ObjectAt( pos[0], pos[1], pos[2], t, tolerence, wref, 
				   wgeom );
  if( so )
    {
      MObject	*mo = dynamic_cast<MObject *>( so );
      if( mo )
	return( objectAt( mo, pos, t, tolerence, wref, wgeom ) );
    }
  return( so );
}
