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

#ifndef ANATOMIST_MODULE_SURFPAINTTOOLS_H
#define ANATOMIST_MODULE_SURFPAINTTOOLS_H

#include <anatomist/surface/texsurface.h>
#include <anatomist/surface/texture.h>
#include <anatomist/surface/surface.h>
#include <anatomist/surface/triangulated.h>
#include <anatomist/surface/glcomponent.h>
#include <aims/utility/converter_texture.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/object/objectConverter.h>
#include <anatomist/control/surfpaintcontrol.h>
#include <qwidget.h>
#include <qspinbox.h>
#include <aims/qtcompat/qhgroupbox.h>
#include <aims/qtcompat/qvgroupbox.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <aims/qtcompat/qvbox.h>
#include <aims/qtcompat/qhbox.h>
#include <aims/qtcompat/qbuttongroup.h>
#include <aims/qtcompat/qvbuttongroup.h>
#include <aims/qtcompat/qtoolbutton.h>
#include <aims/qtcompat/qbutton.h>
#include <qmessagebox.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qtooltip.h>
#include <qcombobox.h>
#include <qfiledialog.h>
#include <qaction.h>
#include <qmenubar.h>


class AWindow3D;

namespace aims
{
  class GeodesicPath;
}

namespace anatomist
{
  class SurfpaintTools: public QWidget
  {
    Q_OBJECT

    public:

      SurfpaintTools();
      virtual ~SurfpaintTools() ;

      bool initSurfPaintModule( AWindow3D *w3 );
      void addToolBarControls( AWindow3D *w3 );
      void removeToolBarControls( AWindow3D *w3 );
      void addToolBarInfosTexture( AWindow3D *w3 );
      void removeToolBarInfosTexture( AWindow3D *w3 );
      AObject* workingObject() const { return objselect; }

      void setPolygon(int p) { IDPolygonSpinBox->setValue(p); }
      void setMaxPoly(int max) { IDPolygonSpinBox->setRange(-1,max); }
      void setVertex(int v) { IDVertexSpinBox->setValue(v); }
      void setMaxVertex(int max) { IDVertexSpinBox->setRange(-1,max); }

      void setMinMaxTexture( float min, float max )
      { textureFloatSpinBox->setRange( min, max ); }
      float getTextureValueFloat() const
      { return textureFloatSpinBox->value(); }
      void setTextureValueFloat( double v )
      { textureFloatSpinBox->setValue(v); }
      void updateTextureValue( int indexVertex, float value );
      void updateTexture ( std::vector<float> values );
      void restoreTextureValue( int indexVertex );
      void floodFillStart( int indexVertex );
      void floodFillStop();
      void fastFillMove( int indexVertex, float newTextureValue,
                         float oldTextureValue );
      bool magicBrushStarted() const
      {
        return !listIndexVertexBrushPath.empty() || !listIndexVertexHolesPath.empty() || !holesObject.empty();
      }

      void fillHolesOnPath();

      std::string getPathType() { return shortestPathSelectedType; }

      void setClosePath( bool c ) { pathClosed = c; }
      bool pathIsClosed() { return pathClosed; }

      aims::GeodesicPath* getMeshStructSP() { return sp; }
      aims::GeodesicPath* getMeshStructSulciP() { return sp_sulci; }
      aims::GeodesicPath* getMeshStructGyriP() { return sp_gyri; }

      void addGeodesicPath( int indexNearestVertex,
                            Point3df positionNearestVertex);
      void undoGeodesicPath();
      void redoGeodesicPath();
      void undoHolesPaths();
      void redoHolesPaths();

      void computeDistanceMap( int indexNearestVertex );

      void changeControl( int control ) { IDActiveControl = control; }
      int getActiveControl() { return IDActiveControl; }
      AWindow3D* getWindow3D() { return win3D; }

      /** start a new buffer to store texture modifications.
          This is called when starting a new brush stroke (button press) etc,
          and should always be ended with endEditOperation().
      */
      void newEditOperation();
      void undoTextureOperation();
      void redoTextureOperation();

    public slots:

      void colorPicker();
      void magicSelection();
      void path();
      void shortestPath();
      void sulciPath();
      void gyriPath();
      void brush();
      void magicBrush();
      void validateEdit();
      void distance();
      void clearPath();
      void clearRegion();
      void clearHoles();
      void clearAll();
      void erase();
      void save();
      /// undo the last texture modification
      void undo();
      /// redo the last texture modification
      void redo();

      void updateConstraintList();
      void loadConstraintsList( std::vector<std::string> clist );

      void changeToleranceSpinBox( int v );
      void changeConstraintPathSpinBox( int v );

      void changeMinValueSpinBox( double v );
      void changeMaxValueSpinBox( double v );

    private :
      void popAllButtonPaintToolBar();
      void clearUndoneGeodesicPath();
      void addSimpleShortPath( int indexSource, int indexDest );
      void undoSimpleShortPath();
      void redoSimpleShortPath();
      void clearUndoneHolesPaths();
      float currentTextureValue( unsigned vertexIndex ) const;
      void clearRedoBuffer();

      struct Private;
      Private *d;

      ATexSurface *go;
      ATriangulated *as;
      carto::rc_ptr<AimsSurfaceTriangle> mesh;
      ATexture *at;
      TimeTexture<float> texCurv;

      carto::Object options;

      Texture1d *surfpaintTexInit;
      AWindow3D *win3D;

      AObject *objselect;
      QToolBar  *tbTextureValue;
      QDoubleSpinBox *textureFloatSpinBox;

      std::string textype;
      std::string objtype;

      QToolBar  *tbInfos3D;
      QSpinBox *IDPolygonSpinBox;
      QSpinBox *IDVertexSpinBox;

      QToolBar  *tbControls;

      QToolButton *colorPickerAction;
      QToolButton *selectionAction;
      QToolButton *pathAction;
      QAction     *shortestPathAction;
      QAction     *sulciPathAction;
      QAction     *gyriPathAction;

      QToolButton *paintBrushAction;
      QToolButton *magicBrushAction;

      QToolButton *distanceAction;

      QToolButton *validateEditAction;
      QToolButton *eraseAction;
      QToolButton *clearPathAction;
      QToolButton *saveAction;

      QComboBox *constraintList;

      QSpinBox *toleranceSpinBox;
      QLabel *toleranceSpinBoxLabel;
      QSpinBox *constraintPathSpinBox;
      QLabel *constraintPathSpinBoxLabel;

      QDoubleSpinBox *textureValueMinSpinBox;
      QDoubleSpinBox *textureValueMaxSpinBox;

      GLuint constraintPathValue;
      GLuint toleranceValue;
      float stepToleranceValue;

      int IDActiveControl;
      std::string shortestPathSelectedType;

      aims::GeodesicPath *sp;
      aims::GeodesicPath *sp_sulci;
      aims::GeodesicPath *sp_gyri;

      std::vector<std::set<uint> >  neighbours;
      bool pathClosed;

      std::vector<unsigned> listIndexVertexPathSP;
      std::vector<unsigned> listIndexVertexSelectSP;

      std::vector<unsigned> listIndexVertexBrushPath;
      std::vector<unsigned> listIndexVertexHolesPath;

      std::vector<unsigned> listIndexVertexSelectFill;
      std::set<int> listIndexVertexFill;

      std::vector<carto::rc_ptr<ATriangulated> > pathObject;
      std::vector<carto::rc_ptr<ATriangulated> > fillObject;
      std::vector<carto::rc_ptr<ATriangulated> > holesObject;
  };
}
#endif
