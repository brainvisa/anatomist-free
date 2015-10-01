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

#ifndef ANA_WINDOW_VIEWSTATE_H
#define ANA_WINDOW_VIEWSTATE_H

#include <aims/vector/vector.h>

namespace aims
{
  class Quaternion;
}


namespace anatomist
{
  class Referential;
  class Geometry;
  struct SliceViewState;
  class AWindow;


  /** ViewState holds information about how a view wants to see an object.

  The most common information is time because most (if not all) views want 
  to represent differently an object at different time positions. But for 
  some view/object combinations, more information will be needed to qualify 
  the way the object will show: a volume in a 2D window will only diaplay a 
  slice, so slice position and orientation will be needed in this case. More 
  specific information is possible by subclassing ViewState.

  ViewState objects are created by the views who ask for OpenGL display lists, 
  so the view must exactly know which information it can provide to the object 
  to have a specific display.
  */
  struct ViewState
  {
    enum glSelectRenderMode
    {
      /// no selection
      glSELECTRENDER_NONE,
      /// select a single object
      glSELECTRENDER_OBJECT,
      /// select multiple objects in a neighbourhood
      glSELECTRENDER_OBJECTS,
      /// select a polygon in a mesh
      glSELECTRENDER_POLYGON,
    };

    ViewState( float t = 0, AWindow* win = 0,
               glSelectRenderMode = glSELECTRENDER_NONE  );
    virtual ~ViewState();

    virtual SliceViewState *sliceVS() { return 0; }
    virtual const SliceViewState* sliceVS() const { return 0; }

    float	time;
    AWindow     *window;
    glSelectRenderMode selectRenderMode;
  };


  /** Specialization for a sliceable object. SliceViewState holds parameters 
      needed for 2D mode rendering
  */
  struct SliceViewState : public ViewState
  {
    SliceViewState( float t = 0, bool slicewanted = false, 
                    const Point3df & pos = Point3df(), 
                    const aims::Quaternion* orient = 0, 
                    const Referential* wref = 0, const Geometry* wgeom = 0,
                    const aims::Quaternion* vorient = 0, AWindow* win = 0,
                    glSelectRenderMode = glSELECTRENDER_NONE );
    virtual ~SliceViewState();

    virtual SliceViewState *sliceVS() { return this; }
    virtual const SliceViewState* sliceVS() const { return this; }

    bool		wantslice;
    Point3df		position;
    const aims::Quaternion	*orientation;
    const Referential	*winref;
    const Geometry	*wingeom;
    const aims::Quaternion *vieworientation;
  };

}

#endif



