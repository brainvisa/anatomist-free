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


#ifndef ANA_MOBJECT_FUSION2D_H
#define ANA_MOBJECT_FUSION2D_H


#include <anatomist/mobject/objectVector.h>
#include <anatomist/object/sliceable.h>


namespace anatomist
{

  class Referential;
  class Geometry;


  /**	Anatomist object which contains two volumes: one for the MRI anatomic 
	image, and another for the functional data (fMRI, PET...)
  */
  class Fusion2D : public ObjectVector, public Sliceable
  {
  public:
    Fusion2D( const std::vector<AObject *> & obj );
    virtual ~Fusion2D();

    virtual const GLComponent* glAPI() const { return this; }
    virtual GLComponent* glAPI() { return this; }
    virtual const Sliceable* sliceableAPI() const { return this; }
    virtual Sliceable* sliceableAPI() { return this; }

    AObject* mri() const { return _data.begin()->get(); }
    AObject* functional() const 
    { const_iterator i = begin(); ++i; return *i; }
    virtual int MType() const { return( AObject::FUSION2D ); }
    virtual Point3df VoxelSize() const;
    virtual Point3df glVoxelSize() const;
    virtual Point4df glMin2D() const;
    virtual Point4df glMax2D() const;
    virtual AObject* fallbackReferentialInheritance() const;

    virtual bool CanRemove( AObject* obj );

    virtual bool render( PrimList &, const ViewState & );
    virtual bool update2DTexture( AImage &, const Point3df & posbase,
                                  const SliceViewState &, 
                                  unsigned tex = 0 ) const;
    /// Can be displayed in 2D windows.
    virtual bool Is2DObject() { return(true); }
    /// Can't be displayed in 3D windows.
    virtual bool Is3DObject() { return(false); }
    virtual bool textured2D() const { return( true ); }

    /** move the sub-object \c vol to position \c dstpos. 
        If dstpos < 0, the object is moved to the end of the list */
    void moveVolume( AObject* vol, int dstpos );
    /** move the sub-object at position \c srcpos to position \c dstpos.
        If dstpos < 0, the object is moved to the end of the list */
    void moveVolume( int srcpos, int dstpos );
    /// Reorder sub-objects
    void reorder( const std::vector<AObject *> & );

    virtual void glClearHasChangedFlags() const;
    virtual void glSetChanged( glPart, bool = true ) const;
    virtual void glSetTexImageChanged( bool x = true, 
                                       unsigned tex = 0 ) const ;
    virtual void glSetTexEnvChanged( bool x = true, unsigned tex = 0 ) const;
    unsigned glNumTextures( const ViewState & ) const;
    std::set<glTextureMode> glAllowedTexModes( unsigned tex = 0 ) const;

    virtual void update(const Observable* observable, void* arg);

    virtual const AObjectPalette* glPalette( unsigned tex = 0 ) const;
    const Referential *getReferential() const;

    virtual bool hasTexture() const { return( true ); }
    virtual unsigned dimTexture() const { return( 2 ); }
    virtual float mixedTexValue( const Point3df & pos, float time ) const;
    virtual float mixedTexValue( const Point3df & pos, float time,
                                 const Referential* orgRef ) const;
    virtual std::vector<float> texValues( const Point3df & pos,
                                          float time ) const;
    virtual std::vector<float> texValues( const Point3df & pos, float time,
                                          const Referential* orgRef ) const;
    virtual float mixedValue( const std::vector<float> & pv ) const;
    virtual const Material *glMaterial() const;
    virtual bool isTransparent() const;

    virtual Tree* optionTree() const;
    static Tree*	_optionTree;

  protected:
    ///	Disable public edition
    virtual void insert( AObject* o ) { ObjectVector::insert( o ); }
    virtual void insert( AObject* o, int pos )
    { ObjectVector::insert( o, pos ); }
    virtual void insert( const carto::shared_ptr<AObject> & o,
                         int pos = -1 )
    { ObjectVector::insert( o, pos  ); }
    virtual void erase( iterator & i ) { ObjectVector::erase( i ); }
  };


  inline bool Fusion2D::CanRemove( AObject * )
  {
    return( false );
  }

}


#endif
