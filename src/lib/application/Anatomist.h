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


#ifndef ANATOMIST_APPLICATION_ANATOMIST
#define ANATOMIST_APPLICATION_ANATOMIST

#include <anatomist/config/anatomist_config.h>
#include <aims/getopt/getopt2.h>
#include <qwidget.h>
#include <set>
#include <map>

class ReferentialWindow;
class ControlWindow;


namespace anatomist
{

  class AObject;
  class MObject;
  class PaletteList;
  class Transformation;
  class AWindow;
  class Referential;
  class GlobalConfiguration;
  class CommandWriter;

  namespace internal
  {
    class AnatomistPrivate;
  }

  /**	The Anatomist class holds information about all data handled by 
	Anatomist: the objects, the windows, and other application-wide lists.
	Althrough the Control and Referential Windows are one amongst the many
	Anatomist windows, they are in fact the image of the application, so
	they have a particular place in the Anatomist class.
  */
  class ANATOMIST_API Anatomist : public aims::AimsApplication
  {

  public:
    enum Cursor
    {
      Normal, 
      Working
    };

    Anatomist( int argc, const char **argv, 
	       const std::string &documentation );
    virtual ~Anatomist();

    virtual void initialize();
    /// full version ("1.30.3" or "1.31beta")
    static std::string versionString();
    /// shorter version matching the library number (2 numbers: "1.31")
    static std::string libraryVersionString();

    PaletteList & palettes();
    const PaletteList & palettes() const;

    virtual void Refresh();

    void NotifyMapWindow( AWindow* );
    void NotifyUnmapWindow( AWindow* );

    virtual void createControlWindow();
    ControlWindow* getControlWindow() const;
    /// Gets an empty QWidget that is created in Anatomist application to become a default parent for all Anatomist windows
    QWidget* getQWidgetAncestor() const;

    /// Create the referential window and update it
    virtual void createReferentialWindow();
    ReferentialWindow* getReferentialWindow() const;

    bool hasWindow( const AWindow* win ) const;
    ///	Links the given windows, and returns the new group number
    int groupWindows( std::set<AWindow*> & winL, int groupnum = -1 );
    ///	Unlinks the windows with the given group number
    void ungroupWindows( int group );
    ///	Retruns the set of windows in a given group
    std::set<AWindow*> getWindowsInGroup( int group );
    void NotifyWindowChange( AWindow* win );

    /**	Destroys an object if possible. \\
	@return	true if the object has actually been deleted, \\
	false if the object cannot be deleted (it is used in other 
	compound objects which require it and cannot release it)
    */
    int destroyObject( AObject* );
    bool hasObject( const AObject* obj ) const;
    ///	Makes an object visible (ie seen in control window(s))
    void mapObject( AObject* obj );
    ///	Makes an object unvisible (ie not seen in control window(s))
    void unmapObject( AObject* obj );

    virtual void UpdateInterface();

    ///	Builds a unique object name
    std::string makeObjectName( const std::string & name );
    std::set<AObject* > getObjects() const;
    std::set<AWindow*> getWindows() const;
    const std::set<Referential*> & getReferentials() const;
    bool hasReferential( const Referential * );
    Transformation* getTransformation( const Referential*, 
				       const Referential* );
    const Transformation* getTransformation( const Referential*, 
					     const Referential* ) const;

    std::list<AObject *> loadObject( const std::string & filename,
                                     const std::string & objname = "",
                                     carto::Object options = carto::none() );

    void registerObject( AObject* obj, int inctrl=1 );
    void unregisterObject( AObject* obj );
    /** releases the reference counter kept in Anatomist application for
        the given object, so that external reference counting will behave
        normally. If no external reference counter exists, the window will
        be immediately deleted.
     */
    void releaseObject( AObject* obj );
    /// change object reference type to WeakShared (standard in anatomist)
    void takeObjectRef( AObject* obj );
    void NotifyObjectChange( AObject* obj );
    void registerSubObject( MObject* parent, AObject* obj );
    void registerReferential( Referential* ref );
    void unregisterReferential( Referential* ref );
    void registerWindow( AWindow* win );
    void unregisterWindow( AWindow* win );
    /** releases the reference counter kept in Anatomist application for
        the given window, so that external reference counting will behave
        normally. If no external reference counter exists, the window will
        be immediately deleted.
    */
    void releaseWindow( AWindow* win );
    /// change window reference type to WeakShared (standard in anatomist)
    void takeWindowRef( AWindow* obj );
    void registerObjectName( const std::string& name, AObject* obj );
    void unregisterObjectName( const std::string& name );

    bool initialized() const;

    const CommandWriter & historyWriter() const;
    CommandWriter & historyWriter();
    GlobalConfiguration* config();

    ///	Sets the shape of the mouse cursor on every window
    void setCursor( Cursor c );

    const std::string & objectsFileFilter() const;
    void addObjectsFileFilter( const std::string & filter );

    virtual bool glMakeCurrent();
    Referential* centralReferential() const;
    std::string language() const;
    /** The user level drives the GUI to show or hide some features for
      simplicity or richness. Values mean:
      - 0: basic
      - 1: advanced
      - 2: expert
      - >=3: debugger with all unstable features enabled
    */
    int userLevel() const;
    void setUserLevel( int );
    void setLastPosition( const Point3df & pos, Referential * fromref = 0 );
    Point3df lastPosition( const Referential* toref = 0 ) const;
    /// returns true if the Anatomist application is currently being destroyed
    bool destroying() const;
    /// setup/update extensions list for readable files
    void updateFileDialogObjectsFilter();
    /// lock / unlock the objects list mutex, used in threaded load operations
    void lockObjects( bool locked=true );

  private:
    friend class anatomist::internal::AnatomistPrivate;
    struct Anatomist_privateData;

    Anatomist_privateData		*_privData;
  };

}


///	Pointer to the Anatomist application
extern anatomist::Anatomist* ANATOMIST_API theAnatomist;


#endif
