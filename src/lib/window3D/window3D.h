/* Copyright (c) 1995-2006 CEA
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


#ifndef ANA_WINDOW3D_WINDOW3D_H
#define ANA_WINDOW3D_WINDOW3D_H


#include <anatomist/window/controlledWindow.h>
#include <anatomist/observer/Observable.h>
#include <anatomist/object/Object.h>
#include <anatomist/primitive/primitive.h>

namespace aims
{
  class Quaternion;
}


namespace anatomist
{
  class Light;
  class GLWidgetManager;
}

class QGLWidget;


/** 3D window, OpenGL rendering
 */
class AWindow3D : public ControlledWindow, public anatomist::Observable
{ 
  Q_OBJECT

public:
  enum ViewType
    {
      Oblique, 
      Axial, 
      Sagittal, 
      Coronal, 
      ThreeD
    };

  enum RenderingMode
    {
      Normal, 
      Wireframe, 
      Outlined, 
      HiddenWireframe, 
      Fast
    };

  enum ClipMode
  {
    NoClip, 
    Single, 
    Double
  };

  /// Functions that can modify on-the-fly display primitives of an object
  class ObjectModifier
  {
  public:
    ObjectModifier( AWindow3D* w );
    virtual ~ObjectModifier();
    virtual void modify( anatomist::AObject*, anatomist::GLPrimitives & ) = 0;
    AWindow3D *window() { return _window; }

  private:
    AWindow3D	*_window;
  };


  AWindow3D( ViewType t = Oblique, QWidget* parent = 0, 
             carto::Object params = carto::none(), 
             Qt::WFlags f = Qt::WType_TopLevel | Qt::WDestructiveClose );
  virtual ~AWindow3D();

  /// Get the window type (2D, 3D or control)
  virtual Type type() const;
  virtual SubType subtype() const;
  virtual void registerObject( anatomist::AObject* obj, 
			       bool temporaryObject = false );
  virtual void unregisterObject( anatomist::AObject* obj, 
				 bool temporaryObject = false );
  virtual bool positionFromCursor( int x, int y, Point3df & pos );
  void printPositionAndValue();
  virtual void displayClickPoint();

  ///		set the view of the scene
  void setViewPoint( float *quaternion, 
		     const float zoom );
  anatomist::Light *light();
  void setLight( const anatomist::Light &light );
  ///		Compass handling methods
  void setOrientationCube( bool state );
  bool hasOrientationCube() const;
  ///		Frame handling methods
  void setBoundingFrame( bool state );
  bool hasBoundingFrame() const;
  ///	Rendering mode (normal, wireframe, fast)
  void setRenderingMode( RenderingMode mode );
  RenderingMode renderingMode() const;

  virtual const std::set<unsigned> & typeCount() const;
  virtual std::set<unsigned> & typeCount();
  virtual const std::string & baseTitle() const;
  virtual void SetPosition( const Point3df& position, 
			    const anatomist::Referential* orgref );
  virtual void updateWindowGeometry();

  static anatomist::Geometry 
  setupWindowGeometry( const std::list<carto::shared_ptr<anatomist::AObject> >
                       & objects,
                       const aims::Quaternion & slicequat,
                       const anatomist::Referential *wref = 0, 
                       QGLWidget* glw = 0 );

  virtual anatomist::View* view();
  virtual const anatomist::View* view() const;

  /// Mute into a new view type (Axial, Sagittal, Coronal or Oblique)
  virtual void setViewType( ViewType t );
  ViewType viewType() const;

  bool perspectiveEnabled() const;
  void enablePerspective( bool );

  const aims::Quaternion & sliceQuaternion() const;
  void setSliceQuaternion( const aims::Quaternion & q );
  /// Tries to resize the viewing area to given size
  void resizeView( int w, int h );
  bool boundingBox( Point3df & bmin, Point3df & bmax, float & tmin, 
		    float & tmax ) const;
  ClipMode clipMode() const;
  void setClipMode( ClipMode m );
  float clipDistance() const;
  void setClipDistance( float d );
  bool transparentZEnabled() const;
  void enableTransparentZ( bool );
  bool cullingEnabled() const;
  void setCulling( bool );
  bool flatShading() const;
  void setFlatShading( bool );
  bool smoothing() const;
  void setSmoothing( bool );
  void setFog( bool );
  bool fog() const;
  virtual void setReferential( anatomist::Referential* ref );
  virtual void update( const Observable* obs, void * );
  void setLinkedCursorOnSliderChange( bool x );
  bool linkedCursorOnSliderChange() const;
  AWindow3D* leftEyeWindow();
  AWindow3D* rightEyeWindow();
  void setLeftEyeWindow( AWindow3D* );
  void setRightEyeWindow( AWindow3D* );
  virtual void showToolBars( int state = 2 );
  virtual void showStatusBar( int state = 2 );

  typedef anatomist::GLWidgetManager* (*GLWidgetCreator)
      ( anatomist::AWindow* win, QWidget* parent, const char* name,
        const QGLWidget * shareWidget, Qt::WFlags f );
  /// hook to create inherited QAGLWidgets (Vtk-enabled for instance)
  static void setGLWidgetCreator( GLWidgetCreator );

public slots:
  virtual void polish();
  void resizeView();
  void changeSlice( int );
  void changeTime( int );
  void changeReferential();
  void muteAxial();
  void muteCoronal();
  void muteSagittal();
  void muteOblique();
  void mute3D();
  void lightView();
  void pointsOfView();
  void tools();
  void syncViews( bool keepextrema = false );
  void focusView();
  void toolsWinDestroyed();
  void povWinDestroyed();
  void lightWinDestroyed();
  virtual void Refresh();
  /// Refresh the window (redraw the contained objects).
  virtual void refreshNow();
  /// Redraws temporary objects only
  void refreshTemp();
  void refreshTempNow();
  /** Only redraws existing objects (in most cases, just calls existing OpelGL
      lists, except for observer-dependent objects)
  */
  void refreshLightView();
  void refreshLightViewNow();
  void setAutoRotationCenter();
  void askZoom();

  // Necessary for the movie action
  int getSliceSliderPosition() ;
  int getTimeSliderPosition() ;

  int getSliceSliderMaxPosition() ;
  int getTimeSliderMaxPosition() ;

  void setSliceSliderPosition( int position ) ;
  void setTimeSliderPosition( int position ) ;
  void switchToolbox();
  void setLinkedCursorPos();
  void openStereoView();
  void toggleStatusBarVisibility();

protected slots:
  void freeResize();

protected:
  void showReferential();
  /// Display the click point
  void displayClickPos( Point3df clickPos );
  void setupTimeSlider( float mint, float maxt );
  void setupSliceSlider( float mins, float maxs );
  void setupSliceSlider();
  void updateViewTypeToolBar();
  void updateObject( anatomist::AObject* obj, anatomist::PrimList* pl = 0 );
  void updateObject2D( anatomist::AObject* obj, anatomist::PrimList* pl = 0 );
  void updateObject3D( anatomist::AObject* obj, anatomist::PrimList* pl = 0 );
  anatomist::GLPrimitives cursorGLL() const;
  int updateSliceSlider();
  /// Allows changing display lists from normal objects DLists
  void registerObjectModifier( ObjectModifier *mod );
  void unregisterObjectModifier( ObjectModifier *mod );

  /// 3D windows static counter
  static std::set<unsigned>	_count3d;
  static std::string		_baseTitle;

private:
  struct Private;

  Private	*d;
};


#endif
