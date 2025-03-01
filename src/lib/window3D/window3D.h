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


#ifndef ANA_WINDOW3D_WINDOW3D_H
#define ANA_WINDOW3D_WINDOW3D_H


#include <anatomist/window/controlledWindow.h>
#include <anatomist/observer/Observable.h>
#include <anatomist/object/Object.h>
#include <anatomist/primitive/primitive.h>
#include <anatomist/window/viewstate.h>


namespace aims
{
  class Quaternion;
}

namespace anatomist
{
  class Light;
  class GLWidgetManager;
  class OrientationAnnotation;

  namespace internal
  {
    class AGraphicsView;
  }

}

#ifdef ANA_USE_QOPENGLWIDGET
class QOpenGLWidget;
#else
class QGLWidget;
#endif
class QSlider;


/** 3D window, OpenGL rendering
 */
class AWindow3D : public ControlledWindow
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
    AWindow3D *_window;
  };


  AWindow3D( ViewType t = Oblique, QWidget* parent = 0, 
             carto::Object params = carto::none(), 
             Qt::WindowFlags f = Qt::WindowFlags() );
  virtual ~AWindow3D();

  /// Get the window type (2D, 3D or control)
  virtual Type type() const;
  virtual SubType subtype() const;
  virtual void registerObject( anatomist::AObject* obj,
                               bool temporaryObject = false,
                               int position = -1 );
  virtual void unregisterObject( anatomist::AObject* obj );
  virtual bool positionFromCursor( int x, int y, Point3df & pos );
  /// pick the object at the cursor 2D position
  virtual anatomist::AObject* objectAtCursorPosition( int x, int y );
  /// pick several objects at the cursor 2D position
  virtual std::list<anatomist::AObject*> *objectsAtCursorPosition( int x, int y, int tolerenceRadius );
  /// pick a polygon on a selected object at the cursor 2D position
  virtual int polygonAtCursorPosition( int x, int y,const anatomist::AObject* obj );

  /// print all infos about vertex picked on a polygon selected
  int computeNearestVertexFromPolygonPoint(
    const anatomist::ViewState & vs, int poly,
    const anatomist::GLComponent* glc,
    const Point3df & position, Point3df & positionNearestVertex );
  void getInfos3DFromClickPoint( int x, int y, Point3df & position, int *poly,
      anatomist::AObject *objselect, std::string & objtype,
      std::vector<float> &texvalue, std::string & textype,
      Point3df & positionNearestVertex, int* indexNearestVertex,
      std::vector<std::string> & texlabels );
  /** same as above but from a 3+D position.
      The polygon "poly" must be provided if info about vertices are to be
      retreived, because we cannot guess it from the 3D position. If poly is
      negative, no vertex info will be provided.
  */
  void getInfos3DFromPosition( const std::vector<float> & position, int poly,
      anatomist::AObject *objselect, std::string & objtype,
      std::vector<float> &texvalue, std::string & textype,
      Point3df & positionNearestVertex, int* indexNearestVertex,
      std::vector<std::string> & texlabels );

  bool surfpaintIsVisible();
  void setVisibleSurfpaint(bool b);
  bool constraintEditorIsActive();
  void setActiveConstraintEditor(bool b);

  void loadConstraintData(
    const std::vector<std::string> & constraintList, int constraintType,
    anatomist::AObject *texConstraint );
  std::vector<std::string> getConstraintList();
  int getConstraintType() const;
  anatomist::AObject* getConstraintTexture();

  void printPositionAndValue();
  void displayInfoAtClickPosition( int x, int y );
  std::string displayInfoAtClickPositionAsText( int x, int y,
                                                bool html = false );
  virtual void displayClickPoint();
  ///   set the view of the scene
  void setViewPoint( float *quaternion, 
         const float zoom );
  anatomist::Light *light();
  void setLight( const anatomist::Light &light );
  ///   Compass handling methods
  void setOrientationCube( bool state );
  bool hasOrientationCube() const;
  ///   Frame handling methods
  void setBoundingFrame( bool state );
  bool hasBoundingFrame() const;
  /// Rendering mode (normal, wireframe, fast)
  void setRenderingMode( RenderingMode mode );
  RenderingMode renderingMode() const;

  virtual const std::set<unsigned> & typeCount() const;
  virtual std::set<unsigned> & typeCount();
  virtual const std::string & baseTitle() const;
  virtual void setPosition( const Point3df& position,
          const anatomist::Referential* orgref );
  virtual void setPosition( const std::vector<float> & position,
                            const anatomist::Referential* orgref );
  virtual void updateWindowGeometry();

  static anatomist::Geometry 
  setupWindowGeometry( const std::list<carto::shared_ptr<anatomist::AObject> >
                       & objects,
                       const aims::Quaternion & slicequat,
                       const anatomist::Referential *wref = 0, 
#ifdef ANA_USE_QOPENGLWIDGET
                       QOpenGLWidget* glw = 0, bool with3d = false );
#else
                       QGLWidget* glw = 0, bool with3d = false );
#endif

  virtual anatomist::View* view();
  virtual const anatomist::View* view() const;

  /// Mute into a new view type (Axial, Sagittal, Coronal or Oblique)
  virtual void setViewType( ViewType t );
  ViewType viewType() const;
  bool isViewOblique() const;

  bool perspectiveEnabled() const;
  void enablePerspective( bool );

  const aims::Quaternion & sliceQuaternion() const;
  void setSliceQuaternion( const aims::Quaternion & q );
  void setSliceOrientation( const Point3df & normal );
  /// Tries to resize the viewing area to given size
  void resizeView( int w, int h );
  /// Get bounding box in N-dimension mode (new in anatomist 4.6)
  bool boundingBox( std::vector<float> & bmin,
                    std::vector<float> & bmax ) const;
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
#ifdef ANA_USE_QOPENGLWIDGET
        const QOpenGLWidget * shareWidget, Qt::WindowFlags f );
#else
        const QGLWidget * shareWidget, Qt::WindowFlags f );
#endif
  /// hook to create inherited QAGLWidgets (Vtk-enabled for instance)
  static void setGLWidgetCreator( GLWidgetCreator );

  QSlider* getSliceSlider (void) const;

  /** adds a rendering order constraint: obj will be rendered immediately
      after afterthis. If afterthis is null, then constraints for obj are
      cleared.
  */
  void renderAfter( anatomist::AObject* obj, anatomist::AObject* afterthis );
  /** adds a rendering order constraint: obj will be rendered immediately
      before beforethis. If beforethis is null, then constraints for obj are
      cleared.
  */
  void renderBefore( anatomist::AObject* obj, anatomist::AObject* beforethis );
  /** calculates the objects rendering order, according to various constraints
      (opaque/transparent, order at register time, before/after constraints).
      \return the iterator on the first transparent object (or end if none is)
  */
  std::list<anatomist::AObject *>::iterator processRenderingOrder(
    std::list<anatomist::AObject *> & opaque ) const;

  bool polygonsSortingEnabled() const;
  void setPolygonsSortingEnabled( bool );
  /// returns false for back-to-front, true for front-to-back
  bool polygonsSortingDirection() const;
  void setPolygonsSortingDirection( bool dir );
  virtual void sortPolygons( bool force = false );
  bool positionToVertex( const anatomist::AObject* obj, unsigned vert );
  bool positionToPolygon( const anatomist::AObject* obj, unsigned poly );
  bool positionToTexture( const anatomist::AObject* obj, float texval );


signals:
  void refreshed();
  void sliderChanged( int dim, int slice );
  void orientationChanged();

public slots:
  virtual void polish();
  void resizeView();
  void changeSlice( int );
  void changeTime( int );
  void changeTimeSliders( int, int );
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
  //void painttoolsWinDestroyed();
  void povWinDestroyed();
  void lightWinDestroyed();
  virtual void Refresh();
  /// Refresh the window (redraw the contained objects).
  virtual void refreshNow();
  /// Redraws temporary objects only
  void refreshTemp();
  void refreshTempNow();
  /** Only redraws existing objects (in most cases, just calls existing OpenGL
      lists, except for observer-dependent objects)
  */
  void refreshLightView();
  void refreshLightViewNow();
  void setAutoRotationCenter();
  void askZoom();
  // Gets the window zoom factor
  float getZoom() const;

  // Necessary for the movie action
  int getSliceSliderPosition() ;
  int getTimeSliderPosition() ;

  int getSliceSliderMaxPosition() ;
  int getTimeSliderMaxPosition() ;

  void setSliceSliderPosition( int position ) ;
  void setTimeSliderPosition( int position ) ;
  void switchToolbox();
  void setLinkedCursorPos();
  void findPrimitive();
  void openStereoView();
  void toggleStatusBarVisibility();
  bool toopTipsEnabled() const;
  void enableToolTips( bool );
  // Refreshs the window when resized
  void resizeEvent( QResizeEvent * ) ;

  /** Get a snapshot of the window, after a refresh if needed.
      mode: 0: RGB, 2: alpha, 4: RGBA, 8: depth, 16: luminance
  */
  QImage snapshotImage( int width=0, int height=0, int bufmode=0 );
  virtual carto::rc_ptr<anatomist::ViewState> viewState(bool slice=false);
  void saveObject();

  friend class anatomist::internal::AGraphicsView;

protected slots:
  void freeResize();
  void saveSnapshot();
  void saveSnapshotWithCustomSize();


protected:
  void showReferential();

  // Updates left/right annotations
  void updateLeftRightAnnotations();

  /// Display the click point
  void displayClickPos( Point3df clickPos );
  void setupTimeSlider( const std::vector<float> & bmin,
                        const std::vector<float> & bmax );
  void setupSliceSlider( float mins, float maxs );
  void setupSliceSlider();
  void updateViewTypeToolBar();
  void updateObject( anatomist::AObject* obj, anatomist::PrimList* pl = 0,
                     anatomist::ViewState::glSelectRenderMode selectmode
                         = anatomist::ViewState::glSELECTRENDER_NONE );
  void updateObject2D( anatomist::AObject* obj, anatomist::PrimList* pl = 0,
                       anatomist::ViewState::glSelectRenderMode selectmode
                           = anatomist::ViewState::glSELECTRENDER_NONE );
  void updateObject3D( anatomist::AObject* obj, anatomist::PrimList* pl = 0,
                       anatomist::ViewState::glSelectRenderMode selectmode
                           = anatomist::ViewState::glSELECTRENDER_NONE );
  anatomist::GLPrimitives cursorGLL() const;
  int updateSliceSlider();
  void updateTimeSliders();
  /// Allows changing display lists from normal objects DLists
  void registerObjectModifier( ObjectModifier *mod );
  void unregisterObjectModifier( ObjectModifier *mod );
  /** build or modify the auto Fusion2D object when adding a new object.
      Returns true if the object is actually added to the auto fusion.
  */
  bool autoFusion2D( anatomist::AObject *obj );
  /// delete or modify the auto Fusion2D object when removing an object
  void removeFromAutoFusion2D( anatomist::AObject *obj );
  bool inAutoFusion( const anatomist::AObject *obj ) const;
  void updateSaveButtonStatus();

  /// 3D windows static counter
  static std::set<unsigned> _count3d;
  static std::string    _baseTitle;

public :
  void renderSelectionBuffer( anatomist::ViewState::glSelectRenderMode mode,
                              const anatomist::AObject *selectedobject = 0 );

private:
  struct Private;

  Private *d;
};


#endif
