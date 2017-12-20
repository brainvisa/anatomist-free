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


#ifndef ANA_OBJECT_OBJECT_H
#define ANA_OBJECT_OBJECT_H

#include <anatomist/config/anatomist_config.h>
#include <anatomist/color/Material.h>
#include <anatomist/observer/Observable.h>
#include <anatomist/observer/Observer.h>
#include <anatomist/object/objectmenu.h>
#include <cartobase/smart/rcptr.h>
#include <cartobase/object/object.h>
#include <aims/vector/vector.h>

#include <string>
#include <list>
#include <set>
#include <map>
#include <vector>


class ColorMap;
class Tree;

namespace aims
{
  class Quaternion;
}

namespace anatomist
{

  class AWindow;
  class Referential;
  class Geometry;
  class AObjectPalette;
  class MObject;
  class GLItem;
  class GLComponent;
  struct ViewState;
  typedef std::list<carto::rc_ptr<GLItem> > PrimList;
  class ObjectMenuRegistrerClass;


  struct AImage
  {
    int		width;
    int		height;
    int		depth;
    int		effectiveWidth;
    int		effectiveHeight;
    char	*data;
  };

  /*
     TODO:
     remove:
     MinT(), MaxT()
   */

  /**	 Base Anatomist object (abstract)
   */
  class ANATOMIST_API AObject : public Observable, public Observer
  {
  public:
    /// Storage type for parent objects (multi-objects containing this one)
    typedef std::set<MObject*> ParentList;

    /**	Base object type identifiers. Some of these base types are still 
	unused now. New objects (unknown to the base Anatomist library) use 
	dynamic identifiers that they must register by calling 
	registerObjectType() once (and only once) */
    enum ObjectType
    {
      VOLUME,
      BUCKET,
      FACET,
      TRIANG, 
      LIST, 
      VECTOR, 
      MAP, 
      SET, 
      GRAPH, 
      GRAPHOBJECT, 
      VOLSURF, 
      MULTISURF, 
      MULTIBUCKET, 
      MULTIVOLUME, 
      FUSION2D, 
      FUSION3D, 
      FASCICLE,
      FASCICLEGRAPH,
      TEXTURE,
      TEXSURFACE,
      FUSION2DMESH,
      VECTORFIELD,
      ///	External object type (unknown from the base anatomist library)
      OTHER
    };

    /// dynamic menu registration function
    typedef ObjectMenu* (*ObjectMenuRegistrerFunction)
      ( const AObject* objtype, ObjectMenu* menu );

    AObject( const std::string & filename = "" );
    /// AObject subclasses must call cleanup() in their destructor
    virtual ~AObject();

    /** Makes a copy of the object, with a duplicated object structure,
    palette and material, but which may share the underlying low-level
    data object (Aims object). This is especially useful to show the same
    object with several palettes.
    This virtual method returns null by default if it is not overloaded.
    */
    virtual AObject* clone( bool shallow = true );

    /// OpenGL objects const API
    virtual const GLComponent* glAPI() const { return 0; }
    /// OpenGL objects API
    virtual GLComponent* glAPI() { return 0; }
    virtual const MObject* mObjectAPI() const { return 0; }
    virtual MObject* mObjectAPI() { return 0; }

    /**	Object type identifier. Should be virtual, each object shouldn't hold 
	its own type */
    int type() const      { return( _type ); }   
    /** Method to set type id, because type() is not virtual. Usefull when
        registering new AObject class from python. */
    void setType(int type) { _type = type; };
    ///	Unique ID assigned upon construction, but somewhat unused now...
    int id() const        { return( _id ); }
    ///	Name shown in control window
    std::string name() const     { return( _name ); }
    ///	File name (if any) for loaded objects
    std::string fileName() const { return( _filename ); }
    void setName( const std::string & n );
    void setFileName( const std::string & filename );
    void setId( int id );
    ///	maybe not necessary ?: we can use dynamic_cast instead
    virtual int isMultiObject() const { return( false ); }

    /// Obsolete, deprecated
    virtual float MinT() const;
    /// Obsolete, deprecated
    virtual float MaxT() const;

    /** Bounding box in 2D views mode. In mm, may be the same as boundingBox()
        if the object field of view is the same in 3D and 2D modes.
        It normally inccludes voxels size (for a volume, bmin will be -vs/2
        where vs is the voxel size, for the 3 first coordinates).

        The default implementation just calls boundingBox().
    */
    virtual bool boundingBox2D( std::vector<float> & bmin,
                                std::vector<float> & bmax ) const;

    /** Fills \a bmin and \a bmax with the N-D bounding box extrema in the
        object's referential coordinates.

        Changed in Anatomist 4.6. The older API was using Point3df instead of
        vectord and informed only the spatial dimensions.

        An object with no spatial information (a texture for instance) may
        still have time information. For this reason, the resulting bounding
        box should always have 4 components (at least), then 4th and additional
        ones should be valid, even if the function returns false (no spatial
        bounding box).

        \return true if object has a "spatial" bounding box (x, y, z coordinates)
    */
    virtual bool boundingBox( std::vector<float> & bmin,
                              std::vector<float> & bmax ) const;

    /** rendering (generally 2D or 3D using OpenGL).
        Calls GLComponent API ig glAPI() is not null, otherwise nothing is
        done and this method should be overloaded.
    */
    virtual bool render( PrimList &, const ViewState & );
    /// Returns at least 4 sizes. For 3D objects, returns (1, 1, 1, 1)
    virtual std::vector<float> voxelSize() const;
    virtual void setVoxelSize( const std::vector<float> & ) {}
    /**	Updates the state of the object.
	(when a part has changed and other parts depend on this change). 
	Does nothing by default */
    virtual void internalUpdate();

    /**	Creates or updates object palette according to object values 
	(if needed) */
    virtual void adjustPalette() {}
    /**	Normally, getOrCreatePalette() should be used instead of this function 
	in most cases */
    virtual AObjectPalette* palette() { return( _palette ); }
    virtual const AObjectPalette* palette() const { return( _palette ); }
    virtual void setPalette( const AObjectPalette & palette );
    ///	User normally calls this function
    virtual const AObjectPalette* getOrCreatePalette() const;
    /**	function called by getOrCreatePalette() - overloadable, need not
	be called directly */
    virtual void createDefaultPalette( const std::string & name = "" );

    virtual void SetMaterial( const Material & mat );
    virtual Material & GetMaterial() { return _material; }
    virtual const Material & material() const
    { return const_cast<AObject *>(this)->GetMaterial(); }

    Referential* getReferential() const;
    /// if not null, the object referential is inherited from this object
    AObject *referentialInheritance() const;
    virtual void setReferentialInheritance( AObject* ao );
    virtual void setReferential( Referential* ref );
    /// object to take referential from when no ref is assigned
    virtual AObject* fallbackReferentialInheritance() const;
    /** \brief Referential that the object was in before the last change

    This function is only here to overcome a problem in the observer pattern, 
    it is only used by observers that must react to a referential change
    */
    const Referential* previousReferential() const;

    ///	Menu tree for new options, see object/optionMatcher.h
    virtual Tree* optionTree() const;
    virtual ObjectMenu* optionMenu() const;
    ///	For objects loading only when needed (not used yet...)
    int InMemory() const { return _inMemory; }
    ///	Visibility in control window
    int Visible() const { return _visible; }
    void SetVisibility( int v ) { _visible = v; }
    ///	List of multi-objects containing this one. Obsolete: use parents()
    ParentList & Parents() { return _parents; }
    ///	List of multi-objects containing this one
    ParentList & parents() { return _parents; }
    ///	List of multi-objects containing this one
    const ParentList & parents() const { return _parents; }
    virtual void RegisterParent( MObject *pob ) { _parents.insert( pob ); }
    virtual void UnregisterParent( MObject *pob ) { _parents.erase( pob ); }
    ///	List of windows showing this object
    const std::set<AWindow*> & WinList() { return _winList; }
    virtual void registerWindow(AWindow* window);
    virtual void unregisterWindow(AWindow* window);
    virtual bool Is2DObject() = 0;
    virtual bool Is3DObject() = 0;
    /// true if 2D rendering uses a textured plane (not a full openGL object)
    virtual bool textured2D() const { return false; }
    virtual bool isTransparent() const;
    /** true only if the rendering (openGL) of the object changes with
        the observer position/orientation of the view (which is rare, but
        typically needed for Volume Rendering)
    **/
    virtual bool renderingIsObserverDependent() const;
    /** Allows / unallows destruction of object. 
	(cannot be destroyed if part of some other kind of multi-object)
    */
    virtual int CanBeDestroyed();

    virtual void setReferenceChanged();
    bool hasReferenceChanged() const;
    /** Notifies some underlying lower-level objects have changed.
        Useful especially after an underlying AIMS object has been modified */
    virtual void setInternalsChanged();
    /**	Reset has-changed flags.
	Must be called after a call to notifyObservers function 
	to reset changes.
	This system is somewhat insuficient in fact: some observers don't 
	need to update just now, and won't see the change later...
    */
    virtual void clearHasChangedFlags() const;

    /**	Find the object (sub-object) at given postion with a tolerence
        @return	0 if not found
    */
    virtual AObject* objectAt( const std::vector<float> & pos, float tol = 0 );
    /**	Same with origin window referential
     */
    virtual AObject* objectAt( const std::vector<float> & pos,
                               float tol, const Referential* orgref,
                               const Point3df & orggeom );

    ///	Scans the object internals and determines its geometry extrema
    virtual void setGeomExtrema() {}
    ///	Scans the object internals and determines its (texture) extrema values
    virtual void SetExtrema() {}
    ///	Textured objects have values associated with a geometric coordinate
    virtual bool hasTexture() const { return( false ); }
    ///	Number of texture values for a point
    virtual unsigned dimTexture() const { return( 1 ); }
    /** Gets a "mixed" texture value at a given space / time location.
        The value is mixed, for multi-dimensional textures, according to
        an internal method (each object can use its own). Space coordinates
        are transformed from the incoming referential to the object's one.

        Note: in Anatomist 4.4 and earlier, this method was taking an
        additional argument, org_voxel_size. This was kind of pointless
        since it more or less supposed that coordinates were passed in
        voxels rather in mm, which is wrong. In Anatomist 4.5 coords are
        officially always in mm.
    */
    virtual float mixedTexValue( const std::vector<float> & pos,
                                 const Referential* orgRef ) const;
    /** Same as above except that coordinates are not transformed but taken
        in object coordinates system */
    virtual float mixedTexValue( const std::vector<float> & pos ) const;
    /** Gets the array of texture values at a given location

        Note: in Anatomist 4.4 and earlier, this method was taking an
        additional argument, org_voxel_size. This was kind of pointless
        since it more or less supposed that coordinates were passed in
        voxels rather in mm, which is wrong. In Anatomist 4.5 coords are
        officially always in mm.
    */
    virtual std::vector<float> texValues( const std::vector<float> & pos,
                                          const Referential* orgRef ) const;
    virtual std::vector<float>
    texValues( const std::vector<float> & pos ) const;

    virtual bool loadable() const { return false; }
    virtual bool savable() const { return false; }
    /** Re-reads objects from disk. Only called if loadable()
        is \c true and fileName() is not empty. A new filename can be passed
        to the reload function, so that files uncompressed by the ObjectReader
        in temporary locations can be processed. Overload this function in
        inherited classes to implement it.

        If \c onlyoutdated is true, reloading will only be done if the files 
        containing the object have been modified since the object has been 
        created/loaded/saved
    */
    virtual bool reload( const std::string & filename );

    /** Get Object Full Type Name. It should be objectTypeName(_type) for
        most anatomist objects and completed with <T> for templated anatomist
        objects. For instance : AVolume<AimsRGBA>::objectFullTypeName()
        return : VOLUME<AimsRGBA>
     */
    virtual std::string objectFullTypeName(void) const
    { return objectTypeName(_type); };
    /** */
    virtual bool save( const std::string & filename );
    /// should be replaced by a real referential
    virtual bool printTalairachCoord( const Point3df &, 
				      const Referential* ) const
    { return( false ); }
    /** Set some object properties according to the header (.minf), such as 
        material, palette etc */
    void setHeaderOptions();
    /// Same as setHeaderOptions() and used by it, allows passing a dictionary
    virtual void setProperties( carto::Object options );
    /** Store some object properties into the header (.minf), such as
        material, palette etc before saving the object.

        basically calls makeHeaderOptions() and stored them in the object
        header.
    */
    virtual void storeHeaderOptions();
    /** get object properties into a generic object, such as
        material, palette etc before saving the object */
    virtual carto::Object makeHeaderOptions() const;
    /// Time the object was created, loaded or reloaded
    long loadDate() const;
    void setLoadDate( long t );
    virtual void update( const Observable *observable, void *arg );
    bool isCopy() const;
    void setCopyFlag( bool x = true );
    /** tooltip displayed in 3D views (HTML).

        The default impementation returns an empty string, and the default tooltip will be displayed (see QAViewToolTip)
    */
    virtual std::string toolTip() const;

    /// Static object loader: creates an objects, loads its contents
    static std::list<AObject *> load( const std::string & filename );
    /// Reads from disk again
    static bool reload( AObject* object, bool onlyoutdated = false );
    /// Creates a new object type number and returns it
    static int registerObjectType( const std::string & id );
    static std::string objectTypeName( int type );
    static void setObjectMenu(std::string type, carto::rc_ptr<ObjectMenu> om)
    {
      _objectmenu_map[type] = om;
    }
    static carto::rc_ptr<ObjectMenu> getObjectMenu(std::string type)
    {
      return _objectmenu_map[type];
    }
    static std::map<std::string, carto::rc_ptr<ObjectMenu> >
        &getObjectMenuMap()
    {
      return _objectmenu_map;
    }
    static void addObjectMenuRegistration( ObjectMenuRegistrerFunction );
    static void addObjectMenuRegistration( ObjectMenuRegistrerClass * );

    /// cleanup static global variables (called when quitting anatomist)
    static void cleanStatic();

    ///
    carto::Object aimsMeshFromGLComponent();

  protected:
    const PrimList & primitives() const;
    PrimList & primitives();
    /** must be called by objects destructors - Must be explicitly called by 
	each object destructor since it can call virtual functions */
    virtual void cleanup();
    /// sets the object to get referential from to null, unregister it
    virtual void clearReferentialInheritance();
    /// provide access to derived classes
    AObject *& _referentialInheritance();
    virtual void unregisterObservable( Observable * );
    /// copy constructor, protected and used only to reimplement clone()
    AObject( const AObject & );

    ///	Should be static in each object class
    int			_type;
    int			_id;
    std::string		_name;
    std::string		_filename;

    std::set<AWindow*>	_winList;
    int			_inMemory;
    int			_visible;
    ParentList		_parents;

    ///	Should be a pointer: some objects don't have a material (2D objects)
    Material		_material;
    /// Referentiel.
    Referential 	*_referential;
    ///		Reference has-changed flag
    mutable bool	_referenceHasChanged;

    /// Palette
    mutable AObjectPalette	*_palette;

    ///	name-to-type map
    static std::map<std::string, int>	_objectTypes;
    ///	type-to-name map
    static std::map<int,std::string>	_objectTypeNames;

    /// Object Menu Map
    static std::map<std::string, carto::rc_ptr<ObjectMenu> >
        _objectmenu_map;

    friend class StaticInitializers;

  private:
    struct Private;
    Private	*d;
  };

}


inline float
anatomist::AObject::mixedTexValue( const std::vector<float> & ) const
{
  return 0;
}


inline std::vector<float> 
anatomist::AObject::texValues( const std::vector<float> & ) const
{
  std::vector<float> t;
  return t;
}


namespace carto
{
  DECLARE_GENERIC_OBJECT_TYPE( anatomist::AObject * )
  DECLARE_GENERIC_OBJECT_TYPE( std::set<anatomist::AObject *> )
  DECLARE_GENERIC_OBJECT_TYPE( std::vector<anatomist::AObject *> )
  DECLARE_GENERIC_OBJECT_TYPE( std::list<anatomist::AObject *> )
  DECLARE_GENERIC_OBJECT_TYPE( carto::shared_ptr<anatomist::AObject> )
}

#endif

