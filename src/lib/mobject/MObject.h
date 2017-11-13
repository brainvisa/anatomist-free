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


#ifndef ANA_MOBJECT_MOBJECT_H
#define ANA_MOBJECT_MOBJECT_H

#include <anatomist/object/Object.h>


namespace anatomist
{
  class ObjectListIterator;
  class const_ObjectListIterator;
  class ObjectVectorIterator;
  class const_ObjectVectorIterator;
  class AGraphIterator;
  class const_AGraphIterator;
  class Fusion2DIterator;
  class const_Fusion2DIterator;
  class Fusion3DIterator;
  class const_Fusion3DIterator;
  class ATexSurfaceIterator;
  class const_ATexSurfaceIterator;
  class Referential;
  class Geometry;

  /**	
   */
  class BaseIterator
  {
  public:
    virtual ~BaseIterator() {}
    virtual BaseIterator* clone() const = 0;
    bool operator == ( const BaseIterator & ) const;
    virtual bool operator != ( const BaseIterator & ) const;
    virtual bool operator != ( const ObjectListIterator & ) const
    { return( true ); }
    virtual bool operator != ( const const_ObjectListIterator & ) const
    { return( true ); }
    virtual bool operator != ( const ObjectVectorIterator & ) const
    { return( true ); }
    virtual bool operator != ( const const_ObjectVectorIterator & ) const
    { return( true ); }
    virtual bool operator != ( const AGraphIterator & ) const
    { return( true ); }
    virtual bool operator != ( const const_AGraphIterator & ) const
    { return( true ); }
    virtual bool operator != ( const Fusion2DIterator & ) const
    { return( true ); }
    virtual bool operator != ( const const_Fusion2DIterator & ) const
    { return( true ); }
    virtual bool operator != ( const Fusion3DIterator & ) const
    { return( true ); }
    virtual bool operator != ( const const_Fusion3DIterator & ) const
    { return( true ); }
    virtual bool operator != ( const ATexSurfaceIterator & ) const
    { return( true ); }
    virtual bool operator != ( const const_ATexSurfaceIterator & ) const
    { return( true ); }
    virtual AObject* operator * () const = 0;
    virtual BaseIterator & operator ++ () = 0;
    virtual BaseIterator & operator -- () = 0;
  };


  inline bool BaseIterator::operator == 
  ( const BaseIterator & x ) const
  {
    //cout << "BaseIterator::operator == ( const BaseIterator & x )\n";
    return( ! (operator != (x)) );
  }


  inline bool BaseIterator::operator != 
  ( const BaseIterator & i ) const
  {
    return( i.operator != ( *this )  );
  }


  /**	Multi-object generic iterator
   */
  class AIterator
  {
  public:
    AIterator();
    AIterator( const AIterator & );
    AIterator( BaseIterator * );
    ~AIterator();

    AIterator & operator = ( const AIterator & );
    bool operator == (const AIterator & ) const;
    bool operator != ( const AIterator & ) const;
    AIterator & operator ++();
    AIterator operator ++ ( int );
    AIterator & operator -- ();
    AIterator operator -- ( int );
    AObject *operator *() const;
    BaseIterator & subIterator() { return( *_iterator ); }

    ///	Data storage type, to be redefined by children classes
    typedef void datatype;

  protected:
    ///	Underlying specific iterator
    BaseIterator *_iterator;
  };


  inline AIterator::AIterator() 
    : _iterator( 0 )
  {
  }


  inline AIterator::AIterator( const AIterator & x )
    : _iterator( x._iterator->clone() )
  {
  }


  inline AIterator::AIterator( BaseIterator *i )
    : _iterator( i )
  {
  }


  inline AIterator::~AIterator()
  {
    if( _iterator ) delete _iterator;
  }


  inline AIterator & 
  AIterator::operator = ( const AIterator & i )
  {
    if( this == &i ) return( *this );
    if( _iterator ) delete _iterator;
    _iterator = i._iterator->clone();
    return( *this );
  }


  inline bool 
  AIterator::operator == ( const AIterator & i ) const
  {
    //cout << "AIterator::operator ==\n";
    return( *_iterator == *i._iterator );
  }


  inline bool 
  AIterator::operator != ( const AIterator & i ) const
  {
    return( *_iterator != *i._iterator );
  }


  inline AIterator & 
  AIterator::operator ++ ()
  {
    ++ *_iterator;
    return( *this );
  }


  inline AIterator 
  AIterator::operator ++ ( int )
  {
    AIterator tmp = _iterator->clone();

    ++ *_iterator;
    return( tmp );
  }


  inline AIterator & 
  AIterator::operator -- ()
  {
    -- *_iterator;
    return( *this );
  }


  inline AIterator
  AIterator::operator -- ( int )
  {
    AIterator tmp = _iterator->clone();

    -- *_iterator;
    return( tmp );
  }


  inline AObject *AIterator::operator * () const
  {
    return( **_iterator );
  }


  /**	Multi-object : base abstract class for objects that contain others
   */
  class MObject : public AObject
  {
  public:

    typedef AIterator iterator;
    typedef AIterator const_iterator;

    MObject();
    virtual ~MObject();

    virtual const MObject* mObjectAPI() const;
    virtual MObject* mObjectAPI();

    /**@name	Identification handling functions */
    //@{
    int isMultiObject() const { return( true ); }
    ///	Precise type of multi-object
    virtual int MType() const = 0;
    //@}
    virtual void setReferential( Referential* ref );
    virtual void setReferentialInheritance( AObject* );

    /**@name	Container methods */
    //@{
    virtual size_t size() const = 0;
    virtual iterator begin() = 0;
    virtual const_iterator begin() const = 0;
    virtual iterator end() = 0;
    virtual const_iterator end() const = 0;
    virtual void insert( AObject * ) = 0;
    virtual const_iterator find( const AObject * ) const = 0;
    virtual void erase( iterator & ) = 0;
    void eraseObject( AObject *obj );
    //@}

    virtual float MinT() const;
    virtual float MaxT() const;

    virtual float MinX2D() const;
    virtual float MinY2D() const;
    virtual float MinZ2D() const;
    virtual float MaxX2D() const;
    virtual float MaxY2D() const;
    virtual float MaxZ2D() const;
    virtual bool boundingBox2D( std::vector<float> & bmin,
                                std::vector<float> & bmax ) const;
    virtual bool boundingBox2D( Point3df & bmin, Point3df & bmax ) const
    { return AObject::boundingBox2D( bmin, bmax ); }

    virtual std::vector<float> voxelSize() const;
    virtual bool boundingBox( Point3df & bmin, Point3df & bmax ) const;
    virtual bool boundingBox( std::vector<float> & bmin,
                              std::vector<float> & bmax ) const;

    /**@name	Contents / volume of labels handling */
    //@{
    /**	Find the object (sub-object) at given postion with a tolerence
	@return	0 if not found
    */
    virtual AObject* ObjectAt( float x, float y, float z, float t, 
			       float tol = 0 );
    //@}


    /**@name	Graphics rendering inherited functions */
    //@{
    virtual bool render( PrimList &, const ViewState & );
    virtual bool renderingIsObserverDependent() const;
    /// Can be display in 2D windows.
    virtual bool Is2DObject();
    /// Can be display in 3D windows.
    virtual bool Is3DObject();
    virtual bool isTransparent() const;
    //@}


    /**@name	Observer inherited functions */
    //@{
    /**
     *	This class is an Observer of each of the AObject it groups.
     */
    virtual void update(const Observable* observable, void* arg);
    //@}

    virtual bool CanRemove( AObject *obj );


    /**@name	Handling has-changed flags  */
    //@{
    virtual void setContentChanged() const;
    bool hasContentChanged() const;
    /**
     *  		Reset has-changed flags
     *  		Must be called after a call to notifyObservers function
     *		to reset changes
     */
    virtual void clearHasChangedFlags() const;
    //@}

    virtual void SetMaterial( const Material & mat );
    virtual void setPalette( const AObjectPalette & pal );
    /** tells whether children objects should be removed from views when
        this MObject is removed from a view. Useful for graphs. */
    virtual bool shouldRemoveChildrenWithMe() const;
    /** Children objects which have been used to build the current MObject

        Typically in a fusion, some objects have been used to generate the
        fusion object, but the fusion may have additional children which are
        built to hold intermediate data.

        The default implementation returns all children.
    */
    virtual std::list<AObject *> generativeChildren() const;

  protected:
    /// must be called by all subclasses in their insert() implementation
    virtual void _insertObject( AObject* o );
    /// must be called by all subclasses if they reimplement erase()
    virtual void _eraseObject( AObject* o );
    virtual void updateSubObjectReferential( const AObject* o );
    virtual void clearReferentialInheritance();

    mutable bool _contentHasChanged;
  };


  inline MObject::MObject() : AObject()
  {
    _contentHasChanged = false;
  }


  inline void MObject::eraseObject( AObject * obj )
  {
    iterator	i = find( obj );
    if( i != end() )
      {
        erase( i );
        _contentHasChanged = true;
        setChanged();
      }
    else
      {
        std::cout << "unable to erase object " << obj->name() 
                  << " from MObject "
                  << name() << " : object not found\n";
      }
  }



  inline bool MObject::CanRemove( AObject * ) 
  { 
    //   always allowed by default 
    return( true ); 
  }

}


#endif
