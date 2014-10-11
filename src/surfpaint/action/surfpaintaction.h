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


#ifndef SURFPAINT_ACTION_H
#define SURFPAINT_ACTION_H

#include <anatomist/controler/action.h>

class AWindow3D ;

namespace anatomist
{
  class AWindow ;
  class AGraph ;
  class AGraphObject ;
  class AObject ;
  class SurfpaintTools;

  class SurfpaintToolsAction : public Action
  {
    public:
      SurfpaintToolsAction() ;
      virtual ~SurfpaintToolsAction() ;

      // Action inputs
      virtual std::string name() const;
      static Action* creator();

      void pressRightButton( int x, int y, int globalX, int globalY  );
      void longLeftButtonStart( int x, int y, int globalX, int globalY  );
      void longLeftButtonMove( int x, int y, int globalX, int globalY  );
      void longLeftButtonStop( int x, int y, int globalX, int globalY  );

      void colorpicker( int x, int y, int globalX, int globalY  );

      void magicselection( int x, int y, int globalX, int globalY );

      void distanceMove ( int x, int y, int globalX, int globalY  );
      void distanceStart( int x, int y, int globalX, int globalY );
      void distanceStop( int x, int y, int globalX, int globalY );

      void brushMove ( int x, int y, int globalX, int globalY  );
      void brushStart( int x, int y, int globalX, int globalY );
      void brushStop( int x, int y, int globalX, int globalY );

      void magicbrushMove ( int x, int y, int globalX, int globalY  );
      void magicbrushStart( int x, int y, int globalX, int globalY );
      void magicbrushStop( int x, int y, int globalX, int globalY );

      void eraseMove ( int x, int y, int globalX, int globalY  );
      void eraseStart( int x, int y, int globalX, int globalY );
      void eraseStop( int x, int y, int globalX, int globalY );

      void shortestpathStart( int x, int y, bool newedit );
      void shortestpathStop( int x, int y, int globalX, int globalY  );
      void shortestpathClose( int x, int y, int globalX, int globalY  );

      void editValidate();
      void editCancel();
      void undo();
      void redo();

      float textureValue() const { return texvalue; }

      SurfpaintTools* getTools(){ return myTools; } ;
      void setupTools();

    private:

      AWindow3D *win3D;
      float texvalue;
      SurfpaintTools *myTools;
  };

}

#endif
