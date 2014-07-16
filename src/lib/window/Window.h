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


#ifndef ANA_WINDOW_WINDOW_H
#define ANA_WINDOW_WINDOW_H

//--- header files ------------------------------------------------------------

#include <anatomist/observer/Observer.h>
#include <anatomist/config/anatomist_config.h>
#include <aims/vector/vector.h>
#include <aims/rgb/rgb.h>
#include <cartobase/object/object.h>
#include <cartobase/smart/sharedptr.h>


namespace anatomist
{

  class AObject;
  class Referential;
  class Geometry;

  /** Abstract base class Anatomist window
   */
  class ANATOMIST_API AWindow : public carto::SharedObject, public Observer
  {
  public:
    enum Type
      {
        CTRL_WINDOW = 100,
        WINDOW_2D = 101,
        WINDOW_3D = 102,
      };

    enum SubType
      {
        AXIAL_WINDOW = 200,
        SAGITTAL_WINDOW = 201,
        CORONAL_WINDOW = 202,
        OBLIQUE_WINDOW = 203,
        OTHER = 299
      };

    enum RecordingState
      {
        OFF,
        ON,
        DISABLED
      };

    /// Unregisters from the application and other objects
    virtual ~AWindow();

    /**@name Usual window operations*/
    //@{
    /** Affiche les coordonnees de Talairach qui correspondent a la position du
        curseur lie (window 2D) */
    void displayTalairach();
    virtual void iconify();
    virtual void unIconify();
    virtual void show();
    virtual void hide();
    virtual bool close() = 0;
    /** Shows or hides all tools (menu, toolbars) around the main view area.
        0: hide, 1: show, 2: toggle */
    virtual void showToolBars( int state = 2 );
    virtual bool toolBarsVisible() const;
    virtual void setFullScreen( int state = 2 );
    virtual bool isFullScreen() const;
    virtual void setGeometry( int x, int y, unsigned w, unsigned h ) = 0;
    /// Get position and dimensions of the window
    virtual void geometry( int *x, int *y, unsigned *w, unsigned *h ) = 0;
    virtual void Refresh();
    virtual void showReferential();
    ///        Sets correct size and lookup of the window
    virtual void setupWindow() {}
    //@}

    virtual std::string Title() const;
    //virtual const char* Title_c() const;
    virtual void registerObject( AObject* object,
                                 bool temporaryObject = false,
                                 int position = -1 );
    virtual void unregisterObject( AObject* object );

    /// Set the window identifier
    void setId(int id);
    /// Set the refresh flag on
    void SetRefreshFlag();
    /// Set the refresh flag off
    void ResetRefreshFlag();
    bool lookupChanged() const { return( _lookupChanged ); }
    void setLookupChanged( bool flg = true )
    { _lookupChanged = flg; if( flg ) SetRefreshFlag(); }
    /// Get the window identifier
    int id() const;
    /// Get the window type (2D, 3D or control)
    virtual Type type() const = 0;
    virtual SubType subtype() const { return( (SubType) 0 ); }
    /// Get the refresh flag status
    bool RefreshFlag() const;
    /// Get the objects
    std::set<AObject*> Objects() const;
    bool hasObject( AObject * obj ) const;
    /// Get position of cursor
    virtual Point3df getPosition() const;
    /// Get time position of cursor
    float getTime() const;
    /// Set position of cursor
    virtual void setPosition( const Point3df& position ,
                              const Referential *refdep );
    /// Set time position of cursor
    virtual void setTime( float time );
    virtual void setTitle( const std::string & title );
    Referential* getReferential() const { return _referential; }
    virtual void setReferential( Referential* ref );
    Geometry* windowGeometry() const { return _geometry; }
    void setWindowGeometry( Geometry *geom );
    /// adapts geometry to the current contents and referential
    virtual void updateWindowGeometry() {}
    int Group() const { return _group; }
    void SetGroup( int group ) { _group = group; }
    void setHasCursor( int hasCursor );
    /// takes both the global and own flags into account
    bool hasCursor() const;
    /// returns the own flag of the window (see hasGlobalCursor) (tri-state)
    int hasSelfCursor() const;

    static void setLeftRightDisplay( bool state )
    { _leftRightDisplay = state; }
    static int leftRightDisplay() { return( _leftRightDisplay ); }
    static void setGlobalHasCursor(bool hasCursor){ _hasCursor = hasCursor; }
    static int hasGlobalCursor() { return( _hasCursor ); }
    static void setCursorSize(int cursorSize){ _cursorSize = cursorSize; }
    static int cursorSize() { return(_cursorSize); }
    /// Set the default color cursor flag.
    static void setUseDefaultCursorColor( bool state )
    { _useDefaultCursorColor = state; }
    /// Give the default color cursor flag.
    static int useDefaultCursorColor() { return( _useDefaultCursorColor ); }
    /// Set the color cursor.
    static void setCursorColor( const AimsRGB & cursCol );
    /// Give the color cursor.
    static AimsRGB cursorColor();
    /// Selection tolerence distance
    static float selectTolerence() { return( _selectTolerence ); }
    /// Set the selection tolerence distance
    static void setSelectTolerence( float tol ) { _selectTolerence = tol; }
    virtual void displayClickPoint() {}
    /**        Translates mouse position to Anatomist geometry position
        @return        false if posision cannot be computed (out of viewport)
    */
    virtual bool positionFromCursor( int x, int y, Point3df & pos );
    virtual AObject* objectAt( float x, float y, float z, float t );

    ///        Selects (highlights) object at a given 4D space position
    virtual void selectObject( float x, float y, float z, float t,
                               int modifier );
    ///        handles button3 click (menu)
    virtual void button3clicked( int x, int y );
    ///        finds objects at given position (internal)
    virtual void findObjectsAt( float x, float y, float z, float t,
                                std::set<AObject *> & shown,
                                std::set<AObject *> & hidden );

    virtual void update( const Observable* observable, void* arg );

    static void recordCbk( void* clientdata );
    virtual void recordImages();
    virtual RecordingState recordingState() const { return( DISABLED ); }
    virtual void startRecord();
    virtual void startRecord( const std::string & filename );
    virtual void stopRecord() {}
    /// Creates a new title for the window
    virtual void createTitle() {}

    virtual const std::set<unsigned> & typeCount() const;
    virtual std::set<unsigned> & typeCount();
    virtual const std::string & baseTitle() const;
    /** returns true if a refresh has been triggered and not performed yet
        (in subclasses: AWindow always returns false) */
    virtual bool needsRedraw() const { return( false ); }
    bool isTemporary( AObject* o ) const;
    const std::set<AObject *> & temporaryObjects() const
    { return( _tempObjects ); }

  protected:
    /// AWindow constructor registers itself in the application
    AWindow();

    /// Set the title of the window.
    void setTitleWindow();
    virtual void unregisterObservable( Observable* obs );

    /// Window identificator
    int _id;
    /// Should the window be refreshed?
    bool _refresh;
    /// Lookup has changed (title or geometry)
    bool _lookupChanged;
    /// List of the objects contained in the window
    std::list<carto::shared_ptr<AObject> > _objects;
    /// Same but as a set (for fast search)
    std::set<AObject *>        _sobjects;
    std::set<AObject *>        _tempObjects;
    /// Cursor time
    float _time;
    /// Referentiel.
    Referential *_referential;
    /// Geometry.
    Geometry *_geometry;
    /// Title of the window
    std::string _title;
    /**        Number of instance of the window class. \\
        This counter is used by child non-virtual classes to count instances
        of each specific class.
    **/
    int _instNumber;
    ///        Group number, for linked windows
    int _group;
    /// Click position.
    Point3df _position;

  private:
    struct Private;

    Private        *d;
    /// Copy constructor
    AWindow(const AWindow& x);
    /// Assignment operator
    AWindow& operator=(const AWindow& x);
    /// Right-left display flag
    static bool _leftRightDisplay;
    /// Cursor flag
    static bool _hasCursor;
    /// Cursor size
    static int _cursorSize;
    /// default color cursor flag
    static bool _useDefaultCursorColor;
    /// color cursor
    static AimsRGB _cursorColor;
    /// tolerence distance for selections
    static float _selectTolerence;
  };


  //--- inline methods --------------------------------------------------------

  inline
  void AWindow::setId( int id )
  {
    _id = id;
  }

  inline
  void AWindow::SetRefreshFlag()
  {
    _refresh = true;
  }

  inline
  void AWindow::ResetRefreshFlag()
  {
    _refresh = false;
  }

  inline
  int AWindow::id() const
  {
    return _id;
  }


  inline
  bool AWindow::RefreshFlag() const
  {
    return _refresh;
  }

  inline
  std::set<AObject*> AWindow::Objects() const
  {
    return _sobjects;
  }

  inline
  float AWindow::getTime() const
  {
    return _time;
  }

  inline
  std::string AWindow::Title() const
  {
    return( _title );
  }

  /*inline
    const char* AWindow::Title_c() const
    {
    return( _title.c_str() );
    }*/


  inline bool AWindow::positionFromCursor( int, int, Point3df & )
  {
    return( false );
  }


  inline void AWindow::startRecord( const std::string & )
  {
  }

}


namespace carto
{
  DECLARE_GENERIC_OBJECT_TYPE( anatomist::AWindow * )
  DECLARE_GENERIC_OBJECT_TYPE( std::set<anatomist::AWindow *> )
  DECLARE_GENERIC_OBJECT_TYPE( std::vector<anatomist::AWindow *> )
  DECLARE_GENERIC_OBJECT_TYPE( std::list<anatomist::AWindow *> )
}


#endif

